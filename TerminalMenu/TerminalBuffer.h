#pragma once

#include "External.h"

class TerminalBuffer
{
public:
    static const int Cols = 40;
    static const int Rows = 30;

    static void Clear();
    static void Write(int x, int y, const char* text, bool wrap = true);
    static void ScrollUp();

    static const char* GetBuffer();
    static int GetCols();
    static int GetRows();
};
