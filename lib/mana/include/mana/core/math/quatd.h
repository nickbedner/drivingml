#pragma once

#include <mana/core/corecommon.h>
#include <mana/core/math/real64.h>
#include <mana/core/math/vec3d.h>

typedef struct quatd {
  union {
    struct {
      r64 x, y, z, w;
    };
    struct {
      r64 r, i, j, k;
    };
    r64 data[4];
  };
} quatd;

global const quatd QUATD_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0};
global const quatd QUATD_DEFAULT = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 1.0};

quatd quaterniond_create_from_axix_angle(vec3d axis, r64 angle);
