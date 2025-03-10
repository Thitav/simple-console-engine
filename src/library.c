#include "library.h"

#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define ASCII_FULL_BLOCK 0xDB

bool console_init(Console *console, const unsigned short int width, const unsigned short int height)
{
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (console_handle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "[ERROR][CONSOLE_INIT] Failed to get console output handle\n");
		return false;
	}

	SMALL_RECT window_rect = {0, 0, 1, 1};
	if (!SetConsoleWindowInfo(console_handle, TRUE, &window_rect))
	{
		// error
		return false;
	}

	if (!SetConsoleScreenBufferSize(console_handle, (COORD){(SHORT) width, (SHORT) height}))
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
	if (!SetCurrentConsoleFontEx(console_handle, FALSE, &console_font))
	{
		// error
		return false;
	}

	const CONSOLE_CURSOR_INFO console_cursor = {
		.dwSize = 1,
		.bVisible = FALSE
	};
	if (!SetConsoleCursorInfo(console_handle, &console_cursor))
	{
		// error
		return false;
	}

	window_rect = (SMALL_RECT){0, 0, (SHORT) (width - 1), (SHORT) (height - 1)};
	if (!SetConsoleWindowInfo(console_handle, TRUE, &window_rect))
	{
		// error
		return false;
	}

	CHAR_INFO *screen_buffer = calloc(width * height, sizeof(CHAR_INFO));
	if (screen_buffer == NULL)
	{
		// error
		return false;
	}

	console->handle = console_handle;
	console->screen_buffer = screen_buffer;
	console->window_rect = window_rect;
	console->width = width;
	console->height = height;

	return true;
}

void console_set_pixel(const Console *console, const unsigned short int x, const unsigned short int y)
{
	console->screen_buffer[x * console->width + y] = (CHAR_INFO){
		.Char = ASCII_FULL_BLOCK, .Attributes = FOREGROUND_WHITE | BACKGROUND_WHITE
	};
}

void console_update(const Console *console)
{
	WriteConsoleOutputA(console->handle, console->screen_buffer,
	                    (COORD){(SHORT) console->width, (SHORT) console->height},
	                    (COORD){0, 0}, (PSMALL_RECT) &console->window_rect);
}

