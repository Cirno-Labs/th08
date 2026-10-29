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
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    unsigned char unk_0[0x1fe];
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    unsigned char unk_200[0x8];
    // Where it is drawn.
    D3DXVECTOR3 pos;
    unsigned char unk_214[0x6];
    // The script AnmLoaded::StartScript started.
    short scriptIndex;
    unsigned char unk_21c[0x6c];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    unsigned char unk_294[0x4];
    // Set to 0xf by AnmLoaded::StartScript.
    BYTE unk_298;
    BYTE unk_299;
    unsigned char unk_29a[0xa];
};
ASSERT_SIZE(Sprite, 0x2a4);

// An ANM file loaded by SpriteEngine::LoadAnm (0x465800), with its
// scripts. New in th08: th07 kept the loaded files in SpriteEngine.
class AnmLoaded
{
public:
    // Starts `script` on `vm` (0x45e580).
    void SetScript(Sprite *vm, void *script);
    // Starts script `script` of this file on `vm`, at the origin with
    // no offset. Inline, but too large to expand in /Ob1 files: they
    // call the copy at 0x40b910 and keep its temporaries' slots. That
    // copy stays unnamed in symbols.csv: ours comes from the /Os App
    // object, the original's from an /Od one.
    void StartScript(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->unk_299 = 0xf;
        vm->unk_298 = 0xf;
        this->SetScript(vm, this->scripts[script]);
    }

    short index;
    unsigned char unk_2[0xe];
    void **scripts;
};

class ScreenEffect;

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

    // Has CopyBackBufferToTexture's copy done after the frame is drawn,
    // as th07's, unless one is already pending: then returns -1, else 0.
    int SetPendingCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft, int dstTop,
                       int dstWidth, int dstHeight)
    {
        if (this->pendingCopyTexture >= 0)
        {
            return -1;
        }
        this->pendingCopyTexture = texture;
        this->pendingCopySrcLeft = srcLeft;
        this->pendingCopySrcTop = srcTop;
        this->pendingCopySrcWidth = srcWidth;
        this->pendingCopySrcHeight = srcHeight;
        this->pendingCopyDstLeft = dstLeft;
        this->pendingCopyDstTop = dstTop;
        this->pendingCopyDstWidth = dstWidth;
        this->pendingCopyDstHeight = dstHeight;
        return 0;
    }

    unsigned char unk_0[0x8];
    // The texture SetPendingCopy's copy goes to; negative for none. The
    // rest of the copy's parameters are at the end.
    int pendingCopyTexture;
    unsigned char unk_c[0x202c];
    IDirect3DSurface8 *surfaces[0x20];
    unsigned char unk_20b8[0x414];
    IDirect3DVertexBuffer8 *vertexBuffer;
    unsigned char unk_24d0[0x2a0080];
    int pendingCopySrcLeft;
    int pendingCopySrcTop;
    int pendingCopySrcWidth;
    int pendingCopySrcHeight;
    int pendingCopyDstLeft;
    int pendingCopyDstTop;
    int pendingCopyDstWidth;
    int pendingCopyDstHeight;
};
ASSERT_SIZE(SpriteEngine, 0x2a2570);
extern SpriteEngine *g_SpriteEngine;

// The loading screen's sprites, right after g_App (0x17ceac0).
extern Sprite g_LoadingSprites[3];
