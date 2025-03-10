#ifndef WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H
#define WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H

#include <stdbool.h>
#include <windows.h>

typedef struct
{
  HANDLE handle;
  CHAR_INFO *screen_buffer;
  SMALL_RECT window_rect;
  unsigned short int width;
  unsigned short int height;
} Console;

bool console_init(Console *console, unsigned short int width, unsigned short int height);

void console_set_pixel(const Console *console, const unsigned short int x, const unsigned short int y);

void console_update(const Console *console);

#endif
