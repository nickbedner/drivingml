#include "mana/core/math/real32.h"

b8 real32_isnan(r32 x) {
  return x != x;
}

b8 real32_isinf(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  return exp_bits == 0xff && frac_bits == 0;
}

r32 real32_sqrt(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp = v.u & 0x7f800000U;
  u32 frac = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp == 0x7f800000U) {
    if (frac) return x;                 // NaN
    return x > 0.0f ? x : 0.0f / 0.0f;  // +inf -> +inf, -inf -> NaN
  }

  // Preserve signed zero
  if (x == 0.0f) return x;

  // Negative finite -> NaN
  if (x < 0.0f) return 0.0f / 0.0f;

  // Handle subnormals by scaling
  if (exp == 0) {
    r32 y = real32_sqrt(x * 16777216.0f);  // 2^24
    return y * 0.000244140625f;            // 2^-12
  }

  // Bit-level initial guess
  union {
    r32 f;
    u32 u;
  } y;

  y.u = (v.u >> 1) + 0x1fc00000U;

  r32 r = y.f;

  // Newton-Raphson refinement
  r = 0.5f * (r + x / r);
  r = 0.5f * (r + x / r);
  r = 0.5f * (r + x / r);
  r = 0.5f * (r + x / r);

  return r;
}

r32 real32_fmod(r32 x, r32 y) {
  union {
    r32 f;
    u32 u;
  } ux = {x}, uy = {y};

  u32 sx = ux.u & 0x80000000U;
  u32 hx = ux.u & 0x7fffffffU;
  u32 hy = uy.u & 0x7fffffffU;

  // NaN propagation
  if (hx > 0x7f800000U)
    return x;
  if (hy > 0x7f800000U)
    return y;

  // fmod(x, 0) = NaN
  if (hy == 0)
    return 0.0f / 0.0f;

  // fmod(inf, y) = NaN
  if (hx == 0x7f800000U)
    return 0.0f / 0.0f;

  // fmod(x, inf) = x
  if (hy == 0x7f800000U)
    return x;

  // fmod(+-0, y) = +-0
  if (hx == 0)
    return x;

  // |x| < |y|
  if (hx < hy)
    return x;

  // |x| == |y|
  if (hx == hy) {
    ux.u = sx;
    return ux.f;
  }

  i32 ex;
  i32 ey;
  u32 mx;
  u32 my;

  // Normalize x
  if (hx < 0x00800000U) {
    ex = -126;
    mx = hx;
    while ((mx & 0x00800000U) == 0) {
      mx <<= 1;
      ex -= 1;
    }
  } else {
    ex = (i32)(hx >> 23) - 127;
    mx = (hx & 0x007fffffU) | 0x00800000U;
  }

  // Normalize y
  if (hy < 0x00800000U) {
    ey = -126;
    my = hy;
    while ((my & 0x00800000U) == 0) {
      my <<= 1;
      ey -= 1;
    }
  } else {
    ey = (i32)(hy >> 23) - 127;
    my = (hy & 0x007fffffU) | 0x00800000U;
  }

  // Fixed-point modulo
  while (ex > ey) {
    if (mx >= my) mx -= my;
    mx <<= 1;
    ex -= 1;
  }

  if (mx >= my) mx -= my;

  // Exact zero
  if (mx == 0) {
    ux.u = sx;
    return ux.f;
  }

  // Normalize result
  while ((mx & 0x00800000U) == 0) {
    mx <<= 1;
    ex -= 1;
  }

  // Repack result
  if (ex > -127) {
    ux.u = sx | (u32)(ex + 127) << 23 | (mx & 0x007fffffU);
  } else {
    mx >>= (-126 - ex);
    ux.u = sx | mx;
  }

  return ux.f;
}

