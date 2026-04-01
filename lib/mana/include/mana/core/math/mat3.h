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

global inline mat3 mat_inertia_tensor_coeffs(r32 ix, r32 iy, r32 iz, r32 ixy, r32 ixz, r32 iyz);
global inline mat3 mat3_block_intertia_tensor(vec3 v1, r32 mass);
global inline mat3 mat3_skew_symmetric(vec3 v1);
global inline vec3 mat3_transform(mat3 m1, vec3 v1);
global inline vec3 mat3_transform_transpose(mat3 m1, vec3 v1);
global inline vec3 mat3_get_row_vector(mat3 m1, int i);
global inline vec3 mat3_get_axis_vector(mat3 m1, int i);
global inline mat3 mat3_inverse(mat3 m1);
global inline mat3 mat3_transpose(mat3 m1);
global inline mat3 mat3_mul_mat3(mat3 m1, mat3 m2);
global inline mat3 mat3_mul_scalar(mat3 m1, r32 scalar);
global inline mat3 mat3_add_mat3(mat3 m1, mat3 m2);
global inline mat3 mat3_orientation(mat3 q1);
global inline mat3 mat3_liner_interpolate(mat3 m1, mat3 m2, r32 prop);

global inline mat3 mat3_block_intertia_tensor(vec3 v1, r32 mass) {
  vec3 squares = vec3_component_product(v1, v1);
  vec3 tensor_val = (vec3){.x = 0.3f * mass * (squares.data[1] + squares.data[2]),
                           .y = 0.3f * mass * (squares.data[0] + squares.data[2]),
                           .z = 0.3f * mass * (squares.data[0] + squares.data[1])};

  return (mat3){.data[0] = tensor_val.x,
                .data[1] = 0.0f,
                .data[2] = 0.0f,
                .data[3] = 0.0f,
                .data[4] = tensor_val.y,
                .data[5] = 0.0f,
                .data[6] = 0.0f,
                .data[7] = 0.0f,
                .data[8] = tensor_val.z};
}

global inline mat3 mat3_skew_symmetric(vec3 v1) {
  return (mat3){.data[0] = 0,
                .data[1] = -v1.data[2],
                .data[2] = v1.data[1],
                .data[3] = v1.data[2],
                .data[4] = 0,
                .data[5] = -v1.data[0],
                .data[6] = -v1.data[1],
                .data[7] = v1.data[0],
                .data[8] = 0};
}

// Mul vector
global inline vec3 mat3_transform(mat3 m1, vec3 v1) {
  return (vec3){.data[0] = v1.data[0] * m1.data[0] + v1.data[1] * m1.data[1] + v1.data[2] * m1.data[2],
                .data[1] = v1.data[0] * m1.data[3] + v1.data[1] * m1.data[4] + v1.data[2] * m1.data[5],
                .data[2] = v1.data[0] * m1.data[6] + v1.data[1] * m1.data[7] + v1.data[2] * m1.data[8]};
}

global inline vec3 mat3_transform_transpose(mat3 m1, vec3 v1) {
  return (vec3){.data[0] = v1.data[0] * m1.data[0] + v1.data[1] * m1.data[3] + v1.data[2] * m1.data[6],
                .data[1] = v1.data[0] * m1.data[1] + v1.data[1] * m1.data[4] + v1.data[2] * m1.data[7],
                .data[2] = v1.data[0] * m1.data[2] + v1.data[1] * m1.data[5] + v1.data[2] * m1.data[8]};
}

global inline vec3 mat3_get_row_vector(mat3 m1, int i) {
  return (vec3){.data[0] = m1.data[i * 3], .data[1] = m1.data[i * 3 + 1], .data[2] = m1.data[i * 3 + 2]};
}

global inline vec3 mat3_get_axis_vector(mat3 m1, int i) {
  return (vec3){.data[0] = m1.data[i], .data[1] = m1.data[i + 3], .data[2] = m1.data[i + 6]};
}

