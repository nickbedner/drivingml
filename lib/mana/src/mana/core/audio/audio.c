#ifndef COBJMACROS
#define COBJMACROS
#endif

#include "mana/core/audio/audio.h"

#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

internal REFERENCE_TIME to_reference_time(r64 seconds) {
  return (REFERENCE_TIME)(seconds * 10000000.0 + 0.5);
}

internal u32 read_u32_le(const u8* p) {
  return ((u32)p[0]) | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

internal u32 audio_clip_frame_count(const struct AudioClip* clip) {
  if (!clip || !clip->header.block_align)
    return 0;
  return clip->header.subchunk2_size / clip->header.block_align;
}

internal u32 audio_clip_sample_count(const struct AudioClip* clip) {
  if (!clip)
    return 0;
  return audio_clip_frame_count(clip) * clip->header.num_channels;
}

internal b32 wav_find_data_chunk(const u8* wav_file, u32 file_size, u32* out_data_offset, u32* out_data_size) {
  u32 offset;

  if (!wav_file || !out_data_offset || !out_data_size)
    return 0;
  if (file_size < 12)
    return 0;

  if (memcmp(wav_file + 0, "RIFF", 4) != 0)
    return 0;
  if (memcmp(wav_file + 8, "WAVE", 4) != 0)
    return 0;

  offset = 12;

  while (offset + 8 <= file_size) {
    const u8* chunk = wav_file + offset;
    u32 chunk_size = read_u32_le(chunk + 4);
    u32 next_offset = offset + 8 + chunk_size + (chunk_size & 1u);

    if (memcmp(chunk + 0, "data", 4) == 0) {
      if (offset + 8 + chunk_size > file_size)
        return 0;

      *out_data_offset = offset + 8;
      *out_data_size = chunk_size;
      return 1;
    }

    if (next_offset <= offset || next_offset > file_size) break;
    offset = next_offset;
  }

  return 0;
}

internal b32 wav_find_fmt_chunk(const u8* wav_file, u32 file_size, u32* out_fmt_offset, u32* out_fmt_size) {
  u32 offset;

  if (!wav_file || !out_fmt_offset || !out_fmt_size)
    return 0;
  if (file_size < 12)
    return 0;

  if (memcmp(wav_file + 0, "RIFF", 4) != 0)
    return 0;
  if (memcmp(wav_file + 8, "WAVE", 4) != 0)
    return 0;

  offset = 12;

  while (offset + 8 <= file_size) {
    const u8* chunk = wav_file + offset;
    u32 chunk_size = read_u32_le(chunk + 4);
    u32 next_offset = offset + 8 + chunk_size + (chunk_size & 1u);

    if (memcmp(chunk + 0, "fmt ", 4) == 0) {
      if (offset + 8 + chunk_size > file_size)
        return 0;

      *out_fmt_offset = offset + 8;
      *out_fmt_size = chunk_size;
      return 1;
    }

    if (next_offset <= offset || next_offset > file_size)
      break;
    offset = next_offset;
  }

  return 0;
}

internal r32 clamp_sample_f32(r32 x) {
  if (x < -1.0f)
    return -1.0f;
  if (x > 1.0f)
    return 1.0f;
  return x;
}

internal r32* audio_clip_to_float32_interleaved(const struct AudioClip* clip, u32* out_sample_count) {
  u32 sample_count;
  r32* out;
  u32 i;

  if (!clip || !out_sample_count)
    return 0;

  sample_count = audio_clip_sample_count(clip);
  if (!sample_count)
    return 0;

  out = (r32*)malloc(sample_count * sizeof(r32));
  if (!out)
    return 0;

  if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 8) {
    for (i = 0; i < sample_count; i++) {
      u8 s = (u8)clip->samples.int8[i];
      out[i] = ((r32)s - 128.0f) / 128.0f;
    }
  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 16) {
    for (i = 0; i < sample_count; i++)
      out[i] = (r32)clip->samples.int16[i] / 32768.0f;

  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 24) {
    for (i = 0; i < sample_count; i++)
      out[i] = (r32)clip->samples.int32[i] / 8388608.0f;

  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 32) {
    for (i = 0; i < sample_count; i++)
      out[i] = (r32)clip->samples.int32[i] / 2147483648.0f;

  } else if (clip->header.audio_format == 3 && clip->header.bits_per_sample == 32) {
    for (i = 0; i < sample_count; i++)
      out[i] = clamp_sample_f32(clip->samples.float32[i]);

  } else {
    free(out);
    return 0;
  }

  *out_sample_count = sample_count;
  return out;
}

