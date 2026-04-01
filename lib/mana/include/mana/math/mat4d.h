#pragma once

#include "mana/core/corecommon.h"
#include "mana/math/quat.h"
#include "mana/math/vec3d.h"
#include "mana/math/vec4d.h"

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

global inline mat4d mat4d_look_at(vec3d eye, vec3d center, vec3d up);

global inline mat4d mat4d_look_at(vec3d eye, vec3d center, vec3d up) {
  mat4d dest;
  vec3d f, u, s;

  f = vec3d_sub(center, eye);
  f = vec3d_normalise(f);

  s = vec3d_normalise(vec3d_cross_product(f, up));
  u = vec3d_cross_product(s, f);

  dest.vecs[0].data[0] = s.data[0];
  dest.vecs[0].data[1] = u.data[0];
  dest.vecs[0].data[2] = -f.data[0];
  dest.vecs[1].data[0] = s.data[1];
  dest.vecs[1].data[1] = u.data[1];
  dest.vecs[1].data[2] = -f.data[1];
  dest.vecs[2].data[0] = s.data[2];
  dest.vecs[2].data[1] = u.data[2];
  dest.vecs[2].data[2] = -f.data[2];
  dest.vecs[3].data[0] = -vec3d_dot(s, eye);
  dest.vecs[3].data[1] = -vec3d_dot(u, eye);
  dest.vecs[3].data[2] = vec3d_dot(f, eye);
  dest.vecs[0].data[3] = dest.vecs[1].data[3] = dest.vecs[2].data[3] = 0.0;
  dest.vecs[3].data[3] = 1.0;

  return dest;
}
