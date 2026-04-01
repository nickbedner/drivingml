#include "mana/core/math/real64.h"

b8 real64_isnan(r64 x) {
  return x != x;
}

b8 real64_isinf(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  return exp_bits == 0x7ff && frac_bits == 0;
}

r64 real64_sqrt(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp = v.u & 0x7ff0000000000000ULL;
  u64 frac = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp == 0x7ff0000000000000ULL) {
    if (frac)
      return x;                      // NaN
    return x > 0.0 ? x : 0.0 / 0.0;  // +inf -> +inf, -inf -> NaN
  }

  // Preserve signed zero
  if (x == 0.0) return x;

  // Negative finite -> NaN
  if (x < 0.0) return 0.0 / 0.0;

  // Handle subnormals by scaling
  if (exp == 0) {
    r64 y = real64_sqrt(x * 4503599627370496.0);  // 2^52
    return y * 1.490116119384765625e-8;           // 2^-26
  }

  // Bit-level initial guess
  union {
    r64 f;
    u64 u;
  } y;

  y.u = (v.u >> 1) + 0x1ff8000000000000ULL;

  r64 r = y.f;

  // Newton-Raphson refinement
  r = 0.5 * (r + x / r);
  r = 0.5 * (r + x / r);
  r = 0.5 * (r + x / r);
  r = 0.5 * (r + x / r);
  r = 0.5 * (r + x / r);
  r = 0.5 * (r + x / r);

  return r;
}

r64 real64_fmod(r64 x, r64 y) {
  union {
    r64 f;
    u64 u;
  } ux = {x}, uy = {y};

  u64 sx = ux.u & 0x8000000000000000ULL;
  u64 hx = ux.u & 0x7fffffffffffffffULL;
  u64 hy = uy.u & 0x7fffffffffffffffULL;

  u64 x_exp = (ux.u >> 52) & 0x7ff;
  u64 x_frac = ux.u & 0x000fffffffffffffULL;
  u64 y_exp = (uy.u >> 52) & 0x7ff;
  u64 y_frac = uy.u & 0x000fffffffffffffULL;

  // NaN propagation
  if (x_exp == 0x7ff && x_frac)
    return x;
  if (y_exp == 0x7ff && y_frac)
    return y;

  // fmod(x, 0) = NaN
  if (hy == 0)
    return 0.0 / 0.0;

  // fmod(inf, y) = NaN
  if (x_exp == 0x7ff)
    return 0.0 / 0.0;

  // fmod(x, inf) = x
  if (y_exp == 0x7ff)
    return x;

  // fmod(+-0, y) = +-0
  if (hx == 0)
    return x;

  // Compare magnitudes by absolute-value bit patterns
  if (hx < hy)
    return x;
  if (hx == hy) {
    ux.u = sx;
    return ux.f;
  }

  // Shift-subtract remainder on normalized significands
  i32 ex = (i32)x_exp - 1023;
  i32 ey = (i32)y_exp - 1023;

  u64 mx;
  u64 my;

  if (x_exp == 0) {
    // normalize subnormal x
    mx = x_frac;
    ex = -1022;
    while ((mx & (1ULL << 52)) == 0) {
      mx <<= 1;
      ex -= 1;
    }
  } else
    mx = x_frac | (1ULL << 52);

  if (y_exp == 0) {
    // normalize subnormal y
    my = y_frac;
    ey = -1022;
    while ((my & (1ULL << 52)) == 0) {
      my <<= 1;
      ey -= 1;
    }
  } else
    my = y_frac | (1ULL << 52);

  while (ex > ey) {
    u64 t = my << (ex - ey);
    if (t <= mx && (t >> (ex - ey)) == my)
      mx -= t;
    else
      break;
  }

  while (ex >= ey) {
    if (mx >= my) mx -= my;
    if (mx == 0) {
      ux.u = sx;
      return ux.f;
    }
    mx <<= 1;
    ex -= 1;
  }

  while ((mx & (1ULL << 52)) == 0) {
    mx <<= 1;
    ex -= 1;
  }

  // Repack result
  if (ex < -1022) {
    mx >>= (-1022 - ex);
    ux.u = sx | (mx & 0x000fffffffffffffULL);
  } else
    ux.u = sx | ((u64)(ex + 1023) << 52) | (mx & 0x000fffffffffffffULL);

  return ux.f;
}