void audio_free_clip(struct AudioClip* clip) {
  if (!clip)
    return;

  if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 8)
    free(clip->samples.int8);
  else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 16)
    free(clip->samples.int16);
  else if (clip->header.audio_format == 1 && (clip->header.bits_per_sample == 24 || clip->header.bits_per_sample == 32))
    free(clip->samples.int32);
  else if (clip->header.audio_format == 3 && clip->header.bits_per_sample == 32)
    free(clip->samples.float32);

  memset(clip, 0, sizeof(*clip));
}

int load_audio(char* file, struct AudioClip* clip) {
  char* wav_file;
  u32 wav_size;
  u32 fmt_offset;
  u32 fmt_size;
  u32 data_offset;
  u32 data_size;

  if (!file || !clip) return 1;
  memset(clip, 0, sizeof(*clip));

  wav_file = read_file(file);
  if (!wav_file) {
    log_message(LOG_SEVERITY_WARNING, "Warning: Failed to read WAV file.\n");
    return 1;
  }

  wav_size = (u32)file_size(file);
  if (wav_size < 12) {
    log_message(LOG_SEVERITY_WARNING, "Warning: WAV file too small.\n");
    free(wav_file);
    return 1;
  }

  if (memcmp(wav_file + 0, "RIFF", 4) != 0 || memcmp(wav_file + 8, "WAVE", 4) != 0) {
    log_message(LOG_SEVERITY_WARNING, "Warning: Not a valid WAV file.\n");
    free(wav_file);
    return 1;
  }

  if (!wav_find_fmt_chunk((const u8*)wav_file, wav_size, &fmt_offset, &fmt_size)) {
    log_message(LOG_SEVERITY_WARNING, "Warning: WAV fmt chunk not found.\n");
    free(wav_file);
    return 1;
  }

  if (fmt_size < 16) {
    log_message(LOG_SEVERITY_WARNING, "Warning: WAV fmt chunk too small.\n");
    free(wav_file);
    return 1;
  }

  memset(&clip->header, 0, sizeof(clip->header));
  memcpy(clip->header.chunk_id, wav_file + 0, 4);
  clip->header.chunk_size = read_u32_le((const u8*)wav_file + 4);
  memcpy(clip->header.format, wav_file + 8, 4);

  memcpy(clip->header.subchunk1_id, "fmt ", 4);
  clip->header.subchunk1_size = fmt_size;
  clip->header.audio_format = (u16)(wav_file[fmt_offset + 0] | ((u8)wav_file[fmt_offset + 1] << 8));
  clip->header.num_channels = (u16)(wav_file[fmt_offset + 2] | ((u8)wav_file[fmt_offset + 3] << 8));
  clip->header.sample_rate = read_u32_le((const u8*)wav_file + fmt_offset + 4);
  clip->header.byte_rate = read_u32_le((const u8*)wav_file + fmt_offset + 8);
  clip->header.block_align = (u16)(wav_file[fmt_offset + 12] | ((u8)wav_file[fmt_offset + 13] << 8));
  clip->header.bits_per_sample = (u16)(wav_file[fmt_offset + 14] | ((u8)wav_file[fmt_offset + 15] << 8));

  if (!wav_find_data_chunk((const u8*)wav_file, wav_size, &data_offset, &data_size)) {
    log_message(LOG_SEVERITY_WARNING, "Warning: WAV data chunk not found.\n");
    free(wav_file);
    return 1;
  }

  memcpy(clip->header.subchunk2_id, "data", 4);
  clip->header.subchunk2_size = data_size;

  if (clip->header.num_channels == 0 || clip->header.block_align == 0) {
    log_message(LOG_SEVERITY_WARNING, "Warning: Invalid WAV channel/block alignment.\n");
    free(wav_file);
    return 1;
  }

  if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 8) {
    clip->samples.int8 = (i8*)malloc(data_size);
    if (!clip->samples.int8) {
      free(wav_file);
      return 1;
    }
    memcpy(clip->samples.int8, wav_file + data_offset, data_size);
  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 16) {
    clip->samples.int16 = (i16*)malloc(data_size);
    if (!clip->samples.int16) {
      free(wav_file);
      return 1;
    }
    memcpy(clip->samples.int16, wav_file + data_offset, data_size);

  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 24) {
    u32 sample_count = data_size / 3;
    const u8* src = (const u8*)(wav_file + data_offset);
    u32 i;

    clip->samples.int32 = (i32*)malloc(sample_count * sizeof(i32));
    if (!clip->samples.int32) {
      free(wav_file);
      return 1;
    }

    for (i = 0; i < sample_count; ++i) {
      u32 b0 = src[i * 3 + 0];
      u32 b1 = src[i * 3 + 1];
      u32 b2 = src[i * 3 + 2];
      i32 sample = (i32)(b0 | (b1 << 8) | (b2 << 16));

      if (sample & 0x00800000)
        sample |= (i32)0xFF000000;

      clip->samples.int32[i] = sample;
    }

  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 32) {
    clip->samples.int32 = (i32*)malloc(data_size);
    if (!clip->samples.int32) {
      free(wav_file);
      return 1;
    }
    memcpy(clip->samples.int32, wav_file + data_offset, data_size);

  } else if (clip->header.audio_format == 3 && clip->header.bits_per_sample == 32) {
    clip->samples.float32 = (r32*)malloc(data_size);
    if (!clip->samples.float32) {
      free(wav_file);
      return 1;
    }
    memcpy(clip->samples.float32, wav_file + data_offset, data_size);

  } else {
    log_message(LOG_SEVERITY_WARNING, "Warning: Unsupported WAV format.\n");
    free(wav_file);
    return 1;
  }

  free(wav_file);
  return 0;
}

