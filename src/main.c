#include <stdio.h>

#include "library.h"

int main(void)
{
  Console console;
  if (!console_init(&console, 80, 80, (ConsoleBufferSettings)
                    {
                      BACKGROUND_RED | FOREGROUND_WHITE,
                      ASCII_FULL_BLOCK,
                      OVERLAP_XOR | CLIP_WRAP
                    }
    )
  )
  {
    // error
    return EXIT_FAILURE;
  }

  console_set_cell(&console, 0, 0);
  console_set_cell(&console, 80, 1);
  console_update(&console);

  while (1)
  {
  }

  return EXIT_SUCCESS;
}
