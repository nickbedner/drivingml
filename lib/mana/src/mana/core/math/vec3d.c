#include "mana/core/math/vec3d.h"

vec3d vec3d_set(r64 s) {
  return (vec3d){.data[0] = s, .data[1] = s, .data[2] = s};
}

vec3d vec3d_add(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

vec3d vec3d_sub(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

vec3d vec3d_mul(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

vec3d vec3d_div(vec3d a, vec3d b) {
  return (vec3d){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

vec3d vec3d_divs(vec3d a, r64 s) {
  return (vec3d){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

vec3d vec3d_scale(vec3d a, r64 s) {
  return (vec3d){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}

r64 vec3d_dot(vec3d a, vec3d b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
}

vec3d vec3d_cross_product(vec3d v1, vec3d v2) {
  return (vec3d){.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1], .data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2], .data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0]};
}

vec3d vec3d_component_product(vec3d v1, vec3d v2) {
  return (vec3d){.data[0] = v1.data[0] * v2.data[0], .data[1] = v1.data[1] * v2.data[1], .data[2] = v1.data[2] * v2.data[2]};
}

vec3d vec3d_add_scaled_vector(vec3d v1, vec3d v2, r64 scale) {
  return (vec3d){.data[0] = v1.data[0] + v2.data[0] * scale, .data[1] = v1.data[1] + v2.data[1] * scale, .data[2] = v1.data[2] + v2.data[2] * scale};
}

r64 vec3d_magnitude(vec3d v1) {
  return real64_sqrt(v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2]);
}

r64 vec3d_square_magnitude(vec3d v1) {
  return v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
}

vec3d vec3d_normalise(vec3d v1) {
  r64 ls = v1.data[0] * v1.data[0] + v1.data[1] * v1.data[1] + v1.data[2] * v1.data[2];
  r64 length = real64_sqrt(ls);
  if (length > 0)
    return (vec3d){.x = v1.data[0] / length, .y = v1.data[1] / length, .z = v1.data[2] / length};
  return v1;
}
