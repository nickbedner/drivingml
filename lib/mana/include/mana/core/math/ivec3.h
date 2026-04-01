#pragma once

#include "mana/core/corecommon.h"

typedef struct ivec3 {
  union {
    struct {
      i32 x, y, z;
    };
    struct {
      i32 id0, id1, id2;
    };
    i32 data[3];
    // simd_align_max int data[3];
  };
} ivec3;

global const ivec3 IVEC3_ZERO = {.data[0] = 0, .data[1] = 0, .data[2] = 0};

ivec3 ivec3_set(i32 num);
ivec3 ivec3_add(ivec3 a, ivec3 b);
ivec3 ivec3_sub(ivec3 a, ivec3 b);
ivec3 ivec3_mul(ivec3 a, ivec3 b);
ivec3 ivec3_div(ivec3 a, ivec3 b);
ivec3 ivec3_divs(ivec3 a, i32 s);
ivec3 ivec3_scale(ivec3 a, i32 s);
