#pragma once

#include "mana/core/corecommon.h"

typedef struct ivec3 {
  union {
    struct {
      int32_t x, y, z;
    };
    struct {
      int32_t id0, id1, id2;
    };
    int32_t data[3];
    // simd_align_max int data[3];
  };
} ivec3;

static const ivec3 IVEC3_ZERO = {.data[0] = 0, .data[1] = 0, .data[2] = 0};

static inline ivec3 ivec3_add(ivec3 a, ivec3 b);
static inline ivec3 ivec3_sub(ivec3 a, ivec3 b);
static inline ivec3 ivec3_mul(ivec3 a, ivec3 b);
static inline ivec3 ivec3_div(ivec3 a, ivec3 b);
static inline ivec3 ivec3_divs(ivec3 a, int32_t s);
static inline ivec3 ivec3_scale(ivec3 a, int32_t s);

static inline ivec3 ivec3_set(int num) {
  return (ivec3){.data[0] = num, .data[1] = num, .data[2] = num};
}

static inline ivec3 ivec3_add(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] + b.data[0], .data[1] = a.data[1] + b.data[1], .data[2] = a.data[2] + b.data[2]};
}

static inline ivec3 ivec3_sub(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] - b.data[0], .data[1] = a.data[1] - b.data[1], .data[2] = a.data[2] - b.data[2]};
}

static inline ivec3 ivec3_mul(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] * b.data[0], .data[1] = a.data[1] * b.data[1], .data[2] = a.data[2] * b.data[2]};
}

static inline ivec3 ivec3_div(ivec3 a, ivec3 b) {
  return (ivec3){.data[0] = a.data[0] / b.data[0], .data[1] = a.data[1] / b.data[1], .data[2] = a.data[2] / b.data[2]};
}

static inline ivec3 ivec3_divs(ivec3 a, int32_t s) {
  return (ivec3){.data[0] = a.data[0] / s, .data[1] = a.data[1] / s, .data[2] = a.data[2] / s};
}

static inline ivec3 ivec3_scale(ivec3 a, int32_t s) {
  return (ivec3){.data[0] = a.data[0] * s, .data[1] = a.data[1] * s, .data[2] = a.data[2] * s};
}
