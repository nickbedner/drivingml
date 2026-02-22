#pragma once

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Make a sse 4.2 version of this that works on 4 unique values. It should have the same input and output as the reference implementation, so don't use intrinsics that could change the value AT ALL
// Complete this function

#include "mana/noise/randomvectors.h"

#ifdef __arm64__
#include <arm_neon.h>
#else
#include <immintrin.h>
#include <smmintrin.h>
// Note: To make clangd happy, we need to include the header files for the intrinsics we use
#include <avx2intrin.h>
#include <avx512fintrin.h>
#include <avxintrin.h>
#include <fmaintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>

#define SSE (1 << 25)
#define SSE2 (1 << 26)
#define SSE3 (1 << 0)
#define SSSE3 (1 << 9)
#define SSE4_1 (1 << 19)
#define SSE4_2 (1 << 20)
#define AVX (1 << 28)
#define AVX2 (1 << 30)
#define FMA (1 << 12)  // Note: FMA is an extension of AVX2 and AVX512F has the equivalent
#define AVX512F (1 << 16)
#endif

#define SQRT_3 1.7320508075688772935
#define X_NOISE_GEN 1619
#define Y_NOISE_GEN 31337
#define Z_NOISE_GEN 6971
#define SEED_NOISE_GEN 1013
#define SHIFT_NOISE_GEN 8

enum NoiseQuality {
  QUALITY_FAST,
  QUALITY_STANDARD,
  QUALITY_BEST
};

// SSE2
static inline __m128i sse2_mm_mullo_epi32(__m128i a, __m128i b);
static inline int sse2_mm_extract_epi32(__m128i a, int index);
static inline __m128 make_int_32_range_sse2(__m128 n);
static inline __m128 s_curve3_sse2(__m128 a);
static inline __m128 s_curve5_sse2(__m128 a);
static inline __m128 linear_interp_sse2(__m128 n0, __m128 n1, __m128 a);
static inline __m128i int_value_noise_3d_sse2_full(__m128i x, __m128i y, __m128i z, int seed);
static inline __m128i int_value_noise_3d_sse2(__m128i x, int y, int z, int seed);
static inline __m128 value_noise_3d_sse2_full(__m128i x, __m128i y, __m128i z, int seed);
static inline __m128 value_noise_3d_sse2(__m128i x, int y, int z, int seed);
static inline __m128 gradient_noise_3d_sse2(__m128 fx, float fy, float fz, __m128i ix, int iy, int iz, int seed);
static inline __m128 gradient_coherent_noise_3d_sse2(__m128 x, float y, float z, int seed, enum NoiseQuality noise_quality);
// SSE4_1
static inline __m128i int_value_noise_3d_sse4_1_full(__m128i x, __m128i y, __m128i z, int seed);
static inline __m128i int_value_noise_3d_sse4_1(__m128i x, int y, int z, int seed);
static inline __m128 value_noise_3d_sse4_1_full(__m128i x, __m128i y, __m128i z, int seed);
static inline __m128 value_noise_3d_sse4_1(__m128i x, int y, int z, int seed);
static inline __m128 gradient_noise_3d_sse4_1(__m128 fx, float fy, float fz, __m128i ix, int iy, int iz, int seed);
static inline __m128 gradient_coherent_noise_3d_sse4_1(__m128 x, float y, float z, int seed, enum NoiseQuality noise_quality);
// AVX
static inline __m256 make_int_32_range_avx(__m256 n);
static inline __m256 s_curve3_avx(__m256 a);
static inline __m256 s_curve5_avx(__m256 a);
static inline __m256 linear_interp_avx(__m256 n0, __m256 n1, __m256 a);
static inline __m256i int_value_noise_3d_avx_full(__m256i x, __m256i y, __m256i z, int seed);
static inline __m256i int_value_noise_3d_avx(__m256i x, int y, int z, int seed);
static inline __m256 value_noise_3d_avx_full(__m256i x, __m256i y, __m256i z, int seed);
static inline __m256 value_noise_3d_avx(__m256i x, int y, int z, int seed);
static inline __m256 gradient_noise_3d_avx(__m256 fx, float fy, float fz, __m256i ix, int iy, int iz, int seed);
static inline __m256 gradient_coherent_noise_3d_avx(__m256 x, float y, float z, int seed, enum NoiseQuality noise_quality);
// AVX2
static inline __m256i fast_floor_avx2(__m256 x);
static inline __m256i int_value_noise_3d_avx2_full(__m256i x, __m256i y, __m256i z, int seed);
static inline __m256i int_value_noise_3d_avx2(__m256i x, int y, int z, int seed);
static inline __m256 value_noise_3d_avx2_full(__m256i x, __m256i y, __m256i z, int seed);
static inline __m256 value_noise_3d_avx2(__m256i x, int y, int z, int seed);
static inline __m256 gradient_noise_3d_avx2(__m256 fx, float fy, float fz, __m256i ix, int iy, int iz, int seed);
static inline __m256 gradient_noise_3d_avx2_normals(__m256 fx, __m256 fy, __m256 fz, __m256i ix, __m256i iy, __m256i iz, int seed);
static inline __m256 gradient_coherent_noise_3d_avx2(__m256 x, float y, float z, int seed, enum NoiseQuality noise_quality);
static inline __m256 gradient_coherent_noise_3d_avx2_normals(__m256 x, __m256 y, __m256 z, int seed, enum NoiseQuality noise_quality);
// Fallback
static inline float make_int_32_range(float n);
static inline float cubic_interp(float n0, float n1, float n2, float n3, float a);
static inline float s_curve3(float a);
static inline float s_curve5(float a);
static inline float linear_interp(float n0, float n1, float a);
static inline int int_value_noise_3d(int x, int y, int z, int seed);
static inline float value_noise_3d(int x, int y, int z, int seed);
static inline float gradient_noise_3d(float fx, float fy, float fz, int ix, int iy, int iz, int seed);
static inline float gradient_coherent_noise_3d(float x, float y, float z, int seed, enum NoiseQuality noise_quality);

static inline float noise_get(float *noise_set, int x_size, int y_size, int z_size, int x, int y, int z) {
  return *(noise_set + (x + (y * x_size) + (z * (x_size * y_size))));
}

