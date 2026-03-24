#include "drivingml/core/ac_model.h"

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

  int32_t dims[5];
  fread(dims, sizeof(int32_t), 5, f);
  if (dims[0] != 7 || dims[1] != 128 || dims[2] != 128 || dims[3] != 2) {
    printf("Model dimensions mismatch\n");
    fclose(f);
    return 0;
  }

#define READ(arr) fread(arr, sizeof(float), sizeof(arr) / sizeof(float), f)
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
  printf("Model loaded successfully\n");
  return 1;
}

void linear_layer(const float* weights, const float* bias, int input_size, int output_size, const float* input, float* output) {
  for (int out_neuron = 0; out_neuron < output_size; out_neuron++) {
    float neuron_sum = bias[out_neuron];

    const float* weight_row = weights + out_neuron * input_size;

    for (int in_feature = 0; in_feature < input_size; in_feature++)
      neuron_sum += weight_row[in_feature] * input[in_feature];

    output[out_neuron] = neuron_sum;
  }
}

// Note: Remember relu = Rectified Linear Unit, which introduces nonlinearity into the model
void relu_activation(float* values, int count) {
  for (int neuron = 0; neuron < count; neuron++) {
    if (values[neuron] < 0.0f)
      values[neuron] = 0.0f;
  }
}

// Forward pass and inference in C, which is the process of moving input data through the network's layers from input to output to generate a prediction
void ac_forward(const struct ACModel* model, const float state_vector[INPUTS], float action_output[ACTIONS], float* value_prediction) {
  float h1[HIDDEN_1];
  float h2[HIDDEN_2];
  float mean[ACTIONS];
  float value[1];

  linear_layer(model->input_to_hidden1_weights, model->hidden1_bias, INPUTS, HIDDEN_1, state_vector, h1);
  relu_activation(h1, HIDDEN_1);

  linear_layer(model->hidden1_to_hidden2_weights, model->hidden2_bias, HIDDEN_1, HIDDEN_2, h1, h2);
  relu_activation(h2, HIDDEN_2);

  linear_layer(model->hidden2_to_action_weights, model->action_bias, HIDDEN_2, ACTIONS, h2, mean);
  linear_layer(model->hidden2_to_value_weights, model->value_bias, HIDDEN_2, 1, h2, value);

  // Deterministic eval mode: action = tanh(mean), remvember tanh converts the network’s raw output into a valid action range
  for (int i = 0; i < ACTIONS; i++) {
    action_output[i] = tanhf(mean[i]);

    // Same clamp as python
    const float eps = 1e-6f;
    if (action_output[i] < -1.0f + eps) action_output[i] = -1.0f + eps;
    if (action_output[i] > 1.0f - eps) action_output[i] = 1.0f - eps;
  }

  // Critic value
  *value_prediction = value[0];
}
