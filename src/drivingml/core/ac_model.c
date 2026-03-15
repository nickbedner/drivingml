#include "drivingml/core/ac_model.h"

void relu(float* x, int n);

int load_ac_model(const char* path, ACModel* m) {
  FILE* f = fopen(path, "rb");
  if (!f) {
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

  int32_t dims[5];
  fread(dims, sizeof(int32_t), 5, f);

  if (dims[0] != 5 || dims[1] != 128 || dims[2] != 128 || dims[3] != 2) {
    printf("Model dimensions mismatch\n");
    fclose(f);
    return 0;
  }

#define READ(arr) fread(arr, sizeof(float), sizeof(arr) / sizeof(float), f)

  READ(m->w1);
  READ(m->b1);
  READ(m->w2);
  READ(m->b2);
  READ(m->wa);
  READ(m->ba);
  READ(m->wc);
  READ(m->bc);
  READ(m->log_std);

  fclose(f);
  printf("Model loaded successfully\n");
  return 1;
}

void linear(const float* W, const float* b, int in, int out, const float* x, float* y) {
  for (int o = 0; o < out; o++) {
    float sum = b[o];
    const float* row = W + o * in;
    for (int i = 0; i < in; i++)
      sum += row[i] * x[i];
    y[o] = sum;
  }
}

// void relu(float* x, int n) {
//   for (int i = 0; i < n; i++)
//     if (x[i] < 0.0f) x[i] = 0.0f;
// }

void ac_forward(const ACModel* m, const float state[5], float action_out[2], float* value_out) {
  float h1[H1];
  float h2[H2];
  float mean[ACT];
  float value[1];

  linear(m->w1, m->b1, IN, H1, state, h1);
  relu(h1, H1);

  linear(m->w2, m->b2, H1, H2, h1, h2);
  relu(h2, H2);

  linear(m->wa, m->ba, H2, ACT, h2, mean);
  linear(m->wc, m->bc, H2, 1, h2, value);

  // Deterministic eval mode: action = tanh(mean)
  for (int i = 0; i < ACT; i++) {
    action_out[i] = tanhf(mean[i]);

    // Same clamp as python
    const float eps = 1e-6f;
    if (action_out[i] < -1.0f + eps) action_out[i] = -1.0f + eps;
    if (action_out[i] > 1.0f - eps) action_out[i] = 1.0f - eps;
  }

  *value_out = value[0];
}
