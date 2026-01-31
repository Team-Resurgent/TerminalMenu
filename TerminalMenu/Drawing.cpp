#include "Drawing.h"
#include "Resources.h"
#include <map>

#define SSFN_IMPLEMENTATION
#define SFFN_MAXLINES 8192
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free
#include "ssfn.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
} recti;

typedef struct {
    float x;
    float y;
    float width;
    float height;
} rectf;

#define LINE_HEIGHT 25
#define SPACING 25
#define FONT_TEXTURE_DIMENSION 1024

/* One vertex for terminal batch: XYZ + diffuse + UV (matches D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) */
struct terminal_vertex_t {
    float x, y, z;
    DWORD diffuse;
    float u, v;
};

/* Max terminal size for single draw; 80*40*6 vertices */
#define TERMINAL_MAX_COLS 80
#define TERMINAL_MAX_ROWS 40
#define TERMINAL_MAX_VERTS (TERMINAL_MAX_COLS * TERMINAL_MAX_ROWS * 6)

namespace
{
	ssfn_t* mFontContext = NULL;
    LPDIRECT3DDEVICE8 mD3dDevice;
    static DWORD s_bufferWidth;
    static DWORD s_bufferHeight;

    std::map<uint32_t, recti> charMap;

    int texture_width;
    int texture_height;
    D3DTexture* font_texture;

    int lineHeight;
    int spacing;

    static terminal_vertex_t s_terminalVerts[TERMINAL_MAX_VERTS];
}

void Drawing::SetD3dDevice(LPDIRECT3DDEVICE8 d3dDevice) {
    mD3dDevice = d3dDevice;
}

LPDIRECT3DDEVICE8 Drawing::GetD3dDevice() {
    return mD3dDevice;
}

void Drawing::SetBufferWidth(DWORD width) {
    s_bufferWidth = width;
}

void Drawing::SetBufferHeight(DWORD height) {
    s_bufferHeight = height;
}

DWORD Drawing::GetBufferWidth() {
    return s_bufferWidth;
}

DWORD Drawing::GetBufferHeight() {
    return s_bufferHeight;
}

void Drawing::Swizzle(const void* src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void* dest)
{
  for (UINT y = 0; y < height; y++)
  {
    UINT sy = 0;
    if (y < width)
    {
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
    }
    else
    {
      UINT y_mask = y % width;
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y_mask >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
      sy += (y / width) * width * width;
    }
    BYTE *s = (BYTE *)src + y * width * depth;
    for (UINT x = 0; x < width; x++)
    {
      UINT sx = 0;
      if (x < height * 2)
      {
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x >> bit) & 1) << (2*bit);
      }
      else
      {
        int x_mask = x % (2*height);
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x_mask >> bit) & 1) << (2*bit);
        sx += (x / (2 * height)) * 2 * height * height;
      }
      BYTE *d = (BYTE *)dest + (sx + sy)*depth;
      for (unsigned int i = 0; i < depth; ++i)
        *d++ = *s++;
    }
  }
}

void Drawing::CreateImage(uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	if (FAILED(D3DXCreateTexture(mD3dDevice, width, height, 1, 0, format, D3DPOOL_DEFAULT, &font_texture)))
	{
		return;
	}

	D3DSURFACE_DESC surfaceDesc;
	font_texture->GetLevelDesc(0, &surfaceDesc);

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(font_texture->LockRect(0, &lockedRect, NULL, 0)))
	{
		uint8_t* tempBuffer = (uint8_t*)malloc(surfaceDesc.Size);
		memset(tempBuffer, 0, surfaceDesc.Size);
		uint8_t* src = imageData;
		uint8_t* dst = tempBuffer;
		for (int32_t y = 0; y < height; y++)
		{
			memcpy(dst, src, width * 4);
			src += width * 4;
			dst += surfaceDesc.Width * 4;
		}
		Swizzle(tempBuffer, 4, surfaceDesc.Width, surfaceDesc.Height, lockedRect.pBits);
		free(tempBuffer);
		font_texture->UnlockRect(0);
	}
}

