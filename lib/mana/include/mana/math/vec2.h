#pragma once

#include "mana/core/corecommon.h"

typedef struct vec2 {
  union {
    struct {
      float x, y;
    };
    struct {
      float u, v;
    };
    struct {
      float s, t;
    };
    float data[2];
  };
} vec2;

static const vec2 VEC2_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f};

static inline vec2 vec2_add(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1]};
}

static inline vec2 vec2_sub(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1]};
}

static inline vec2 vec2_mul(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1]};
}

static inline vec2 vec2_div(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1]};
}

static inline vec2 vec2_divs(vec2 a, float s) {
  return (vec2){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s};
}

static inline vec2 vec2_scale(vec2 a, float s) {
  return (vec2){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s};
}

static inline float vec2_dot(vec2 a, vec2 b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1];
}

static inline bool vec2_equals(vec2 v1, vec2 v2) {
  return (fabsf(v1.x - v2.x) < FLT_EPSILON) && (fabsf(v1.y - v2.y) < FLT_EPSILON);
}

// Note: Have no clue how this works going off the book
static inline vec2 vec2_normalise(vec2 v1) {
  float mag = sqrtf((v1.data[0] * v1.data[0]) + (v1.data[1] * v1.data[1]));
  float inv_mag = 1.0f / mag;
  if (fabsf(0.0f * inv_mag) < FLT_EPSILON) {
    v1.x = v1.x * inv_mag;
    v1.y = v1.y * inv_mag;
  }
  return v1;
}

inline float vec2_mul_inner(vec2 const a, vec2 const b) {
  float p = 0.0f;
  int i;
  for (i = 0; i < 2; ++i)
    p += b.data[i] * a.data[i];
  return p;
}

static inline float vec2_len(vec2 const v) {
  return sqrtf(vec2_mul_inner(v, v));
}

// TODO: Figure out which norm is better
static inline vec2 vec2_norm(vec2 const v) {
  float k = 1.0f / vec2_len(v);
  return vec2_scale(v, k);
}

static inline int vec2_nonzero_sign(float n) {
  return 2 * (n > 0) - 1;
}

static inline float vec2_cross(vec2 a, vec2 b) {
  return a.data[0] * b.data[1] - a.data[1] * b.data[0];
}

static inline vec2 vec2_mix(vec2 a, vec2 b, float weight) {
  return (vec2){.data[0] = (1 - weight) * a.data[0] + weight * b.data[0], .data[1] = (1 - weight) * a.data[1] + weight * b.data[1]};
}
