#ifndef SCE_SOUND_H
#define SCE_SOUND_H

#include <stdint.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "bitmap.h"

typedef struct
{
  uint8_t channels;
  uint32_t frame_size;
  uint32_t frame_count;
  float *frame_data;
} SCE_AudioSource;

typedef struct
{
  SCE_AudioSource *audio_source;
  uint32_t frame_index;
  IAudioClient *audio_client;
  IAudioRenderClient *render_client;
  UINT32 buffer_frame_count;
} SCE_AudioPlayingSource;

typedef struct
{
  IMMDeviceEnumerator *device_enumerator;
  IMMDevice *device;
  IAudioClient *audio_client;
  WAVEFORMATEX *mix_format;

  SCE_AudioPlayingSource playing_sources[8];
  SCE_Bitmap used_playing_sources[1];
} SCE_AudioEngine;

#endif
