#include "mana/core/math/mat4d.h"

mat4d mat4d_look_at(vec3d eye, vec3d center, vec3d up) {
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
