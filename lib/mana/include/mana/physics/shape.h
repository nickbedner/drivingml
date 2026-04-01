#pragma once

#include "mana/math/quat.h"
#include "mana/math/vec3.h"

enum SHAPE_TYPE {
  SHAPE_SPHERE = 0
};

struct Shape {
  enum SHAPE_TYPE shape_type;
  union {
    struct SphereShape {
      r32 radius;
    };
  };
};