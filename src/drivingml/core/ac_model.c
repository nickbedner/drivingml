#include "drivingml/core/ac_model.h"

int load_ac_model(const char* path, struct ACModel* model) {
  FILE* f = NULL;
  if (fopen_s(&f, path, "rb") != 0 || !f) {
    printf("Failed to open %s\n", path);
    return 0;
  }

  char magic[4];
  fread(magic, 1, 4, f);
  if (memcmp(magic, "ACv1", 4) != 0) {
    printf("Invalid model file\n");
    fclose(f);
    return 0;
  }

  i32 dims[5];
  fread(dims, sizeof(i32), 5, f);
  if (dims[0] != INPUTS || dims[1] != HIDDEN_1 || dims[2] != HIDDEN_2 || dims[3] != ACTIONS || dims[4] != 1) {
    printf("Model dimensions mismatch. Got %d %d %d %d %d, expected %d %d %d %d 1\n", dims[0], dims[1], dims[2], dims[3], dims[4], INPUTS, HIDDEN_1, HIDDEN_2, ACTIONS);
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
#undef READ

  fclose(f);
  return 1;
}

void linear_layer(const r32* weights, const r32* bias, int input_size, int output_size, const r32* input, r32* output) {
  for (int out_neuron = 0; out_neuron < output_size; out_neuron++) {
    r32 neuron_sum = bias[out_neuron];
    const r32* weight_row = weights + out_neuron * input_size;
    for (int in_feature = 0; in_feature < input_size; in_feature++) neuron_sum += weight_row[in_feature] * input[in_feature];
    output[out_neuron] = neuron_sum;
  }
}

void relu_activation(r32* values, int count) {
  for (int neuron = 0; neuron < count; neuron++) {
    if (values[neuron] < 0.0f) values[neuron] = 0.0f;
  }
}

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

  for (int i = 0; i < ACTIONS; i++) {
    const r32 eps = 1e-6f;
    action_output[i] = real32_tanh(mean[i]);
    if (action_output[i] < -1.0f + eps) action_output[i] = -1.0f + eps;
    if (action_output[i] > 1.0f - eps) action_output[i] = 1.0f - eps;
  }

  *value_prediction = value[0];
}
