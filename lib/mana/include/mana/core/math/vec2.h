#pragma once

#include <mana/core/corecommon.h>
#include <mana/core/math/real32.h>

typedef struct vec2 {
  union {
    struct {
      r32 x, y;
    };
    struct {
      r32 u, v;
    };
    struct {
      r32 s, t;
    };
    r32 data[2];
  };
} vec2;

global const vec2 VEC2_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f};

vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_mul(vec2 a, vec2 b);
vec2 vec2_div(vec2 a, vec2 b);
vec2 vec2_divs(vec2 a, r32 s);
vec2 vec2_scale(vec2 a, r32 s);
r32 vec2_dot(vec2 a, vec2 b);
b8 vec2_equals(vec2 v1, vec2 v2);
vec2 vec2_normalise(vec2 v1);
r32 vec2_mul_inner(vec2 const a, vec2 const b);
r32 vec2_len(vec2 const v);
vec2 vec2_norm(vec2 const v);
i32 vec2_nonzero_sign(r32 n);
r32 vec2_cross(vec2 a, vec2 b);
vec2 vec2_mix(vec2 a, vec2 b, r32 weight);
