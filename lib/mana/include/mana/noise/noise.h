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
#define DEFAULT_PARALLEL TRUE

#define DEFAULT_SPHERE_RADIUS 8.0
#define DEFAULT_SPHERE_ORIGIN_X 0.0
#define DEFAULT_SPHERE_ORIGIN_Y 0.0
#define DEFAULT_SPHERE_ORIGIN_Z 0.0

#define DEFAULT_VORONOI_DISPLACEMENT 1.0f
#define DEFAULT_VORONOI_ENABLE_DISTANCE TRUE

// TODO: This should be a full noise module
void noise_eval(struct Noise* noise, r32* memory, u8 bits, size_t x, size_t y, size_t z);

// Note: Intrinsics support should be stored

// Note: Interesting they suggest ridged for mountains, billow for plains, and perlin for selecting between the two
// https://libnoise.sourceforge.net/tutorials/tutorial7.html

// Note: Can spare memory here, access speed is most important
struct Noise {
  void (*eval_ptr)(struct Noise*, r32*, u8, size_t, size_t, size_t);
  r32 position[3];
  r32 step;
  int_fast32_t seed;
  b8 parallel;
  union {
    struct {  // Billow
      r32 frequency;
      r32 lacunarity;
      r32 persistence;
      u8 octave_count;
    } billow;
    struct {  // Perlin
      r32 frequency;
      r32 lacunarity;
      r32 persistence;
      u8 octave_count;
    } perlin;
    struct {  // Ridged Fractal
      r32 frequency;
      r32 lacunarity;
      u8 octave_count;
    } ridged_fractal;
    struct {  // Sphere
      r32 radius;
      r32 sphere_origin[3];
    } sphere;
    struct {  // Voronoi
      r32 frequency;
      r32 displacement;
      b8 enable_distance;  // Note: Disabled is sharp, enabled is smooth
    } voronoi;
  };
};

void noise_billow_eval(struct Noise* noise, r32* memory, u8 bits, size_t x_size, size_t y_size, size_t z_size);

// clang-format off
#define BILLOW_NOISE_INIT { noise_billow_eval, { DEFAULT_POSITION_X, DEFAULT_POSITION_Y, DEFAULT_POSITION_Z }, DEFAULT_STEP, DEFAULT_SEED, DEFAULT_PARALLEL, { { DEFAULT_FREQUENCY, DEFAULT_LACUNARITY, DEFAULT_PERSISTENCE, DEFAULT_OCTAVE_COUNT } } }
// clang-format on
