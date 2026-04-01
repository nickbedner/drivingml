#pragma once

#include "mana/core/corecommon.h"

#define R32_EPSILON 1.1920928955078125e-07f
#define R32_MAX 3.4028234663852886e+38f
#define R32_E 2.7182817459106445f
#define R32_LOG2E 1.4426950216293335f
#define R32_LOG10E 0.4342944920063019f
#define R32_LN2 0.6931471824645996f
#define R32_LN10 2.3025851249694824f
#define R32_PI 3.1415927410125732f
#define R32_PI_2 1.5707963705062866f
#define R32_PI_4 0.7853981852531433f
#define R32_1_PI 0.3183098733425140f
#define R32_2_PI 0.6366197466850281f
#define R32_2_SQRTPI 1.1283792257308960f
#define R32_SQRT2 1.4142135381698608f
#define R32_SQRT1_2 0.7071067690849304f

b8 real32_isnan(r32 x);
b8 real32_isinf(r32 x);
r32 real32_sqrt(r32 x);
r32 real32_fmod(r32 x, r32 y);
r32 real32_log2(r32 x);
r32 real32_floor(r32 x);
r32 real32_fabs(r32 x);
r32 real32_fmax(r32 x, r32 y);
r32 real32_exp(r32 x);
r32 real32_atan(r32 x);
r32 real32_sin(r32 x);
r32 real32_cos(r32 x);
r32 real32_atan2(r32 y, r32 x);
r32 real32_asin(r32 x);
r32 real32_tan(r32 x);
r32 real32_tanh(r32 x);
