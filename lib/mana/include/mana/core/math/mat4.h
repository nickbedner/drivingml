#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/quat.h"
#include "mana/core/math/vec3.h"
#include "mana/core/math/vec4.h"

typedef struct mat4 {
  union {
    struct
    {
      r32 m00, m01, m02, m03,
          m10, m11, m12, m13,
          m20, m21, m22, m23,
          m30, m31, m32, m33;
    };
    r32 data[16];
    vec4 vecs[4];
  };
} mat4;

global const mat4 MAT4_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f, .data[4] = 0.0f, .data[5] = 0.0f, .data[6] = 0.0f, .data[7] = 0.0f, .data[8] = 0.0f, .data[9] = 0.0f, .data[10] = 0.0f, .data[11] = 0.0f, .data[12] = 0.0f, .data[13] = 0.0f, .data[14] = 0.0f, .data[15] = 0.0f};
global const mat4 MAT4_IDENTITY = {.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f, .data[4] = 0.0f, .data[5] = 1.0f, .data[6] = 0.0f, .data[7] = 0.0f, .data[8] = 0.0f, .data[9] = 0.0f, .data[10] = 1.0f, .data[11] = 0.0f, .data[12] = 0.0f, .data[13] = 0.0f, .data[14] = 0.0f, .data[15] = 1.0f};

mat4 mat4_mul(mat4 m1, mat4 m2);
vec4 mat4_mul_vec4(mat4 m1, vec4 v1);
mat4 mat4_translate(mat4 m1, vec3 v1);
vec3 mat4_transform(mat4 m1, vec3 v1);
mat4 mat4_scale(mat4 m1, vec3 v1);
r32 mat4_determinant(mat4 m1);
mat4 mat4_inverse(mat4 m1);
vec3 mat4_transform_direction(mat4 m1, vec3 v1);
vec3 mat4_transform_inverse_direction(mat4 m1, vec3 v1);
vec3 mat4_transform_inverse(mat4 m1, vec3 v1);
vec3 mat4_get_axis_vector(mat4 m1, int i);
mat4 mat4_orientation_and_pos(quat q1, vec3 v1);
void mat4_fill_gl_array(mat4 m1, r32* array);
mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up);
mat4 mat4_transpose(mat4 m1);
mat4 mat4_rotate(mat4 m1, r32 angle, vec3 axis);