global inline mat3 mat3_inverse(mat3 m1) {
  r32 t4 = m1.data[0] * m1.data[4];
  r32 t6 = m1.data[0] * m1.data[5];
  r32 t8 = m1.data[1] * m1.data[3];
  r32 t10 = m1.data[2] * m1.data[3];
  r32 t12 = m1.data[1] * m1.data[6];
  r32 t14 = m1.data[2] * m1.data[6];
  r32 t16 = (t4 * m1.data[8] - t6 * m1.data[7] - t8 * m1.data[8] + t10 * m1.data[7] + t12 * m1.data[5] - t14 * m1.data[4]);
  if (t16 == 0.0f)
    return m1;

  r32 t17 = 1.0f / t16;
  return (mat3){.data[0] = (m1.data[4] * m1.data[8] - m1.data[5] * m1.data[7]) * t17,
                .data[1] = -(m1.data[1] * m1.data[8] - m1.data[2] * m1.data[7]) * t17,
                .data[2] = (m1.data[1] * m1.data[5] - m1.data[2] * m1.data[4]) * t17,
                .data[3] = -(m1.data[3] * m1.data[8] - m1.data[5] * m1.data[6]) * t17,
                .data[4] = (m1.data[0] * m1.data[8] - t14) * t17,
                .data[5] = -(t6 - t10) * t17,
                .data[6] = (m1.data[3] * m1.data[7] - m1.data[4] * m1.data[6]) * t17,
                .data[7] = -(m1.data[0] * m1.data[7] - t12) * t17,
                .data[8] = (t4 - t8) * t17};
}

global inline mat3 mat3_transpose(mat3 m1) {
  return (mat3){.data[0] = m1.data[0],
                .data[1] = m1.data[3],
                .data[2] = m1.data[6],
                .data[3] = m1.data[1],
                .data[4] = m1.data[4],
                .data[5] = m1.data[7],
                .data[6] = m1.data[2],
                .data[7] = m1.data[5],
                .data[8] = m1.data[8]};
}

global inline mat3 mat3_mul_mat3(mat3 m1, mat3 m2) {
  // return (mat3){.data[0] = m1.data[0] * m2.data[0] + m1.data[1] * m2.data[3] + m1.data[2] * m2.data[6],
  //               .data[1] = m1.data[0] * m2.data[1] + m1.data[1] * m2.data[4] + m1.data[2] * m2.data[7],
  //               .data[2] = m1.data[0] * m2.data[2] + m1.data[1] * m2.data[5] + m1.data[2] * m2.data[8],
  //               .data[3] = m1.data[3] * m2.data[0] + m1.data[4] * m2.data[3] + m1.data[5] * m2.data[6],
  //               .data[4] = m1.data[3] * m2.data[1] + m1.data[4] * m2.data[4] + m1.data[5] * m2.data[7],
  //               .data[5] = m1.data[3] * m2.data[2] + m1.data[4] * m2.data[5] + m1.data[5] * m2.data[8],
  //               .data[6] = m1.data[6] * m2.data[0] + m1.data[7] * m2.data[3] + m1.data[8] * m2.data[6],
  //               .data[7] = m1.data[6] * m2.data[1] + m1.data[7] * m2.data[4] + m1.data[8] * m2.data[7],
  //               .data[8] = m1.data[6] * m2.data[2] + m1.data[7] * m2.data[5] + m1.data[8] * m2.data[8]};

  r32 t1;
  r32 t2;
  r32 t3;

  t1 = m1.data[0] * m2.data[0] + m1.data[1] * m2.data[3] + m1.data[2] * m2.data[6];
  t2 = m1.data[0] * m2.data[1] + m1.data[1] * m2.data[4] + m1.data[2] * m2.data[7];
  t3 = m1.data[0] * m2.data[2] + m1.data[1] * m2.data[5] + m1.data[2] * m2.data[8];
  m1.data[0] = t1;
  m1.data[1] = t2;
  m1.data[2] = t3;

  t1 = m1.data[3] * m2.data[0] + m1.data[4] * m2.data[3] + m1.data[5] * m2.data[6];
  t2 = m1.data[3] * m2.data[1] + m1.data[4] * m2.data[4] + m1.data[5] * m2.data[7];
  t3 = m1.data[3] * m2.data[2] + m1.data[4] * m2.data[5] + m1.data[5] * m2.data[8];
  m1.data[3] = t1;
  m1.data[4] = t2;
  m1.data[5] = t3;

  t1 = m1.data[6] * m2.data[0] + m1.data[7] * m2.data[3] + m1.data[8] * m2.data[6];
  t2 = m1.data[6] * m2.data[1] + m1.data[7] * m2.data[4] + m1.data[8] * m2.data[7];
  t3 = m1.data[6] * m2.data[2] + m1.data[7] * m2.data[5] + m1.data[8] * m2.data[8];
  m1.data[6] = t1;
  m1.data[7] = t2;
  m1.data[8] = t3;

  return m1;
}