r64 real64_log2(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;                      // NaN
    return x > 0.0 ? x : 0.0 / 0.0;  // +inf -> +inf, -inf -> NaN
  }

  // Zero -> -inf
  if (x == 0.0)
    return -1.0 / 0.0;

  // Negative finite -> NaN
  if (x < 0.0)
    return 0.0 / 0.0;

  i32 exp;

  // Handle subnormals by scaling
  if (exp_bits == 0) {
    x *= 4503599627370496.0;  // 2^52
    v.f = x;
    exp_bits = (v.u >> 52) & 0x7ff;
    frac_bits = v.u & 0x000fffffffffffffULL;
    exp = (i32)exp_bits - 1023 - 52;
  } else
    exp = (i32)exp_bits - 1023;

  // Normalize mantissa to [1, 2)
  r64 m = 1.0 + (r64)frac_bits * (1.0 / 4503599627370496.0);  // 2^-52

  // Range reduction to keep the series centered near 1
  if (m >= 1.4142135623730951) {
    m *= 0.5;
    exp += 1;
  }

  // log2(m) = ln(m) / ln(2)
  // ln(m) = 2 * (y + y^3/3 + y^5/5 + ...)
  // where y = (m - 1) / (m + 1)
  r64 y = (m - 1.0) / (m + 1.0);
  r64 y2 = y * y;

  r64 s = y;
  r64 t = y;

  t *= y2;
  s += t / 3.0;

  t *= y2;
  s += t / 5.0;

  t *= y2;
  s += t / 7.0;

  t *= y2;
  s += t / 9.0;

  t *= y2;
  s += t / 11.0;

  t *= y2;
  s += t / 13.0;

  r64 log2_m = (2.0 * s) * 1.4426950408889634074;  // 1 / ln(2)

  return (r64)exp + log2_m;
}

r64 real64_floor(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  i32 exp = (i32)exp_bits - 1023;

  // NaN or infinity
  if (exp_bits == 0x7ff)
    return x;

  // |x| < 1
  if (exp < 0) {
    if (x == 0.0)
      return x;  // preserve signed zero
    return x < 0.0 ? -1.0 : 0.0;
  }

  // Already integral
  if (exp >= 52)
    return x;

  u64 mask = (1ULL << (52 - exp)) - 1;

  // Already integral
  if ((v.u & mask) == 0)
    return x;

  // Clear fractional bits
  v.u &= ~mask;
  r64 r = v.f;

  // Negative values round down one more step
  if (x < 0.0)
    r -= 1.0;

  return r;
}

r64 real64_fabs(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  v.u &= 0x7fffffffffffffffULL;
  return v.f;
}

r64 real64_fmax(r64 x, r64 y) {
  if (x != x)
    return y;  // x is NaN
  if (y != y)
    return x;  // y is NaN

  if (x > y)
    return x;
  if (y > x)
    return y;

  // x and y compare equal here. Prefer +0 over -0.
  union {
    r64 f;
    u64 u;
  } vx = {x};

  return (vx.u >> 63) ? y : x;
}

r64 real64_exp(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;                // NaN
    return x > 0.0 ? x : 0.0;  // +inf -> +inf, -inf -> 0
  }

  // Overflow / underflow cutoffs
  if (x > 709.782712893384)
    return 1.0 / 0.0;
  if (x < -745.1332191019411)
    return 0.0;

  // Range reduction:
  // x = n * ln(2) + r, with r near 0
  r64 z = x * 1.4426950408889634074;  // log2(e)
  i32 n = (i32)(z >= 0.0 ? z + 0.5 : z - 0.5);

  r64 r = x;
  r -= (r64)n * 0.6931471803691238;
  r -= (r64)n * 1.9082149292705877e-10;

  // exp(r) polynomial on a small interval around 0
  r64 p = 1.0 + r;
  r64 t = r * r;

  p += t * 0.5;

  t *= r;
  p += t * (1.0 / 6.0);

  t *= r;
  p += t * (1.0 / 24.0);

  t *= r;
  p += t * (1.0 / 120.0);

  t *= r;
  p += t * (1.0 / 720.0);

  t *= r;
  p += t * (1.0 / 5040.0);

  t *= r;
  p += t * (1.0 / 40320.0);

  // Scale by 2^n
  if (n > 1023)
    return 1.0 / 0.0;
  if (n < -1074)
    return 0.0;

  union {
    r64 f;
    u64 u;
  } scale;

  if (n >= -1022) {
    scale.u = (u64)(n + 1023) << 52;
    return p * scale.f;
  } else {
    scale.u = 1ULL << (n + 1074);
    return p * scale.f;
  }
}