static inline int detect_simd_support() {
#ifdef __arm64__
#else
  uint32_t eax, ebx, ecx, edx;
  int8_t vendor[13];

  // function 0: vendor string
  __asm__ volatile(
      "cpuid;"
      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
      : "a"(0));

  memcpy(vendor, &ebx, 4);
  memcpy(vendor + 4, &edx, 4);
  memcpy(vendor + 8, &ecx, 4);
  vendor[12] = 0;

  printf("CPU vendor: %s\n", vendor);

  // function 1: feature bits
  __asm__ volatile(
      "cpuid;"
      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
      : "a"(1));

  if (edx & SSE)
    printf("CPU supports SSE\n");
  if (edx & SSE2)
    printf("CPU supports SSE2\n");
  if (ecx & SSE3)
    printf("CPU supports SSE3\n");
  if (ecx & SSSE3)
    printf("CPU supports SSSE3\n");
  if (ecx & SSE4_1)
    printf("CPU supports SSE4.1\n");
  if (ecx & SSE4_2)
    printf("CPU supports SSE4.2\n");
  if (ecx & AVX)
    printf("CPU supports AVX\n");
  if (ecx & FMA)
    printf("CPU supports FMA\n");
  if (edx & AVX2)
    printf("CPU supports AVX2\n");
  if (ecx & AVX512F)
    printf("CPU supports AVX-512F\n");
#endif
  return 0;
}

#ifdef __arm64__
#else
// SSE2 compatible mullo
static inline __m128i sse2_mm_mullo_epi32(__m128i a, __m128i b) {
  __m128i tmp_1 = _mm_mul_epu32(a, b);
  __m128i tmp_2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp_1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp_2, _MM_SHUFFLE(0, 0, 2, 0)));
}

// SSE2 compatible extract32
static inline int sse2_mm_extract_epi32(__m128i a, int index) {
  return *(((int *)&a) + index);
  // Older OSX devices don't support _mm_shuffle_epi32
  // switch (index) {
  //  case 0:{
  //    return _mm_cvtsi128_si32(a);
  // }
  //  case 1:{
  //    return _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0x55));
  // }
  //  case 2:{
  //    return _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0xAA));
  // }
  //  case 3:{
  //    return _mm_cvtsi128_si32(_mm_shuffle_epi32(a, 0xFF));
  // }
  //}
  // return 0;
}

// TODO: Clean this up slow way of doing this
static inline __m128 make_int_32_range_sse2(__m128 n) {
  __m128 new_n;

  for (int loop_num = 0; loop_num < 4; loop_num++) {
    float extracted_num = *(((float *)&n) + loop_num);
    if (extracted_num >= 1073741824.0)
      *(((float *)&new_n) + loop_num) = (2.0 * fmod(extracted_num, 1073741824.0)) - 1073741824.0;
    else if (extracted_num <= -1073741824.0)
      *(((float *)&new_n) + loop_num) = (2.0 * fmod(extracted_num, 1073741824.0)) + 1073741824.0;
    else
      *(((float *)&new_n) + loop_num) = extracted_num;
  }
  return new_n;
}

static inline __m128 s_curve3_sse2(__m128 a) {
  return _mm_mul_ps(a, _mm_mul_ps(a, _mm_sub_ps(_mm_set1_ps(3.0), _mm_mul_ps(_mm_set1_ps(2.0), a))));
}

static inline __m128 s_curve5_sse2(__m128 a) {
  __m128 a3 = _mm_mul_ps(a, _mm_mul_ps(a, a));
  __m128 a4 = _mm_mul_ps(a3, a);
  __m128 a5 = _mm_mul_ps(a4, a);

  return _mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_set1_ps(6.0), a5), _mm_mul_ps(_mm_set1_ps(15.0), a4)), _mm_mul_ps(_mm_set1_ps(10.0), a3));
}

static inline __m128 linear_interp_sse2(__m128 n0, __m128 n1, __m128 a) {
  return _mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0), a), n0), _mm_mul_ps(a, n1));
}

// These 2 need work
static inline __m128i int_value_noise_3d_sse2_full(__m128i x, __m128i y, __m128i z, int seed) {
  __m128i n;
  for (int value_num = 0; value_num < 4; value_num++) {
    int32_t x_extract = *(((int32_t *)&x) + value_num);
    int32_t y_extract = *(((int32_t *)&y) + value_num);
    int32_t z_extract = *(((int32_t *)&z) + value_num);
    int32_t n_val = (X_NOISE_GEN * x_extract + Y_NOISE_GEN * y_extract + Z_NOISE_GEN * z_extract + SEED_NOISE_GEN * seed) & 0x7fffffff;
    n_val = (n_val >> 13) ^ n_val;
    *(((int32_t *)&n) + value_num) = (n_val * (n_val * n_val * 60493 + 19990303) + 1376312589) & 0x7fffffff;
  }
  return n;
}

static inline __m128i int_value_noise_3d_sse2(__m128i x, int y, int z, int seed) {
  __m128i n;
  for (int value_num = 0; value_num < 4; value_num++) {
    int32_t x_extract = *(((int32_t *)&x) + value_num);
    int32_t n_val = (X_NOISE_GEN * x_extract + Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed) & 0x7fffffff;
    n_val = (n_val >> 13) ^ n_val;
    *(((int32_t *)&n) + value_num) = (n_val * (n_val * n_val * 60493 + 19990303) + 1376312589) & 0x7fffffff;
  }
  return n;
}

static inline __m128 value_noise_3d_sse2_full(__m128i x, __m128i y, __m128i z, int seed) {
  return _mm_sub_ps(_mm_set1_ps(1.0), _mm_div_ps(_mm_cvtepi32_ps(int_value_noise_3d_sse2_full(x, y, z, seed)), _mm_set1_ps(1073741824.0)));
}

static inline __m128 value_noise_3d_sse2(__m128i x, int y, int z, int seed) {
  return _mm_sub_ps(_mm_set1_ps(1.0), _mm_div_ps(_mm_cvtepi32_ps(int_value_noise_3d_sse2(x, y, z, seed)), _mm_set1_ps(1073741824.0)));
}

