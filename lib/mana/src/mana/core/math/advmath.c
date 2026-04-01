#include "mana/core/math/advmath.h"

r32 degree_to_radian(r32 degree) {
  return degree * (r32)M_PI / 180.0f;
}

r64 degree_to_radian_d(r64 degree) {
  return degree * M_PI / 180.0;
}

r32 radian_to_degree(r32 radian) {
  return radian * 180.0f / (r32)M_PI;
}

vec4 vec3_to_vec4(vec3 v1) {
  return (vec4){.data[0] = v1.data[0], .data[1] = v1.data[1], .data[2] = v1.data[2], 0.0f};
}

vec3 quaternion_to_vec3(quat q1) {
  return (vec3){.data[0] = q1.data[0], .data[1] = q1.data[1], .data[2] = q1.data[2]};
}

mat4 quaternion_to_mat4(quat rotation) {
  r32 xy = rotation.data[0] * rotation.data[1];
  r32 xz = rotation.data[0] * rotation.data[2];
  r32 xw = rotation.data[0] * rotation.data[3];
  r32 yz = rotation.data[1] * rotation.data[2];
  r32 yw = rotation.data[1] * rotation.data[3];
  r32 zw = rotation.data[2] * rotation.data[3];
  r32 xSquared = rotation.data[0] * rotation.data[0];
  r32 ySquared = rotation.data[1] * rotation.data[1];
  r32 zSquared = rotation.data[2] * rotation.data[2];

  mat4 dest;
  dest.m00 = 1 - 2 * (ySquared + zSquared);
  dest.m01 = 2 * (xy - zw);
  dest.m02 = 2 * (xz + yw);
  dest.m03 = 0;
  dest.m10 = 2 * (xy + zw);
  dest.m11 = 1 - 2 * (xSquared + zSquared);
  dest.m12 = 2 * (yz - xw);
  dest.m13 = 0;
  dest.m20 = 2 * (xz - yw);
  dest.m21 = 2 * (yz + xw);
  dest.m22 = 1 - 2 * (xSquared + ySquared);
  dest.m23 = 0;
  dest.m30 = 0;
  dest.m31 = 0;
  dest.m32 = 0;
  dest.m33 = 1;

  return dest;
}

mat4 quaternion_to_mat4_other(quat q) {
  r32 w, x, y, z,
      xx, yy, zz,
      xy, yz, xz,
      wx, wy, wz, norm, s;

  norm = quaternion_magnitude(q);
  s = norm > 0.0f ? 2.0f / norm : 0.0f;

  x = q.data[0];
  y = q.data[1];
  z = q.data[2];
  w = q.data[3];

  xx = s * x * x;
  xy = s * x * y;
  wx = s * w * x;
  yy = s * y * y;
  yz = s * y * z;
  wy = s * w * y;
  zz = s * z * z;
  xz = s * x * z;
  wz = s * w * z;

  mat4 dest;

  dest.vecs[0].data[0] = 1.0f - yy - zz;
  dest.vecs[1].data[1] = 1.0f - xx - zz;
  dest.vecs[2].data[2] = 1.0f - xx - yy;
  dest.vecs[0].data[1] = xy + wz;
  dest.vecs[1].data[2] = yz + wx;
  dest.vecs[2].data[0] = xz + wy;
  dest.vecs[1].data[0] = xy - wz;
  dest.vecs[2].data[1] = yz - wx;
  dest.vecs[0].data[2] = xz - wy;
  dest.vecs[0].data[3] = 0.0f;
  dest.vecs[1].data[3] = 0.0f;
  dest.vecs[2].data[3] = 0.0f;
  dest.vecs[3].data[0] = 0.0f;
  dest.vecs[3].data[1] = 0.0f;
  dest.vecs[3].data[2] = 0.0f;
  dest.vecs[3].data[3] = 1.0f;

  return dest;
}

