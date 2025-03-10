#include <stdio.h>

#include "library.h"

int main(void)
{
  Console console;
  if (!console_init(&console, 88, 88))
  {
    // error
    return EXIT_FAILURE;
  }

  console_set_pixel(&console, 0, 0);
  console_set_pixel(&console, 1, 1);
  console_update(&console);

  while (1)
  {
  }

  return EXIT_SUCCESS;
}
