#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef COBJMACROS
#define COBJMACROS
#endif

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <mmreg.h>

#include "mana/core/math/advmath.h"
#include "mana/core/utilities/fileio.h"

static const CLSID MANA_CLSID_MMDeviceEnumerator = {0xbcde0395, 0xe52f, 0x467c, {0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e}};
static const IID MANA_IID_IMMDeviceEnumerator = {0xa95664d2, 0x9614, 0x4f35, {0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6}};
static const IID MANA_IID_IAudioClient = {0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2}};
static const IID MANA_IID_IAudioRenderClient = {0xf294acfc, 0x3146, 0x4483, {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}};

#pragma pack(push, 1)
struct WAVHeader {
  i8 chunk_id[4];
  u32 chunk_size;
  i8 format[4];

  i8 subchunk1_id[4];
  u32 subchunk1_size;
  u16 audio_format;
  u16 num_channels;
  u32 sample_rate;
  u32 byte_rate;
  u16 block_align;
  u16 bits_per_sample;

  i8 subchunk2_id[4];
  u32 subchunk2_size;
};
#pragma pack(pop)

struct AudioClip {
  union {
    i8* int8;
    i16* int16;
    i32* int32;
    r32* float32;
    void* data;
  } samples;

  struct WAVHeader header;
};

void outputSineWave(void);

int load_audio(char* file, struct AudioClip* clip);
void audio_free_clip(struct AudioClip* clip);
int play_audio_wasapi(struct AudioClip* clip);