static inline __m128 gradient_noise_3d_sse2(__m128 fx, float fy, float fz, __m128i ix, int iy, int iz, int seed) {
  __m128i vector_index = _mm_and_si128(_mm_add_epi32(sse2_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), ix), _mm_set1_epi32(Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0xffffffff));
  vector_index = _mm_xor_si128(vector_index, _mm_srli_epi32(vector_index, SHIFT_NOISE_GEN));
  vector_index = _mm_and_si128(vector_index, _mm_set1_epi32(0xff));
  vector_index = _mm_slli_epi32(vector_index, 2);

  __m128 xv_gradient = _mm_set_ps(g_random_vectors[sse2_mm_extract_epi32(vector_index, 3)], g_random_vectors[sse2_mm_extract_epi32(vector_index, 2)], g_random_vectors[sse2_mm_extract_epi32(vector_index, 1)], g_random_vectors[sse2_mm_extract_epi32(vector_index, 0)]);
  __m128 yv_gradient = _mm_set_ps(g_random_vectors[sse2_mm_extract_epi32(vector_index, 3) + 1], g_random_vectors[sse2_mm_extract_epi32(vector_index, 2) + 1], g_random_vectors[sse2_mm_extract_epi32(vector_index, 1) + 1], g_random_vectors[sse2_mm_extract_epi32(vector_index, 0) + 1]);
  __m128 zv_gradient = _mm_set_ps(g_random_vectors[sse2_mm_extract_epi32(vector_index, 3) + 2], g_random_vectors[sse2_mm_extract_epi32(vector_index, 2) + 2], g_random_vectors[sse2_mm_extract_epi32(vector_index, 1) + 2], g_random_vectors[sse2_mm_extract_epi32(vector_index, 0) + 2]);

  __m128 xv_point = _mm_sub_ps(fx, _mm_cvtepi32_ps(ix));
  __m128 yv_point = _mm_sub_ps(_mm_set1_ps(fy), _mm_cvtepi32_ps(_mm_set1_epi32(iy)));
  __m128 zv_point = _mm_sub_ps(_mm_set1_ps(fz), _mm_cvtepi32_ps(_mm_set1_epi32(iz)));

  return _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(xv_gradient, xv_point), _mm_mul_ps(yv_gradient, yv_point)), _mm_mul_ps(zv_gradient, zv_point)), _mm_set1_ps(2.12));
}

static inline __m128 gradient_coherent_noise_3d_sse2(__m128 x, float y, float z, int seed, enum NoiseQuality noise_quality) {
  __m128i x0;
  for (int x_num = 0; x_num < 4; x_num++)
    *(((int32_t *)&x0) + x_num) = (*(((float *)&x) + x_num) > 0.0 ? (int)*(((float *)&x) + x_num) : (int)*(((float *)&x) + x_num) - 1);
  //__m128 x0_mask = _mm_cmpgt_ps(x, _mm_setzero_ps());
  //__m128i x0 = _mm_cvtps_epi32(_mm_floor_ps(_mm_or_ps(_mm_and_ps(x, x0_mask), _mm_andnot_ps(x0_mask, _mm_sub_ps(x, _mm_set1_ps(1))))));
  __m128i x1 = _mm_add_epi32(x0, _mm_set1_epi32(1));
  int y0 = (y > 0.0 ? (int)y : (int)y - 1);
  int y1 = y0 + 1;
  int z0 = (z > 0.0 ? (int)z : (int)z - 1);
  int z1 = z0 + 1;

  __m128 xs;
  float ys, zs;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = _mm_sub_ps(x, _mm_cvtepi32_ps(x0));
      ys = (y - (float)y0);
      zs = (z - (float)z0);
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3_sse2(_mm_sub_ps(x, _mm_cvtepi32_ps(x0)));
      ys = s_curve3(y - (float)y0);
      zs = s_curve3(z - (float)z0);
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5_sse2(_mm_sub_ps(x, _mm_cvtepi32_ps(x0)));
      ys = s_curve5(y - (float)y0);
      zs = s_curve5(z - (float)z0);
      break;
    }
  }
  __m128 n0 = gradient_noise_3d_sse2(x, y, z, x0, y0, z0, seed);
  __m128 n1 = gradient_noise_3d_sse2(x, y, z, x1, y0, z0, seed);
  __m128 ix0 = linear_interp_sse2(n0, n1, xs);
  n0 = gradient_noise_3d_sse2(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d_sse2(x, y, z, x1, y1, z0, seed);
  __m128 ix1 = linear_interp_sse2(n0, n1, xs);
  __m128 iy0 = linear_interp_sse2(ix0, ix1, _mm_set1_ps(ys));
  n0 = gradient_noise_3d_sse2(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d_sse2(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp_sse2(n0, n1, xs);
  n0 = gradient_noise_3d_sse2(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d_sse2(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp_sse2(n0, n1, xs);
  __m128 iy1 = linear_interp_sse2(ix0, ix1, _mm_set1_ps(ys));
  return linear_interp_sse2(iy0, iy1, _mm_set1_ps(zs));
}

static inline __m128i int_value_noise_3d_sse4_1_full(__m128i x, __m128i y, __m128i z, int seed) {
  __m128i n = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), x), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Y_NOISE_GEN), y), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Z_NOISE_GEN), z), _mm_set1_epi32(SEED_NOISE_GEN * seed)))), _mm_set1_epi32(0x7fffffff));
  n = _mm_xor_si128(_mm_srli_epi32(n, 13), n);
  return _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n, _mm_add_epi32(_mm_mullo_epi32(n, _mm_mullo_epi32(n, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
}

static inline __m128i int_value_noise_3d_sse4_1(__m128i x, int y, int z, int seed) {
  __m128i n = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), x), _mm_set1_epi32(Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0x7fffffff));
  n = _mm_xor_si128(_mm_srli_epi32(n, 13), n);
  return _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n, _mm_add_epi32(_mm_mullo_epi32(n, _mm_mullo_epi32(n, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
}

static inline __m128 value_noise_3d_sse4_1_full(__m128i x, __m128i y, __m128i z, int seed) {
  return _mm_sub_ps(_mm_set1_ps(1.0), _mm_div_ps(_mm_cvtepi32_ps(int_value_noise_3d_sse4_1_full(x, y, z, seed)), _mm_set1_ps(1073741824.0)));
}

static inline __m128 value_noise_3d_sse4_1(__m128i x, int y, int z, int seed) {
  return _mm_sub_ps(_mm_set1_ps(1.0), _mm_div_ps(_mm_cvtepi32_ps(int_value_noise_3d_sse4_1(x, y, z, seed)), _mm_set1_ps(1073741824.0)));
}

static inline __m128 gradient_noise_3d_sse4_1(__m128 fx, float fy, float fz, __m128i ix, int iy, int iz, int seed) {
  __m128i vector_index = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), ix), _mm_set1_epi32(Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0xffffffff));
  vector_index = _mm_xor_si128(vector_index, _mm_srli_epi32(vector_index, SHIFT_NOISE_GEN));
  vector_index = _mm_and_si128(vector_index, _mm_set1_epi32(0xff));
  vector_index = _mm_slli_epi32(vector_index, 2);

  __m128 xv_gradient = _mm_set_ps(g_random_vectors[_mm_extract_epi32(vector_index, 3)], g_random_vectors[_mm_extract_epi32(vector_index, 2)], g_random_vectors[_mm_extract_epi32(vector_index, 1)], g_random_vectors[_mm_extract_epi32(vector_index, 0)]);
  __m128 yv_gradient = _mm_set_ps(g_random_vectors[_mm_extract_epi32(vector_index, 3) + 1], g_random_vectors[_mm_extract_epi32(vector_index, 2) + 1], g_random_vectors[_mm_extract_epi32(vector_index, 1) + 1], g_random_vectors[_mm_extract_epi32(vector_index, 0) + 1]);
  __m128 zv_gradient = _mm_set_ps(g_random_vectors[_mm_extract_epi32(vector_index, 3) + 2], g_random_vectors[_mm_extract_epi32(vector_index, 2) + 2], g_random_vectors[_mm_extract_epi32(vector_index, 1) + 2], g_random_vectors[_mm_extract_epi32(vector_index, 0) + 2]);

  __m128 xv_point = _mm_sub_ps(fx, _mm_cvtepi32_ps(ix));
  __m128 yv_point = _mm_sub_ps(_mm_set1_ps(fy), _mm_cvtepi32_ps(_mm_set1_epi32(iy)));
  __m128 zv_point = _mm_sub_ps(_mm_set1_ps(fz), _mm_cvtepi32_ps(_mm_set1_epi32(iz)));

  return _mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(xv_gradient, xv_point), _mm_mul_ps(yv_gradient, yv_point)), _mm_mul_ps(zv_gradient, zv_point)), _mm_set1_ps(2.12));
}

