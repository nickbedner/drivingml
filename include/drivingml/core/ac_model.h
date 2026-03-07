#pragma once

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IN 5
#define H1 128
#define H2 128
#define ACT 2

typedef struct {
  float w1[H1 * IN], b1[H1];
  float w2[H2 * H1], b2[H2];
  float wa[ACT * H2], ba[ACT];
  float wc[1 * H2], bc[1];
  float log_std[ACT];
} ACModel;

int load_ac_model(const char* path, ACModel* m);
void linear(const float* W, const float* b, int in, int out, const float* x, float* y);
void relu(float* x, int n);
void ac_forward(const ACModel* m, const float state[5], float action_out[2], float* value_out);
