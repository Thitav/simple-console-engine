#ifndef WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H
#define WINDOWS_CONSOLE_GRAPHICS_LIBRARY_H

#include <stdbool.h>
#include <windows.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define ASCII_FULL_BLOCK 0xDB

typedef enum
{
  OVERLAP_REPLACE = 0x01,
  OVERLAP_XOR = 0x02,
} CONSOLE_BUFFER_OVERLAP_MODE;

typedef enum
{
  CLIP_ERROR = 0x10,
  CLIP_WRAP = 0x20,
} CONSOLE_BUFFER_CLIP_MODE;

typedef unsigned char CONSOLE_BUFFER_ATTRIBUTES;

typedef struct
{
  unsigned short int default_cell_color;
  char default_cell_char;
  CONSOLE_BUFFER_ATTRIBUTES attributes;
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

bool console_init(Console *console, unsigned short int width, unsigned short int height,
                  ConsoleBufferSettings settings);

bool console_set_cell(const Console *console, unsigned short int x, unsigned short int y);

bool console_update(const Console *console);

#endif
