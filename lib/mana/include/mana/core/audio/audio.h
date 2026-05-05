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

// For loading WAV Audio
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

// For playing audio from mixer
struct AudioClipF32 {
  r32* samples;
  u32 frame_count;
  u32 sample_rate;
  u16 channels;
};

#define AUDIO_MAX_VOICES 64
#define AUDIO_COMMAND_CAPACITY 256

enum AudioCommandType {
  AUDIO_COMMAND_NONE = 0,
  AUDIO_COMMAND_PLAY,
  AUDIO_COMMAND_STOP_MUSIC,
  AUDIO_COMMAND_STOP_ALL
};

struct AudioCommand {
  enum AudioCommandType type;

  const struct AudioClipF32* clip;

  r32 volume;
  r32 pan;

  b32 loop;
  b32 music;
};

struct AudioVoice {
  b32 active;
  b32 loop;
  b32 music;

  const struct AudioClipF32* clip;

  r64 position;

  r64 step;

  r32 volume;
  r32 pan;
};

struct AudioEngine {
  HANDLE thread;
  DWORD thread_id;

  HANDLE quit_event;
  HANDLE command_event;

  CRITICAL_SECTION command_lock;

  struct AudioCommand commands[AUDIO_COMMAND_CAPACITY];
  u32 command_read;
  u32 command_write;

  struct AudioVoice voices[AUDIO_MAX_VOICES];

  u32 mix_sample_rate;
  u16 mix_channels;

  b32 running;
};

i32 load_audio(char* file, struct AudioClip* clip);
void audio_free_clip(struct AudioClip* clip);
i32 audio_prepare_clip(const struct AudioClip* source, struct AudioClipF32* dest);
void audio_free_clip_f32(struct AudioClipF32* clip);
i32 audio_engine_start(struct AudioEngine* engine);
void audio_engine_stop(struct AudioEngine* engine);
i32 audio_play_sfx(struct AudioEngine* engine, const struct AudioClipF32* clip, r32 volume);
i32 audio_play_sfx_ex(struct AudioEngine* engine, const struct AudioClipF32* clip, r32 volume, r32 pan);
i32 audio_play_music(struct AudioEngine* engine, const struct AudioClipF32* clip, r32 volume, b32 loop);
i32 audio_stop_music(struct AudioEngine* engine);
i32 audio_stop_all(struct AudioEngine* engine);
