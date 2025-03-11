#include "library.h"

#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#define ASCII_FULL_BLOCK 0xDB

bool console_buffer_init(ConsoleBuffer *console_buffer, const unsigned short int width,
                         const unsigned short int height, const ConsoleBufferSettings settings)
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
	console_buffer->settings = settings;
	return true;
}

bool console_buffer_set_cell(const ConsoleBuffer *console_buffer, const unsigned short int x,
                             const unsigned short int y)
{
	CHAR_INFO *cell;
	if (x > console_buffer->width || y > console_buffer->height)
	{
		switch (console_buffer->settings.clip_mode)
		{
			case CLIP_ERROR:
				// error
				return false;
			case CLIP_WRAP:
				cell = &console_buffer->buffer[console_buffer->width * (y % console_buffer->height) +
				                               x % console_buffer->width];
				break;
			default:
				// error
				return false;
		}
	}
	else
	{
		cell = &console_buffer->buffer[console_buffer->width * y + x];
	}

	if (cell->Char.AsciiChar != 0)
	{
		switch (console_buffer->settings.overlap_mode)
		{
			case OVERLAP_REPLACE:
				*cell = (CHAR_INFO){
					.Char = console_buffer->settings.default_cell_char,
					.Attributes = console_buffer->settings.default_cell_color
				};
				break;
			case OVERLAP_XOR:
				cell->Char.AsciiChar = 0;
				break;
			default:
				// error
				return false;
		}
	}
	else
	{
		*cell = (CHAR_INFO){
			.Char = console_buffer->settings.default_cell_char,
			.Attributes = console_buffer->settings.default_cell_color
		};
	}

	return true;
}

void console_buffer_destroy(ConsoleBuffer *console_buffer)
{
	free(console_buffer->buffer);
	console_buffer->width = 0;
	console_buffer->height = 0;
	console_buffer->buffer = NULL;
}

bool console_init(Console *console, const unsigned short int width, const unsigned short int height,
                  const ConsoleBufferSettings settings)
{
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (console_handle == INVALID_HANDLE_VALUE)
	{
		// error
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

	ConsoleBuffer screen_buffer;
	if (!console_buffer_init(&screen_buffer, width, height, settings))
	{
		// error
		return false;
	}

	console->handle = console_handle;
	console->screen_buffer = screen_buffer;
	console->window_rect = window_rect;
	// console->width = width;
	// console->height = height;

	return true;
}

void console_set_cell(const Console *console, const unsigned short int x, const unsigned short int y)
{
	console_buffer_set_cell(&console->screen_buffer, x, y, console->settings.default_cell_char,
	                        console->settings.default_cell_color);
}

bool console_update(const Console *console)
{
	return WriteConsoleOutputA(console->handle, console->screen_buffer.buffer,
	                           (COORD){(SHORT) console->screen_buffer.width, (SHORT) console->screen_buffer.height},
	                           (COORD){0, 0}, (PSMALL_RECT) &console->window_rect);
}

void console_destroy(Console *console)
{
	console_buffer_destroy(&console->screen_buffer);
	// recover console
}
