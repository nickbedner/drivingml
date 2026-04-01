#pragma once

#include <mana/core/corecommon.h>
#include <mana/core/math/real32.h>

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

vec3 vec3_set(r32 s);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_mul(vec3 a, vec3 b);
vec3 vec3_div(vec3 a, vec3 b);
vec3 vec3_old_skool_divs(vec3 a, r32 s);
vec3 vec3_divs(vec3 a, r32 s);
vec3 vec3_scale(vec3 a, r32 s);
r32 vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross_product(vec3 v1, vec3 v2);
vec3 vec3_component_product(vec3 v1, vec3 v2);
vec3 vec3_add_scaled_vector(vec3 v1, vec3 v2, r32 scale);
r32 vec3_magnitude(vec3 v1);
r32 vec3_square_magnitude(vec3 v1);
vec3 vec3_trim(vec3 v1, r32 size);
vec3 vec3_normalize(vec3 v1);
vec3 vec3_old_skool_normalise(vec3 v1);
b8 vec3_less_than(vec3 v1, vec3 v2);
b8 vec3_greater_than(vec3 v1, vec3 v2);
b8 vec3_less_than_equal(vec3 v1, vec3 v2);
b8 vec3_greater_than_equal(vec3 v1, vec3 v2);
vec3 vec3_invert(vec3 v1);
vec3 vec3_interpolate_linear(vec3 start, vec3 end, r32 progression);