internal DWORD WINAPI audio_thread_proc(void* param);

internal b32 audio_push_command(struct AudioEngine* engine, struct AudioCommand command) {
  u32 next_write;

  if (!engine)
    return 0;

  EnterCriticalSection(&engine->command_lock);

  next_write = (engine->command_write + 1) % AUDIO_COMMAND_CAPACITY;

  if (next_write == engine->command_read) {
    LeaveCriticalSection(&engine->command_lock);
    return 0;
  }

  engine->commands[engine->command_write] = command;
  engine->command_write = next_write;

  LeaveCriticalSection(&engine->command_lock);

  SetEvent(engine->command_event);

  return 1;
}

internal b32 audio_pop_command(struct AudioEngine* engine, struct AudioCommand* out_command) {
  if (!engine || !out_command)
    return 0;

  EnterCriticalSection(&engine->command_lock);

  if (engine->command_read == engine->command_write) {
    LeaveCriticalSection(&engine->command_lock);
    return 0;
  }

  *out_command = engine->commands[engine->command_read];
  engine->command_read = (engine->command_read + 1) % AUDIO_COMMAND_CAPACITY;

  LeaveCriticalSection(&engine->command_lock);

  return 1;
}

int audio_engine_start(struct AudioEngine* engine) {
  if (!engine)
    return 1;

  memset(engine, 0, sizeof(*engine));

  engine->mix_sample_rate = 48000;
  engine->mix_channels = 2;

  InitializeCriticalSection(&engine->command_lock);

  engine->quit_event = CreateEventA(0, TRUE, FALSE, 0);
  if (!engine->quit_event) {
    DeleteCriticalSection(&engine->command_lock);
    memset(engine, 0, sizeof(*engine));
    return 1;
  }

  engine->command_event = CreateEventA(0, FALSE, FALSE, 0);
  if (!engine->command_event) {
    CloseHandle(engine->quit_event);
    DeleteCriticalSection(&engine->command_lock);
    memset(engine, 0, sizeof(*engine));
    return 1;
  }

  engine->running = 1;

  engine->thread = CreateThread(
      0,
      0,
      audio_thread_proc,
      engine,
      0,
      &engine->thread_id);

  if (!engine->thread) {
    CloseHandle(engine->command_event);
    CloseHandle(engine->quit_event);
    DeleteCriticalSection(&engine->command_lock);
    memset(engine, 0, sizeof(*engine));
    return 1;
  }

  return 0;
}

void audio_engine_stop(struct AudioEngine* engine) {
  if (!engine)
    return;

  if (engine->quit_event)
    SetEvent(engine->quit_event);

  if (engine->thread) {
    WaitForSingleObject(engine->thread, INFINITE);
    CloseHandle(engine->thread);
  }

  if (engine->command_event)
    CloseHandle(engine->command_event);

  if (engine->quit_event)
    CloseHandle(engine->quit_event);

  DeleteCriticalSection(&engine->command_lock);

  memset(engine, 0, sizeof(*engine));
}

