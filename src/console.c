#include "console.h"
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

#define INPUT_EVENTS_BATCH_SIZE 32

extern bool sce_console_set_cell(const SCE_Console *console, uint16_t x, uint16_t y, SCE_ConsoleCell cell);
extern bool sce_console_render(const SCE_Console *console);

bool sce_console_buffer_init(SCE_ConsoleBuffer *console_buffer, const uint16_t width,
                             const uint16_t height, const SCE_ConsoleBufferAttributes attributes)
{
  CHAR_INFO *buffer = calloc(width * height, sizeof(CHAR_INFO));
  if (buffer == NULL)
  {
    // error
    return false;
  }

  console_buffer->width = width;
  console_buffer->height = height;
  console_buffer->buffer = buffer;
  console_buffer->attributes = attributes;
  return true;
}

bool sce_console_buffer_set_cell(const SCE_ConsoleBuffer *console_buffer, const uint16_t x,
                                 const uint16_t y, const SCE_ConsoleCell cell)
{
  CHAR_INFO *selected_cell;
  if (x > console_buffer->width - 1 || y > console_buffer->height - 1)
  {
    switch (console_buffer->attributes & 0xF0)
    {
      case SCE_CONSOLE_BUFFER_CLIP_ERROR:
        // error
        return false;
      case SCE_CONSOLE_BUFFER_CLIP_WRAP:
        selected_cell = &console_buffer->buffer[console_buffer->width * (y % console_buffer->height) +
                                                x % console_buffer->width];
        break;
      default:
        // error
        return false;
    }
  }
  else
  {
    selected_cell = &console_buffer->buffer[console_buffer->width * y + x];
  }

  if (selected_cell->Char.AsciiChar != 0)
  {
    switch (console_buffer->attributes & 0x0F)
    {
      case SCE_CONSOLE_BUFFER_OVERLAP_REPLACE:
        selected_cell->Char.AsciiChar = cell.ch;
        selected_cell->Attributes = cell.color;
        break;
      case SCE_CONSOLE_BUFFER_OVERLAP_XOR:
        selected_cell->Char.AsciiChar = 0;
        selected_cell->Attributes = 0;
        break;
      default:
        // error
        return false;
    }
  }
  else
  {
    selected_cell->Char.AsciiChar = cell.ch;
    selected_cell->Attributes = cell.color;
  }

  return true;
}

void sce_console_buffer_destroy(SCE_ConsoleBuffer *console_buffer)
{
  free(console_buffer->buffer);
  console_buffer->width = 0;
  console_buffer->height = 0;
  console_buffer->buffer = NULL;
}

bool sce_console_init(SCE_Console *console, const uint16_t width, const uint16_t height,
                      const SCE_ConsoleBufferAttributes attributes)
{
  HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (input_handle == INVALID_HANDLE_VALUE || output_handle == INVALID_HANDLE_VALUE)
  {
    // error
    return false;
  }

  SMALL_RECT window_rect = {0, 0, 1, 1};
  if (!SetConsoleWindowInfo(output_handle, TRUE, &window_rect))
  {
    // error
    return false;
  }

  if (!SetConsoleScreenBufferSize(output_handle, (COORD){(SHORT) width, (SHORT) height}))
  {
    // error
    return false;
  }

  CONSOLE_FONT_INFOEX console_font = {
    .cbSize = sizeof(CONSOLE_FONT_INFOEX),
    .nFont = 0,
    .dwFontSize = {8, 8},
    .FontFamily = FF_DONTCARE,
    .FontWeight = FW_NORMAL
  };
  if (!SetCurrentConsoleFontEx(output_handle, FALSE, &console_font))
  {
    // error
    return false;
  }

  const CONSOLE_CURSOR_INFO console_cursor = {
    .dwSize = 1,
    .bVisible = FALSE
  };
  if (!SetConsoleCursorInfo(output_handle, &console_cursor))
  {
    // error
    return false;
  }

  window_rect = (SMALL_RECT){0, 0, (SHORT) (width - 1), (SHORT) (height - 1)};
  if (!SetConsoleWindowInfo(output_handle, TRUE, &window_rect))
  {
    // error
    return false;
  }

  SCE_ConsoleBuffer screen_buffer;
  if (!sce_console_buffer_init(&screen_buffer, width, height, attributes))
  {
    // error
    return false;
  }

  memset(console->input_keys, 0, sizeof(console->input_keys));

  console->input_handle = input_handle;
  console->output_handle = output_handle;
  console->screen_buffer = screen_buffer;
  console->window_rect = window_rect;
  // console->width = width;
  // console->height = height;

  return true;
}

bool sce_console_poll_events(SCE_Console *console)
{
  DWORD n_events = 0;
  if (!GetNumberOfConsoleInputEvents(console->input_handle, &n_events))
  {
    // error
    return false;
  }

  // handle n_events > INPUT_EVENTS_BATCH_SIZE
  if (n_events > 0)
  {
    INPUT_RECORD input_events[INPUT_EVENTS_BATCH_SIZE];
    DWORD n_events_read = 0;

    if (!ReadConsoleInput(console->input_handle, input_events, INPUT_EVENTS_BATCH_SIZE, &n_events_read))
    {
      // error
      return false;
    }
    if (n_events_read != n_events)
    {
      // error
      return false;
    }

    for (DWORD i = 0; i < n_events; i++)
    {
      const INPUT_RECORD event = input_events[i];
      switch (event.EventType)
      {
        case KEY_EVENT:
          const KEY_EVENT_RECORD key_event = event.Event.KeyEvent;
          SCE_ConsoleInputKey *input_key = &console->input_keys[key_event.wVirtualKeyCode];

          input_key->held = key_event.bKeyDown && (input_key->pressed || key_event.wRepeatCount > 1);
          input_key->pressed = key_event.bKeyDown;

          break;
        default:
          return false;
      }
    }
  }
  return true;
}

void sce_console_destroy(SCE_Console *console)
{
  sce_console_buffer_destroy(&console->screen_buffer);
  // recover console
}
