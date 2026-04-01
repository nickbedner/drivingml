#include "mana/core/math/ivec3.h"

ivec3 ivec3_set(i32 num) {
  return (ivec3){.data[0] = num, .data[1] = num, .data[2] = num};
}

ivec3 ivec3_add(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

ivec3 ivec3_sub(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

ivec3 ivec3_mul(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

ivec3 ivec3_div(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

ivec3 ivec3_divs(ivec3 a, i32 s) {
  return (ivec3){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

ivec3 ivec3_scale(ivec3 a, i32 s) {
  return (ivec3){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}
