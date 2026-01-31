#include "context.h"

static DWORD s_bufferWidth;
static DWORD s_bufferHeight;
static LPDIRECT3DDEVICE8 s_d3dDevice;

void context::setBufferWidth(DWORD width)
{
    s_bufferWidth = width;
}

void context::setBufferHeight(DWORD height)
{
    s_bufferHeight = height;
}

DWORD context::getBufferWidth()
{
    return s_bufferWidth;
}

DWORD context::getBufferHeight()
{
    return s_bufferHeight;
}

void context::setD3dDevice(LPDIRECT3DDEVICE8 device)
{
    s_d3dDevice = device;
}

LPDIRECT3DDEVICE8 context::getD3dDevice()
{
    return s_d3dDevice;
}
