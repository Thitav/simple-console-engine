#ifndef SCE_SPRITE_H
#define SCE_SPRITE_H

#include "console.h"

typedef struct
{
  SCE_ConsoleBuffer buffer;
  unsigned short int x;
  unsigned short int y;
} SCE_Sprite;

#endif
