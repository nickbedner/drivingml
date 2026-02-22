#pragma once

#include "mana/core/corecommon.h"

typedef uint16_t half;

static inline uint32_t as_uint(const float x);
static inline float as_float(const uint32_t x);
static inline float half_to_float(const half x);
static inline half float_to_half(const float x);

// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion/60047308#60047308
// Half is great for working within a range of -1.0f to 1.0f
// https://stackoverflow.com/questions/872544/what-range-of-numbers-can-be-represented-in-a-16-32-and-64-bit-ieee-754-syste
static inline uint32_t as_uint(const float x) {
  return *(const uint32_t *)&x;
}
static inline float as_float(const uint32_t x) {
  return *(const float *)&x;
}

static inline float half_to_float(const half x) {
  const uint32_t e = (x & 0x7C00) >> 10;
  const uint32_t m = ((uint32_t)x & 0x03FF) << 13;

  // const uint16_t e = (x & 0x7C00) >> 10;
  // const uint16_t m = (x & 0x03FF) << 13;

  const uint32_t v = as_uint((float)m) >> 23;
  return as_float(((uint32_t)x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000)));
}
static inline half float_to_half(const float x) {
  const uint32_t b = as_uint(x) + 0x00001000;
  const uint32_t e = (b & 0x7F800000) >> 23;
  const uint32_t m = b & 0x007FFFFF;

  // Extract exponent and mantissa using bit manipulation operations
  // const uint16_t exponent = (b >> 16) & 0x7F;
  // const uint16_t mantissa = b & 0x3FF;

  return (half)((b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF);
}