static b8 real64_signbit(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  return (b8)(v.u >> 63);
}

static r64 real64_abs(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  v.u &= 0x7fffffffffffffffULL;
  return v.f;
}

static r64 real64_kernel_sin(r64 x) {
  r64 x2 = x * x;

  r64 s = x;
  r64 t = x;

  t *= x2;
  s -= t / 6.0;

  t *= x2;
  s += t / 120.0;

  t *= x2;
  s -= t / 5040.0;

  t *= x2;
  s += t / 362880.0;

  t *= x2;
  s -= t / 39916800.0;

  t *= x2;
  s += t / 6227020800.0;

  return s;
}

static r64 real64_kernel_cos(r64 x) {
  r64 x2 = x * x;

  r64 c = 1.0;
  r64 t = 1.0;

  t *= x2;
  c -= t / 2.0;

  t *= x2;
  c += t / 24.0;

  t *= x2;
  c -= t / 720.0;

  t *= x2;
  c += t / 40320.0;

  t *= x2;
  c -= t / 3628800.0;

  t *= x2;
  c += t / 479001600.0;

  return c;
}

r64 real64_atan(r64 x) {
  const r64 REAL64_PI_2 = 1.57079632679489661923;
  const r64 REAL64_PI_4 = 0.78539816339744830962;

  if (real64_isnan(x))
    return x;
  if (x == 0.0)
    return x;
  if (x < 0.0)
    return -real64_atan(-x);
  if (real64_isinf(x))
    return REAL64_PI_2;

  r64 y = x;
  r64 offset = 0.0;

  // Range reduction:
  // atan(x) = pi/4 + atan((x - 1) / (x + 1)) for x near 1
  // atan(x) = pi/2 + atan(-1 / x)          for large x
  if (y > 2.4142135623730950488) {
    offset = REAL64_PI_2;
    y = -1.0 / y;
  } else if (y > 0.4142135623730950488) {
    offset = REAL64_PI_4;
    y = (y - 1.0) / (y + 1.0);
  }

  r64 y2 = y * y;

  r64 s = y;
  r64 t = y;

  t *= y2;
  s -= t / 3.0;

  t *= y2;
  s += t / 5.0;

  t *= y2;
  s -= t / 7.0;

  t *= y2;
  s += t / 9.0;

  t *= y2;
  s -= t / 11.0;

  t *= y2;
  s += t / 13.0;

  return offset + s;
}

r64 real64_sin(r64 x) {
  const r64 REAL64_INV_PI_2 = 0.63661977236758134308;  // 2 / pi
  const r64 REAL64_PI_2_HI = 1.57079632679489655800;
  const r64 REAL64_PI_2_LO = 6.12323399573676603587e-17;

  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;        // NaN
    return 0.0 / 0.0;  // real64_sin(+/-inf) -> NaN
  }

  // Preserve signed zero
  if (x == 0.0)
    return x;

  // Reduce to r in about [-pi/4, pi/4]
  r64 q = x * REAL64_INV_PI_2;
  r64 qn = q >= 0.0 ? real64_floor(q + 0.5) : -real64_floor(-q + 0.5);

  r64 r = x;
  r -= qn * REAL64_PI_2_HI;
  r -= qn * REAL64_PI_2_LO;

  i32 quadrant = (i32)(qn - 4.0 * real64_floor(qn * 0.25));
  if (quadrant < 0) quadrant += 4;

  switch (quadrant) {
    case 0:
      return real64_kernel_sin(r);
    case 1:
      return real64_kernel_cos(r);
    case 2:
      return -real64_kernel_sin(r);
    default:
      return -real64_kernel_cos(r);
  }
}

r64 real64_cos(r64 x) {
  const r64 REAL64_INV_PI_2 = 0.63661977236758134308;  // 2 / pi
  const r64 REAL64_PI_2_HI = 1.57079632679489655800;
  const r64 REAL64_PI_2_LO = 6.12323399573676603587e-17;

  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;        // NaN
    return 0.0 / 0.0;  // real64_cos(+/-inf) -> NaN
  }

  // Reduce to r in about [-pi/4, pi/4]
  r64 q = x * REAL64_INV_PI_2;
  r64 qn = q >= 0.0 ? real64_floor(q + 0.5) : -real64_floor(-q + 0.5);

  r64 r = x;
  r -= qn * REAL64_PI_2_HI;
  r -= qn * REAL64_PI_2_LO;

  i32 quadrant = (i32)(qn - 4.0 * real64_floor(qn * 0.25));
  if (quadrant < 0) quadrant += 4;

  switch (quadrant) {
    case 0:
      return real64_kernel_cos(r);
    case 1:
      return -real64_kernel_sin(r);
    case 2:
      return -real64_kernel_cos(r);
    default:
      return real64_kernel_sin(r);
  }
}

