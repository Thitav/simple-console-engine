#include "../include/audio.h"
#include <stdbool.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <stdio.h>
#include <string.h>
#include <audioclient.h>

#define REFERENCE_TIME_PER_SEC 10000000

void sce_audio_source_set_format()
{
}

uint32_t sce_audio_source_copy(SCE_AudioSource *audio_source, uint32_t frames_count, const uint8_t *destination,
                               DWORD *flags)
{
  uint32_t copy_frames_count;
  if (audio_source->free_frames < frames_count)
  {
    copy_frames_count = audio_source->free_frames;
  }
  else
  {
    copy_frames_count = frames_count;
  }

  if (copy_frames_count == 0)
  {
    *flags = AUDCLNT_BUFFERFLAGS_SILENT;
    return 0;
  }

  memcpy(&audio_source->frames[audio_source->current_frame], destination, copy_frames_count * audio_source->frame_size);
  audio_source->current_frame += copy_frames_count;
  audio_source->free_frames -= copy_frames_count;

  *flags = 0;
  return copy_frames_count;
}

void sce_audio_source_load_wav(SCE_AudioSource *audio_source, const char *filename)
{
  FILE *file = fopen(filename, "rb");

  char data[4];

  fread(data, sizeof(char), 4, file);
  if (strncmp(data, "RIFF", 4) != 0)
  {
    // error
    return;
  }

  fread(data, sizeof(char), 4, data); // not interested
  fread(data, sizeof(char), 4, data);
  if (strncmp(data, "WAVE", 4) != 0)
  {
    return;
  }
}

bool sce_audio_engine_init(SCE_AudioEngine *audio_engine)
{
  IMMDeviceEnumerator *device_enumerator = NULL;
  CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **) &device_enumerator);

  IMMDevice *device = NULL;
  device_enumerator->lpVtbl->GetDefaultAudioEndpoint(device_enumerator, eRender, eConsole, &device);

  IAudioClient *audio_client = NULL;
  device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void **) audio_client);

  WAVEFORMATEX *mix_format = NULL;
  audio_client->lpVtbl->GetMixFormat(audio_client, &mix_format);

  audio_client->lpVtbl->Initialize(audio_client, AUDCLNT_SHAREMODE_SHARED, 0, REFERENCE_TIME_PER_SEC, 0, mix_format,
                                   NULL);

  audio_engine->device_enumerator = device_enumerator;
  audio_engine->device = device;
  audio_engine->audio_client = audio_client;
  audio_engine->mix_format = mix_format;

  return true;
}

bool sce_audio_engine_play(SCE_AudioEngine *audio_engine, SCE_AudioSource *audio_source)
{
  // set audio source format
  IAudioClient *audio_client = audio_engine->audio_client;

  UINT32 buffer_frame_count;
  audio_client->lpVtbl->GetBufferSize(audio_client, &buffer_frame_count);

  IAudioRenderClient *render_client = NULL;
  audio_client->lpVtbl->GetService(audio_client, &IID_IAudioRenderClient, (void **) &render_client);

  BYTE *buffer_data;
  render_client->lpVtbl->GetBuffer(render_client, buffer_frame_count, &buffer_data);

  DWORD copy_flags;
  sce_audio_source_copy(audio_source, buffer_frame_count, buffer_data, &copy_flags);

  render_client->lpVtbl->ReleaseBuffer(render_client, buffer_frame_count, copy_flags);

  uint8_t i;
  for (i = 0; i < (uint8_t) sizeof(audio_engine->playing_sources); i++)
  {
    if (sce_bitmap_get(audio_engine->used_playing_sources, i) == 0)
    {
      break;
    }
  }

  audio_engine->playing_sources[i] = (SCE_AudioEnginePlayingSource){
    .audio_source = audio_source,
    .audio_client = audio_client,
    .render_client = render_client,
    .buffer_frame_count = buffer_frame_count
  };
  sce_bitmap_set(audio_engine->used_playing_sources, i);
}

bool sce_audio_engine_update(SCE_AudioEngine *audio_engine)
{
  UINT32 frames_padding;
  BYTE *buffer_data;
  DWORD copy_flags;

  for (uint8_t i = 0; i < (uint8_t) sizeof(audio_engine->playing_sources); i++)
  {
    if (sce_bitmap_get(audio_engine->used_playing_sources, i) == 0)
    {
      continue;
    }

    SCE_AudioEnginePlayingSource playing_source = audio_engine->playing_sources[i];

    playing_source.audio_client->lpVtbl->GetCurrentPadding(playing_source.audio_client, &frames_padding);
    UINT32 frames_available = playing_source.buffer_frame_count - frames_padding;
    playing_source.render_client->lpVtbl->GetBuffer(playing_source.render_client, frames_available, &buffer_data);
    sce_audio_source_copy(playing_source.audio_source, frames_available, buffer_data, &copy_flags);
    playing_source.render_client->lpVtbl->ReleaseBuffer(playing_source.render_client, frames_available, copy_flags);
    // remove source from playing sources if silent
  }
}
