#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/quatd.h"
#include "mana/core/math/vec3d.h"

typedef struct mat3d {
  union {
    struct
    {
      r64 m00, m01, m02,
          m10, m11, m12,
          m20, m21, m22;
    };
    r64 data[9];
    vec3d vecs[3];
  };
} mat3d;

global const mat3d MAT3D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 0.0, .data[5] = 0.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 0.0};
global const mat3d MAT3D_IDENTITY = {.data[0] = 1.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 1.0, .data[5] = 0.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 1.0};

vec3d mat3d_transform_transpose(mat3d m1, vec3d v1);
mat3d mat3d_transpose(mat3d m1);
