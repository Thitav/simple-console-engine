#include "../include/bitmap.h"
#include <stdint.h>

extern uint8_t sce_bitmap_get(const SCE_Bitmap *bitmap, uint32_t index);

extern void sce_bitmap_set(SCE_Bitmap *bitmap, uint32_t index);

extern void sce_bitmap_clear(SCE_Bitmap *bitmap, uint32_t index);