int audio_prepare_clip(const struct AudioClip* source, struct AudioClipF32* dest) {
  u32 sample_count;

  if (!source || !dest)
    return 1;

  memset(dest, 0, sizeof(*dest));

  if (source->header.num_channels == 0 || source->header.sample_rate == 0)
    return 1;

  dest->samples = audio_clip_to_float32_interleaved(source, &sample_count);
  if (!dest->samples)
    return 1;

  dest->channels = source->header.num_channels;
  dest->sample_rate = source->header.sample_rate;
  dest->frame_count = sample_count / dest->channels;

  if (!dest->frame_count) {
    free(dest->samples);
    memset(dest, 0, sizeof(*dest));
    return 1;
  }

  return 0;
}

int audio_play_sfx(struct AudioEngine* engine, const struct AudioClipF32* clip, r32 volume) {
  return audio_play_sfx_ex(engine, clip, volume, 0.0f);
}

int audio_play_sfx_ex(struct AudioEngine* engine, const struct AudioClipF32* clip, r32 volume, r32 pan) {
  struct AudioCommand command;

  if (!engine || !clip || !clip->samples)
    return 1;

  if (pan < -1.0f)
    pan = -1.0f;
  if (pan > 1.0f)
    pan = 1.0f;

  memset(&command, 0, sizeof(command));

  command.type = AUDIO_COMMAND_PLAY;
  command.clip = clip;
  command.volume = volume;
  command.pan = pan;
  command.loop = 0;
  command.music = 0;

  return audio_push_command(engine, command) ? 0 : 1;
}

int audio_play_music(
    struct AudioEngine* engine,
    const struct AudioClipF32* clip,
    r32 volume,
    b32 loop) {
  struct AudioCommand stop_command;
  struct AudioCommand play_command;

  if (!engine || !clip || !clip->samples)
    return 1;

  memset(&stop_command, 0, sizeof(stop_command));
  stop_command.type = AUDIO_COMMAND_STOP_MUSIC;
  audio_push_command(engine, stop_command);

  memset(&play_command, 0, sizeof(play_command));

  play_command.type = AUDIO_COMMAND_PLAY;
  play_command.clip = clip;
  play_command.volume = volume;
  play_command.pan = 0.0f;
  play_command.loop = loop;
  play_command.music = 1;

  return audio_push_command(engine, play_command) ? 0 : 1;
}

int audio_stop_music(struct AudioEngine* engine) {
  struct AudioCommand command;

  if (!engine)
    return 1;

  memset(&command, 0, sizeof(command));
  command.type = AUDIO_COMMAND_STOP_MUSIC;

  return audio_push_command(engine, command) ? 0 : 1;
}

int audio_stop_all(struct AudioEngine* engine) {
  struct AudioCommand command;

  if (!engine)
    return 1;

  memset(&command, 0, sizeof(command));
  command.type = AUDIO_COMMAND_STOP_ALL;

  return audio_push_command(engine, command) ? 0 : 1;
}

internal void audio_add_voice(struct AudioEngine* engine, const struct AudioCommand* command) {
  u32 i;
  struct AudioVoice* voice;

  if (!engine || !command || !command->clip)
    return;

  voice = 0;

  for (i = 0; i < AUDIO_MAX_VOICES; ++i) {
    if (!engine->voices[i].active) {
      voice = &engine->voices[i];
      break;
    }
  }

  if (!voice)
    voice = &engine->voices[AUDIO_MAX_VOICES - 1];

  memset(voice, 0, sizeof(*voice));

  voice->active = 1;
  voice->loop = command->loop;
  voice->music = command->music;
  voice->clip = command->clip;
  voice->position = 0.0;
  voice->volume = command->volume;
  voice->pan = command->pan;

  voice->step =
      (r64)command->clip->sample_rate /
      (r64)engine->mix_sample_rate;
}

internal void audio_process_commands(struct AudioEngine* engine) {
  struct AudioCommand command;
  u32 i;

  if (!engine)
    return;

  while (audio_pop_command(engine, &command)) {
    switch (command.type) {
      case AUDIO_COMMAND_PLAY: {
        audio_add_voice(engine, &command);
      } break;

      case AUDIO_COMMAND_STOP_MUSIC: {
        for (i = 0; i < AUDIO_MAX_VOICES; ++i) {
          if (engine->voices[i].active && engine->voices[i].music)
            engine->voices[i].active = 0;
        }
      } break;

      case AUDIO_COMMAND_STOP_ALL: {
        for (i = 0; i < AUDIO_MAX_VOICES; ++i)
          engine->voices[i].active = 0;
      } break;

      default: {
      } break;
    }
  }
}

