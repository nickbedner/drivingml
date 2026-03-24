#pragma once

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://github.com/M-Sai-Pranav/neural-network-forward-pass/tree/main

#define INPUTS 7
#define HIDDEN_1 128
#define HIDDEN_2 128
#define ACTIONS 2

struct ACModel {
  // Input → Hidden Layer 1
  float input_to_hidden1_weights[HIDDEN_1 * INPUTS];
  float hidden1_bias[HIDDEN_1];

  // Hidden Layer 1 → Hidden Layer 2
  float hidden1_to_hidden2_weights[HIDDEN_2 * HIDDEN_1];
  float hidden2_bias[HIDDEN_2];

  // Hidden Layer 2 → Actor (action mean)
  float hidden2_to_action_weights[ACTIONS * HIDDEN_2];
  float action_bias[ACTIONS];

  // Hidden Layer 2 → Critic (value estimate)
  float hidden2_to_value_weights[HIDDEN_2];
  float value_bias[1];

  // Policy standard deviation
  float action_log_std[ACTIONS];
};

int load_ac_model(const char* path, struct ACModel* model);
void linear_layer(const float* weights, const float* bias, int input_size, int output_size, const float* input, float* output);
void relu_activation(float* values, int count);
void ac_forward(const struct ACModel* model, const float state_vector[INPUTS], float action_output[ACTIONS], float* value_prediction);
