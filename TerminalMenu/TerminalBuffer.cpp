#include "TerminalBuffer.h"

namespace
{
    char s_buffer[TerminalBuffer::Rows][TerminalBuffer::Cols];
}

void TerminalBuffer::Clear()
{
    for (int row = 0; row < Rows; row++)
    {
        for (int col = 0; col < Cols; col++)
        {
            s_buffer[row][col] = ' ';
        }
    }
}

void TerminalBuffer::Write(int x, int y, const char* text, bool wrap)
{
    if (!text || y < 0 || y >= Rows)
    {
        return;
    }

    int col = (x >= 0 && x < Cols) ? x : 0;
    int row = y;

    for (const char* p = text; *p; p++)
    {
        if (col >= Cols)
        {
            if (!wrap)
            {
                return;
            }
            col = 0;
            row++;
            if (row >= Rows)
            {
                return;
            }
        }
        if (col >= 0 && row >= 0 && row < Rows)
        {
            s_buffer[row][col] = *p;
        }
        col++;
    }
}

void TerminalBuffer::ScrollUp()
{
    for (int row = 0; row < Rows - 1; row++)
    {
        for (int col = 0; col < Cols; col++)
        {
            s_buffer[row][col] = s_buffer[row + 1][col];
        }
    }
    for (int col = 0; col < Cols; col++)
    {
        s_buffer[Rows - 1][col] = ' ';
    }
}

const char* TerminalBuffer::GetBuffer()
{
    return &s_buffer[0][0];
}

int TerminalBuffer::GetCols()
{
    return Cols;
}

int TerminalBuffer::GetRows()
{
    return Rows;
}
