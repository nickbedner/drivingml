#pragma once

#include "noisecommon.h"

#define DEFAULT_FREQUENCY 1.0f
#define DEFAULT_LACUNARITY 2.0f
#define DEFAULT_PERSISTENCE 0.5
#define DEFAULT_OCTAVE_COUNT 6
#define DEFAULT_SEED 0
#define DEFAULT_POSITION_X 0.0f
#define DEFAULT_POSITION_Y 0.0f
#define DEFAULT_POSITION_Z 0.0f
#define DEFAULT_STEP 0.01f
#define DEFAULT_PARALLEL true

#define DEFAULT_SPHERE_RADIUS 8.0
#define DEFAULT_SPHERE_ORIGIN_X 0.0
#define DEFAULT_SPHERE_ORIGIN_Y 0.0
#define DEFAULT_SPHERE_ORIGIN_Z 0.0

#define DEFAULT_VORONOI_DISPLACEMENT 1.0f
#define DEFAULT_VORONOI_ENABLE_DISTANCE true

// TODO: This should be a full noise module
void noise_eval(struct Noise *noise, float *memory, uint8_t bits, size_t x, size_t y, size_t z);

// Note: Intrinsics support should be stored

// Note: Interesting they suggest ridged for mountains, billow for plains, and perlin for selecting between the two
// https://libnoise.sourceforge.net/tutorials/tutorial7.html

// Note: Can spare memory here, access speed is most important
struct Noise {
  void (*eval_ptr)(struct Noise *, float *, uint8_t, size_t, size_t, size_t);
  float position[3];
  float step;
  int_fast32_t seed;
  bool parallel;
  union {
    struct {  // Billow
      float frequency;
      float lacunarity;
      float persistence;
      uint_fast8_t octave_count;
    } billow;
    struct {  // Perlin
      float frequency;
      float lacunarity;
      float persistence;
      uint_fast8_t octave_count;
    } perlin;
    struct {  // Ridged Fractal
      float frequency;
      float lacunarity;
      uint_fast8_t octave_count;
    } ridged_fractal;
    struct {  // Sphere
      float radius;
      float sphere_origin[3];
    } sphere;
    struct {  // Voronoi
      float frequency;
      float displacement;
      bool enable_distance;  // Note: Disabled is sharp, enabled is smooth
    } voronoi;
  };
};

void noise_billow_eval(struct Noise *noise, float *memory, uint8_t bits, size_t x_size, size_t y_size, size_t z_size);

// clang-format off
#define BILLOW_NOISE_INIT { noise_billow_eval, { DEFAULT_POSITION_X, DEFAULT_POSITION_Y, DEFAULT_POSITION_Z }, DEFAULT_STEP, DEFAULT_SEED, DEFAULT_PARALLEL, { { DEFAULT_FREQUENCY, DEFAULT_LACUNARITY, DEFAULT_PERSISTENCE, DEFAULT_OCTAVE_COUNT } } }
// clang-format on
