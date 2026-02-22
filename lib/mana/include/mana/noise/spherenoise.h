#pragma once

#include "mana/noise/noisecommon.h"

#define DEFAULT_SPHERE_RADIUS 8.0
#define DEFAULT_SPHERE_SPHERE_ORIGIN_X 0.0
#define DEFAULT_SPHERE_SPHERE_ORIGIN_Y 0.0
#define DEFAULT_SPHERE_SPHERE_ORIGIN_Z 0.0
#define DEFAULT_SPHERE_POSITION_X 0.0
#define DEFAULT_SPHERE_POSITION_Y 0.0
#define DEFAULT_SPHERE_POSITION_Z 0.0
#define DEFAULT_SPHERE_STEP 1.0 / 256.0
#define DEFAULT_SPHERE_PARALLEL false

struct SphereNoise {
  float radius;
  float position[3];
  float sphere_origin[3];
  float step;
  bool parallel;
  float *(*sphere_func)(struct SphereNoise *, size_t, size_t, size_t);
  enum NoiseQuality noise_quality;
};

static inline float *sphere_noise_eval_1d(struct SphereNoise *sphere_noise, size_t x_size);
static inline float *sphere_noise_eval_2d(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size);
static inline float *sphere_noise_eval_3d(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float sphere_noise_eval_3d_single(struct SphereNoise *sphere_noise, float x_pos, float y_pos, float z_pos);
static inline float *sphere_noise_eval_3d_fallback(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float *sphere_noise_eval_3d_sse2(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float *sphere_noise_eval_3d_sse4_1(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float *sphere_noise_eval_3d_avx(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float *sphere_noise_eval_3d_avx2(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);
static inline float *sphere_noise_eval_3d_avx512(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size);

static inline void sphere_noise_init(struct SphereNoise *sphere_noise) {
  sphere_noise->radius = DEFAULT_SPHERE_RADIUS;
  sphere_noise->position[0] = DEFAULT_SPHERE_POSITION_X;
  sphere_noise->position[1] = DEFAULT_SPHERE_POSITION_Y;
  sphere_noise->position[2] = DEFAULT_SPHERE_POSITION_Z;
  sphere_noise->sphere_origin[0] = DEFAULT_SPHERE_SPHERE_ORIGIN_X;
  sphere_noise->sphere_origin[1] = DEFAULT_SPHERE_SPHERE_ORIGIN_Y;
  sphere_noise->sphere_origin[2] = DEFAULT_SPHERE_SPHERE_ORIGIN_Z;
  sphere_noise->step = DEFAULT_SPHERE_STEP;
  sphere_noise->parallel = DEFAULT_SPHERE_PARALLEL;

  switch (detect_simd_support()) {
#ifdef ARCH_32_64
    /*case NOISE_SIMD_AVX512F:{
      sphere_noise->sphere_func = &sphere_noise_eval_3d_fallback;
      break;
  }
    case NOISE_SIMD_AVX2:{
      sphere_noise->sphere_func = &sphere_noise_eval_3d_avx2;
      break;
  }
    case NOISE_SIMD_AVX:{
      sphere_noise->sphere_func = &sphere_noise_eval_3d_avx;
      break;
  }
    case NOISE_SIMD_SSE4_1:{
      sphere_noise->sphere_func = &sphere_noise_eval_3d_sse4_1;
      break;
  }
    case NOISE_SIMD_SSE2:{
      sphere_noise->sphere_func = &sphere_noise_eval_3d_sse2;
      break;
  }*/
#else
    case SIMD_NEON: {
      sphere_noise->sphere_func = &sphere_noise_eval_3d_fallback;
      break;
    }
#endif
    default: {
      sphere_noise->sphere_func = &sphere_noise_eval_3d_fallback;
      break;
    }
  }
}

static inline float *sphere_noise_eval_1d(struct SphereNoise *sphere_noise, size_t x_size) {
  return sphere_noise->sphere_func(sphere_noise, x_size, 1, 1);
}

static inline float *sphere_noise_eval_2d(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size) {
  return sphere_noise->sphere_func(sphere_noise, x_size, y_size, 1);
}

static inline float *sphere_noise_eval_3d(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size) {
  return sphere_noise->sphere_func(sphere_noise, x_size, y_size, z_size);
}

// static inline float sphere_noise_eval_3d_single(struct SphereNoise *sphere_noise, float x_pos, float y_pos, float z_pos) {
//   float length_x = sphere_noise->position[0] + sphere_noise->sphere_origin[0] - x_pos;
//   float length_y = sphere_noise->position[1] + sphere_noise->sphere_origin[1] - y_pos;
//   float length_z = sphere_noise->position[2] + sphere_noise->sphere_origin[2] - z_pos;
//
//   float magnitude = sqrtf((length_x * length_x) + (length_y * length_y) + (length_z * length_z));
//
//   return (sphere_noise->radius < magnitude) ? -10.0f : 10.0f;
// }

static inline float sphere_noise_eval_3d_single(struct SphereNoise *sphere_noise, float x_pos, float y_pos, float z_pos) {
  float length_x = sphere_noise->position[0] - x_pos;
  float length_y = sphere_noise->position[1] - y_pos;
  float length_z = sphere_noise->position[2] - z_pos;

  float magnitude = sqrtf((length_x * length_x) + (length_y * length_y) + (length_z * length_z));

  // return (sphere_noise->radius < magnitude) ? 1.0f : -1.0f;

  /*float val = sphere_noise->radius - magnitude;
  if (val < -1.0f)
    val = -1.0f;
  else if (val > 1.0f)
    val = 1.0f;
  return val;*/

  return sphere_noise->radius - magnitude;
}

static inline float *sphere_noise_eval_3d_fallback(struct SphereNoise *sphere_noise, size_t x_size, size_t y_size, size_t z_size) {
  float *noise_set = noise_allocate(sizeof(float), sizeof(float) * x_size * y_size * z_size);

  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim++) {
        float length_x = sphere_noise->position[0] + sphere_noise->sphere_origin[0] - x_dim;
        float length_y = sphere_noise->position[1] + sphere_noise->sphere_origin[1] - y_dim;
        float length_z = sphere_noise->position[2] + sphere_noise->sphere_origin[2] - z_dim;

        float magnitude = sqrtf((length_x * length_x) + (length_y * length_y) + (length_z * length_z));

        *(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size)))) = sphere_noise->radius - magnitude;
      }
    }
  }
  return noise_set;
}
