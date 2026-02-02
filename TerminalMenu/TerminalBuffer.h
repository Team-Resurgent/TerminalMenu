#pragma once

#include "External.h"

#include <string>

class TerminalBuffer
{
public:
    static void Clear();
    static void Write(int x, int y, std::string message, ...);
    static void ScrollUp();
    static const char* GetBuffer();
    static int GetCols();
    static int GetRows();
};