static inline __m128 gradient_coherent_noise_3d_sse4_1(__m128 x, float y, float z, int seed, enum NoiseQuality noise_quality) {
  __m128i x0 = _mm_cvtps_epi32(_mm_floor_ps(_mm_blendv_ps(_mm_sub_ps(x, _mm_set1_ps(1.0)), x, _mm_cmpgt_ps(x, _mm_setzero_ps()))));
  __m128i x1 = _mm_add_epi32(x0, _mm_set1_epi32(1));
  int y0 = (y > 0.0 ? (int)y : (int)y - 1);
  int y1 = y0 + 1;
  int z0 = (z > 0.0 ? (int)z : (int)z - 1);
  int z1 = z0 + 1;

  __m128 xs;
  float ys, zs;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = _mm_sub_ps(x, _mm_cvtepi32_ps(x0));
      ys = (y - (float)y0);
      zs = (z - (float)z0);
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3_sse2(_mm_sub_ps(x, _mm_cvtepi32_ps(x0)));
      ys = s_curve3(y - (float)y0);
      zs = s_curve3(z - (float)z0);
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5_sse2(_mm_sub_ps(x, _mm_cvtepi32_ps(x0)));
      ys = s_curve5(y - (float)y0);
      zs = s_curve5(z - (float)z0);
      break;
    }
  }
  __m128 n0 = gradient_noise_3d_sse4_1(x, y, z, x0, y0, z0, seed);
  __m128 n1 = gradient_noise_3d_sse4_1(x, y, z, x1, y0, z0, seed);
  __m128 ix0 = linear_interp_sse2(n0, n1, xs);
  n0 = gradient_noise_3d_sse4_1(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d_sse4_1(x, y, z, x1, y1, z0, seed);
  __m128 ix1 = linear_interp_sse2(n0, n1, xs);
  __m128 iy0 = linear_interp_sse2(ix0, ix1, _mm_set1_ps(ys));
  n0 = gradient_noise_3d_sse4_1(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d_sse4_1(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp_sse2(n0, n1, xs);
  n0 = gradient_noise_3d_sse4_1(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d_sse4_1(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp_sse2(n0, n1, xs);
  __m128 iy1 = linear_interp_sse2(ix0, ix1, _mm_set1_ps(ys));
  return linear_interp_sse2(iy0, iy1, _mm_set1_ps(zs));
}

// TODO: Clean this up slow way of doing this
static inline __m256 make_int_32_range_avx(__m256 n) {
  __m256 new_n;

  for (int loop_num = 0; loop_num < 8; loop_num++) {
    float extracted_num = *(((float *)&n) + loop_num);
    if (extracted_num >= 1073741824.0)
      *(((float *)&new_n) + loop_num) = (2.0 * fmod(extracted_num, 1073741824.0)) - 1073741824.0;
    else if (extracted_num <= -1073741824.0)
      *(((float *)&new_n) + loop_num) = (2.0 * fmod(extracted_num, 1073741824.0)) + 1073741824.0;
    else
      *(((float *)&new_n) + loop_num) = extracted_num;
  }
  return new_n;
}

static inline __m256 s_curve3_avx(__m256 a) {
  return _mm256_mul_ps(a, _mm256_mul_ps(a, _mm256_sub_ps(_mm256_set1_ps(3.0), _mm256_mul_ps(_mm256_set1_ps(2.0), a))));
}

static inline __m256 s_curve5_avx(__m256 a) {
  __m256 a3 = _mm256_mul_ps(a, _mm256_mul_ps(a, a));
  __m256 a4 = _mm256_mul_ps(a3, a);
  __m256 a5 = _mm256_mul_ps(a4, a);

  return _mm256_add_ps(_mm256_sub_ps(_mm256_mul_ps(_mm256_set1_ps(6.0), a5), _mm256_mul_ps(_mm256_set1_ps(15.0), a4)), _mm256_mul_ps(_mm256_set1_ps(10.0), a3));
}

static inline __m256 linear_interp_avx(__m256 n0, __m256 n1, __m256 a) {
  return _mm256_add_ps(_mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), a), n0), _mm256_mul_ps(a, n1));
}

static inline __m256i int_value_noise_3d_avx_full(__m256i x, __m256i y, __m256i z, int seed) {
  __m128i n_low = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(x, 0)), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Y_NOISE_GEN), _mm256_extractf128_si256(y, 0)), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Z_NOISE_GEN), _mm256_extractf128_si256(z, 0)), _mm_set1_epi32(SEED_NOISE_GEN * seed)))), _mm_set1_epi32(0x7fffffff));
  __m128i n_high = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(x, 1)), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Y_NOISE_GEN), _mm256_extractf128_si256(y, 1)), _mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(Z_NOISE_GEN), _mm256_extractf128_si256(z, 1)), _mm_set1_epi32(SEED_NOISE_GEN * seed)))), _mm_set1_epi32(0x7fffffff));
  n_low = _mm_xor_si128(_mm_srli_epi32(n_low, 13), n_low);
  n_high = _mm_xor_si128(_mm_srli_epi32(n_high, 13), n_high);
  n_low = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n_low, _mm_add_epi32(_mm_mullo_epi32(n_low, _mm_mullo_epi32(n_low, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
  n_high = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n_high, _mm_add_epi32(_mm_mullo_epi32(n_high, _mm_mullo_epi32(n_high, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
  return _mm256_set_m128i(n_high, n_low);
}

static inline __m256i int_value_noise_3d_avx(__m256i x, int y, int z, int seed) {
  __m128i n_low = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(x, 0)), _mm_set1_epi32(Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0x7fffffff));
  __m128i n_high = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(x, 1)), _mm_set1_epi32(Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0x7fffffff));
  n_low = _mm_xor_si128(_mm_srli_epi32(n_low, 13), n_low);
  n_high = _mm_xor_si128(_mm_srli_epi32(n_high, 13), n_high);
  n_low = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n_low, _mm_add_epi32(_mm_mullo_epi32(n_low, _mm_mullo_epi32(n_low, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
  n_high = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(n_high, _mm_add_epi32(_mm_mullo_epi32(n_high, _mm_mullo_epi32(n_high, _mm_set1_epi32(60493))), _mm_set1_epi32(19990303))), _mm_set1_epi32(1376312589)), _mm_set1_epi32(0x7fffffff));
  return _mm256_set_m128i(n_high, n_low);
}

static inline __m256 value_noise_3d_avx_full(__m256i x, __m256i y, __m256i z, int seed) {
  return _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_div_ps(_mm256_cvtepi32_ps(int_value_noise_3d_avx_full(x, y, z, seed)), _mm256_set1_ps(1073741824.0)));
}

static inline __m256 value_noise_3d_avx(__m256i x, int y, int z, int seed) {
  return _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_div_ps(_mm256_cvtepi32_ps(int_value_noise_3d_avx(x, y, z, seed)), _mm256_set1_ps(1073741824.0)));
}

