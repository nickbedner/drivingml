#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/quat.h"
#include "mana/core/math/vec3.h"

typedef struct mat3 {
  union {
    struct
    {
      r32 m00, m01, m02,
          m10, m11, m12,
          m20, m21, m22;
    };
    r32 data[9];
    vec3 vecs[3];
    // simd_align_max r32 data[9];
  };
} mat3;

global const mat3 MAT3_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f, .data[4] = 0.0f, .data[5] = 0.0f, .data[6] = 0.0f, .data[7] = 0.0f, .data[8] = 0.0f};
global const mat3 MAT3_IDENTITY = {.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f, .data[4] = 1.0f, .data[5] = 0.0f, .data[6] = 0.0f, .data[7] = 0.0f, .data[8] = 1.0f};
