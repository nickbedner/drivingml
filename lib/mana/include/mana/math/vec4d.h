#pragma once

#include "mana/core/corecommon.h"

typedef struct vec4d {
  union {
    struct {
      double x, y, z, w;
    };
    struct {
      double r, g, b, a;
    };
    struct {
      double u, v, s, t;
    };
    double data[4];
  };
} vec4d;

static const vec4d VEC4D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0};

static inline vec4d vec4d_add(vec4d a, vec4d b);
static inline vec4d vec4d_sub(vec4d a, vec4d b);
static inline vec4d vec4d_mul(vec4d a, vec4d b);
static inline vec4d vec4d_div(vec4d a, vec4d b);
static inline vec4d vec4d_divs(vec4d a, double s);
static inline vec4d vec4d_scale(vec4d a, double s);
static inline double vec4d_dot(vec4d a, vec4d b);

static inline vec4d vec4d_add(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2], .data[3] = a.data[3] + b.data[3]};
}

static inline vec4d vec4d_sub(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2], .data[3] = a.data[3] - b.data[3]};
}

static inline vec4d vec4d_mul(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2], .data[3] = a.data[3] * b.data[3]};
}

static inline vec4d vec4d_div(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2], .data[3] = a.data[3] / b.data[3]};
}

static inline vec4d vec4d_divs(vec4d a, double s) {
  return (vec4d){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s, .data[3] = a.data[3] / s};
}

static inline vec4d vec4d_scale(vec4d a, double s) {
  return (vec4d){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s, .data[3] = a.data[3] * s};
}

static inline double vec4d_dot(vec4d a, vec4d b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2] + a.data[3] * b.data[3];
}