static inline __m256 gradient_noise_3d_avx(__m256 fx, float fy, float fz, __m256i ix, int iy, int iz, int seed) {
  __m128i vector_index_low = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(ix, 0)), _mm_set1_epi32(Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0xffffffff));
  vector_index_low = _mm_xor_si128(vector_index_low, _mm_srli_epi32(vector_index_low, SHIFT_NOISE_GEN));
  vector_index_low = _mm_and_si128(vector_index_low, _mm_set1_epi32(0xff));
  vector_index_low = _mm_slli_epi32(vector_index_low, 2);
  __m128i vector_index_high = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32(_mm_set1_epi32(X_NOISE_GEN), _mm256_extractf128_si256(ix, 1)), _mm_set1_epi32(Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed)), _mm_set1_epi32(0xffffffff));
  vector_index_high = _mm_xor_si128(vector_index_high, _mm_srli_epi32(vector_index_high, SHIFT_NOISE_GEN));
  vector_index_high = _mm_and_si128(vector_index_high, _mm_set1_epi32(0xff));
  vector_index_high = _mm_slli_epi32(vector_index_high, 2);

  __m256 xv_gradient = _mm256_set_ps(g_random_vectors[_mm_extract_epi32(vector_index_high, 3)], g_random_vectors[_mm_extract_epi32(vector_index_high, 2)], g_random_vectors[_mm_extract_epi32(vector_index_high, 1)], g_random_vectors[_mm_extract_epi32(vector_index_high, 0)], g_random_vectors[_mm_extract_epi32(vector_index_low, 3)], g_random_vectors[_mm_extract_epi32(vector_index_low, 2)], g_random_vectors[_mm_extract_epi32(vector_index_low, 1)], g_random_vectors[_mm_extract_epi32(vector_index_low, 0)]);
  __m256 yv_gradient = _mm256_set_ps(g_random_vectors[_mm_extract_epi32(vector_index_high, 3) + 1], g_random_vectors[_mm_extract_epi32(vector_index_high, 2) + 1], g_random_vectors[_mm_extract_epi32(vector_index_high, 1) + 1], g_random_vectors[_mm_extract_epi32(vector_index_high, 0) + 1], g_random_vectors[_mm_extract_epi32(vector_index_low, 3) + 1], g_random_vectors[_mm_extract_epi32(vector_index_low, 2) + 1], g_random_vectors[_mm_extract_epi32(vector_index_low, 1) + 1], g_random_vectors[_mm_extract_epi32(vector_index_low, 0) + 1]);
  __m256 zv_gradient = _mm256_set_ps(g_random_vectors[_mm_extract_epi32(vector_index_high, 3) + 2], g_random_vectors[_mm_extract_epi32(vector_index_high, 2) + 2], g_random_vectors[_mm_extract_epi32(vector_index_high, 1) + 2], g_random_vectors[_mm_extract_epi32(vector_index_high, 0) + 2], g_random_vectors[_mm_extract_epi32(vector_index_low, 3) + 2], g_random_vectors[_mm_extract_epi32(vector_index_low, 2) + 2], g_random_vectors[_mm_extract_epi32(vector_index_low, 1) + 2], g_random_vectors[_mm_extract_epi32(vector_index_low, 0) + 2]);

  __m256 xv_point = _mm256_sub_ps(fx, _mm256_cvtepi32_ps(ix));
  __m256 yv_point = _mm256_sub_ps(_mm256_set1_ps(fy), _mm256_cvtepi32_ps(_mm256_set1_epi32(iy)));
  __m256 zv_point = _mm256_sub_ps(_mm256_set1_ps(fz), _mm256_cvtepi32_ps(_mm256_set1_epi32(iz)));

  return _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(xv_gradient, xv_point), _mm256_mul_ps(yv_gradient, yv_point)), _mm256_mul_ps(zv_gradient, zv_point)), _mm256_set1_ps(2.12));
}

