#pragma once

#include "mana/core/corecommon.h"

typedef struct vec3d {
  union {
    struct {
      double x, y, z;
    };
    struct {
      double r, g, b;
    };
    struct {
      double u, v, s;
    };
    double data[3];
  };
} vec3d;

static const vec3d VEC3D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0};
static const vec3d VEC3D_ONE = {.data[0] = 1.0, .data[1] = 1.0, .data[2] = 1.0};

static inline vec3d vec3d_set(double s);
static inline vec3d vec3d_add(vec3d a, vec3d b);
static inline vec3d vec3d_sub(vec3d a, vec3d b);
static inline vec3d vec3d_mul(vec3d a, vec3d b);
static inline vec3d vec3d_div(vec3d a, vec3d b);
static inline vec3d vec3d_old_skool_divs(vec3d a, double s);
static inline vec3d vec3d_divs(vec3d a, double s);
static inline vec3d vec3d_scale(vec3d a, double s);
static inline double vec3d_dot(vec3d a, vec3d b);
static inline vec3d vec3d_cross_product(vec3d v1, vec3d v2);
static inline vec3d vec3d_component_product(vec3d v1, vec3d v2);
static inline vec3d vec3d_add_scaled_vector(vec3d v1, vec3d v2, double scale);
static inline double vec3d_magnitude(vec3d v1);
static inline double vec3d_square_magnitude(vec3d v1);
static inline vec3d vec3d_trim(vec3d v1, double size);
static inline vec3d vec3d_normalise(vec3d v1);
static inline vec3d vec3d_old_skool_normalise(vec3d v1);
static inline bool vec3d_equals(vec3d v1, vec3d v2);
static inline bool vec3d_less_than(vec3d v1, vec3d v2);
static inline bool vec3d_greater_than(vec3d v1, vec3d v2);
static inline bool vec3d_less_than_equal(vec3d v1, vec3d v2);
static inline bool vec3d_greater_than_equal(vec3d v1, vec3d v2);
static inline vec3d vec3d_invert(vec3d v1);
static inline vec3d vec3d_interpolate_linear(vec3d start, vec3d end, double progression);

static inline vec3d vec3d_set(double s) {
  return (vec3d){.data[0] = s, .data[1] = s, .data[2] = s};
}

static inline vec3d vec3d_add(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

static inline vec3d vec3d_sub(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

static inline vec3d vec3d_mul(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

static inline vec3d vec3d_div(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

static inline vec3d vec3d_divs(vec3d a, double s) {
  return (vec3d){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

static inline vec3d vec3d_scale(vec3d a, double s) {
  return (vec3d){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}

static inline double vec3d_dot(vec3d a, vec3d b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
}

static inline vec3d vec3d_cross_product(vec3d v1, vec3d v2) {
  return (vec3d){.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1], .data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2], .data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0]};
}

static inline vec3d vec3d_component_product(vec3d v1, vec3d v2) {
  return (vec3d){.data[0] = v1.data[0] * v2.data[0], .data[1] = v1.data[1] * v2.data[1], .data[2] = v1.data[2] * v2.data[2]};
}

static inline vec3d vec3d_add_scaled_vector(vec3d v1, vec3d v2, double scale) {
  return (vec3d){.data[0] = v1.data[0] + v2.data[0] * scale, .data[1] = v1.data[1] + v2.data[1] * scale, .data[2] = v1.data[2] + v2.data[2] * scale};
}

static inline double vec3d_magnitude(vec3d v1) {
  return sqrt(v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2]);
}

static inline double vec3d_square_magnitude(vec3d v1) {
  return v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
}

static inline vec3d vec3d_normalise(vec3d v1) {
  double ls = v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
  double length = sqrt(ls);
  if (length > 0)
    return (vec3d){.x = v1.data[0] / length, .y = v1.data[1] / length, .z = v1.data[2] / length};
  return v1;
}

static inline bool vec3d_equals(vec3d v1, vec3d v2) {
  return memcmp(&v1, &v2, sizeof(vec3d));
}