r32 real32_log2(r32 x) {
  union {
    r32 f;
    i32 u;
  } v = {x};

  // Special cases
  if (real32_isnan(x))
    return x;  // NaN
  if (x == 0.0f)
    return -1.0f / 0.0f;
  if (x < 0.0f)
    return 0.0f / 0.0f;
  if (real32_isinf(x))
    return x;  // +inf

  i32 exp = (i32)((v.u >> 23) & 0xff);
  i32 mant = v.u & 0x7fffff;

  // Handle subnormals
  if (exp == 0) {
    x *= 8388608.0f;  // 2^23
    v.f = x;
    exp = (i32)((v.u >> 23) & 0xff) - 23;
    mant = v.u & 0x7fffff;
  }

  // x = m * 2^e, with m in [1, 2)
  i32 e = exp - 127;
  r32 m = 1.0f + (r32)mant / 8388608.0f;  // 2^23

  // Approximate log2(m) using:
  // ln(m) = 2 * (y + y^3/3 + y^5/5 + y^7/7 + ...)
  // where y = (m - 1) / (m + 1)
  r32 y = (m - 1.0f) / (m + 1.0f);
  r32 y2 = y * y;

  r32 ln_m = 2.0f * (y +
                     y * y2 / 3.0f +
                     y * y2 * y2 / 5.0f +
                     y * y2 * y2 * y2 / 7.0f);

  // log2(m) = ln(m) / ln(2)
  r32 log2_m = ln_m * 1.4426950408889634f;

  return (r32)e + log2_m;
}

r32 real32_floor(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  i32 exp = (i32)exp_bits - 127;

  // NaN or infinity
  if (exp_bits == 0xff)
    return x;

  // |x| < 1
  if (exp < 0) {
    if (x == 0.0f)
      return x;  // preserve signed zero
    return x < 0.0f ? -1.0f : 0.0f;
  }

  // Already integral
  if (exp >= 23)
    return x;

  u32 mask = (1U << (23 - exp)) - 1;

  // Already integral
  if ((v.u & mask) == 0)
    return x;

  // Clear fractional bits
  v.u &= ~mask;
  r32 r = v.f;

  // Negative values round down one more step
  if (x < 0.0f)
    r -= 1.0f;

  return r;
}

r32 real32_fabs(r32 x) {
  union {
    r32 f;
    i32 u;
  } v = {x};

  v.u &= 0x7fffffffU;
  return v.f;
}

static b8 real32_signbit(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  return (b8)(v.u >> 31);
}

r32 real32_fmax(r32 x, r32 y) {
  if (x != x)
    return y;  // x is NaN
  if (y != y)
    return x;  // y is NaN

  if (x > y)
    return x;
  if (y > x)
    return y;

  // x and y compare equal here. Prefer +0 over -0.
  return real32_signbit(x) ? y : x;
}

r32 real32_exp(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp_bits == 0xff) {
    if (frac_bits)
      return x;                  // NaN
    return x > 0.0f ? x : 0.0f;  // +inf -> +inf, -inf -> 0
  }

  // Overflow / underflow cutoffs
  if (x > 88.72283935546875f)
    return 1.0f / 0.0f;
  if (x < -103.97208404541015625f)
    return 0.0f;

  // Range reduction:
  // x = n * ln(2) + r, with r near 0
  r32 z = x * 1.4426950408889634074f;  // log2(e)
  i32 n = (i32)(z >= 0.0f ? z + 0.5f : z - 0.5f);

  r32 r = x;
  r -= (r32)n * 0.693145751953125f;
  r -= (r32)n * 1.428606765330187045e-6f;

  // exp(r) polynomial on a small interval around 0
  r32 p = 1.0f + r;
  r32 t = r * r;

  p += t * 0.5f;

  t *= r;
  p += t * (1.0f / 6.0f);

  t *= r;
  p += t * (1.0f / 24.0f);

  t *= r;
  p += t * (1.0f / 120.0f);

  t *= r;
  p += t * (1.0f / 720.0f);

  // Scale by 2^n
  if (n > 127)
    return 1.0f / 0.0f;
  if (n < -149)
    return 0.0f;

  union {
    r32 f;
    u32 u;
  } scale;

  if (n >= -126) {
    scale.u = (u32)(n + 127) << 23;
    return p * scale.f;
  } else {
    scale.u = 1U << (n + 149);
    return p * scale.f;
  }
}

static r32 real32_abs(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  v.u &= 0x7fffffffU;
  return v.f;
}

static r32 real32_kernel_sin(r32 x) {
  r32 x2 = x * x;

  r32 s = x;
  r32 t = x;

  t *= x2;
  s -= t / 6.0f;

  t *= x2;
  s += t / 120.0f;

  t *= x2;
  s -= t / 5040.0f;

  t *= x2;
  s += t / 362880.0f;

  t *= x2;
  s -= t / 39916800.0f;

  return s;
}

static r32 real32_kernel_cos(r32 x) {
  r32 x2 = x * x;

  r32 c = 1.0f;
  r32 t = 1.0f;

  t *= x2;
  c -= t / 2.0f;

  t *= x2;
  c += t / 24.0f;

  t *= x2;
  c -= t / 720.0f;

  t *= x2;
  c += t / 40320.0f;

  t *= x2;
  c -= t / 3628800.0f;

  return c;
}

