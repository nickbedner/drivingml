#pragma once

#include "mana/noise/noisecommon.h"

#define DEFAULT_PERLIN_FREQUENCY 1.0
#define DEFAULT_PERLIN_LACUNARITY 2.0
#define DEFAULT_PERLIN_PERSISTENCE 0.5
#define DEFAULT_PERLIN_OCTAVE_COUNT 1
#define DEFAULT_PERLIN_SEED 0
#define DEFAULT_PERLIN_POSITION_X 0.0
#define DEFAULT_PERLIN_POSITION_Y 0.0
#define DEFAULT_PERLIN_POSITION_Z 0.0
#define DEFAULT_PERLIN_STEP 1.0 / 256.0
#define DEFAULT_PERLIN_PARALLEL FALSE
#define DEFAULT_PERLIN_QUALITY QUALITY_STANDARD

struct PerlinNoise {
  r32 frequency;
  r32 lacunarity;
  r32 persistence;
  unsigned char octave_count;
  int seed;
  r32 position[3];
  r32 step;
  b8 parallel;
  r32* (*perlin_func)(struct PerlinNoise*, size_t, size_t, size_t);
  enum NoiseQuality noise_quality;
};

global inline r32 perlin_noise_eval(struct PerlinNoise* perlin_noise, r32 x_pos, r32 y_pos, r32 z_pos);
global inline r32* perlin_noise_eval_3d_fallback(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);
global inline r32* perlin_noise_eval_3d_sse2(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);
global inline r32* perlin_noise_eval_3d_sse4_1(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);
global inline r32* perlin_noise_eval_3d_avx(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);
global inline r32* perlin_noise_eval_3d_avx2(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);
global inline r32* perlin_noise_eval_3d_avx512(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size);

global inline void perlin_noise_init(struct PerlinNoise* perlin_noise) {
  perlin_noise->frequency = DEFAULT_PERLIN_FREQUENCY;
  perlin_noise->lacunarity = DEFAULT_PERLIN_LACUNARITY;
  perlin_noise->persistence = DEFAULT_PERLIN_PERSISTENCE;
  perlin_noise->octave_count = DEFAULT_PERLIN_OCTAVE_COUNT;
  perlin_noise->seed = DEFAULT_PERLIN_SEED;
  perlin_noise->noise_quality = DEFAULT_PERLIN_QUALITY;
  perlin_noise->position[0] = DEFAULT_PERLIN_POSITION_X;
  perlin_noise->position[1] = DEFAULT_PERLIN_POSITION_Y;
  perlin_noise->position[2] = DEFAULT_PERLIN_POSITION_Z;
  perlin_noise->step = DEFAULT_PERLIN_STEP;
  perlin_noise->parallel = DEFAULT_PERLIN_PARALLEL;
}

global inline r32 perlin_noise_eval(struct PerlinNoise* perlin_noise, r32 x_pos, r32 y_pos, r32 z_pos) {
  r32 x = perlin_noise->position[0] * perlin_noise->frequency;
  r32 y = perlin_noise->position[1] * perlin_noise->frequency;
  r32 z = perlin_noise->position[2] * perlin_noise->frequency;

  r32 value = 0.0;
  r32 cur_persistence = 1.0;

  for (u8 cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
    int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
    r32 signal = gradient_coherent_noise_3d(x, y, z, cur_seed);
    value += signal * cur_persistence;

    x *= perlin_noise->lacunarity;
    y *= perlin_noise->lacunarity;
    z *= perlin_noise->lacunarity;

    cur_persistence *= perlin_noise->persistence;
  }

  return value;
}

global inline r32* perlin_noise_eval_3d_fallback(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size) {
#ifdef CUSTOM_ALLOCATOR
  r32* noise_set = malloc(sizeof(r32) * x_size * y_size * z_size);
#else
  r32* noise_set = noise_allocate(sizeof(r32), sizeof(r32) * x_size * y_size * z_size);
#endif
  // #pragma omp parallel for collapse(3) if (perlin_noise->parallel)
  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim++) {
        r32 x = (perlin_noise->position[0] * perlin_noise->frequency) + (x_dim * perlin_noise->step);
        r32 y = (perlin_noise->position[1] * perlin_noise->frequency) + (y_dim * perlin_noise->step);
        r32 z = (perlin_noise->position[2] * perlin_noise->frequency) + (z_dim * perlin_noise->step);

        r32 value = 0.0;
        r32 cur_persistence = 1.0;

        for (int cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
          r32 nx = make_int_32_range(x);
          r32 ny = make_int_32_range(y);
          r32 nz = make_int_32_range(z);

          int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
          r32 signal = gradient_coherent_noise_3d(nx, ny, nz, cur_seed, perlin_noise->noise_quality);
          value += signal * cur_persistence;

          x *= perlin_noise->lacunarity;
          y *= perlin_noise->lacunarity;
          z *= perlin_noise->lacunarity;

          cur_persistence *= perlin_noise->persistence;
        }

        *(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size)))) = value;
      }
    }
  }
  return noise_set;
}

