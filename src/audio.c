#include "../include/audio.h"
#include <stdbool.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <stdio.h>
#include <string.h>
#include <audioclient.h>
#include <limits.h>

#define REFERENCE_TIME_PER_SEC 10000000

void sce_audio_source_set_format()
{
}

uint32_t sce_audio_source_copy(const SCE_AudioSource *audio_source, const uint32_t frame_index,
                               const uint32_t frame_count,
                               const float *destination)
{
  uint32_t copied_frame_count = frame_count > audio_source->frame_count - frame_index
                                  ? audio_source->frame_count - frame_index
                                  : frame_count;

  memcpy(&audio_source->frame_data[frame_index], destination,
         copied_frame_count * audio_source->frame_size);
  return copied_frame_count;
}

void sce_audio_source_load_wav(SCE_AudioSource *audio_source, const char *filename)
{
  FILE *file = fopen(filename, "rb");

  char dump[4];

  fread(dump, sizeof(char), 4, file); // "RIFF"
  if (strncmp(dump, "RIFF", 4) != 0)
  {
    // error
    return;
  }

  fread(dump, sizeof(char), 4, file); // not interested
  fread(dump, sizeof(char), 4, file); // "WAVE"
  if (strncmp(dump, "WAVE", 4) != 0)
  {
    return;
  }

  WAVEFORMATEX wave_format;
  fread(dump, sizeof(char), 4, file);                                // "fmt "
  fread(dump, sizeof(char), 4, file);                                // not interested
  fread(&wave_format, sizeof(WAVEFORMATEX) - sizeof(WORD), 1, file); // wave format header

  // reading audio chunks
  int32_t chunk_size = 0;
  fread(dump, sizeof(char), 4, file);            // chunk header
  fread(&chunk_size, sizeof(uint32_t), 1, file); // chunk size
  while (strncmp(dump, "data", 4) != 0)
  {
    // skip non audio data
    fseek(file, chunk_size, SEEK_CUR);
    fread(dump, sizeof(char), 4, file);
    fread(&chunk_size, sizeof(long), 1, file);
  }

  // frame = 1 sample per channel
  const uint32_t sample_bits_max = (1 << wave_format.wBitsPerSample) - 1;
  const uint16_t sample_size = wave_format.wBitsPerSample / CHAR_BIT;
  const uint32_t frame_size = sample_size * wave_format.nChannels;
  const uint32_t sample_count = chunk_size / sample_size;
  const uint32_t frame_count = chunk_size / frame_size;
  int32_t sample_data;

  float *frame_data = malloc(sample_count * sizeof(float));

  for (uint32_t i = 0; i < frame_count; i++)
  {
    for (uint16_t j = 0; j < wave_format.nChannels; j++)
    {
      fread(&sample_data, sample_size, 1, file);
      frame_data[i * wave_format.nChannels + j] = (float) sample_data / (float) sample_bits_max;
    }
  }

  fclose(file);

  audio_source->channels = wave_format.nChannels;
  audio_source->frame_size = frame_size;
  audio_source->frame_count = frame_count;
  audio_source->frame_data = frame_data;
}

uint32_t sce_audio_playing_source_copy(SCE_AudioPlayingSource *playing_source, const uint32_t frame_count,
                                       const float *destination, DWORD *flags)
{
  uint32_t copied_frames_count = sce_audio_source_copy(playing_source->audio_source, playing_source->frame_index,
                                                       frame_count, destination);

  if (copied_frames_count == 0)
  {
    *flags = AUDCLNT_BUFFERFLAGS_SILENT;
  }
  else
  {
    playing_source->frame_index += copied_frames_count;
    *flags = 0;
  }

  return copied_frames_count;
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
  sce_audio_source_copy(audio_source, 0, buffer_frame_count, buffer_data, &copy_flags);

  render_client->lpVtbl->ReleaseBuffer(render_client, buffer_frame_count, copy_flags);

  uint8_t i;
  for (i = 0; i < (uint8_t) sizeof(audio_engine->playing_sources); i++)
  {
    if (sce_bitmap_get(audio_engine->used_playing_sources, i) == 0)
    {
      break;
    }
  }

  audio_engine->playing_sources[i] = (SCE_AudioPlayingSource){
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

    SCE_AudioPlayingSource playing_source = audio_engine->playing_sources[i];

    playing_source.audio_client->lpVtbl->GetCurrentPadding(playing_source.audio_client, &frames_padding);
    UINT32 frames_available = playing_source.buffer_frame_count - frames_padding;
    playing_source.render_client->lpVtbl->GetBuffer(playing_source.render_client, frames_available, &buffer_data);
    sce_audio_playing_source_copy(&playing_source, frames_available, (float *) buffer_data, &copy_flags);
    playing_source.render_client->lpVtbl->ReleaseBuffer(playing_source.render_client, frames_available, copy_flags);
    // remove source from playing sources if silent
  }
}
