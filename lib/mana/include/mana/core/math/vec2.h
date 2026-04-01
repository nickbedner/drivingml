#pragma once

#include "mana/core/corecommon.h"

typedef struct vec2 {
  union {
    struct {
      r32 x, y;
    };
    struct {
      r32 u, v;
    };
    struct {
      r32 s, t;
    };
    r32 data[2];
  };
} vec2;

global const vec2 VEC2_ZERO = {.data[0] = 0.0f, .data[1] = 0.0f};
