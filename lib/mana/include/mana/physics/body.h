#include "shape.h"

struct Body {
  vec3 position;
  quat orientation;
  struct Shape shape;
};