static inline __m256 gradient_coherent_noise_3d_avx(__m256 x, float y, float z, int seed, enum NoiseQuality noise_quality) {
  __m256i x0 = _mm256_cvtps_epi32(_mm256_floor_ps(_mm256_blendv_ps(_mm256_sub_ps(x, _mm256_set1_ps(1.0)), x, _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_GT_OQ))));
  __m128i x1_low = _mm256_extractf128_si256(x0, 0);
  x1_low = _mm_add_epi32(x1_low, _mm_set1_epi32(1));
  __m128i x1_high = _mm256_extractf128_si256(x0, 1);
  x1_high = _mm_add_epi32(x1_high, _mm_set1_epi32(1));
  __m256i x1 = _mm256_set_m128i(x1_high, x1_low);
  int y0 = (y > 0.0 ? (int)y : (int)y - 1);
  int y1 = y0 + 1;
  int z0 = (z > 0.0 ? (int)z : (int)z - 1);
  int z1 = z0 + 1;

  __m256 xs;
  float ys, zs;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = _mm256_sub_ps(x, _mm256_cvtepi32_ps(x0));
      ys = (y - (float)y0);
      zs = (z - (float)z0);
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve3(y - (float)y0);
      zs = s_curve3(z - (float)z0);
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve5(y - (float)y0);
      zs = s_curve5(z - (float)z0);
      break;
    }
  }
  __m256 n0 = gradient_noise_3d_avx(x, y, z, x0, y0, z0, seed);
  __m256 n1 = gradient_noise_3d_avx(x, y, z, x1, y0, z0, seed);
  __m256 ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d_avx(x, y, z, x1, y1, z0, seed);
  __m256 ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy0 = linear_interp_avx(ix0, ix1, _mm256_set1_ps(ys));
  n0 = gradient_noise_3d_avx(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d_avx(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d_avx(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy1 = linear_interp_avx(ix0, ix1, _mm256_set1_ps(ys));
  return linear_interp_avx(iy0, iy1, _mm256_set1_ps(zs));
}

static inline __m256i int_value_noise_3d_avx2_full(__m256i x, __m256i y, __m256i z, int seed) {
  __m256i n = _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(X_NOISE_GEN), x), _mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(Y_NOISE_GEN), y), _mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(Z_NOISE_GEN), z), _mm256_set1_epi32(SEED_NOISE_GEN * seed)))), _mm256_set1_epi32(0x7fffffff));
  n = _mm256_xor_si256(_mm256_srli_epi32(n, 13), n);
  return _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(n, _mm256_add_epi32(_mm256_mullo_epi32(n, _mm256_mullo_epi32(n, _mm256_set1_epi32(60493))), _mm256_set1_epi32(19990303))), _mm256_set1_epi32(1376312589)), _mm256_set1_epi32(0x7fffffff));
}

static inline __m256i int_value_noise_3d_avx2(__m256i x, int y, int z, int seed) {
  __m256i n = _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(X_NOISE_GEN), x), _mm256_set1_epi32(Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed)), _mm256_set1_epi32(0x7fffffff));
  n = _mm256_xor_si256(_mm256_srli_epi32(n, 13), n);
  return _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(n, _mm256_add_epi32(_mm256_mullo_epi32(n, _mm256_mullo_epi32(n, _mm256_set1_epi32(60493))), _mm256_set1_epi32(19990303))), _mm256_set1_epi32(1376312589)), _mm256_set1_epi32(0x7fffffff));
}

static inline __m256 value_noise_3d_avx2_full(__m256i x, __m256i y, __m256i z, int seed) {
  return _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_div_ps(_mm256_cvtepi32_ps(int_value_noise_3d_avx2_full(x, y, z, seed)), _mm256_set1_ps(1073741824.0)));
}

static inline __m256 value_noise_3d_avx2(__m256i x, int y, int z, int seed) {
  return _mm256_sub_ps(_mm256_set1_ps(1.0), _mm256_div_ps(_mm256_cvtepi32_ps(int_value_noise_3d_avx2(x, y, z, seed)), _mm256_set1_ps(1073741824.0)));
}

static inline __m256 gradient_noise_3d_avx2(__m256 fx, float fy, float fz, __m256i ix, int iy, int iz, int seed) {
  __m256i vector_index = _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(X_NOISE_GEN), ix), _mm256_set1_epi32(Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed)), _mm256_set1_epi32(0xffffffff));
  vector_index = _mm256_xor_si256(vector_index, _mm256_srli_epi32(vector_index, SHIFT_NOISE_GEN));
  vector_index = _mm256_and_si256(vector_index, _mm256_set1_epi32(0xff));
  vector_index = _mm256_slli_epi32(vector_index, 2);

  __m256 xv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7)], g_random_vectors[_mm256_extract_epi32(vector_index, 6)], g_random_vectors[_mm256_extract_epi32(vector_index, 5)], g_random_vectors[_mm256_extract_epi32(vector_index, 4)], g_random_vectors[_mm256_extract_epi32(vector_index, 3)], g_random_vectors[_mm256_extract_epi32(vector_index, 2)], g_random_vectors[_mm256_extract_epi32(vector_index, 1)], g_random_vectors[_mm256_extract_epi32(vector_index, 0)]);
  __m256 yv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 6) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 5) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 4) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 3) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 2) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 1) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 0) + 1]);
  __m256 zv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 6) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 5) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 4) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 3) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 2) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 1) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 0) + 2]);

  __m256 xv_point = _mm256_sub_ps(fx, _mm256_cvtepi32_ps(ix));
  __m256 yv_point = _mm256_sub_ps(_mm256_set1_ps(fy), _mm256_cvtepi32_ps(_mm256_set1_epi32(iy)));
  __m256 zv_point = _mm256_sub_ps(_mm256_set1_ps(fz), _mm256_cvtepi32_ps(_mm256_set1_epi32(iz)));

  return _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(xv_gradient, xv_point), _mm256_mul_ps(yv_gradient, yv_point)), _mm256_mul_ps(zv_gradient, zv_point)), _mm256_set1_ps(2.12));
}

