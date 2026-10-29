// Sprites and the engine that draws them. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"

// A sprite run by an ANM script, th07's Sprite (0x24c there). Only the
// fields known so far.
class Sprite
{
public:
    unsigned char unk_0[0x208];
    // Where it is drawn.
    D3DXVECTOR3 pos;
    unsigned char unk_214[0x74];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    unsigned char unk_294[0x10];
};
ASSERT_SIZE(Sprite, 0x2a4);

// The sprite engine, th07's SpriteEngine. Only the fields known so far.
class SpriteEngine
{
public:
    // Copies surface `surface` to the back buffer at (`left`, `top`), as
    // th07's (0x466bb0).
    void CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y);
    // Releases surface `surface` (0x466ae0).
    void ReleaseSurface(int surface);
    // Draws `vm` at its position (0x463210).
    int Draw(Sprite *vm);
    // Frees ANM slot `slot` (0x466060).
    void ReleaseAnm(int slot);
    // Releases the vertex buffer (0x446f0b, in App's object).
    void ReleaseVertexBuffer();
    // Draws the batched quads (0x462e40).
    void FlushQuadBatch();

    IDirect3DSurface8 *GetSurface(int surface)
    {
        return this->surfaces[surface];
    }

    unsigned char unk_0[0x2038];
    IDirect3DSurface8 *surfaces[0x20];
    unsigned char unk_20b8[0x414];
    IDirect3DVertexBuffer8 *vertexBuffer;
};
extern SpriteEngine *g_SpriteEngine;

// The loading screen's sprites, right after g_App (0x17ceac0).
extern Sprite g_LoadingSprites[3];
