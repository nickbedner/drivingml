#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/quat.h"
#include "mana/core/math/vec3d.h"
#include "mana/core/math/vec4d.h"

typedef struct mat4d {
  union {
    struct
    {
      r64 m00, m01, m02, m03,
          m10, m11, m12, m13,
          m20, m21, m22, m23,
          m30, m31, m32, m33;
    };
    r64 data[16];
    vec4d vecs[4];
  };
} mat4d;

global const mat4d MAT4D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 0.0, .data[5] = 0.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 0.0, .data[9] = 0.0, .data[10] = 0.0, .data[11] = 0.0, .data[12] = 0.0, .data[13] = 0.0, .data[14] = 0.0, .data[15] = 0.0};
global const mat4d MAT4D_IDENTITY = {.data[0] = 1.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 0.0, .data[5] = 1.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 0.0, .data[9] = 0.0, .data[10] = 1.0, .data[11] = 0.0, .data[12] = 0.0, .data[13] = 0.0, .data[14] = 0.0, .data[15] = 1.0};

mat4d mat4d_look_at(vec3d eye, vec3d center, vec3d up);
