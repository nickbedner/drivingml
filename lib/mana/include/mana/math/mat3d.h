#pragma once

#include "mana/core/corecommon.h"
#include "mana/math/quatd.h"
#include "mana/math/vec3d.h"

typedef struct mat3d {
  union {
    struct
    {
      double m00, m01, m02,
          m10, m11, m12,
          m20, m21, m22;
    };
    double data[9];
    vec3d vecs[3];
  };
} mat3d;

static const mat3d MAT3D_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 0.0, .data[5] = 0.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 0.0};
static const mat3d MAT3D_IDENTITY = {.data[0] = 1.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0, .data[4] = 1.0, .data[5] = 0.0, .data[6] = 0.0, .data[7] = 0.0, .data[8] = 1.0};

static inline vec3d mat3d_transform_transpose(mat3d m1, vec3d v1) {
  return (vec3d){.data[0] = v1.data[0] * m1.data[0] + v1.data[1] * m1.data[3] + v1.data[2] * m1.data[6],
                 .data[1] = v1.data[0] * m1.data[1] + v1.data[1] * m1.data[4] + v1.data[2] * m1.data[7],
                 .data[2] = v1.data[0] * m1.data[2] + v1.data[1] * m1.data[5] + v1.data[2] * m1.data[8]};
}

static inline mat3d mat3d_transpose(mat3d m1) {
  return (mat3d){.data[0] = m1.data[0],
                 .data[1] = m1.data[3],
                 .data[2] = m1.data[6],
                 .data[3] = m1.data[1],
                 .data[4] = m1.data[4],
                 .data[5] = m1.data[7],
                 .data[6] = m1.data[2],
                 .data[7] = m1.data[5],
                 .data[8] = m1.data[8]};
}
