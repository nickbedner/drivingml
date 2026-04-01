#include "mana/audio/audio.h"
/*
void outputSineWave(void) {
  HRESULT hr = 0;

  // Initialize COM
  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr))
    return;

  IMMDeviceEnumerator *enumerator = NULL;
  IMMDevice *device = NULL;
  IPropertyStore *propertyStore = NULL;
  PROPVARIANT friendlyName;
  PropVariantInit(&friendlyName);

  // Create the device enumerator
  const CLSID clsid_MMDeviceEnumerator = CLSID_MMDeviceEnumerator;
  const IID iid_IMMDeviceEnumerator = IID_IMMDeviceEnumerator;
  hr = CoCreateInstance(&clsid_MMDeviceEnumerator, NULL, CLSCTX_ALL, &iid_IMMDeviceEnumerator, (void **)&enumerator);
  if (FAILED(hr))
    return;

  // Get the default audio output device
  hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eConsole, &device);
  if (FAILED(hr))
    return;

  // Get the device's property store
  hr = device->lpVtbl->OpenPropertyStore(device, STGM_READ, &propertyStore);
  if (FAILED(hr))
    return;

  // Get the device's friendly name
  const PROPERTYKEY pkey_Device_FriendlyName = PKEY_Device_FriendlyName;
  hr = propertyStore->lpVtbl->GetValue(propertyStore, &pkey_Device_FriendlyName, &friendlyName);
  if (FAILED(hr))
    return;

  // Use the friendly name
  wprintf(L"Device name: %s\n", friendlyName.pwszVal);
  _flushall();
  // log_message(LOG_SEVERITY_DEBUG, "Device name: %s\n", friendlyName.pwszVal);

  // Release resources
  PropVariantClear(&friendlyName);
  propertyStore->lpVtbl->Release(propertyStore);
  enumerator->lpVtbl->Release(enumerator);

  // Open the audio device for output
  IAudioClient *client = (IAudioClient *)calloc(1, sizeof(IAudioClient));
  const IID iid_IAudioClient = IID_IAudioClient;
  hr = IMMDevice_Activate(device, &iid_IAudioClient, CLSCTX_ALL, NULL, (void **)&client);
  if (FAILED(hr))
    return;

  // Get the buffer size and format
  // WAVEFORMATEX *format;
  WAVEFORMATEX *format = (WAVEFORMATEX *)calloc(1, sizeof(WAVEFORMATEX));
  hr = client->lpVtbl->GetMixFormat(client, &format);
  if (FAILED(hr))
    return;

  // Initialize the audio stream
  // Calculated by multiplying the sample rate (in samples per second) by the number of channels and by the desired buffer duration (in seconds).
  // UINT32 bufferFrameCount = format->nSamplesPerSec * 2;
  // UINT32 bufferFrameCount = 96000 * 2;
  // Should be the same as the sample rate generally
  // hr = client->lpVtbl->GetBufferSize(client, &bufferFrameCount);
  // if (FAILED(hr))
  //   return;

  // Note: This is exactly one second of buffer format->nSamplesPerSec * format->nChannels * (format->wBitsPerSample / 8)
  // Note: libsoundio uses 4 seconds of buffer so I'll do that because windows seems to freak out if it's too small
  // UINT32 audio_buffer = format->nSamplesPerSec * format->nChannels * (format->wBitsPerSample / 8) * 4;  // to_reference_time(4.0);
  UINT32 audio_buffer = to_reference_time(4.0);  // Note: This gives 192000 for max so that's probably why it's support
  UINT32 periodicity = 0;
  hr = client->lpVtbl->Initialize(client, AUDCLNT_SHAREMODE_SHARED, 0, audio_buffer, periodicity, format, NULL);
  if (FAILED(hr))
    return;

  // Start the audio stream
  hr = client->lpVtbl->Start(client);
  if (FAILED(hr))
    return;

  // Get the buffer
  INT32 *buffer;
  IAudioRenderClient *renderClient;
  const IID iid_IAudioRenderClient = IID_IAudioRenderClient;
  hr = client->lpVtbl->GetService(client, &iid_IAudioRenderClient, (void **)&renderClient);
  if (FAILED(hr))
    return;

  b8 stop = FALSE;
  // Start time at 0
  r64 time = 0.0;

  UINT32 max_frames;
  hr = client->lpVtbl->GetBufferSize(client, &max_frames);
  if (FAILED(hr))
    return;

  // So I think send sound data to audio every frame and make sure to check if the buffer is full with GetCurrentPadding
  while (!stop) {
    // Get the number of frames that have not yet been played
    UINT32 frames_used;
    IAudioClient_GetCurrentPadding(client, &frames_used);

    // Note: 2048 is what I randomly picked before but I'm guessing it should be format->nSamplesPerSec / fps
    // UINT32 frames = format->nSamplesPerSec / 144;
    // UINT32 frames = max_frames;
    //
    // if (frames > max_frames)
    //  frames = max_frames;

    // Note: Random size I'm choosing based on DAW, probably a better way to do this
    UINT32 frames = 2048;

    // Note: Keep some along this in mind because I may need calculation to make sure buffer is not overflowing
    // UINT32 buffer_space_used = frames_used * 10

    // Wait for audio to finish playing
    // Note: This is what it should be but I'm guessing buffer fill delay if (frames_used > bufferFrameCount) {
    // Note: frames seem to be gobbled up so I guess just pick something that isn't too crazy
    // Note: I don't want to be ahead of the audio by more than 1/4 of a second and make sure to check not buffered more than a second
    if (frames_used > format->nSamplesPerSec / 4 || (frames_used + frames) * format->nChannels * (format->wBitsPerSample / 8) > audio_buffer / 4)
      continue;

    // Get the buffer
    hr = renderClient->lpVtbl->GetBuffer(renderClient, frames, (BYTE **)&buffer);
    if (FAILED(hr)) {
      // Note: Overflowed give time to cool off
      if (hr == AUDCLNT_E_BUFFER_ERROR)
        continue;
      else if (hr == AUDCLNT_E_BUFFER_TOO_LARGE)
        return;
      else
        return;
    }
    r64 float_sample_rate = format->nSamplesPerSec;
    r64 seconds_per_frame = 1.0 / float_sample_rate;

    // Generate a sine wave and output it to the buffer
    r64 pitch = 220.0;
    r64 radians_per_second = pitch * 2.0 * M_PI;
    for (u32 j = 0; j < frames; j++) {
      // Generate a sample value using the sin() function
      // r32 frequency = MIN_FREQUENCY + (time * sweep_rate) * (MAX_FREQUENCY - MIN_FREQUENCY);
      // r32 phase = frequency * j / format->nSamplesPerSec * 2 * PI;
      // r32 sample = amplitude * sin(phase);

      r64 sample = sin((time + j * seconds_per_frame) * radians_per_second);

      // Note: Turns sine wave into square wave
      // if (sample > 0.5)
      //  sample = 1.0;
      // else
      //  sample = 0.0;

      for (u8 channel = 0; channel < format->nChannels; channel++) {
        if (format->wBitsPerSample == 8)
          ((INT8 *)buffer)[j * format->nChannels + channel] = (INT8)(sample * 127);
        else if (format->wBitsPerSample == 16)
          ((INT16 *)buffer)[j * format->nChannels + channel] = (INT16)(sample * 32767);
        else if (format->wBitsPerSample == 24) {
          INT32 sample24 = (INT32)(sample * 8388607);
          ((INT8 *)buffer)[(j * format->nChannels + channel) * 3 + 0] = (INT8)(sample24 & 0xFF);
          ((INT8 *)buffer)[(j * format->nChannels + channel) * 3 + 1] = (INT8)((sample24 >> 8) & 0xFF);
          ((INT8 *)buffer)[(j * format->nChannels + channel) * 3 + 2] = (INT8)((sample24 >> 16) & 0xFF);
        } else if (format->wBitsPerSample == 32)
          ((r32 *)buffer)[j * format->nChannels + channel] = (r32)(sample);
      }
    }

    // Release the buffer
    hr = renderClient->lpVtbl->ReleaseBuffer(renderClient, frames, 0);
    if (FAILED(hr))
      return;

    // time += (r32)frames / (r32)format->nSamplesPerSec;
    time += seconds_per_frame * frames;
    Sleep(6);
  }

  // Stop the audio stream
  hr = client->lpVtbl->Stop(client);
  if (FAILED(hr))
    return;

  // Clean up
  renderClient->lpVtbl->Release(renderClient);
  client->lpVtbl->Release(client);
  device->lpVtbl->Release(device);
  CoUninitialize();
}

int load_audio(char *file, struct AudioClip *clip) {
  char *wav_file = read_file(file);

  memcpy(&clip->header, wav_file, sizeof(struct WAVHeader));

  // Verify that the file is a valid WAV file
  if (strncmp((const char *)clip->header.chunk_id, "RIFF", 4) || strncmp((const char *)clip->header.format, "WAVE", 4)) {
    log_message(LOG_SEVERITY_WARNING, "Warning: Not a valid WAV file!\n");
    return 1;
  }

  // Allocate memory for samples
  if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 8) {
    clip->samples.int8 = (i8 *)malloc(clip->header.subchunk2_size);
    memcpy(clip->samples.int8, wav_file + sizeof(struct WAVHeader), clip->header.subchunk2_size);
  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 16) {
    clip->samples.int16 = (i16 *)malloc(clip->header.subchunk2_size);
    memcpy(clip->samples.int16, wav_file + sizeof(struct WAVHeader), clip->header.subchunk2_size);
  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 24) {
    clip->samples.int32 = (i32 *)malloc(clip->header.subchunk2_size);
    memcpy(clip->samples.int32, wav_file + sizeof(struct WAVHeader), clip->header.subchunk2_size);
    const size_t samples = (clip->header.subchunk2_size / (clip->header.bits_per_sample / 8)) / clip->header.num_channels;
    for (size_t sample = 0; sample < samples; sample++)
      clip->samples.int32[sample] >>= 8;
  } else if (clip->header.audio_format == 1 && clip->header.bits_per_sample == 32) {
    clip->samples.int32 = (i32 *)malloc(clip->header.subchunk2_size);
    memcpy(clip->samples.int8, wav_file + sizeof(struct WAVHeader), clip->header.subchunk2_size);
  } else if (clip->header.audio_format == 3 && clip->header.bits_per_sample == 32) {
    clip->samples.float32 = (r32 *)malloc(clip->header.subchunk2_size);
    memcpy(clip->samples.int8, wav_file + sizeof(struct WAVHeader), clip->header.subchunk2_size);
  }

  return 0;
}
*/
