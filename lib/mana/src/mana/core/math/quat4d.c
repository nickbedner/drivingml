#include "mana/core/math/quatd.h"

quatd quaterniond_create_from_axix_angle(vec3d axis, r64 angle) {
  r64 half_angle = angle * 0.5;
  r64 s = real64_sin(half_angle);
  r64 c = real64_cos(half_angle);

  return (quatd){.x = axis.x * s, .y = axis.y * s, .z = axis.z * s, .w = c};
}
