#include <xtl.h>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "context.h"
#include "drawing.h"
#include "Debug.h"
#include "inputManager.h"
#include "External.h"
#include "resources.h"
#include "ssfn.h"

#include <xgraphics.h>

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

typedef struct {
    DWORD dwWidth;
    DWORD dwHeight;
    BOOL  fProgressive;
    BOOL  fWideScreen;
	DWORD dwFreq;
} DISPLAY_MODE;

DISPLAY_MODE displayModes[] =
{
    //{   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    //{   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    //{   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    //{   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    //{   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    //{   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3


	// Width  Height Progressive Widescreen

	// HDTV Progressive Modes
    {  1280,    720,    TRUE,   TRUE,  60 },         // 1280x720 progressive 16x9

	// EDTV Progressive Modes
    {   720,    480,    TRUE,   TRUE,  60 },         // 720x480 progressive 16x9
    {   640,    480,    TRUE,   TRUE,  60 },         // 640x480 progressive 16x9
    {   720,    480,    TRUE,   FALSE, 60 },         // 720x480 progressive 4x3
    {   640,    480,    TRUE,   FALSE, 60 },         // 640x480 progressive 4x3

	// HDTV Interlaced Modes
	//    {  1920,   1080,    FALSE,  TRUE,  60 },         // 1920x1080 interlaced 16x9

	// SDTV PAL-50 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  50 },         // 720x480 interlaced 16x9 50Hz
    {   640,    480,    FALSE,  TRUE,  50 },         // 640x480 interlaced 16x9 50Hz
    {   720,    480,    FALSE,  FALSE, 50 },         // 720x480 interlaced 4x3  50Hz
    {   640,    480,    FALSE,  FALSE, 50 },         // 640x480 interlaced 4x3  50Hz

	// SDTV NTSC / PAL-60 Interlaced Modes
    {   720,    480,    FALSE,  TRUE,  60 },         // 720x480 interlaced 16x9
    {   640,    480,    FALSE,  TRUE,  60 },         // 640x480 interlaced 16x9
    {   720,    480,    FALSE,  FALSE, 60 },         // 720x480 interlaced 4x3
    {   640,    480,    FALSE,  FALSE, 60 },         // 640x480 interlaced 4x3
};

#define NUM_MODES (sizeof(displayModes) / sizeof(displayModes[0]))

bool supportsMode(DISPLAY_MODE mode, DWORD dwVideoStandard, DWORD dwVideoFlags)
{
    if (mode.dwFreq == 60 && !(dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz) && (dwVideoStandard == XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}    
    if (mode.dwFreq == 50 && (dwVideoStandard != XC_VIDEO_STANDARD_PAL_I))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fWideScreen && !(dwVideoFlags & XC_VIDEO_FLAGS_WIDESCREEN ))
	{
		return false;
	}
    if (mode.dwHeight == 480 && mode.fProgressive && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p))
	{
		return false;
	}
    if (mode.dwHeight == 720 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p))
	{
		return false;
	}
    if (mode.dwHeight == 1080 && !(dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i))
	{
		return false;
	}
    return true;
}

bool createDevice()
{
	uint32_t videoFlags = XGetVideoFlags();
	uint32_t videoStandard = XGetVideoStandard();
	uint32_t currentMode;
    for (currentMode = 0; currentMode < NUM_MODES-1; currentMode++)
    {
		if (supportsMode(displayModes[currentMode], videoStandard, videoFlags)) 
		{
			break;
		}
    } 

	LPDIRECT3D8 d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if(d3d == NULL)
	{
		Debug::Print("Failed to create d3d\n");
        return false;
	}

	context::setBufferWidth(720);
	context::setBufferHeight(480);
	//context::setBufferWidth(displayModes[currentMode].dwWidth);
	//context::setBufferHeight(displayModes[currentMode].dwHeight);

	D3DPRESENT_PARAMETERS params; 
    ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = displayModes[currentMode].dwWidth;
    params.BackBufferHeight = displayModes[currentMode].dwHeight;
	params.Flags = displayModes[currentMode].fProgressive ? D3DPRESENTFLAG_PROGRESSIVE : D3DPRESENTFLAG_INTERLACED;
    params.Flags |= displayModes[currentMode].fWideScreen ? D3DPRESENTFLAG_WIDESCREEN : 0;
    params.FullScreen_RefreshRateInHz = displayModes[currentMode].dwFreq;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	LPDIRECT3DDEVICE8 d3dDevice;
    if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &d3dDevice)))
	{
		Debug::Print("Failed to create device\n");
        return false;
	}
	context::setD3dDevice(d3dDevice);
    
    context::getD3dDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
	context::getD3dDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	context::getD3dDevice()->SetVertexShader(D3DFVF_CUSTOMVERTEX);
	context::getD3dDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	context::getD3dDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	context::getD3dDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    // Enable fog
    context::getD3dDevice()->SetRenderState(D3DRS_FOGENABLE, TRUE);
    context::getD3dDevice()->SetRenderState(D3DRS_FOGCOLOR, 0xff000000); // Black fog
    context::getD3dDevice()->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
    float fogStart = 7.0f;
    float fogEnd = 20.0f;
    context::getD3dDevice()->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fogStart);
    context::getD3dDevice()->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fogEnd);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    context::getD3dDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	context::getD3dDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

	context::getD3dDevice()->BeginScene();
	context::getD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
	context::getD3dDevice()->EndScene();
	context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);

	return true;
}

void __cdecl main()
{
	Debug::Print("Welcome to Hermes Menu\n");


	bool deviceCreated = createDevice();

	drawing::generateBitmapFont();


    while (TRUE)
    {
		inputManager::processController();

		drawing::clearBackground();
        
	    // Setup Ortho Camera for 2D effects

        D3DXMATRIX matIdentity;
        D3DXMatrixIdentity(&matIdentity);

        D3DXMATRIX matProjection;
	    D3DXMatrixOrthoOffCenterLH(&matProjection, 0, (float)context::getBufferWidth(), 0, (float)context::getBufferHeight(), 1.0f, 800.0f);

        context::getD3dDevice()->SetTransform(D3DTS_VIEW, &matIdentity);
        context::getD3dDevice()->SetTransform(D3DTS_WORLD, &matIdentity);
        context::getD3dDevice()->SetTransform(D3DTS_PROJECTION, &matProjection);
        context::getD3dDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        context::getD3dDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);


  

        // Present Rendered Results

		context::getD3dDevice()->EndScene();
		context::getD3dDevice()->Present(NULL, NULL, NULL, NULL);
    }
}