#ifndef SCE_SOUND_H
#define SCE_SOUND_H

#include <stdint.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "bitmap.h"

typedef uint8_t SCE_Frame;

typedef struct
{
  uint8_t channels;
  uint16_t frame_size;
  uint32_t current_frame;
  uint32_t free_frames;
  SCE_Frame *frames;
} SCE_AudioSource;

typedef struct
{
  SCE_AudioSource *audio_source;
  IAudioClient *audio_client;
  IAudioRenderClient *render_client;
  UINT32 buffer_frame_count;
} SCE_AudioEnginePlayingSource;

typedef struct
{
  IMMDeviceEnumerator *device_enumerator;
  IMMDevice *device;
  IAudioClient *audio_client;
  WAVEFORMATEX *mix_format;

  SCE_AudioEnginePlayingSource playing_sources[8];
  SCE_Bitmap used_playing_sources[1];
} SCE_AudioEngine;

#endif
