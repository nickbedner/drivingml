#pragma once

#include "mana/core/corecommon.h"

typedef u16 half;

u32 as_uint(const r32 x);
r32 as_float(const u32 x);
r32 half_to_float(const half x);
half float_to_half(const r32 x);

// https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion/60047308#60047308
// Half is great for working within a range of -1.0f to 1.0f
// https://stackoverflow.com/questions/872544/what-range-of-numbers-can-be-represented-in-a-16-32-and-64-bit-ieee-754-syste
