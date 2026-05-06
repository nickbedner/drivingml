#include "drivingml/core/ac_model.h"

void linear_layer_avx2(const r32* weights, const r32* bias, int input_size, int output_size, const r32* input, r32* output);
void relu_activation_avx2(r32* values, int count);

int load_ac_model(const char* path, struct ACModel* model) {
  FILE* f = NULL;
  if (fopen_s(&f, path, "rb") != 0 || !f) {
    printf("Failed to open %s\n", path);
    return 0;
  }
  if (!f) {
    printf("Failed to open %s\n", path);
    return 0;
  }

  // This refers to our model codename aka a magic header to know which one we're using
  char magic[4];
  fread(magic, 1, 4, f);
  if (memcmp(magic, "ACv1", 4) != 0) {
    printf("Invalid model file\n");
    fclose(f);
    return 0;
  }

  i32 dims[5];
  fread(dims, sizeof(i32), 5, f);
  if (dims[0] != 7 || dims[1] != 128 || dims[2] != 128 || dims[3] != 2) {
    printf("Model dimensions mismatch\n");
    fclose(f);
    return 0;
  }

#define READ(arr) fread(arr, sizeof(r32), sizeof(arr) / sizeof(r32), f)
  READ(model->input_to_hidden1_weights);
  READ(model->hidden1_bias);
  READ(model->hidden1_to_hidden2_weights);
  READ(model->hidden2_bias);
  READ(model->hidden2_to_action_weights);
  READ(model->action_bias);
  READ(model->hidden2_to_value_weights);
  READ(model->value_bias);
  READ(model->action_log_std);

  fclose(f);
  return 1;
}

void linear_layer(const r32* weights, const r32* bias, int input_size, int output_size, const r32* input, r32* output) {
  for (int out_neuron = 0; out_neuron < output_size; out_neuron++) {
    r32 neuron_sum = bias[out_neuron];

    const r32* weight_row = weights + out_neuron * input_size;

    for (int in_feature = 0; in_feature < input_size; in_feature++)
      neuron_sum += weight_row[in_feature] * input[in_feature];

    output[out_neuron] = neuron_sum;
  }
}

// Note: Remember relu = Rectified Linear Unit, which introduces nonlinearity into the model
void relu_activation(r32* values, int count) {
  for (int neuron = 0; neuron < count; neuron++) {
    if (values[neuron] < 0.0f)
      values[neuron] = 0.0f;
  }
}

// static inline r32 horizontal_sum_avx2(__m256 v) {
//   __m128 low = _mm256_castps256_ps128(v);
//   __m128 high = _mm256_extractf128_ps(v, 1);
//   __m128 sum = _mm_add_ps(low, high);
//
//   sum = _mm_hadd_ps(sum, sum);
//   sum = _mm_hadd_ps(sum, sum);
//
//   return _mm_cvtss_f32(sum);
// }
//
// void linear_layer_avx2(
//     const r32* weights,
//     const r32* bias,
//     int input_size,
//     int output_size,
//     const r32* input,
//     r32* output) {
//   for (int out_neuron = 0; out_neuron < output_size; out_neuron++) {
//     const r32* weight_row = weights + out_neuron * input_size;
//
//     __m256 sum_vec = _mm256_setzero_ps();
//
//     int in_feature = 0;
//
//     for (; in_feature + 7 < input_size; in_feature += 8) {
//       __m256 w = _mm256_loadu_ps(weight_row + in_feature);
//       __m256 x = _mm256_loadu_ps(input + in_feature);
//
// #if defined(__FMA__)
//       sum_vec = _mm256_fmadd_ps(w, x, sum_vec);
// #else
//       sum_vec = _mm256_add_ps(sum_vec, _mm256_mul_ps(w, x));
// #endif
//     }
//
//     r32 neuron_sum = bias[out_neuron] + horizontal_sum_avx2(sum_vec);
//
//     // Tail loop for leftover inputs when input_size is not divisible by 8.
//     for (; in_feature < input_size; in_feature++) {
//       neuron_sum += weight_row[in_feature] * input[in_feature];
//     }
//
//     output[out_neuron] = neuron_sum;
//   }
// }
//
// void relu_activation_avx2(r32* values, int count) {
//   __m256 zero = _mm256_setzero_ps();
//
//   int neuron = 0;
//
//   for (; neuron + 7 < count; neuron += 8) {
//     __m256 v = _mm256_loadu_ps(values + neuron);
//     v = _mm256_max_ps(v, zero);
//     _mm256_storeu_ps(values + neuron, v);
//   }
//
//   // Tail loop.
//   for (; neuron < count; neuron++) {
//     if (values[neuron] < 0.0f) {
//       values[neuron] = 0.0f;
//     }
//   }
// }

// Forward pass and inference in C, which is the process of moving input data through the network's layers from input to output to generate a prediction
void ac_forward(const struct ACModel* model, const r32 state_vector[INPUTS], r32 action_output[ACTIONS], r32* value_prediction) {
  r32 h1[HIDDEN_1];
  r32 h2[HIDDEN_2];
  r32 mean[ACTIONS];
  r32 value[1];

  linear_layer(model->input_to_hidden1_weights, model->hidden1_bias, INPUTS, HIDDEN_1, state_vector, h1);
  relu_activation(h1, HIDDEN_1);

  linear_layer(model->hidden1_to_hidden2_weights, model->hidden2_bias, HIDDEN_1, HIDDEN_2, h1, h2);
  relu_activation(h2, HIDDEN_2);

  linear_layer(model->hidden2_to_action_weights, model->action_bias, HIDDEN_2, ACTIONS, h2, mean);
  linear_layer(model->hidden2_to_value_weights, model->value_bias, HIDDEN_2, 1, h2, value);

  // Deterministic eval mode: action = tanh(mean), remvember tanh converts the network’s raw output into a valid action range
  for (int i = 0; i < ACTIONS; i++) {
    action_output[i] = real32_tanh(mean[i]);

    // Same clamp as python
    const r32 eps = 1e-6f;
    if (action_output[i] < -1.0f + eps) action_output[i] = -1.0f + eps;
    if (action_output[i] > 1.0f - eps) action_output[i] = 1.0f - eps;
  }

  r32 input[4] = {1.0f, -2.0f, 3.0f, 4.0f};

  r32 weights[8] = {
      0.5f, 1.0f, -1.0f, 2.0f,
      1.0f, 0.0f, 0.5f, -0.5f};

  r32 bias[2] = {0.1f, -0.2f};
  r32 output[2];

  linear_layer_avx2(weights, bias, 4, 2, input, output);
  relu_activation_avx2(output, 2);

  printf("asm %f %f\n", output[0], output[1]);

  // Critic value
  *value_prediction = value[0];
}
