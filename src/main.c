#include <stdio.h>

#include "console.h"

int main(void)
{
  SCE_Console console;
  if (!sce_console_init(&console, 80, 80, SCE_CONSOLE_BUFFER_OVERLAP_XOR | SCE_CONSOLE_BUFFER_CLIP_WRAP)
  )
  {
    // error
    return EXIT_FAILURE;
  }

  sce_console_set_cell(&console, 0, 0,
                       (SCE_ConsoleCell){SCE_ASCII_FULL_BLOCK, SCE_FOREGROUND_WHITE | SCE_BACKGROUND_WHITE});
  sce_console_set_cell(&console, 80, 1,
                       (SCE_ConsoleCell){SCE_ASCII_FULL_BLOCK, SCE_FOREGROUND_WHITE | SCE_BACKGROUND_WHITE});
  sce_console_render(&console);

  while (1)
  {
    sce_console_poll_events(&console);
  }

  return EXIT_SUCCESS;
}
