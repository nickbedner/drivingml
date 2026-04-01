#pragma once

#include "mana/core/corecommon.h"

typedef struct vec4 {
  union {
    struct {
      r32 x, y, z, w;
    };
    struct {
      r32 r, g, b, a;
    };
    struct {
      r32 u, v, s, t;
    };
    r32 data[4];
    // simd_align_max r32 data[4];
  };
} vec4;

global const vec4 VEC4_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f};

vec4 vec4_add(vec4 a, vec4 b);
vec4 vec4_sub(vec4 a, vec4 b);
vec4 vec4_mul(vec4 a, vec4 b);
vec4 vec4_div(vec4 a, vec4 b);
vec4 vec4_divs(vec4 a, r32 s);
vec4 vec4_scale(vec4 a, r32 s);
r32 vec4_dot(vec4 a, vec4 b);