// Collada
quat mat4_to_quaternion(mat4 matrix) {
  quat dest;
  r32 diagonal = matrix.m00 + matrix.m11 + matrix.m22;
  if (diagonal > 0) {
    r32 w4 = (r32)(sqrtf(diagonal + 1.0f) * 2.0f);
    dest.data[3] = w4 / 4.0f;
    dest.data[0] = (matrix.m21 - matrix.m12) / w4;
    dest.data[1] = (matrix.m02 - matrix.m20) / w4;
    dest.data[2] = (matrix.m10 - matrix.m01) / w4;
  } else if ((matrix.m00 > matrix.m11) && (matrix.m00 > matrix.m22)) {
    r32 x4 = (r32)(sqrtf(1.0f + matrix.m00 - matrix.m11 - matrix.m22) * 2.0f);
    dest.data[3] = (matrix.m21 - matrix.m12) / x4;
    dest.data[0] = x4 / 4.0f;
    dest.data[1] = (matrix.m01 + matrix.m10) / x4;
    dest.data[2] = (matrix.m02 + matrix.m20) / x4;
  } else if (matrix.m11 > matrix.m22) {
    r32 y4 = (r32)(sqrtf(1.0f + matrix.m11 - matrix.m00 - matrix.m22) * 2.0f);
    dest.data[3] = (matrix.m02 - matrix.m20) / y4;
    dest.data[0] = (matrix.m01 + matrix.m10) / y4;
    dest.data[1] = y4 / 4.0f;
    dest.data[2] = (matrix.m12 + matrix.m21) / y4;
  } else {
    r32 z4 = (r32)(sqrtf(1.0f + matrix.m22 - matrix.m00 - matrix.m11) * 2.0f);
    dest.data[3] = (matrix.m10 - matrix.m01) / z4;
    dest.data[0] = (matrix.m02 + matrix.m20) / z4;
    dest.data[1] = (matrix.m12 + matrix.m21) / z4;
    dest.data[2] = z4 / 4.0f;
  }

  r32 mag = sqrtf(dest.data[3] * dest.data[3] + dest.data[0] * dest.data[0] + dest.data[1] * dest.data[1] + dest.data[2] * dest.data[2]);
  dest.data[3] /= mag;
  dest.data[0] /= mag;
  dest.data[1] /= mag;
  dest.data[2] /= mag;

  return dest;
}

vec3 ivec3_to_vec3(ivec3 ivec) {
  return (vec3){.x = (r32)ivec.x, .y = (r32)ivec.y, .z = (r32)ivec.z};
}

ivec3 vec3_to_ivec3(vec3 vec) {
  return (ivec3){.x = (i32)vec.x, .y = (i32)vec.y, .z = (i32)vec.z};
}

vec3d vec3d_transform(vec3d value, quatd rot) {
  r64 x2 = rot.x + rot.x;
  r64 y2 = rot.y + rot.y;
  r64 z2 = rot.z + rot.z;

  r64 wx2 = rot.w * x2;
  r64 wy2 = rot.w * y2;
  r64 wz2 = rot.w * z2;
  r64 xx2 = rot.x * x2;
  r64 xy2 = rot.x * y2;
  r64 xz2 = rot.x * z2;
  r64 yy2 = rot.y * y2;
  r64 yz2 = rot.y * z2;
  r64 zz2 = rot.z * z2;

  return (vec3d){.x = value.x * (1.0 - yy2 - zz2) + value.y * (xy2 - wz2) + value.z * (xz2 + wy2),
                 .y = value.x * (xy2 + wz2) + value.y * (1.0 - xx2 - zz2) + value.z * (yz2 - wx2),
                 .z = value.x * (xz2 - wy2) + value.y * (yz2 + wx2) + value.z * (1.0 - xx2 - yy2)};
}

vec3 vec3d_to_vec3(vec3d vec) {
  return (vec3){.x = (r32)vec.x, .y = (r32)vec.y, .z = (r32)vec.z};
}

vec3d vec3_to_vec3d(vec3 vec) {
  return (vec3d){.x = (r64)vec.x, .y = (r64)vec.y, .z = (r64)vec.z};
}

mat4 mat4d_to_mat4(mat4d mat) {
  return (mat4){.m00 = (r32)mat.m00,
                .m01 = (r32)mat.m01,
                .m02 = (r32)mat.m02,
                .m03 = (r32)mat.m03,
                .m10 = (r32)mat.m10,
                .m11 = (r32)mat.m11,
                .m12 = (r32)mat.m12,
                .m13 = (r32)mat.m13,
                .m20 = (r32)mat.m20,
                .m21 = (r32)mat.m21,
                .m22 = (r32)mat.m22,
                .m23 = (r32)mat.m23,
                .m30 = (r32)mat.m30,
                .m31 = (r32)mat.m31,
                .m32 = (r32)mat.m32,
                .m33 = (r32)mat.m33};
}
