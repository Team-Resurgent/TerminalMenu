#include "TerminalBuffer.h"
#include "Drawing.h"

namespace
{
    char s_buffer[255][255];
}

void TerminalBuffer::Clear()
{
    for (int row = 0; row < GetRows(); row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[row][col] = ' ';
        }
    }
}

void TerminalBuffer::Write(int x, int y, std::string message, ...)
{
    if (y < 0 || y >= GetRows())
    {
        return;
    }

    char buffer[1024];
    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);
    buffer[1024 - 1] = '\0';

    int col = (x >= 0 && x < GetCols()) ? x : 0;
    int row = y;

    for (const char* p = buffer; *p; p++)
    {
        if (col >= GetCols())
        {
            col = 0;
            row++;
            if (row >= GetRows())
            {
                return;
            }
        }
        if (col >= 0 && row >= 0 && row < GetRows())
        {
            s_buffer[row][col] = *p;
        }
        col++;
    }
}

void TerminalBuffer::ScrollUp()
{
    for (int row = 0; row < GetRows() - 1; row++)
    {
        for (int col = 0; col < GetCols(); col++)
        {
            s_buffer[row][col] = s_buffer[row + 1][col];
        }
    }
    for (int col = 0; col < GetCols(); col++)
    {
        s_buffer[GetRows() - 1][col] = ' ';
    }
}

const char* TerminalBuffer::GetBuffer()
{
    return &s_buffer[0][0];
}

int TerminalBuffer::GetCols()
{
    return Drawing::GetBufferWidth() / 16;
}

int TerminalBuffer::GetRows()
{
    return Drawing::GetBufferHeight() / 16;;
}
