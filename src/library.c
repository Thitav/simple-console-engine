#include "library.h"

#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#define INPUT_EVENTS_BATCH_SIZE 32

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
	if (x > console_buffer->width - 1 || y > console_buffer->height - 1)
	{
		switch (console_buffer->settings.attributes & 0xF0)
		{
			case CONSOLE_BUFFER_CLIP_ERROR:
				// error
				return false;
			case CONSOLE_BUFFER_CLIP_WRAP:
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
		switch (console_buffer->settings.attributes & 0x0F)
		{
			case CONSOLE_BUFFER_OVERLAP_REPLACE:
				*cell = (CHAR_INFO){
					.Char = console_buffer->settings.default_cell_char,
					.Attributes = console_buffer->settings.default_cell_color
				};
				break;
			case CONSOLE_BUFFER_OVERLAP_XOR:
				cell->Char.AsciiChar = 0;
				cell->Attributes = 0;
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

void console_buffer_clear(const ConsoleBuffer *console_buffer)
{
	memset(console_buffer->buffer, 0, console_buffer->width * console_buffer->height * sizeof(CHAR_INFO));
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

	ConsoleBuffer screen_buffer;
	if (!console_buffer_init(&screen_buffer, width, height, settings))
	{
		// error
		return false;
	}

	console->input_handle = input_handle;
	console->output_handle = output_handle;
	console->screen_buffer = screen_buffer;
	console->window_rect = window_rect;
	// console->width = width;
	// console->height = height;

	return true;
}

bool console_set_cell(const Console *console, const unsigned short int x, const unsigned short int y)
{
	return console_buffer_set_cell(&console->screen_buffer, x, y);
}

void console_clear(const Console *console)
{
	console_buffer_clear(&console->screen_buffer);
}

bool console_poll_events(Console *console)
{
	DWORD n_events;
	if (!GetNumberOfConsoleInputEvents(console->input_handle, &n_events))
	{
		// error
		return false;
	}

	INPUT_RECORD input_events[INPUT_EVENTS_BATCH_SIZE];

	// handle n_events > INPUT_EVENTS_BATCH_SIZE
	if (n_events > 0)
	{
		DWORD n_events_read = 0;
		if (!ReadConsoleInputA(console->input_handle, input_events, INPUT_EVENTS_BATCH_SIZE, &n_events_read))
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
			INPUT_RECORD event = input_events[i];
			switch (event.EventType)
			{
				case KEY_EVENT:
					printf("%d ", event.Event.KeyEvent.wVirtualKeyCode);
					break;
			}
			// ReadConsoleInputA();
		}
	}

	return true;
}

bool console_render(const Console *console)
{
	return WriteConsoleOutputA(console->output_handle, console->screen_buffer.buffer,
	                           (COORD){
		                           (SHORT) console->screen_buffer.width, (SHORT) console->screen_buffer.height
	                           },
	                           (COORD){0, 0}, (PSMALL_RECT) &console->window_rect);
}

void console_destroy(Console *console)
{
	console_buffer_destroy(&console->screen_buffer);
	// recover console
}
