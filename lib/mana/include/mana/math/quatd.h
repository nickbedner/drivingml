#pragma once

#include "mana/core/corecommon.h"
#include "mana/math/vec3d.h"

typedef struct quatd {
  union {
    struct {
      double x, y, z, w;
    };
    struct {
      double r, i, j, k;
    };
    double data[4];
  };
} quatd;

static const quatd QUATD_ZERO = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 0.0};
static const quatd QUATD_DEFAULT = {.data[0] = 0.0, .data[1] = 0.0, .data[2] = 0.0, .data[3] = 1.0};

static inline quatd quaterniond_create_from_axix_angle(vec3d axis, double angle) {
  double half_angle = angle * 0.5;
  double s = sin(half_angle);
  double c = cos(half_angle);

  return (quatd){.x = axis.x * s, .y = axis.y * s, .z = axis.z * s, .w = c};
}