r64 real64_atan2(r64 y, r64 x) {
  const r64 REAL64_PI = 3.14159265358979323846;
  const r64 REAL64_PI_2 = 1.57079632679489661923;
  const r64 REAL64_PI_4 = 0.78539816339744830962;

  if (real64_isnan(x))
    return x;
  if (real64_isnan(y))
    return y;

  b8 x_neg = real64_signbit(x);
  b8 y_neg = real64_signbit(y);

  if (real64_isinf(x) && real64_isinf(y)) {
    if (!x_neg && !y_neg)
      return REAL64_PI_4;
    if (x_neg && !y_neg)
      return 3.0 * REAL64_PI_4;
    if (x_neg && y_neg)
      return -3.0 * REAL64_PI_4;
    return -REAL64_PI_4;
  }

  if (real64_isinf(x)) {
    if (x_neg)
      return y_neg ? -REAL64_PI : REAL64_PI;
    return y_neg ? -0.0 : 0.0;
  }

  if (real64_isinf(y)) {
    return y_neg ? -REAL64_PI_2 : REAL64_PI_2;
  }

  if (y == 0.0) {
    if (!x_neg)
      return y;  // preserves signed zero for +x and +0
    return y_neg ? -REAL64_PI : REAL64_PI;
  }

  if (x == 0.0) {
    return y_neg ? -REAL64_PI_2 : REAL64_PI_2;
  }

  if (x > 0.0) {
    return real64_atan(y / x);
  }

  if (y < 0.0) {
    return real64_atan(y / x) - REAL64_PI;
  }

  return real64_atan(y / x) + REAL64_PI;
}

r64 real64_asin(r64 x) {
  const r64 REAL64_PI_2 = 1.57079632679489661923;

  if (real64_isnan(x))
    return x;

  r64 ax = real64_abs(x);

  if (ax > 1.0)
    return 0.0 / 0.0;
  if (ax == 1.0)
    return real64_signbit(x) ? -REAL64_PI_2 : REAL64_PI_2;
  if (x == 0.0)
    return x;

  // asin(x) = atan2(x, real64_sqrt((1 - x) * (1 + x)))
  r64 y = real64_sqrt((1.0 - x) * (1.0 + x));
  return real64_atan2(x, y);
}

r64 real64_tan(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;        // NaN
    return 0.0 / 0.0;  // tan(+/-inf) -> NaN
  }

  // Preserve signed zero
  if (x == 0.0)
    return x;

  r64 s = real64_sin(x);
  r64 c = real64_cos(x);

  return s / c;
}

r64 real64_tanh(r64 x) {
  union {
    r64 f;
    u64 u;
  } v = {x};

  u64 exp_bits = (v.u >> 52) & 0x7ff;
  u64 frac_bits = v.u & 0x000fffffffffffffULL;

  // NaN or infinity
  if (exp_bits == 0x7ff) {
    if (frac_bits)
      return x;                   // NaN
    return x > 0.0 ? 1.0 : -1.0;  // +/-inf -> +/-1
  }

  // Preserve signed zero
  if (x == 0.0)
    return x;

  b8 neg = x < 0.0;
  r64 ax = neg ? -x : x;

  // tanh(x) is extremely close to 1 for large x
  if (ax > 20.0)
    return neg ? -1.0 : 1.0;

  // Small-x polynomial
  if (ax < 0.625) {
    r64 x2 = x * x;

    r64 r = x;
    r64 t = x;

    t *= x2;
    r -= t / 3.0;

    t *= x2;
    r += (2.0 * t) / 15.0;

    t *= x2;
    r -= (17.0 * t) / 315.0;

    t *= x2;
    r += (62.0 * t) / 2835.0;

    t *= x2;
    r -= (1382.0 * t) / 155925.0;

    return r;
  }

  // Rational approximation on the remaining interval
  r64 x2 = ax * ax;
  r64 n = ax * (135135.0 + x2 * (17325.0 + x2 * (378.0 + x2)));
  r64 d = 135135.0 + x2 * (62370.0 + x2 * (3150.0 + 28.0 * x2));
  r64 r = n / d;
  return neg ? -r : r;
}