r32 real32_atan(r32 x) {
  const r32 REAL32_PI_2 = 1.57079632679489661923f;
  const r32 REAL32_PI_4 = 0.78539816339744830962f;

  if (x != x)
    return x;
  if (x == 0.0f)
    return x;
  if (x < 0.0f)
    return -real32_atan(-x);
  if (real32_isinf(x))
    return REAL32_PI_2;

  r32 y = x;
  r32 offset = 0.0f;

  // Range reduction:
  // atan(x) = pi/4 + atan((x - 1) / (x + 1)) for x near 1
  // atan(x) = pi/2 + atan(-1 / x)            for large x
  if (y > 2.4142135623730950488f) {
    offset = REAL32_PI_2;
    y = -1.0f / y;
  } else if (y > 0.4142135623730950488f) {
    offset = REAL32_PI_4;
    y = (y - 1.0f) / (y + 1.0f);
  }

  r32 y2 = y * y;

  r32 s = y;
  r32 t = y;

  t *= y2;
  s -= t / 3.0f;

  t *= y2;
  s += t / 5.0f;

  t *= y2;
  s -= t / 7.0f;

  t *= y2;
  s += t / 9.0f;

  t *= y2;
  s -= t / 11.0f;

  return offset + s;
}

r32 real32_sin(r32 x) {
  const r32 REAL32_INV_PI_2 = 0.63661977236758134308f;  // 2 / pi
  const r32 REAL32_PI_2_HI = 1.57079625129699707031f;
  const r32 REAL32_PI_2_LO = 7.54978941586159635335e-8f;

  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp_bits == 0xff) {
    if (frac_bits)
      return x;          // NaN
    return 0.0f / 0.0f;  // real64_sin(+/-inf) -> NaN
  }

  // Preserve signed zero
  if (x == 0.0f) return x;

  // Reduce to r in about [-pi/4, pi/4]
  r32 q = x * REAL32_INV_PI_2;
  i32 n = (i32)(q >= 0.0f ? q + 0.5f : q - 0.5f);

  r32 r = x;
  r -= (r32)n * REAL32_PI_2_HI;
  r -= (r32)n * REAL32_PI_2_LO;

  i32 quadrant = n % 4;
  if (quadrant < 0) quadrant += 4;

  switch (quadrant) {
    case 0:
      return real32_kernel_sin(r);
    case 1:
      return real32_kernel_cos(r);
    case 2:
      return -real32_kernel_sin(r);
    default:
      return -real32_kernel_cos(r);
  }
}

r32 real32_cos(r32 x) {
  const r32 REAL32_INV_PI_2 = 0.63661977236758134308f;  // 2 / pi
  const r32 REAL32_PI_2_HI = 1.57079625129699707031f;
  const r32 REAL32_PI_2_LO = 7.54978941586159635335e-8f;

  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp_bits == 0xff) {
    if (frac_bits)
      return x;          // NaN
    return 0.0f / 0.0f;  // real64_cos(+/-inf) -> NaN
  }

  // Reduce to r in about [-pi/4, pi/4]
  r32 q = x * REAL32_INV_PI_2;
  i32 n = (i32)(q >= 0.0f ? q + 0.5f : q - 0.5f);

  r32 r = x;
  r -= (r32)n * REAL32_PI_2_HI;
  r -= (r32)n * REAL32_PI_2_LO;

  i32 quadrant = n % 4;
  if (quadrant < 0)
    quadrant += 4;

  switch (quadrant) {
    case 0:
      return real32_kernel_cos(r);
    case 1:
      return -real32_kernel_sin(r);
    case 2:
      return -real32_kernel_cos(r);
    default:
      return real32_kernel_sin(r);
  }
}

