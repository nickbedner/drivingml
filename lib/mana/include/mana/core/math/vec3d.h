#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/real64.h"

typedef struct vec3d {
  union {
    struct {
      r64 x, y, z;
    };
    struct {
      r64 r, g, b;
    };
    struct {
      r64 u, v, s;
    };
    r64 data[3];
  };
} vec3d;

global const vec3d VEC3D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0};
global const vec3d VEC3D_ONE = {.data[0] = 1.0, .data[1] = 1.0, .data[2] = 1.0};

vec3d vec3d_set(r64 s);
vec3d vec3d_add(vec3d a, vec3d b);
vec3d vec3d_sub(vec3d a, vec3d b);
vec3d vec3d_mul(vec3d a, vec3d b);
vec3d vec3d_div(vec3d a, vec3d b);
vec3d vec3d_old_skool_divs(vec3d a, r64 s);
vec3d vec3d_divs(vec3d a, r64 s);
vec3d vec3d_scale(vec3d a, r64 s);
r64 vec3d_dot(vec3d a, vec3d b);
vec3d vec3d_cross_product(vec3d v1, vec3d v2);
vec3d vec3d_component_product(vec3d v1, vec3d v2);
vec3d vec3d_add_scaled_vector(vec3d v1, vec3d v2, r64 scale);
r64 vec3d_magnitude(vec3d v1);
r64 vec3d_square_magnitude(vec3d v1);
vec3d vec3d_trim(vec3d v1, r64 size);
vec3d vec3d_normalise(vec3d v1);
vec3d vec3d_old_skool_normalise(vec3d v1);
b8 vec3d_less_than(vec3d v1, vec3d v2);
b8 vec3d_greater_than(vec3d v1, vec3d v2);
b8 vec3d_less_than_equal(vec3d v1, vec3d v2);
b8 vec3d_greater_than_equal(vec3d v1, vec3d v2);
vec3d vec3d_invert(vec3d v1);
vec3d vec3d_interpolate_linear(vec3d start, vec3d end, r64 progression);
