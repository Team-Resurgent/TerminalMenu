#pragma once

#include "External.h"
#include "math.h"

#include <string>
#include <map>

class drawing
{
public:
    static void setD3dDevice(LPDIRECT3DDEVICE8 d3dDevice);
    static LPDIRECT3DDEVICE8 getD3dDevice();

    static void swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest);
    static void createImage(uint8_t* imageData, D3DFORMAT format, int width, int height);
    static void generateBitmapFont();
    static void init();
    static void clearBackground();
    static void drawBitmapString(std::string message, uint32_t color, int x, int y);

	//static image* createImage(uint8_t* imageData, D3DFORMAT format, int width, int height);
	//static void addImage(std::string key, uint8_t* imageData, D3DFORMAT format, int width, int height);
	//static bool loadImage(const uint8_t* buffer, uint32_t length, std::string key);
	//static bool loadFont(const uint8_t* data);
	
	//static bool imageExists(std::string key);
	//static image* getImage(std::string key);
	//static void setTint(unsigned int color);

	//static bitmapFont* generateBitmapFont(std::string fontName, int fontStyle, int fontSize, int lineHeight, int spacing, int textureDimension);
	//static void drawBitmapString(bitmapFont* font, std::string message, uint32_t color, int x, int y);
};
