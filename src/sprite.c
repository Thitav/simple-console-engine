#include "sprite.h"
#include <stdint.h>
#include "console.h"

void sce_sprite_init(SCE_Sprite *sprite, const uint16_t width, const uint16_t height)
{
  sce_console_buffer_init(&sprite->buffer, width, height,
                          SCE_CONSOLE_BUFFER_CLIP_ERROR | SCE_CONSOLE_BUFFER_OVERLAP_REPLACE);
}
