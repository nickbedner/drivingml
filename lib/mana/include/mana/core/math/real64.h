#pragma once

#include "mana/core/corecommon.h"

#define R64_EPSILON 2.2204460492503131E-16
#define R64_MAX 1.7976931348623157e+308
#define R64_E 2.71828182845904523536
#define R64_LOG2E 1.44269504088896340736
#define R64_LOG10E 0.434294481903251827651
#define R64_LN2 0.693147180559945309417
#define R64_LN10 2.30258509299404568402
#define R64_PI 3.14159265358979323846
#define R64_PI_2 1.57079632679489661923
#define R64_PI_4 0.785398163397448309616
#define R64_1_PI 0.318309886183790671538
#define R64_2_PI 0.636619772367581343076
#define R64_2_SQRTPI 1.12837916709551257390
#define R64_SQRT2 1.41421356237309504880
#define R64_SQRT1_2 0.707106781186547524401

b8 real64_isnan(r64 x);
b8 real64_isinf(r64 x);
r64 real64_sqrt(r64 x);
r64 real64_fmod(r64 x, r64 y);
r64 real64_log2(r64 x);
r64 real64_floor(r64 x);
r64 real64_fabs(r64 x);
r64 real64_fmax(r64 x, r64 y);
r64 real64_exp(r64 x);
r64 real64_atan(r64 x);
r64 real64_sin(r64 x);
r64 real64_cos(r64 x);
r64 real64_atan2(r64 y, r64 x);
r64 real64_asin(r64 x);
r64 real64_tan(r64 x);
r64 real64_tanh(r64 x);