internal r32 audio_clip_f32_sample(
    const struct AudioClipF32* clip,
    u32 frame,
    u32 channel) {
  if (!clip || !clip->samples)
    return 0.0f;

  if (frame >= clip->frame_count)
    return 0.0f;

  if (clip->channels == 1)
    return clip->samples[frame];

  if (channel >= clip->channels)
    channel = clip->channels - 1;

  return clip->samples[frame * clip->channels + channel];
}

internal r32 audio_clip_f32_sample_linear(
    const struct AudioClipF32* clip,
    r64 position,
    u32 channel,
    b32 loop) {
  u32 frame0;
  u32 frame1;
  r32 t;
  r32 a;
  r32 b;

  if (!clip || !clip->samples || clip->frame_count == 0)
    return 0.0f;

  frame0 = (u32)position;

  if (frame0 >= clip->frame_count)
    return 0.0f;

  frame1 = frame0 + 1;

  if (frame1 >= clip->frame_count) {
    if (loop)
      frame1 = 0;
    else
      frame1 = frame0;
  }

  t = (r32)(position - (r64)frame0);

  a = audio_clip_f32_sample(clip, frame0, channel);
  b = audio_clip_f32_sample(clip, frame1, channel);

  return a + ((b - a) * t);
}

internal void audio_mix_frames(
    struct AudioEngine* engine,
    r32* output,
    u32 frame_count) {
  u32 frame_index;
  u32 voice_index;

  if (!engine || !output)
    return;

  memset(output, 0, frame_count * engine->mix_channels * sizeof(r32));

  for (frame_index = 0; frame_index < frame_count; ++frame_index) {
    r32 mix_l;
    r32 mix_r;

    mix_l = 0.0f;
    mix_r = 0.0f;

    for (voice_index = 0; voice_index < AUDIO_MAX_VOICES; ++voice_index) {
      struct AudioVoice* voice;
      const struct AudioClipF32* clip;
      r32 source_l;
      r32 source_r;
      r32 pan_l;
      r32 pan_r;

      voice = &engine->voices[voice_index];

      if (!voice->active)
        continue;

      clip = voice->clip;

      if (!clip || !clip->samples || clip->frame_count == 0) {
        voice->active = 0;
        continue;
      }

      if (voice->position >= (r64)clip->frame_count) {
        if (voice->loop) {
          while (voice->position >= (r64)clip->frame_count)
            voice->position -= (r64)clip->frame_count;
        } else {
          voice->active = 0;
          continue;
        }
      }

      source_l = audio_clip_f32_sample_linear(
          clip,
          voice->position,
          0,
          voice->loop);

      source_r = audio_clip_f32_sample_linear(
          clip,
          voice->position,
          1,
          voice->loop);

      pan_l = 1.0f;
      pan_r = 1.0f;

      if (voice->pan > 0.0f)
        pan_l = 1.0f - voice->pan;
      else if (voice->pan < 0.0f)
        pan_r = 1.0f + voice->pan;

      mix_l += source_l * voice->volume * pan_l;
      mix_r += source_r * voice->volume * pan_r;

      voice->position += voice->step;
    }

    output[frame_index * 2 + 0] = clamp_sample_f32(mix_l);
    output[frame_index * 2 + 1] = clamp_sample_f32(mix_r);
  }
}

