#include "mana/core/math/mat3d.h"

vec3d mat3d_transform_transpose(mat3d m1, vec3d v1) {
  return (vec3d){.data[0] = v1.data[0] * m1.data[0] + v1.data[1] * m1.data[3] + v1.data[2] * m1.data[6],
                 .data[1] = v1.data[0] * m1.data[1] + v1.data[1] * m1.data[4] + v1.data[2] * m1.data[7],
                 .data[2] = v1.data[0] * m1.data[2] + v1.data[1] * m1.data[5] + v1.data[2] * m1.data[8]};
}

mat3d mat3d_transpose(mat3d m1) {
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
