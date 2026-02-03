#include "TerminalBuffer.h"
#include "Drawing.h"

namespace
{
    char* s_buffer = NULL;
    int s_cursor_x = 0;
    int s_cursor_y = 0;
}

void TerminalBuffer::Init()
{
    if (s_buffer != NULL)
    {
        return;
    }
    s_buffer = (char*)malloc(GetRows() * GetCols());
}

void TerminalBuffer::Clear()
{
    Init();

    for (int row = 0; row < GetRows(); row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[(row * GetCols()) + col] = ' ';
        }
    }
    s_cursor_x = 0;
    s_cursor_y = 0;
}

void TerminalBuffer::SetCursor(int x, int y)
{
    s_cursor_x = (x >= 0 && x < GetCols()) ? x : 0;
    s_cursor_y = (y >= 0 && y < GetRows()) ? y : 0;
}

void TerminalBuffer::Write(std::string message, ...)
{
    Init();

    char buffer[1024];
    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);
    buffer[1024 - 1] = '\0';

    for (const char* p = buffer; *p; p++)
    {
        if (*p == '\n')
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
            continue;
        }

        if (s_cursor_x >= GetCols())
        {
            s_cursor_x = 0;
            s_cursor_y++;
            if (s_cursor_y >= GetRows())
            {
                ScrollUp();
                s_cursor_y = GetRows() - 1;
            }
        }

        if (s_cursor_y >= 0 && s_cursor_y < GetRows() && s_cursor_x >= 0 && s_cursor_x < GetCols())
        {
            s_buffer[(s_cursor_y * GetCols()) + s_cursor_x] = *p;
        }
        s_cursor_x++;
    }

    Drawing::DrawTerminal(GetBuffer(), 0xff00ff00);
}

void TerminalBuffer::ScrollUp()
{
    Init();

    for (int row = 0; row < GetRows() - 1; row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[(row * GetCols()) + col] = s_buffer[((row + 1) * GetCols()) + col];
        }
    }
    for (int col = 0; col < GetCols(); col++)
    {
        s_buffer[((GetRows() - 1) * GetCols()) + col] = ' ';
    }
}

const char* TerminalBuffer::GetBuffer()
{
    return s_buffer;
}

int TerminalBuffer::GetCols()
{
    return Drawing::GetBufferWidth() / TERMINAL_FONT_SIZE_WIDTH;
}

int TerminalBuffer::GetRows()
{
    return Drawing::GetBufferHeight() / TERMINAL_FONT_SIZE_HEIGHT;
}

int TerminalBuffer::GetCursorX()
{
    return s_cursor_x;
}

int TerminalBuffer::GetCursorY()
{
    return s_cursor_y;
}
