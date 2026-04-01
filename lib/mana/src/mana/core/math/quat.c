#include "mana/core/math/quat.h"

quat quaternion_set(r32 r, r32 i, r32 j, r32 k) {
  return (quat){.data[0] = r, .data[1] = i, .data[2] = j, .data[3] = k};
}

quat quaternion_normalise(quat q1) {
  r32 d = q1.data[0] * q1.data[0] + q1.data[1] * q1.data[1] + q1.data[2] * q1.data[2] + q1.data[3] * q1.data[3];

  // if (d < 0.0f)
  if (d < R32_EPSILON)
    return QUAT_DEFAULT;

  d = 1.0f / real32_sqrt(d);
  return (quat){.data[0] = q1.data[0] * d, .data[1] = q1.data[1] * d, .data[2] = q1.data[2] * d, .data[3] = q1.data[3] * d};
}

r32 quaternion_magnitude(quat q1) {
  return real32_sqrt(q1.data[0] * q1.data[0] + q1.data[1] * q1.data[1] + q1.data[2] * q1.data[2] + q1.data[3] * q1.data[3]);
}

quat quaternion_add(quat q1, quat q2) {
  return (quat){.data[0] = q1.data[0] + q2.data[0], .data[1] = q1.data[1] + q2.data[1], .data[2] = q1.data[2] + q2.data[2], .data[3] = q1.data[3] + q2.data[3]};
}

quat quaternion_mul(quat p, quat q) {
  quat dest;
  dest.data[0] = p.data[3] * q.data[0] + p.data[0] * q.data[3] + p.data[1] * q.data[2] - p.data[2] * q.data[1];
  dest.data[1] = p.data[3] * q.data[1] - p.data[0] * q.data[2] + p.data[1] * q.data[3] + p.data[2] * q.data[0];
  dest.data[2] = p.data[3] * q.data[2] + p.data[0] * q.data[1] - p.data[1] * q.data[0] + p.data[2] * q.data[3];
  dest.data[3] = p.data[3] * q.data[3] - p.data[0] * q.data[0] - p.data[1] * q.data[1] - p.data[2] * q.data[2];
  return dest;
  // return (quat){.data[0] = q1.data[0] * q2.data[0] - q1.data[1] * q2.data[1] - q1.data[2] * q2.data[2] - q1.data[3] * q2.data[3],
  //               .data[1] = q1.data[0] * q2.data[1] + q1.data[1] * q2.data[0] + q1.data[2] * q2.data[3] - q1.data[3] * q2.data[2],
  //               .data[2] = q1.data[0] * q2.data[2] + q1.data[2] * q2.data[0] + q1.data[3] * q2.data[1] - q1.data[1] * q2.data[3],
  //               .data[3] = q1.data[0] * q2.data[3] + q1.data[3] * q2.data[0] + q1.data[1] * q2.data[2] - q1.data[2] * q2.data[1]};
}

quat quaternion_conjugate(quat q1) {
  return (quat){.data[0] = -q1.data[0], .data[1] = -q1.data[1], .data[2] = -q1.data[2], .data[3] = -q1.data[3]};
}

quat quaternion_add_scaled_vector(quat q1, vec3 v1, r32 scale) {
  quat temp = (quat){.data[0] = v1.data[0] * scale, .data[1] = v1.data[1] * scale, .data[2] = v1.data[2] * scale, .data[3] = 0.0f};  // quaternion_mul(q1, (quat){.data[0] = v1.data[0] * scale, .data[1] = v1.data[1] * scale, .data[2] = v1.data[2] * scale, .data[3] = 0.0f});
  temp = quaternion_mul(temp, q1);
  return (quat){.data[0] = q1.data[0] + temp.data[0] * 0.5f, .data[1] = q1.data[1] + temp.data[1] * 0.5f, .data[2] = q1.data[2] + temp.data[2] * 0.5f, .data[3] = q1.data[3] + temp.data[3] * 0.5f};
}

quat quaternion_rotate_by_vector(quat q1, vec3 v1) {
  return quaternion_mul(q1, (quat){.data[0] = v1.data[0], .data[1] = v1.data[1], .data[2] = v1.data[2], .data[3] = 0.0f});
}

quat quat_interpolate_linear(quat a, quat b, r32 blend) {
  quat dest;
  r32 dot = a.data[3] * b.data[3] + a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
  r32 blendI = 1.0f - blend;
  if (dot < 0) {
    dest.data[3] = blendI * a.data[3] + blend * -b.data[3];
    dest.data[0] = blendI * a.data[0] + blend * -b.data[0];
    dest.data[1] = blendI * a.data[1] + blend * -b.data[1];
    dest.data[2] = blendI * a.data[2] + blend * -b.data[2];
  } else {
    dest.data[3] = blendI * a.data[3] + blend * b.data[3];
    dest.data[0] = blendI * a.data[0] + blend * b.data[0];
    dest.data[1] = blendI * a.data[1] + blend * b.data[1];
    dest.data[2] = blendI * a.data[2] + blend * b.data[2];
  }

  r32 mag = real32_sqrt(dest.data[3] * dest.data[3] + dest.data[0] * dest.data[0] + dest.data[1] * dest.data[1] + dest.data[2] * dest.data[2]);
  dest.data[3] /= mag;
  dest.data[0] /= mag;
  dest.data[1] /= mag;
  dest.data[2] /= mag;

  return dest;
}
