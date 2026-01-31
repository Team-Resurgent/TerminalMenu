#pragma once

#include <xtl.h>

class context
{
public:
    static void setBufferWidth(DWORD width);
    static void setBufferHeight(DWORD height);
    static DWORD getBufferWidth();
    static DWORD getBufferHeight();

    static void setD3dDevice(LPDIRECT3DDEVICE8 device);
    static LPDIRECT3DDEVICE8 getD3dDevice();


};