global inline mat3 mat3_mul_scalar(mat3 m1, r32 scalar) {
  return (mat3){.data[0] = m1.data[0] * scalar,
                .data[1] = m1.data[1] * scalar,
                .data[2] = m1.data[2] * scalar,
                .data[3] = m1.data[3] * scalar,
                .data[4] = m1.data[4] * scalar,
                .data[5] = m1.data[5] * scalar,
                .data[6] = m1.data[6] * scalar,
                .data[7] = m1.data[7] * scalar,
                .data[8] = m1.data[8] * scalar};
}

global inline mat3 mat3_add_mat3(mat3 m1, mat3 m2) {
  return (mat3){.data[0] = m1.data[0] + m2.data[0],
                .data[1] = m1.data[1] + m2.data[1],
                .data[2] = m1.data[2] + m2.data[2],
                .data[3] = m1.data[3] + m2.data[3],
                .data[4] = m1.data[4] + m2.data[4],
                .data[5] = m1.data[5] + m2.data[5],
                .data[6] = m1.data[6] + m2.data[6],
                .data[7] = m1.data[7] + m2.data[7],
                .data[8] = m1.data[8] + m2.data[8]};
}

global inline mat3 mat3_orientation(mat3 q1) {
  return (mat3){.data[0] = 1 - (2 * q1.data[2] * q1.data[2] + 2 * q1.data[3] * q1.data[3]),
                .data[1] = 2 * q1.data[1] * q1.data[2] + 2 * q1.data[3] * q1.data[0],
                .data[2] = 2 * q1.data[1] * q1.data[3] - 2 * q1.data[2] * q1.data[0],
                .data[3] = 2 * q1.data[1] * q1.data[2] - 2 * q1.data[3] * q1.data[0],
                .data[4] = 1 - (2 * q1.data[1] * q1.data[1] + 2 * q1.data[3] * q1.data[3]),
                .data[5] = 2 * q1.data[2] * q1.data[3] + 2 * q1.data[1] * q1.data[0],
                .data[6] = 2 * q1.data[1] * q1.data[3] + 2 * q1.data[2] * q1.data[0],
                .data[7] = 2 * q1.data[2] * q1.data[3] - 2 * q1.data[1] * q1.data[0],
                .data[8] = 1 - (2 * q1.data[1] * q1.data[1] + 2 * q1.data[2] * q1.data[2])};
}

global inline mat3 mat3_liner_interpolate(mat3 m1, mat3 m2, r32 prop) {
  return (mat3){.data[0] = m1.data[0] * (1 - prop) + m2.data[0] * prop,
                .data[1] = m1.data[1] * (1 - prop) + m2.data[1] * prop,
                .data[2] = m1.data[2] * (1 - prop) + m2.data[2] * prop,
                .data[3] = m1.data[3] * (1 - prop) + m2.data[3] * prop,
                .data[4] = m1.data[4] * (1 - prop) + m2.data[4] * prop,
                .data[5] = m1.data[5] * (1 - prop) + m2.data[5] * prop,
                .data[6] = m1.data[6] * (1 - prop) + m2.data[6] * prop,
                .data[7] = m1.data[7] * (1 - prop) + m2.data[7] * prop,
                .data[8] = m1.data[8] * (1 - prop) + m2.data[8] * prop};
}
