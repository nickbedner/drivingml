#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/math/vec3.h"
#include "mana/core/math/vec4.h"

typedef struct quat {
  union {
    struct {
      r32 x, y, z, w;
    };
    struct {
      r32 r, i, j, k;
    };
    // vec4 vec;
    r32 data[4];
    // simd_align_max r32 data[4];
  };
} quat;

global const quat QUAT_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 0.0f};
global const quat QUAT_DEFAULT = {.data[0] = 0.0f, .data[1] = 0.0f, .data[2] = 0.0f, .data[3] = 1.0f};

quat quaternion_set(r32 r, r32 i, r32 j, r32 k);
quat quaternion_normalise(quat q1);
r32 quaternion_magnitude(quat q1);
quat quaternion_add(quat q1, quat q2);
quat quaternion_mul(quat q1, quat q2);
quat quaternion_conjugate(quat q1);
quat quaternion_add_scaled_vector(quat q1, vec3 v1, r32 scale);
quat quaternion_rotate_by_vector(quat q1, vec3 v1);
quat quat_interpolate_linear(quat a, quat b, r32 blend);
