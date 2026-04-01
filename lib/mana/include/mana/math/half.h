#pragma once

#include "mana/core/corecommon.h"

typedef u16 half;

global inline u32 as_uint(const r32 x);
global inline r32 as_float(const u32 x);
global inline r32 half_to_float(const half x);
global inline half float_to_half(const r32 x);

// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion/60047308#60047308
// Half is great for working within a range of -1.0f to 1.0f
// https://stackoverflow.com/questions/872544/what-range-of-numbers-can-be-represented-in-a-16-32-and-64-bit-ieee-754-syste
global inline u32 as_uint(const r32 x) {
  return *(const u32*)&x;
}
global inline r32 as_float(const u32 x) {
  return *(const r32*)&x;
}

global inline r32 half_to_float(const half x) {
  const u32 e = (x & 0x7C00) >> 10;
  const u32 m = ((u32)x & 0x03FF) << 13;

  // const u16 e = (x & 0x7C00) >> 10;
  // const u16 m = (x & 0x03FF) << 13;

  const u32 v = as_uint((r32)m) >> 23;
  return as_float(((u32)x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000)));
}
global inline half float_to_half(const r32 x) {
  const u32 b = as_uint(x) + 0x00001000;
  const u32 e = (b & 0x7F800000) >> 23;
  const u32 m = b & 0x007FFFFF;

  // Extract exponent and mantissa using bit manipulation operations
  // const u16 exponent = (b >> 16) & 0x7F;
  // const u16 mantissa = b & 0x3FF;

  return (half)((b & 0x80000000) >> 16 | (e > 112) * ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF);
}