void Drawing::GenerateBitmapFont()
{
	if (mFontContext != NULL)
	{
        return;
    }

	mFontContext = (ssfn_t*)malloc(sizeof(ssfn_t)); 
    if (mFontContext == NULL)
    {
        return;
    }
	memset(mFontContext, 0, sizeof(ssfn_t));

	if (ssfn_load(mFontContext, &terminal_sfn[0]) != 0)
    {
        return;
    }

	ssfn_select(mFontContext, SSFN_FAMILY_ANY, "CascadiaCode", SSFN_STYLE_REGULAR, 25);

	int textureWidth = FONT_TEXTURE_DIMENSION;
	int textureHeight = FONT_TEXTURE_DIMENSION; 
	uint32_t* imageData = (uint32_t*)malloc(textureWidth * textureHeight * 4);
    if (imageData == NULL)
    {
        return;
    }

	memset(imageData, 0, textureWidth * textureHeight * 4);  

	int x = 2;
	int y = 2;

	char* charsToEncode = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	char* currentCharPos = charsToEncode;
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

        int bounds_x;
        int bounds_y;
        int bounds_width;
        int bounds_height;
        int ret = ssfn_bbox(mFontContext, currentChar, &bounds_width, &bounds_height, &bounds_x, &bounds_y);
		if (ret != 0)
		{
			continue;
		}

		if ((x + bounds_width + 2) > textureWidth)
		{
			x = 2;
			y = y + bounds_height + 2;
		}

		recti* rect = &charMap[unicode];
		rect->x = x;
		rect->y = y;
		rect->width = bounds_width;
		rect->height = bounds_height;

		ssfn_buf_t buffer; 
		memset(&buffer, 0, sizeof(buffer));
		buffer.ptr = (uint8_t*)imageData;
		buffer.x = x + bounds_x;
		buffer.y = y + bounds_y;
		buffer.w = textureWidth;
		buffer.h = textureHeight;
		buffer.p = textureWidth * 4;
		buffer.bg = 0;                                  /* transparent so texture has proper alpha mask */
		buffer.fg = 0xffffffff;                         /* white; SSFN writes coverage into alpha */   

		ssfn_render(mFontContext, &buffer, currentChar);

		x = x + bounds_width + 2;   
		free(currentChar);
	}

	CreateImage((uint8_t*)imageData, D3DFMT_A8R8G8B8, textureWidth, textureHeight);
	free(imageData);
}

void Drawing::Init()
{
    GenerateBitmapFont();
}

void Drawing::ClearBackground()
{
	mD3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
}

void Drawing::DrawTerminal(const char* buffer, int cols, int rows, uint32_t color)
{
    if (cols <= 0 || rows <= 0 || cols > TERMINAL_MAX_COLS || rows > TERMINAL_MAX_ROWS)
    {
        return;
    }

    const int cellW = Drawing::GetBufferWidth() / cols;
    const int cellH = Drawing::GetBufferHeight() / rows;
    const float bufH = (float)Drawing::GetBufferHeight();
    const float invDim = 1.0f / (float)FONT_TEXTURE_DIMENSION;

    terminal_vertex_t* v = s_terminalVerts;
    int nVerts = 0;

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            char c = buffer[row * cols + col];
            std::map<uint32_t, recti>::iterator it = charMap.find((uint32_t)(unsigned char)c);
            if (it == charMap.end())
                continue;

            const recti& r = it->second;
            float u0 = r.x * invDim;
            float v0 = r.y * invDim;
            float u1 = (r.x + r.width) * invDim;
            float v1 = (r.y + r.height) * invDim;

            float px = (float)(col * cellW) - 0.5f;
            float py = bufH - (float)((row + 1) * cellH) - 0.5f;
            float pz = 0.0f;
            float fw = (float)cellW;
            float fh = (float)cellH;

            v[0] = { px + fw, py + fh, pz, color, u1, v0 };
            v[1] = { px + fw, py,      pz, color, u1, v1 };
            v[2] = { px,      py,      pz, color, u0, v1 };
            v[3] = { px + fw, py + fh, pz, color, u1, v0 };
            v[4] = { px,      py,      pz, color, u0, v1 };
            v[5] = { px,      py + fh, pz, color, u0, v0 };
            v += 6;
            nVerts += 6;
        }
    }

    if (nVerts == 0)
    {
        return;
    }

    mD3dDevice->SetTexture(0, font_texture);
    mD3dDevice->DrawPrimitiveUP(
        D3DPT_TRIANGLELIST,
        nVerts / 3,
        s_terminalVerts,
        sizeof(terminal_vertex_t));
}