internal DWORD WINAPI audio_thread_proc(void* param) {
  struct AudioEngine* engine;

  HRESULT hr;
  b32 com_initialized;

  IMMDeviceEnumerator* device_enumerator;
  IMMDevice* device;
  IAudioClient* audio_client;
  IAudioRenderClient* render_client;

  HANDLE buffer_event;
  HANDLE wait_handles[3];

  UINT32 buffer_frame_count;

  WAVEFORMATEX wave_format;

  engine = (struct AudioEngine*)param;

  hr = S_OK;
  com_initialized = 0;

  device_enumerator = 0;
  device = 0;
  audio_client = 0;
  render_client = 0;
  buffer_event = 0;
  buffer_frame_count = 0;

  hr = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hr))
    goto cleanup;

  com_initialized = 1;

  memset(&wave_format, 0, sizeof(wave_format));

  wave_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
  wave_format.nChannels = engine->mix_channels;
  wave_format.nSamplesPerSec = engine->mix_sample_rate;
  wave_format.wBitsPerSample = 32;
  wave_format.nBlockAlign = (WORD)(wave_format.nChannels * sizeof(r32));
  wave_format.nAvgBytesPerSec =
      wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  wave_format.cbSize = 0;

  hr = CoCreateInstance(
      &MANA_CLSID_MMDeviceEnumerator,
      0,
      CLSCTX_ALL,
      &MANA_IID_IMMDeviceEnumerator,
      (void**)&device_enumerator);
  if (FAILED(hr))
    goto cleanup;

  hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(
      device_enumerator,
      eRender,
      eConsole,
      &device);
  if (FAILED(hr))
    goto cleanup;

  hr = IMMDevice_Activate(
      device,
      &MANA_IID_IAudioClient,
      CLSCTX_ALL,
      0,
      (void**)&audio_client);
  if (FAILED(hr))
    goto cleanup;

  hr = IAudioClient_Initialize(
      audio_client,
      AUDCLNT_SHAREMODE_SHARED,
      AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
          AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
          AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
      0,
      0,
      &wave_format,
      0);
  if (FAILED(hr))
    goto cleanup;

  hr = IAudioClient_GetBufferSize(audio_client, &buffer_frame_count);
  if (FAILED(hr))
    goto cleanup;

  hr = IAudioClient_GetService(
      audio_client,
      &MANA_IID_IAudioRenderClient,
      (void**)&render_client);
  if (FAILED(hr))
    goto cleanup;

  buffer_event = CreateEventA(0, FALSE, FALSE, 0);
  if (!buffer_event)
    goto cleanup;

  hr = IAudioClient_SetEventHandle(audio_client, buffer_event);
  if (FAILED(hr))
    goto cleanup;

  {
    BYTE* data;
    data = 0;

    hr = IAudioRenderClient_GetBuffer(
        render_client,
        buffer_frame_count,
        &data);
    if (FAILED(hr))
      goto cleanup;

    memset(data, 0, buffer_frame_count * wave_format.nBlockAlign);

    hr = IAudioRenderClient_ReleaseBuffer(
        render_client,
        buffer_frame_count,
        0);
    if (FAILED(hr))
      goto cleanup;
  }

  hr = IAudioClient_Start(audio_client);
  if (FAILED(hr))
    goto cleanup;

  wait_handles[0] = engine->quit_event;
  wait_handles[1] = engine->command_event;
  wait_handles[2] = buffer_event;

  for (;;) {
    DWORD wait_result;

    wait_result = WaitForMultipleObjects(
        3,
        wait_handles,
        FALSE,
        INFINITE);

    if (wait_result == WAIT_OBJECT_0) {
      break;
    } else if (wait_result == WAIT_OBJECT_0 + 1) {
      audio_process_commands(engine);
    } else if (wait_result == WAIT_OBJECT_0 + 2) {
      UINT32 padding;
      UINT32 frames_available;

      padding = 0;
      frames_available = 0;

      audio_process_commands(engine);

      hr = IAudioClient_GetCurrentPadding(audio_client, &padding);
      if (FAILED(hr))
        break;

      frames_available = buffer_frame_count - padding;

      if (frames_available > 0) {
        BYTE* data;

        data = 0;

        hr = IAudioRenderClient_GetBuffer(
            render_client,
            frames_available,
            &data);
        if (FAILED(hr))
          break;

        audio_mix_frames(
            engine,
            (r32*)data,
            frames_available);

        hr = IAudioRenderClient_ReleaseBuffer(
            render_client,
            frames_available,
            0);
        if (FAILED(hr))
          break;
      }
    }
  }

cleanup:
  if (audio_client)
    IAudioClient_Stop(audio_client);

  if (buffer_event)
    CloseHandle(buffer_event);

  if (render_client)
    IAudioRenderClient_Release(render_client);

  if (audio_client)
    IAudioClient_Release(audio_client);

  if (device)
    IMMDevice_Release(device);

  if (device_enumerator)
    IMMDeviceEnumerator_Release(device_enumerator);

  if (com_initialized)
    CoUninitialize();

  if (engine)
    engine->running = 0;

  return 0;
}

void audio_free_clip_f32(struct AudioClipF32* clip) {
  if (!clip)
    return;

  free(clip->samples);
  memset(clip, 0, sizeof(*clip));
}
