// The file with ScreenEffect, from 0x45b020: th07's screeneffect.cpp, with
// StartEffect in the middle.

#include <string.h>

#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void ClearScreen(D3DCOLOR color)
{
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
}

void ClearViewport(D3DCOLOR color)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->FlushQuadBatch();
    }
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.viewport.MinZ = 0.0f;
    g_App.viewport.MaxZ = 1.0f;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    ClearScreen(color);
}
