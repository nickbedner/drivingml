#pragma once

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/corecommon.h"

#define INPUTS 7
#define HIDDEN_1 128
#define HIDDEN_2 128
#define ACTIONS 2

struct ACModel {
  // Input → Hidden Layer 1
  r32 input_to_hidden1_weights[HIDDEN_1 * INPUTS];
  r32 hidden1_bias[HIDDEN_1];

  // Hidden Layer 1 → Hidden Layer 2
  r32 hidden1_to_hidden2_weights[HIDDEN_2 * HIDDEN_1];
  r32 hidden2_bias[HIDDEN_2];

  // Hidden Layer 2 → Actor (action mean)
  r32 hidden2_to_action_weights[ACTIONS * HIDDEN_2];
  r32 action_bias[ACTIONS];

  // Hidden Layer 2 → Critic (value estimate)
  r32 hidden2_to_value_weights[HIDDEN_2];
  r32 value_bias[1];

  // Policy standard deviation
  r32 action_log_std[ACTIONS];
};

int load_ac_model(const char* path, struct ACModel* model);
void linear_layer(const r32* weights, const r32* bias, int input_size, int output_size, const r32* input, r32* output);
void relu_activation(r32* values, int count);
void ac_forward(const struct ACModel* model, const r32 state_vector[INPUTS], r32 action_output[ACTIONS], r32* value_prediction);
