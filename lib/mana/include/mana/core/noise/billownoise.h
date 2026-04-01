#pragma once

#include "mana/noise/noisecommon.h"

#define DEFAULT_BILLOW_FREQUENCY 1.0
#define DEFAULT_BILLOW_LACUNARITY 2.0
#define DEFAULT_BILLOW_PERSISTENCE 0.5
#define DEFAULT_BILLOW_OCTAVE_COUNT 6
#define DEFAULT_BILLOW_SEED 0
#define DEFAULT_BILLOW_POSITION_X 0.0
#define DEFAULT_BILLOW_POSITION_Y 0.0
#define DEFAULT_BILLOW_POSITION_Z 0.0
#define DEFAULT_BILLOW_STEP 0.01
#define DEFAULT_BILLOW_PARALLEL FALSE
#define DEFAULT_BILLOW_QUALITY QUALITY_STANDARD

struct BillowNoise {
  r32 frequency;
  r32 lacunarity;
  r32 persistence;
  unsigned char octave_count;
  int seed;
  r32 position[3];
  r32 step;
};

global inline r32 billow_noise_eval(struct BillowNoise* billow_noise, r32 x_pos, r32 y_pos, r32 z_pos);

global inline void billow_noise_init(struct BillowNoise* billow_noise) {
  billow_noise->frequency = DEFAULT_BILLOW_FREQUENCY;
  billow_noise->lacunarity = DEFAULT_BILLOW_LACUNARITY;
  billow_noise->persistence = DEFAULT_BILLOW_PERSISTENCE;
  billow_noise->octave_count = DEFAULT_BILLOW_OCTAVE_COUNT;
  billow_noise->seed = DEFAULT_BILLOW_SEED;
  billow_noise->position[0] = DEFAULT_BILLOW_POSITION_X;
  billow_noise->position[1] = DEFAULT_BILLOW_POSITION_Y;
  billow_noise->position[2] = DEFAULT_BILLOW_POSITION_X;
  billow_noise->step = DEFAULT_BILLOW_STEP;
}

global inline r32 billow_noise_eval(struct BillowNoise* billow_noise, r32 x_pos, r32 y_pos, r32 z_pos) {
  r32 x = (billow_noise->position[0] + (x_pos * billow_noise->step)) * billow_noise->frequency;
  r32 y = (billow_noise->position[1] + (y_pos * billow_noise->step)) * billow_noise->frequency;
  r32 z = (billow_noise->position[2] + (z_pos * billow_noise->step)) * billow_noise->frequency;

  r32 value = 0.0;
  r32 cur_persistence = 1.0;

  for (int cur_octave = 0; cur_octave < billow_noise->octave_count; cur_octave++) {
    r32 nx = make_int_32_range(x);
    r32 ny = make_int_32_range(y);
    r32 nz = make_int_32_range(z);

    int cur_seed = (billow_noise->seed + cur_octave) & 0xffffffff;
    r32 signal = gradient_coherent_noise_3d(nx, ny, nz, cur_seed);
    signal = 2.0 * fabs(signal) - 1.0;
    value += signal * cur_persistence;

    x *= billow_noise->lacunarity;
    y *= billow_noise->lacunarity;
    z *= billow_noise->lacunarity;

    cur_persistence *= billow_noise->persistence;
  }

  value += 0.5;
  return value;
}
