#pragma once

#include "mana/core/corecommon.h"

typedef struct vec3 {
  union {
    struct {
      r32 x, y, z;
    };
    struct {
      r32 r, g, b;
    };
    struct {
      r32 u, v, s;
    };
    r32 data[3];
  };
} vec3;

global const vec3 VEC3_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f};
global const vec3 VEC3_ONE = {.data[0] = 1.0f, .data[1] = 1.0f, .data[2] = 1.0f};

global inline vec3 vec3_set(r32 s);
global inline vec3 vec3_add(vec3 a, vec3 b);
global inline vec3 vec3_sub(vec3 a, vec3 b);
global inline vec3 vec3_mul(vec3 a, vec3 b);
global inline vec3 vec3_div(vec3 a, vec3 b);
global inline vec3 vec3_old_skool_divs(vec3 a, r32 s);
global inline vec3 vec3_divs(vec3 a, r32 s);
global inline vec3 vec3_scale(vec3 a, r32 s);
global inline r32 vec3_dot(vec3 a, vec3 b);
global inline vec3 vec3_cross_product(vec3 v1, vec3 v2);
global inline vec3 vec3_component_product(vec3 v1, vec3 v2);
global inline vec3 vec3_add_scaled_vector(vec3 v1, vec3 v2, r32 scale);
global inline r32 vec3_magnitude(vec3 v1);
global inline r32 vec3_square_magnitude(vec3 v1);
global inline vec3 vec3_trim(vec3 v1, r32 size);
global inline vec3 vec3_normalize(vec3 v1);
global inline vec3 vec3_old_skool_normalise(vec3 v1);
global inline b8 vec3_less_than(vec3 v1, vec3 v2);
global inline b8 vec3_greater_than(vec3 v1, vec3 v2);
global inline b8 vec3_less_than_equal(vec3 v1, vec3 v2);
global inline b8 vec3_greater_than_equal(vec3 v1, vec3 v2);
global inline vec3 vec3_invert(vec3 v1);
global inline vec3 vec3_interpolate_linear(vec3 start, vec3 end, r32 progression);

global inline vec3 vec3_set(r32 s) {
  return (vec3){.data[0] = s, .data[1] = s, .data[2] = s};
}

global inline vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

global inline vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

global inline vec3 vec3_mul(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

global inline vec3 vec3_div(vec3 a, vec3 b) {
  return (vec3){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

global inline vec3 vec3_old_skool_divs(vec3 a, r32 s) {
  r32 factor = 1 / (r32)s;
  a.x *= factor;
  a.y *= factor;
  a.z *= factor;
  return a;
}

global inline vec3 vec3_divs(vec3 a, r32 s) {
  return (vec3){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

global inline vec3 vec3_scale(vec3 a, r32 s) {
  return (vec3){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}

// Scalar product
global inline r32 vec3_dot(vec3 a, vec3 b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
}

// Vector product
// TODO: remove products from these?
global inline vec3 vec3_cross_product(vec3 v1, vec3 v2) {
  return (vec3){.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1], .data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2], .data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0]};
}

global inline vec3 vec3_component_product(vec3 v1, vec3 v2) {
  return (vec3){.data[0] = v1.data[0] * v2.data[0], .data[1] = v1.data[1] * v2.data[1], .data[2] = v1.data[2] * v2.data[2]};
}

global inline vec3 vec3_add_scaled_vector(vec3 v1, vec3 v2, r32 scale) {
  return (vec3){.data[0] = v1.data[0] + v2.data[0] * scale, .data[1] = v1.data[1] + v2.data[1] * scale, .data[2] = v1.data[2] + v2.data[2] * scale};
}

global inline r32 vec3_magnitude(vec3 v1) {
  return sqrtf(v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2]);
}

global inline r32 vec3_square_magnitude(vec3 v1) {
  return v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
}

global inline vec3 vec3_trim(vec3 v1, r32 size) {
  if (vec3_square_magnitude(v1) > size * size) {
    vec3 temp = vec3_normalize(v1);
    return (vec3){.data[0] = temp.data[0] * size, .data[1] = temp.data[1] * size, .data[2] = temp.data[2] * size};
  }

  return (vec3){.data[0] = v1.data[0], .data[1] = v1.data[1], .data[2] = v1.data[2]};
}

global inline vec3 vec3_normalize(vec3 v1) {
  r32 ls = v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
  r32 length = sqrtf(ls);
  if (length > 0)
    return (vec3){.x = v1.data[0] / length, .y = v1.data[1] / length, .z = v1.data[2] / length};
  return v1;
}

global inline vec3 vec3_old_skool_normalise(vec3 v1) {
  r32 factor = 1.0f / sqrtf((v1.data[0] * v1.data[0]) + (v1.data[1] * v1.data[1]) + (v1.data[2] * v1.data[2]));
  return (vec3){.data[0] = v1.data[0] * factor, .data[1] = v1.data[1] * factor, .data[2] = v1.data[2] * factor};
}

global inline b8 vec3_less_than(vec3 v1, vec3 v2) {
  return v1.data[0] < v2.data[0] && v1.data[1] < v2.data[1] && v1.data[2] < v2.data[2];
}

global inline b8 vec3_greater_than(vec3 v1, vec3 v2) {
  return v1.data[0] > v2.data[0] && v1.data[1] > v2.data[1] && v1.data[2] > v2.data[2];
}

global inline b8 vec3_less_than_equal(vec3 v1, vec3 v2) {
  return v1.data[0] <= v2.data[0] && v1.data[1] <= v2.data[1] && v1.data[2] <= v2.data[2];
}

global inline b8 vec3_greater_than_equal(vec3 v1, vec3 v2) {
  return v1.data[0] >= v2.data[0] && v1.data[1] >= v2.data[1] && v1.data[2] >= v2.data[2];
}

global inline vec3 vec3_invert(vec3 v1) {
  return (vec3){.data[0] = -v1.data[0], .data[1] = -v1.data[1], .data[2] = -v1.data[2]};
}

// global inline vec3 vec3_lerp(vec3 v1, vec3 v2, r32 t) {
//   return (vec3){.data[0] = v1.data[0] * (1.0f - t) + v2.data[0] * t, .data[1] = v1.data[1] * (1.0f - t) + v2.data[1] * t, .data[2] = v1.data[2] * (1.0f - t) + v2.data[2] * t};
// }

global inline vec3 vec3_interpolate_linear(vec3 start, vec3 end, r32 progression) {
  return (vec3){.data[0] = start.data[0] + (end.data[0] - start.data[0]) * progression,
                .data[1] = start.data[1] + (end.data[1] - start.data[1]) * progression,
                .data[2] = start.data[2] + (end.data[2] - start.data[2]) * progression};
}
