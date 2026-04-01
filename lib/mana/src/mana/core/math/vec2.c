#include "mana/core/math/vec2.h"

vec2 vec2_add(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1]};
}

vec2 vec2_sub(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1]};
}

vec2 vec2_mul(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1]};
}

vec2 vec2_div(vec2 a, vec2 b) {
  return (vec2){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1]};
}

vec2 vec2_divs(vec2 a, r32 s) {
  return (vec2){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s};
}

vec2 vec2_scale(vec2 a, r32 s) {
  return (vec2){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s};
}

r32 vec2_dot(vec2 a, vec2 b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1];
}

b8 vec2_equals(vec2 v1, vec2 v2) {
  return (real32_fabs(v1.x - v2.x) < R32_EPSILON) && (real32_fabs(v1.y - v2.y) < R32_EPSILON);
}

// Note: Have no clue how this works going off the book
vec2 vec2_normalise(vec2 v1) {
  r32 mag = real32_sqrt((v1.data[0] * v1.data[0]) + (v1.data[1] * v1.data[1]));
  r32 inv_mag = 1.0f / mag;
  if (real32_fabs(0.0f * inv_mag) < R32_EPSILON) {
    v1.x = v1.x * inv_mag;
    v1.y = v1.y * inv_mag;
  }
  return v1;
}

r32 vec2_mul_inner(vec2 const a, vec2 const b) {
  r32 p = 0.0f;
  int i;
  for (i = 0; i < 2; ++i)
    p += b.data[i] * a.data[i];
  return p;
}

r32 vec2_len(vec2 const v) {
  return real32_sqrt(vec2_mul_inner(v, v));
}

// TODO: Figure out which norm is better
vec2 vec2_norm(vec2 const v) {
  r32 k = 1.0f / vec2_len(v);
  return vec2_scale(v, k);
}

i32 vec2_nonzero_sign(r32 n) {
  return 2 * (n > 0) - 1;
}

r32 vec2_cross(vec2 a, vec2 b) {
  return a.data[0] * b.data[1] - a.data[1] * b.data[0];
}

vec2 vec2_mix(vec2 a, vec2 b, r32 weight) {
  return (vec2){.data[0] = (1 - weight) * a.data[0] + weight * b.data[0], .data[1] = (1 - weight) * a.data[1] + weight * b.data[1]};
}
