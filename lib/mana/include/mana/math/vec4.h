#pragma once

#include "mana/core/corecommon.h"

typedef struct vec4 {
  union {
    struct {
      float x, y, z, w;
    };
    struct {
      float r, g, b, a;
    };
    struct {
      float u, v, s, t;
    };
    float data[4];
    // simd_align_max float data[4];
  };
} vec4;

static const vec4 VEC4_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f};

static inline vec4 vec4_add(vec4 a, vec4 b);
static inline vec4 vec4_sub(vec4 a, vec4 b);
static inline vec4 vec4_mul(vec4 a, vec4 b);
static inline vec4 vec4_div(vec4 a, vec4 b);
static inline vec4 vec4_divs(vec4 a, float s);
static inline vec4 vec4_scale(vec4 a, float s);
static inline float vec4_dot(vec4 a, vec4 b);

static inline vec4 vec4_add(vec4 a, vec4 b) {
  return (vec4){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2], .data[3] = a.data[3] + b.data[3]};
}

static inline vec4 vec4_sub(vec4 a, vec4 b) {
  return (vec4){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2], .data[3] = a.data[3] - b.data[3]};
}

static inline vec4 vec4_mul(vec4 a, vec4 b) {
  return (vec4){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2], .data[3] = a.data[3] * b.data[3]};
}

static inline vec4 vec4_div(vec4 a, vec4 b) {
  return (vec4){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2], .data[3] = a.data[3] / b.data[3]};
}

static inline vec4 vec4_divs(vec4 a, float s) {
  return (vec4){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s, .data[3] = a.data[3] / s};
}

static inline vec4 vec4_scale(vec4 a, float s) {
  return (vec4){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s, .data[3] = a.data[3] * s};
}

static inline float vec4_dot(vec4 a, vec4 b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2] + a.data[3] * b.data[3];
}