#ifdef __arm64__
#else
global inline r32* perlin_noise_eval_3d_sse2(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size) {
  r32* noise_set = noise_allocate(sizeof(__m128), sizeof(r32) * x_size * y_size * z_size);
  // #pragma omp parallel for collapse(3) if (perlin_noise->parallel)
  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim += 4) {
        __m128 x_vec = _mm_mul_ps(_mm_add_ps(_mm_set1_ps(perlin_noise->position[0]), _mm_mul_ps(_mm_set_ps(x_dim + 3.0, x_dim + 2.0, x_dim + 1.0, x_dim), _mm_set1_ps(perlin_noise->step))), _mm_set1_ps(perlin_noise->frequency));
        r32 y = (perlin_noise->position[1] + (y_dim * perlin_noise->step)) * perlin_noise->frequency;
        r32 z = (perlin_noise->position[2] + (z_dim * perlin_noise->step)) * perlin_noise->frequency;

        __m128 value = _mm_set1_ps(0.0);
        r32 cur_persistence = 1.0;

        for (int cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
          __m128 nx = make_int_32_range_sse2(x_vec);
          r32 ny = make_int_32_range(y);
          r32 nz = make_int_32_range(z);

          int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
          __m128 signal = gradient_coherent_noise_3d_sse2(nx, ny, nz, cur_seed, perlin_noise->noise_quality);
          value = _mm_add_ps(value, _mm_mul_ps(signal, _mm_set1_ps(cur_persistence)));

          x_vec = _mm_mul_ps(x_vec, _mm_set1_ps(perlin_noise->lacunarity));
          y *= perlin_noise->lacunarity;
          z *= perlin_noise->lacunarity;

          cur_persistence = cur_persistence * perlin_noise->persistence;
        }

        _mm_store_ps(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size))), value);
      }
    }
  }
  return noise_set;
}

global inline r32* perlin_noise_eval_3d_sse4_1(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size) {
  r32* noise_set = noise_allocate(sizeof(__m128), sizeof(r32) * x_size * y_size * z_size);
  // #pragma omp parallel for collapse(3) if (perlin_noise->parallel)
  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim += 4) {
        __m128 x_vec = _mm_mul_ps(_mm_add_ps(_mm_set1_ps(perlin_noise->position[0]), _mm_mul_ps(_mm_set_ps(x_dim + 3.0, x_dim + 2.0, x_dim + 1.0, x_dim), _mm_set1_ps(perlin_noise->step))), _mm_set1_ps(perlin_noise->frequency));
        r32 y = (perlin_noise->position[1] + (y_dim * perlin_noise->step)) * perlin_noise->frequency;
        r32 z = (perlin_noise->position[2] + (z_dim * perlin_noise->step)) * perlin_noise->frequency;

        __m128 value = _mm_set1_ps(0.0);
        r32 cur_persistence = 1.0;

        for (int cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
          __m128 nx = make_int_32_range_sse2(x_vec);
          r32 ny = make_int_32_range(y);
          r32 nz = make_int_32_range(z);

          int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
          __m128 signal = gradient_coherent_noise_3d_sse4_1(nx, ny, nz, cur_seed, perlin_noise->noise_quality);
          value = _mm_add_ps(value, _mm_mul_ps(signal, _mm_set1_ps(cur_persistence)));

          x_vec = _mm_mul_ps(x_vec, _mm_set1_ps(perlin_noise->lacunarity));
          y *= perlin_noise->lacunarity;
          z *= perlin_noise->lacunarity;

          cur_persistence = cur_persistence * perlin_noise->persistence;
        }

        _mm_store_ps(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size))), value);
      }
    }
  }
  return noise_set;
}

