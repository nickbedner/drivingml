#pragma once

#include "mana/core/corecommon.h"

typedef struct vec4d {
  union {
    struct {
      r64 x, y, z, w;
    };
    struct {
      r64 r, g, b, a;
    };
    struct {
      r64 u, v, s, t;
    };
    r64 data[4];
  };
} vec4d;

global const vec4d VEC4D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0};

vec4d vec4d_add(vec4d a, vec4d b);
vec4d vec4d_sub(vec4d a, vec4d b);
vec4d vec4d_mul(vec4d a, vec4d b);
vec4d vec4d_div(vec4d a, vec4d b);
vec4d vec4d_divs(vec4d a, r64 s);
vec4d vec4d_scale(vec4d a, r64 s);
r64 vec4d_dot(vec4d a, vec4d b);
