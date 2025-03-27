#ifndef SCE_BITMAP_H
#define SCE_BITMAP_H

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define SCE_BITMAP_SIZE_BYTES(size) (size / CHAR_BIT)

typedef uint8_t SCE_Bitmap;

inline uint8_t sce_bitmap_get(const SCE_Bitmap *bitmap, const uint32_t index)
{
  return bitmap[index / CHAR_BIT] >> index % CHAR_BIT & 1;
}

inline void sce_bitmap_set(SCE_Bitmap *bitmap, const uint32_t index)
{
  bitmap[index / CHAR_BIT] |= 1 << index % CHAR_BIT;
}

inline void sce_bitmap_clear(SCE_Bitmap *bitmap, const uint32_t index)
{
  bitmap[index / CHAR_BIT] &= ~(1 << index % CHAR_BIT);
}

#endif
