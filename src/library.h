#ifndef WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H
#define WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H

#include <stdbool.h>
#include <windows.h>

typedef enum
{
  OVERLAP_REPLACE,
  OVERLAP_XOR
} CONSOLE_BUFFER_OVERLAP_MODE;

typedef enum
{
  CLIP_ERROR,
  CLIP_WRAP
} CONSOLE_BUFFER_CLIP_MODE;

typedef struct
{
  unsigned short int default_cell_color;
  char default_cell_char;
  CONSOLE_BUFFER_OVERLAP_MODE overlap_mode;
  CONSOLE_BUFFER_CLIP_MODE clip_mode;
} ConsoleBufferSettings;

typedef struct
{
  CHAR_INFO *buffer;
  unsigned short int width;
  unsigned short int height;
  ConsoleBufferSettings settings;
} ConsoleBuffer;

typedef struct
{
  HANDLE handle;
  ConsoleBuffer screen_buffer;
  SMALL_RECT window_rect;
  // unsigned short int width;
  // unsigned short int height;
} Console;

bool console_init(Console *console, unsigned short int width, unsigned short int height);

void console_set_pixel(const Console *console, unsigned short int x, unsigned short int y);

bool console_update(const Console *console);

#endif