static inline __m256 gradient_noise_3d_avx2_normals(__m256 fx, __m256 fy, __m256 fz, __m256i ix, __m256i iy, __m256i iz, int seed) {
  __m256i y = _mm256_mullo_epi32(_mm256_set1_epi32(Y_NOISE_GEN), iy);
  __m256i z = _mm256_mullo_epi32(_mm256_set1_epi32(Z_NOISE_GEN), iz);
  __m256i y_z_seed = _mm256_add_epi32(_mm256_add_epi32(y, _mm256_set1_epi32(SEED_NOISE_GEN * seed)), z);
  __m256i vector_index = _mm256_and_si256(_mm256_add_epi32(_mm256_mullo_epi32(_mm256_set1_epi32(X_NOISE_GEN), ix), y_z_seed), _mm256_set1_epi32(0xffffffff));
  vector_index = _mm256_xor_si256(vector_index, _mm256_srli_epi32(vector_index, SHIFT_NOISE_GEN));
  vector_index = _mm256_and_si256(vector_index, _mm256_set1_epi32(0xff));
  vector_index = _mm256_slli_epi32(vector_index, 2);

  __m256 xv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7)], g_random_vectors[_mm256_extract_epi32(vector_index, 6)], g_random_vectors[_mm256_extract_epi32(vector_index, 5)], g_random_vectors[_mm256_extract_epi32(vector_index, 4)], g_random_vectors[_mm256_extract_epi32(vector_index, 3)], g_random_vectors[_mm256_extract_epi32(vector_index, 2)], g_random_vectors[_mm256_extract_epi32(vector_index, 1)], g_random_vectors[_mm256_extract_epi32(vector_index, 0)]);
  __m256 yv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 6) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 5) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 4) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 3) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 2) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 1) + 1], g_random_vectors[_mm256_extract_epi32(vector_index, 0) + 1]);
  __m256 zv_gradient = _mm256_set_ps(g_random_vectors[_mm256_extract_epi32(vector_index, 7) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 6) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 5) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 4) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 3) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 2) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 1) + 2], g_random_vectors[_mm256_extract_epi32(vector_index, 0) + 2]);

  __m256 xv_point = _mm256_sub_ps(fx, _mm256_cvtepi32_ps(ix));
  __m256 yv_point = _mm256_sub_ps(fy, _mm256_cvtepi32_ps(iy));
  __m256 zv_point = _mm256_sub_ps(fz, _mm256_cvtepi32_ps(iz));

  return _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(xv_gradient, xv_point), _mm256_mul_ps(yv_gradient, yv_point)), _mm256_mul_ps(zv_gradient, zv_point)), _mm256_set1_ps(2.12));
}