global inline r32* perlin_noise_eval_3d_avx(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size) {
  r32* noise_set = noise_allocate(sizeof(__m256), sizeof(r32) * x_size * y_size * z_size);
  // #pragma omp parallel for collapse(3) if (perlin_noise->parallel)
  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim += 8) {
        __m256 x_vec = _mm256_mul_ps(_mm256_add_ps(_mm256_set1_ps(perlin_noise->position[0]), _mm256_mul_ps(_mm256_set_ps(x_dim + 7.0, x_dim + 6.0, x_dim + 5.0, x_dim + 4.0, x_dim + 3.0, x_dim + 2.0, x_dim + 1.0, x_dim), _mm256_set1_ps(perlin_noise->step))), _mm256_set1_ps(perlin_noise->frequency));
        r32 y = (perlin_noise->position[1] + (y_dim * perlin_noise->step)) * perlin_noise->frequency;
        r32 z = (perlin_noise->position[2] + (z_dim * perlin_noise->step)) * perlin_noise->frequency;

        __m256 value = _mm256_set1_ps(0.0);
        r32 cur_persistence = 1.0;

        for (int cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
          __m256 nx = make_int_32_range_avx(x_vec);
          r32 ny = make_int_32_range(y);
          r32 nz = make_int_32_range(z);

          int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
          __m256 signal = gradient_coherent_noise_3d_avx(nx, ny, nz, cur_seed, perlin_noise->noise_quality);
          value = _mm256_add_ps(value, _mm256_mul_ps(signal, _mm256_set1_ps(cur_persistence)));

          x_vec = _mm256_mul_ps(x_vec, _mm256_set1_ps(perlin_noise->lacunarity));
          y *= perlin_noise->lacunarity;
          z *= perlin_noise->lacunarity;

          cur_persistence = cur_persistence * perlin_noise->persistence;
        }

        _mm256_store_ps(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size))), value);
      }
    }
  }
  return noise_set;
}

global inline r32* perlin_noise_eval_3d_avx2(struct PerlinNoise* perlin_noise, size_t x_size, size_t y_size, size_t z_size) {
  r32* noise_set = noise_allocate(sizeof(__m256), sizeof(r32) * x_size * y_size * z_size);
  // #pragma omp parallel for collapse(3) if (perlin_noise->parallel)
  for (int z_dim = 0; z_dim < z_size; z_dim++) {
    for (int y_dim = 0; y_dim < y_size; y_dim++) {
      for (int x_dim = 0; x_dim < x_size; x_dim += 8) {
        __m256 x_vec = _mm256_mul_ps(_mm256_add_ps(_mm256_set1_ps(perlin_noise->position[0]), _mm256_mul_ps(_mm256_set_ps(x_dim + 7.0, x_dim + 6.0, x_dim + 5.0, x_dim + 4.0, x_dim + 3.0, x_dim + 2.0, x_dim + 1.0, x_dim), _mm256_set1_ps(perlin_noise->step))), _mm256_set1_ps(perlin_noise->frequency));
        r32 y = (perlin_noise->position[1] + (y_dim * perlin_noise->step)) * perlin_noise->frequency;
        r32 z = (perlin_noise->position[2] + (z_dim * perlin_noise->step)) * perlin_noise->frequency;

        __m256 value = _mm256_set1_ps(0.0);
        r32 cur_persistence = 1.0;

        for (int cur_octave = 0; cur_octave < perlin_noise->octave_count; cur_octave++) {
          __m256 nx = make_int_32_range_avx(x_vec);
          r32 ny = make_int_32_range(y);
          r32 nz = make_int_32_range(z);

          int cur_seed = (perlin_noise->seed + cur_octave) & 0xffffffff;
          __m256 signal = gradient_coherent_noise_3d_avx2(nx, ny, nz, cur_seed, perlin_noise->noise_quality);
          value = _mm256_add_ps(value, _mm256_mul_ps(signal, _mm256_set1_ps(cur_persistence)));

          x_vec = _mm256_mul_ps(x_vec, _mm256_set1_ps(perlin_noise->lacunarity));
          y *= perlin_noise->lacunarity;
          z *= perlin_noise->lacunarity;

          cur_persistence = cur_persistence * perlin_noise->persistence;
        }

        _mm256_store_ps(noise_set + (x_dim + (y_dim * x_size) + (z_dim * (x_size * y_size))), value);
      }
    }
  }
  return noise_set;
}
#endif
