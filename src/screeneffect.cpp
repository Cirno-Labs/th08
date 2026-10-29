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

int ScreenEffect::OnUpdateFadeIn(ScreenEffect *effect)
{
    if (effect->duration != 0)
    {
        effect->alpha = 255.0f - effect->timer.AsFloat() * 255.0f / effect->duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= effect->duration)
    {
        return 0;
    }
    effect->timer++;
    return 1;
}

void DrawColorRect(float *rect, D3DCOLOR color)
{
    g_SpriteEngine->FlushQuadBatch();
    VertexDiffuseXyzrwh vertices[4];

    vertices[0].pos = D3DXVECTOR3(rect[0], rect[1], 0.0f);
    vertices[1].pos = D3DXVECTOR3(rect[2], rect[1], 0.0f);
    vertices[2].pos = D3DXVECTOR3(rect[0], rect[3], 0.0f);
    vertices[3].pos = D3DXVECTOR3(rect[2], rect[3], 0.0f);
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = color;
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.GetNoDepthTest())
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

void DrawGradientRect(float *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft, D3DCOLOR bottomRight)
{
    g_SpriteEngine->FlushQuadBatch();
    VertexDiffuseXyzrwh vertices[4];

    vertices[0].pos = D3DXVECTOR3(rect[0], rect[1], 0.0f);
    vertices[1].pos = D3DXVECTOR3(rect[2], rect[1], 0.0f);
    vertices[2].pos = D3DXVECTOR3(rect[0], rect[3], 0.0f);
    vertices[3].pos = D3DXVECTOR3(rect[2], rect[3], 0.0f);
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    vertices[0].diffuse = topLeft;
    vertices[1].diffuse = topRight;
    vertices[2].diffuse = bottomLeft;
    vertices[3].diffuse = bottomRight;
    if (!g_App.GetNoColorBlending())
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.GetNoDepthTest())
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    if (!g_App.GetNoColorBlending())
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

int ScreenEffect::OnUpdateFadeOut(ScreenEffect *effect)
{
    if (g_Unknown18b8a2c != 0)
    {
        return 0;
    }
    if (effect->duration != 0)
    {
        effect->alpha = effect->timer.AsFloat() * 255.0f / effect->duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= effect->duration)
    {
        return 0;
    }
    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        effect->timer++;
    }
    return 1;
}

int ScreenEffect::OnUpdateLoadingFade(ScreenEffect *effect)
{
    if (!effect->finishing)
    {
        if (effect->duration != 0 && effect->timer <= effect->duration)
        {
            effect->alpha = effect->timer.AsFloat() * 128.0f / effect->duration;
        }
    }
    else if (effect->timer <= 8)
    {
        effect->alpha = 0x80 - (int)(effect->timer.AsFloat() * 128.0f / 8.0f);
    }
    else
    {
        return 0;
    }
    effect->timer++;
    return 1;
}
