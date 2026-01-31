#include "drawing.h"
#include "resources.h"
#include "context.h"
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

namespace
{
	ssfn_t* mFontContext = NULL;
    LPDIRECT3DDEVICE8 mD3dDevice;

    std::map<uint32_t, recti> charMap;

    int texture_width;
    int texture_height;
    D3DTexture* font_texture;

    int lineHeight;
    int spacing;
}

void drawing::setD3dDevice(LPDIRECT3DDEVICE8 d3dDevice) {
    mD3dDevice = d3dDevice;
}

LPDIRECT3DDEVICE8 drawing::getD3dDevice() {
    return mD3dDevice;
}

void drawing::swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest)
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

void drawing::createImage(uint8_t* imageData, D3DFORMAT format, int width, int height)
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
		swizzle(tempBuffer, 4, surfaceDesc.Width, surfaceDesc.Height, lockedRect.pBits);
		free(tempBuffer);
		font_texture->UnlockRect(0);
	}
}

void drawing::generateBitmapFont()
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

	ssfn_select(mFontContext, SSFN_FAMILY_ANY, "FreeSans", SSFN_STYLE_REGULAR, 25);

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
		buffer.bg = 0xffffffff;
		buffer.fg = 0xffffffff;   

		ssfn_render(mFontContext, &buffer, currentChar);

		x = x + bounds_width + 2;   
		free(currentChar);
	}

	createImage((uint8_t*)imageData, D3DFMT_A8R8G8B8, textureWidth, textureHeight);
	free(imageData);
}

void drawing::init()
{
    generateBitmapFont();
}

void drawing::clearBackground()
{
	mD3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff00ff00, 1.0f, 0L);
}

void drawing::drawBitmapString(std::string message, uint32_t color, int x, int y)
{
    int xPos = x;
    int yPos = y;

    const char* currentCharPos = message.c_str();

    while (*currentCharPos)
    {
        const char* nextCharPos = currentCharPos;
        uint32_t unicode = ssfn_utf8((char**)&nextCharPos);

        int32_t length = (int32_t)(nextCharPos - currentCharPos);
        currentCharPos = nextCharPos;

        // Newline handling
        if (length == 1 && unicode == '\n')
        {
            xPos = x;
            yPos += LINE_HEIGHT;
            continue;
        }

        // Lookup glyph (SAFE: no insertion)
        std::map<uint32_t, recti>::iterator it = charMap.find(unicode);
        if (it == charMap.end())
            continue;

        const recti& char_rect = it->second;

        // Build UVs
        math::rectF uvRect;
        uvRect.x      = char_rect.x / FONT_TEXTURE_DIMENSION;
        uvRect.y      = char_rect.y / FONT_TEXTURE_DIMENSION;
        uvRect.width  = char_rect.width  / FONT_TEXTURE_DIMENSION;
        uvRect.height = char_rect.height / FONT_TEXTURE_DIMENSION;

        mD3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, color);

        float newY = (float)(context::getBufferHeight())
                   - (yPos + char_rect.height);

        mD3dDevice->SetTexture(0, font_texture);

        float px = (float)(xPos) - 0.5f;
        float py = newY - 0.5f;
        float pz = 0.0f;
        float w = (float)(char_rect.width);
        float h = (float)(char_rect.height);
        // D3DFVF_XYZ | D3DFVF_TEX1: x, y, z, u, v per vertex (no diffuse)
        float u0 = uvRect.x, u1 = uvRect.x + uvRect.width;
        float v0 = uvRect.y, v1 = uvRect.y + uvRect.height;
        float coords[30] = {
            px + w, py + h, pz, u1, v0,  /* v3 */
            px + w, py,     pz, u1, v1,  /* v2 */
            px,     py,     pz, u0, v1,  /* v1 */
            px + w, py + h, pz, u1, v0,  /* v3 */
            px,     py,     pz, u0, v1,  /* v1 */
            px,     py + h, pz, u0, v0   /* v4 */
        };

        mD3dDevice->DrawPrimitiveUP(
            D3DPT_TRIANGLELIST,
            2,
            coords,
            5 * sizeof(float));

        xPos += char_rect.width + SPACING;
    }
}