r32 real32_atan2(r32 y, r32 x) {
  const r32 REAL32_PI = 3.14159265358979323846f;
  const r32 REAL32_PI_2 = 1.57079632679489661923f;
  const r32 REAL32_PI_4 = 0.78539816339744830962f;

  if (x != x)
    return x;
  if (y != y)
    return y;

  b8 x_neg = real32_signbit(x);
  b8 y_neg = real32_signbit(y);

  if (real32_isinf(x) && real32_isinf(y)) {
    if (!x_neg && !y_neg)
      return REAL32_PI_4;
    if (x_neg && !y_neg)
      return 3.0f * REAL32_PI_4;
    if (x_neg && y_neg)
      return -3.0f * REAL32_PI_4;
    return -REAL32_PI_4;
  }

  if (real32_isinf(x)) {
    if (x_neg)
      return y_neg ? -REAL32_PI : REAL32_PI;
    return y_neg ? -0.0f : 0.0f;
  }

  if (real32_isinf(y))
    return y_neg ? -REAL32_PI_2 : REAL32_PI_2;

  if (y == 0.0f) {
    if (!x_neg)
      return y;  // preserves signed zero for +x and +/-0
    return y_neg ? -REAL32_PI : REAL32_PI;
  }

  if (x == 0.0f)
    return y_neg ? -REAL32_PI_2 : REAL32_PI_2;

  if (x > 0.0f)
    return real32_atan(y / x);

  if (y < 0.0f)
    return real32_atan(y / x) - REAL32_PI;

  return real32_atan(y / x) + REAL32_PI;
}

r32 real32_asin(r32 x) {
  const r32 REAL32_PI_2 = 1.57079632679489661923f;

  if (x != x)
    return x;

  r32 ax = real32_abs(x);

  if (ax > 1.0f)
    return 0.0f / 0.0f;
  if (ax == 1.0f)
    return real32_signbit(x) ? -REAL32_PI_2 : REAL32_PI_2;
  if (x == 0.0f)
    return x;

  // asin(x) = atan2(x, real64_sqrt((1 - x) * (1 + x)))
  r32 y = real32_sqrt((1.0f - x) * (1.0f + x));
  return real32_atan2(x, y);
}

r32 real32_tan(r32 x) {
  const r32 REAL32_INV_PI_2 = 0.63661977236758134308f;  // 2 / pi
  const r32 REAL32_PI_2_HI = 1.57079625129699707031f;
  const r32 REAL32_PI_2_LO = 7.54978941586159635335e-8f;

  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp_bits == 0xff) {
    if (frac_bits)
      return x;          // NaN
    return 0.0f / 0.0f;  // tan(+/-inf) -> NaN
  }

  // Preserve signed zero
  if (x == 0.0f)
    return x;

  // Reduce to r in about [-pi/4, pi/4]
  r32 q = x * REAL32_INV_PI_2;
  i32 n = (i32)(q >= 0.0f ? q + 0.5f : q - 0.5f);

  r32 r = x;
  r -= (r32)n * REAL32_PI_2_HI;
  r -= (r32)n * REAL32_PI_2_LO;

  i32 quadrant = n % 4;
  if (quadrant < 0)
    quadrant += 4;

  r32 s;
  r32 c;

  switch (quadrant) {
    case 0:
      s = real32_kernel_sin(r);
      c = real32_kernel_cos(r);
      break;
    case 1:
      s = real32_kernel_cos(r);
      c = -real32_kernel_sin(r);
      break;
    case 2:
      s = -real32_kernel_sin(r);
      c = -real32_kernel_cos(r);
      break;
    default:
      s = -real32_kernel_cos(r);
      c = real32_kernel_sin(r);
      break;
  }

  return s / c;
}

r32 real32_tanh(r32 x) {
  union {
    r32 f;
    u32 u;
  } v = {x};

  u32 exp_bits = (v.u >> 23) & 0xff;
  u32 frac_bits = v.u & 0x007fffffU;

  // NaN or infinity
  if (exp_bits == 0xff) {
    if (frac_bits)
      return x;                      // NaN
    return x > 0.0f ? 1.0f : -1.0f;  // +/-inf -> +/-1
  }

  // Preserve signed zero
  if (x == 0.0f)
    return x;

  b8 neg = x < 0.0f;
  r32 ax = neg ? -x : x;

  // tanh(x) is extremely close to 1 for large x
  if (ax > 10.0f)
    return neg ? -1.0f : 1.0f;

  // Small-x polynomial
  if (ax < 0.625f) {
    r32 x2 = x * x;

    r32 r = x;
    r32 t = x;

    t *= x2;
    r -= t / 3.0f;

    t *= x2;
    r += (2.0f * t) / 15.0f;

    t *= x2;
    r -= (17.0f * t) / 315.0f;

    t *= x2;
    r += (62.0f * t) / 2835.0f;

    return r;
  }

  // Rational approximation on the remaining interval
  r32 x2 = ax * ax;
  r32 n = ax * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
  r32 d = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + 28.0f * x2));
  r32 r = n / d;
  return neg ? -r : r;
}
