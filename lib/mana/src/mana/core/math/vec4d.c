#include "mana/core/math/vec4d.h"

vec4d vec4d_add(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2], .data[3] = a.data[3] + b.data[3]};
}

vec4d vec4d_sub(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2], .data[3] = a.data[3] - b.data[3]};
}

vec4d vec4d_mul(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2], .data[3] = a.data[3] * b.data[3]};
}

vec4d vec4d_div(vec4d a, vec4d b) {
  return (vec4d){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2], .data[3] = a.data[3] / b.data[3]};
}

vec4d vec4d_divs(vec4d a, r64 s) {
  return (vec4d){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s, .data[3] = a.data[3] / s};
}

vec4d vec4d_scale(vec4d a, r64 s) {
  return (vec4d){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s, .data[3] = a.data[3] * s};
}

r64 vec4d_dot(vec4d a, vec4d b) {
  return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2] + a.data[3] * b.data[3];
}