static inline __m256 gradient_coherent_noise_3d_avx2(__m256 x, float y, float z, int seed, enum NoiseQuality noise_quality) {
  __m256i x0 = _mm256_cvtps_epi32(_mm256_floor_ps(_mm256_blendv_ps(_mm256_sub_ps(x, _mm256_set1_ps(1.0)), x, _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_GT_OQ))));
  __m256i x1 = _mm256_add_epi32(x0, _mm256_set1_epi32(1));
  int y0 = (y > 0.0 ? (int)y : (int)y - 1);
  int y1 = y0 + 1;
  int z0 = (z > 0.0 ? (int)z : (int)z - 1);
  int z1 = z0 + 1;

  __m256 xs;
  float ys, zs;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = _mm256_sub_ps(x, _mm256_cvtepi32_ps(x0));
      ys = (y - (float)y0);
      zs = (z - (float)z0);
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve3(y - (float)y0);
      zs = s_curve3(z - (float)z0);
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve5(y - (float)y0);
      zs = s_curve5(z - (float)z0);
      break;
    }
  }

  __m256 n0 = gradient_noise_3d_avx2(x, y, z, x0, y0, z0, seed);
  __m256 n1 = gradient_noise_3d_avx2(x, y, z, x1, y0, z0, seed);
  __m256 ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx2(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d_avx2(x, y, z, x1, y1, z0, seed);
  __m256 ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy0 = linear_interp_avx(ix0, ix1, _mm256_set1_ps(ys));
  n0 = gradient_noise_3d_avx2(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d_avx2(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx2(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d_avx2(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy1 = linear_interp_avx(ix0, ix1, _mm256_set1_ps(ys));

  return linear_interp_avx(iy0, iy1, _mm256_set1_ps(zs));
}

static inline __m256 gradient_coherent_noise_3d_avx2_normals(__m256 x, __m256 y, __m256 z, int seed, enum NoiseQuality noise_quality) {
  __m256i x0 = _mm256_cvtps_epi32(_mm256_floor_ps(_mm256_blendv_ps(_mm256_sub_ps(x, _mm256_set1_ps(1.0)), x, _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_GT_OQ))));
  __m256i x1 = _mm256_add_epi32(x0, _mm256_set1_epi32(1));
  __m256i y0 = _mm256_cvtps_epi32(_mm256_floor_ps(_mm256_blendv_ps(_mm256_sub_ps(y, _mm256_set1_ps(1.0)), y, _mm256_cmp_ps(y, _mm256_setzero_ps(), _CMP_GT_OQ))));
  __m256i y1 = _mm256_add_epi32(y0, _mm256_set1_epi32(1));
  __m256i z0 = _mm256_cvtps_epi32(_mm256_floor_ps(_mm256_blendv_ps(_mm256_sub_ps(z, _mm256_set1_ps(1.0)), z, _mm256_cmp_ps(z, _mm256_setzero_ps(), _CMP_GT_OQ))));
  __m256i z1 = _mm256_add_epi32(z0, _mm256_set1_epi32(1));

  __m256 xs, ys, zs;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = _mm256_sub_ps(x, _mm256_cvtepi32_ps(x0));
      ys = _mm256_sub_ps(y, _mm256_cvtepi32_ps(y0));
      zs = _mm256_sub_ps(z, _mm256_cvtepi32_ps(z0));
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve3_avx(_mm256_sub_ps(y, _mm256_cvtepi32_ps(y0)));
      zs = s_curve3_avx(_mm256_sub_ps(z, _mm256_cvtepi32_ps(z0)));
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5_avx(_mm256_sub_ps(x, _mm256_cvtepi32_ps(x0)));
      ys = s_curve5_avx(_mm256_sub_ps(y, _mm256_cvtepi32_ps(y0)));
      zs = s_curve5_avx(_mm256_sub_ps(z, _mm256_cvtepi32_ps(z0)));
      break;
    }
  }

  __m256 n0 = gradient_noise_3d_avx2_normals(x, y, z, x0, y0, z0, seed);
  __m256 n1 = gradient_noise_3d_avx2_normals(x, y, z, x1, y0, z0, seed);
  __m256 ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx2_normals(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d_avx2_normals(x, y, z, x1, y1, z0, seed);
  __m256 ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy0 = linear_interp_avx(ix0, ix1, ys);
  n0 = gradient_noise_3d_avx2_normals(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d_avx2_normals(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp_avx(n0, n1, xs);
  n0 = gradient_noise_3d_avx2_normals(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d_avx2_normals(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp_avx(n0, n1, xs);
  __m256 iy1 = linear_interp_avx(ix0, ix1, ys);

  return linear_interp_avx(iy0, iy1, zs);
}
#endif

static inline float make_int_32_range(float n) {
  if (n >= 1073741824.0)
    return (2.0 * fmod(n, 1073741824.0)) - 1073741824.0;
  else if (n <= -1073741824.0)
    return (2.0 * fmod(n, 1073741824.0)) + 1073741824.0;
  else
    return n;
}

static inline float cubic_interp(float n0, float n1, float n2, float n3, float a) {
  float p = (n3 - n2) - (n0 - n1);
  float q = (n0 - n1) - p;
  float r = n2 - n0;
  float s = n1;
  return p * a * a * a + q * a * a + r * a + s;
}

static inline float s_curve3(float a) {
  return (a * a * (3.0 - 2.0 * a));
}

static inline float s_curve5(float a) {
  float a3 = a * a * a;
  float a4 = a3 * a;
  float a5 = a4 * a;
  return (6.0 * a5) - (15.0 * a4) + (10.0 * a3);
}

static inline float linear_interp(float n0, float n1, float a) {
  return ((1.0 - a) * n0) + (a * n1);
}

static inline int int_value_noise_3d(int x, int y, int z, int seed) {
  int n = (X_NOISE_GEN * x + Y_NOISE_GEN * y + Z_NOISE_GEN * z + SEED_NOISE_GEN * seed) & 0x7fffffff;
  n = (n >> 13) ^ n;
  return (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
}

static inline float value_noise_3d(int x, int y, int z, int seed) {
  return 1.0 - ((float)int_value_noise_3d(x, y, z, seed) / 1073741824.0);
}

static inline float gradient_noise_3d(float fx, float fy, float fz, int ix, int iy, int iz, int seed) {
  // NOTE: A memory lookup seems to be faster than the following on my benchmarks
  /*int random_x = (*(int *)&fx) ^ (*(int *)&fx >> 16);
  int random_y = (*(int *)&fy) ^ (*(int *)&fy >> 16);
  int random_z = (*(int *)&fz) ^ (*(int *)&fz >> 16);

  random_x = seed ^ (X_NOISE_GEN * random_x);
  random_y = seed ^ (Y_NOISE_GEN * random_y);
  random_z = seed ^ (Z_NOISE_GEN * random_z);

  float xv_gradient = (random_x * random_x * random_x * 60493) / 2147483648.0;
  float yv_gradient = (random_y * random_y * random_y * 60493) / 2147483648.0;
  float zv_gradient = (random_z * random_z * random_z * 60493) / 2147483648.0;

  float xv_point = (fx - (float)ix);
  float yv_point = (fy - (float)iy);
  float zv_point = (fz - (float)iz);

  return ((xv_gradient * xv_point) + (yv_gradient * yv_point) + (zv_gradient * zv_point)) * 2.12;*/

  int vector_index = (X_NOISE_GEN * ix + Y_NOISE_GEN * iy + Z_NOISE_GEN * iz + SEED_NOISE_GEN * seed) & 0xffffffff;
  vector_index ^= (vector_index >> SHIFT_NOISE_GEN);
  vector_index &= 0xff;
  vector_index <<= 2;

  float xv_gradient = g_random_vectors[vector_index];
  float yv_gradient = g_random_vectors[vector_index + 1];
  float zv_gradient = g_random_vectors[vector_index + 2];

  float xv_point = (fx - (float)ix);
  float yv_point = (fy - (float)iy);
  float zv_point = (fz - (float)iz);

  return ((xv_gradient * xv_point) + (yv_gradient * yv_point) + (zv_gradient * zv_point)) * 2.12;
}

static inline float gradient_coherent_noise_3d(float x, float y, float z, int seed, enum NoiseQuality noise_quality) {
  int x0 = (x > 0.0 ? (int)x : (int)x - 1);
  int x1 = x0 + 1;
  int y0 = (y > 0.0 ? (int)y : (int)y - 1);
  int y1 = y0 + 1;
  int z0 = (z > 0.0 ? (int)z : (int)z - 1);
  int z1 = z0 + 1;

  // TODOO: Likely not worth having this jump here for performance reasons, benchmark then decide for use cases
  float xs = 0.0f, ys = 0.0f, zs = 0.0f;
  switch (noise_quality) {
    case QUALITY_FAST: {
      xs = (x - (float)x0);
      ys = (y - (float)y0);
      zs = (z - (float)z0);
      break;
    }
    case QUALITY_STANDARD: {
      xs = s_curve3(x - (float)x0);
      ys = s_curve3(y - (float)y0);
      zs = s_curve3(z - (float)z0);
      break;
    }
    case QUALITY_BEST: {
      xs = s_curve5(x - (float)x0);
      ys = s_curve5(y - (float)y0);
      zs = s_curve5(z - (float)z0);
      break;
    }
  }

  float n0 = gradient_noise_3d(x, y, z, x0, y0, z0, seed);
  float n1 = gradient_noise_3d(x, y, z, x1, y0, z0, seed);
  float ix0 = linear_interp(n0, n1, xs);
  n0 = gradient_noise_3d(x, y, z, x0, y1, z0, seed);
  n1 = gradient_noise_3d(x, y, z, x1, y1, z0, seed);
  float ix1 = linear_interp(n0, n1, xs);
  float iy0 = linear_interp(ix0, ix1, ys);
  n0 = gradient_noise_3d(x, y, z, x0, y0, z1, seed);
  n1 = gradient_noise_3d(x, y, z, x1, y0, z1, seed);
  ix0 = linear_interp(n0, n1, xs);
  n0 = gradient_noise_3d(x, y, z, x0, y1, z1, seed);
  n1 = gradient_noise_3d(x, y, z, x1, y1, z1, seed);
  ix1 = linear_interp(n0, n1, xs);
  float iy1 = linear_interp(ix0, ix1, ys);

  return linear_interp(iy0, iy1, zs);
}
