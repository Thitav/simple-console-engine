#ifndef SCE_CONSOLE_H
#define SCE_CONSOLE_H

#include <stdbool.h>
#include <windows.h>
#include <stdint.h>

#define SCE_FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define SCE_BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define SCE_ASCII_FULL_BLOCK 0xDB

typedef struct
{
  char ch;
  uint16_t color;
} SCE_ConsoleCell;

typedef enum
{
  SCE_CONSOLE_BUFFER_OVERLAP_REPLACE,
  SCE_CONSOLE_BUFFER_OVERLAP_XOR,
} SCE_CONSOLE_BUFFER_OVERLAP_MODE;

typedef enum
{
  SCE_CONSOLE_BUFFER_CLIP_ERROR,
  SCE_CONSOLE_BUFFER_CLIP_WRAP,
} SCE_CONSOLE_BUFFER_CLIP_MODE;

typedef struct
{
  uint8_t overlap_mode: 4;
  uint8_t clip_mode: 4;
} SCE_ConsoleBufferAttributes;

typedef struct
{
  CHAR_INFO *buffer;
  uint16_t width;
  uint16_t height;
  SCE_ConsoleBufferAttributes attributes;
} SCE_ConsoleBuffer;

typedef struct
{
  bool pressed;
  bool held;
} SCE_ConsoleInputKey;

typedef struct
{
  HANDLE output_handle;
  HANDLE input_handle;
  SCE_ConsoleBuffer screen_buffer;
  SMALL_RECT window_rect;
  SCE_ConsoleInputKey input_keys[0xFF];
  // unsigned short int width;
  // unsigned short int height;
} SCE_Console;

bool sce_console_buffer_init(SCE_ConsoleBuffer *console_buffer, unsigned short int width,
                             unsigned short int height, SCE_ConsoleBufferAttributes attributes);

bool sce_console_buffer_set_cell(const SCE_ConsoleBuffer *console_buffer, uint16_t x,
                                 uint16_t y, SCE_ConsoleCell cell);


void sce_console_buffer_destroy(SCE_ConsoleBuffer *console_buffer);

bool sce_console_init(SCE_Console *console, uint16_t width, uint16_t height,
                      SCE_ConsoleBufferAttributes attributes);

bool sce_console_poll_events(SCE_Console *console);

void sce_console_destroy(SCE_Console *console);

inline void sce_console_buffer_clear(const SCE_ConsoleBuffer *console_buffer)
{
  memset(console_buffer->buffer, 0, console_buffer->width * console_buffer->height * sizeof(CHAR_INFO));
}

inline bool sce_console_set_cell(const SCE_Console *console, const uint16_t x, const uint16_t y,
                                 const SCE_ConsoleCell cell)
{
  return sce_console_buffer_set_cell(&console->screen_buffer, x, y, cell);
}

inline void sce_console_clear(const SCE_Console *console)
{
  sce_console_buffer_clear(&console->screen_buffer);
}

inline bool sce_console_render(const SCE_Console *console)
{
  return WriteConsoleOutputA(console->output_handle, console->screen_buffer.buffer,
                             (COORD){
                               (SHORT) console->screen_buffer.width, (SHORT) console->screen_buffer.height
                             },
                             (COORD){0, 0}, (PSMALL_RECT) &console->window_rect);
}

inline SCE_ConsoleInputKey sce_console_get_key(const SCE_Console *console, const uint8_t key)
{
  return console->input_keys[key];
}

#endif
