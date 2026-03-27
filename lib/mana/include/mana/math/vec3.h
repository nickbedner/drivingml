#pragma once

#include "mana/core/corecommon.h"

typedef struct vec3 {
  union {
    struct {
      float x, y, z;
    };
    struct {
      float r, g, b;
    };
    struct {
      float u, v, s;
    };
    float data[3];
  };
} vec3;

static const vec3 VEC3_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f};
static const vec3 VEC3_ONE = {.data[0] = 1.0f, .data[1] = 1.0f, .data[2] = 1.0f};

static inline vec3 vec3_set(float s);
static inline vec3 vec3_add(vec3 a, vec3 b);
static inline vec3 vec3_sub(vec3 a, vec3 b);
static inline vec3 vec3_mul(vec3 a, vec3 b);
static inline vec3 vec3_div(vec3 a, vec3 b);
static inline vec3 vec3_old_skool_divs(vec3 a, float s);
static inline vec3 vec3_divs(vec3 a, float s);
static inline vec3 vec3_scale(vec3 a, float s);
static inline float vec3_dot(vec3 a, vec3 b);
static inline vec3 vec3_cross_product(vec3 v1, vec3 v2);
static inline vec3 vec3_component_product(vec3 v1, vec3 v2);
static inline vec3 vec3_add_scaled_vector(vec3 v1, vec3 v2, float scale);
static inline float vec3_magnitude(vec3 v1);
static inline float vec3_square_magnitude(vec3 v1);
static inline vec3 vec3_trim(vec3 v1, float size);
static inline vec3 vec3_normalize(vec3 v1);
static inline vec3 vec3_old_skool_normalise(vec3 v1);
static inline bool vec3_equals(vec3 v1, vec3 v2);
static inline bool vec3_less_than(vec3 v1, vec3 v2);
static inline bool vec3_greater_than(vec3 v1, vec3 v2);
static inline bool vec3_less_than_equal(vec3 v1, vec3 v2);
static inline bool vec3_greater_than_equal(vec3 v1, vec3 v2);
static inline vec3 vec3_invert(vec3 v1);
static inline vec3 vec3_interpolate_linear(vec3 start, vec3 end, float progression);

static inline vec3 vec3_set(float s) {
  return (vec3){.data[0] = s, .data[1] = s, .data[2] = s};
}

static inline vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

static inline vec3 vec3_mul(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

static inline vec3 vec3_div(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

static inline vec3 vec3_old_skool_divs(vec3 a, float s) {
  float factor = 1 / (float)s;
  a.x *= factor;
  a.y *= factor;
  a.z *= factor;
  return a;
}

static inline vec3 vec3_divs(vec3 a, float s) {
  return (vec3){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

static inline vec3 vec3_scale(vec3 a, float s) {
  return (vec3){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}

// Scalar product
static inline float vec3_dot(vec3 a, vec3 b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
}

// Vector product
// TODO: remove products from these?
static inline vec3 vec3_cross_product(vec3 v1, vec3 v2) {
  return (vec3){.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1], .data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2], .data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0]};
}

static inline vec3 vec3_component_product(vec3 v1, vec3 v2) {
  return (vec3){.data[0] = v1.data[0] * v2.data[0], .data[1] = v1.data[1] * v2.data[1], .data[2] = v1.data[2] * v2.data[2]};
}

static inline vec3 vec3_add_scaled_vector(vec3 v1, vec3 v2, float scale) {
  return (vec3){.data[0] = v1.data[0] + v2.data[0] * scale, .data[1] = v1.data[1] + v2.data[1] * scale, .data[2] = v1.data[2] + v2.data[2] * scale};
}

static inline float vec3_magnitude(vec3 v1) {
  return sqrtf(v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2]);
}

static inline float vec3_square_magnitude(vec3 v1) {
  return v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
}

static inline vec3 vec3_trim(vec3 v1, float size) {
  if (vec3_square_magnitude(v1) > size * size) {
    vec3 temp = vec3_normalize(v1);
    return (vec3){.data[0] = temp.data[0] * size, .data[1] = temp.data[1] * size, .data[2] = temp.data[2] * size};
  }

  return (vec3){.data[0] = v1.data[0], .data[1] = v1.data[1], .data[2] = v1.data[2]};
}

static inline vec3 vec3_normalize(vec3 v1) {
  float ls = v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
  float length = sqrtf(ls);
  if (length > 0)
    return (vec3){.x = v1.data[0] / length, .y = v1.data[1] / length, .z = v1.data[2] / length};
  return v1;
  // float length = vec3_magnitude(v1);
  // if (length > 0)
  //   return vec3_divs(v1, length);
  ////return vec3_scale(v1, ((float)1.0) / length);
  // return v1;
}

static inline vec3 vec3_old_skool_normalise(vec3 v1) {
  float factor = 1.0f / sqrtf((v1.data[0] * v1.data[0]) + (v1.data[1] * v1.data[1]) + (v1.data[2] * v1.data[2]));
  return (vec3){.data[0] = v1.data[0] * factor, .data[1] = v1.data[1] * factor, .data[2] = v1.data[2] * factor};
}

// TODO: Might just need to pass memory location of first variable
// TODO: Compare each struct member this is NOT safe because of padding issues
static inline bool vec3_equals(vec3 v1, vec3 v2) {
  return memcmp(&v1, &v2, sizeof(vec3));
}

static inline bool vec3_less_than(vec3 v1, vec3 v2) {
  return v1.data[0] < v2.data[0] && v1.data[1] < v2.data[1] && v1.data[2] < v2.data[2];
}

static inline bool vec3_greater_than(vec3 v1, vec3 v2) {
  return v1.data[0] > v2.data[0] && v1.data[1] > v2.data[1] && v1.data[2] > v2.data[2];
}

static inline bool vec3_less_than_equal(vec3 v1, vec3 v2) {
  return v1.data[0] <= v2.data[0] && v1.data[1] <= v2.data[1] && v1.data[2] <= v2.data[2];
}

static inline bool vec3_greater_than_equal(vec3 v1, vec3 v2) {
  return v1.data[0] >= v2.data[0] && v1.data[1] >= v2.data[1] && v1.data[2] >= v2.data[2];
}

static inline vec3 vec3_invert(vec3 v1) {
  return (vec3){.data[0] = -v1.data[0], .data[1] = -v1.data[1], .data[2] = -v1.data[2]};
}

// static inline vec3 vec3_lerp(vec3 v1, vec3 v2, float t) {
//   return (vec3){.data[0] = v1.data[0] * (1.0f - t) + v2.data[0] * t, .data[1] = v1.data[1] * (1.0f - t) + v2.data[1] * t, .data[2] = v1.data[2] * (1.0f - t) + v2.data[2] * t};
// }

static inline vec3 vec3_interpolate_linear(vec3 start, vec3 end, float progression) {
  return (vec3){.data[0] = start.data[0] + (end.data[0] - start.data[0]) * progression,
                .data[1] = start.data[1] + (end.data[1] - start.data[1]) * progression,
                .data[2] = start.data[2] + (end.data[2] - start.data[2]) * progression};
}
