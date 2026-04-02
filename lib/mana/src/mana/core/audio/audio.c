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

int play_audio_wasapi(struct AudioClip* clip) {
  HRESULT hr = S_OK;
  IMMDeviceEnumerator* device_enumerator = 0;
  IMMDevice* device = 0;
  IAudioClient* audio_client = 0;
  IAudioRenderClient* render_client = 0;
  HANDLE buffer_event = 0;

  r32* float_samples = 0;
  u32 total_sample_count = 0;
  u32 total_frame_count = 0;
  u32 frames_written = 0;

  UINT32 buffer_frame_count = 0;
  UINT32 padding = 0;

  WAVEFORMATEX wave_format;

  UINT32 frames_to_write;
  REFERENCE_TIME latency;

  if (!clip) return 1;
  if (clip->header.num_channels == 0)
    return 1;
  if (clip->header.sample_rate == 0)
    return 1;

  memset(&wave_format, 0, sizeof(wave_format));
  wave_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
  wave_format.nChannels = clip->header.num_channels;
  wave_format.nSamplesPerSec = clip->header.sample_rate;
  wave_format.wBitsPerSample = 32;
  wave_format.nBlockAlign = (WORD)(wave_format.nChannels * sizeof(r32));
  wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
  wave_format.cbSize = 0;

  float_samples = audio_clip_to_float32_interleaved(clip, &total_sample_count);
  if (!float_samples)
    return 1;

  total_frame_count = total_sample_count / clip->header.num_channels;

  hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
  if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
    free(float_samples);
    return 1;
  }

  hr = CoCreateInstance(&MANA_CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, &MANA_IID_IMMDeviceEnumerator, (void**)&device_enumerator);
  if (FAILED(hr))
    goto fail;

  hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(device_enumerator, eRender, eConsole, &device);
  if (FAILED(hr))
    goto fail;

  hr = IMMDevice_Activate(device, &MANA_IID_IAudioClient, CLSCTX_ALL, 0, (void**)&audio_client);
  if (FAILED(hr))
    goto fail;

  hr = IAudioClient_Initialize(audio_client, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, 0, 0, &wave_format, 0);
  if (FAILED(hr))
    goto fail;

  hr = IAudioClient_GetBufferSize(audio_client, &buffer_frame_count);
  if (FAILED(hr))
    goto fail;

  hr = IAudioClient_GetService(audio_client, &MANA_IID_IAudioRenderClient, (void**)&render_client);
  if (FAILED(hr))
    goto fail;

  buffer_event = CreateEventA(0, FALSE, FALSE, 0);
  if (!buffer_event)
    goto fail;

  hr = IAudioClient_SetEventHandle(audio_client, buffer_event);
  if (FAILED(hr))
    goto fail;

  frames_to_write = buffer_frame_count;

  if (frames_to_write > total_frame_count) {
    frames_to_write = total_frame_count;
  }

  if (frames_to_write > 0) {
    BYTE* dst = 0;

    hr = IAudioRenderClient_GetBuffer(render_client, frames_to_write, &dst);
    if (FAILED(hr)) goto fail;

    memcpy(dst, float_samples, frames_to_write * wave_format.nBlockAlign);

    frames_written += frames_to_write;

    hr = IAudioRenderClient_ReleaseBuffer(render_client, frames_to_write, 0);
    if (FAILED(hr))
      goto fail;
  }

  hr = IAudioClient_Start(audio_client);
  if (FAILED(hr))
    goto fail;

  while (frames_written < total_frame_count) {
    DWORD wait_result = WaitForSingleObject(buffer_event, 2000);

    if (wait_result != WAIT_OBJECT_0)
      break;

    hr = IAudioClient_GetCurrentPadding(audio_client, &padding);
    if (FAILED(hr))
      break;

    UINT32 frames_available = buffer_frame_count - padding;
    UINT32 frames_remaining = total_frame_count - frames_written;
    UINT32 frames_to_write_int = frames_available < frames_remaining ? frames_available : frames_remaining;

    if (!frames_to_write_int)
      continue;

    BYTE* dst = 0;

    hr = IAudioRenderClient_GetBuffer(render_client, frames_to_write_int, &dst);
    if (FAILED(hr))
      break;

    memcpy(dst, float_samples + (frames_written * clip->header.num_channels), frames_to_write_int * wave_format.nBlockAlign);

    frames_written += frames_to_write_int;

    hr = IAudioRenderClient_ReleaseBuffer(render_client, frames_to_write_int, 0);
    if (FAILED(hr))
      break;
  }

  latency = 0;
  if (SUCCEEDED(IAudioClient_GetStreamLatency(audio_client, &latency)))
    Sleep((DWORD)(latency / 10000) + 10);
  else
    Sleep(100);

  IAudioClient_Stop(audio_client);

  free(float_samples);
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
  CoUninitialize();

  return 0;

fail:
  if (audio_client)
    IAudioClient_Stop(audio_client);
  free(float_samples);
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
  CoUninitialize();

  return 1;
}
