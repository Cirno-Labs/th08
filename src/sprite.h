// Sprites and the engine that draws them. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "common.h"

class AnmLoaded;

// A sprite run by an ANM script, th07's Sprite (0x24c there). Only the
// fields known so far.
class Sprite
{
public:
    // Out of line at 0x406720.
    Sprite();

    // Inline; /Os files call the copy at 0x407120.
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    unsigned char unk_0[0x1f0];
    D3DCOLOR color;
    unsigned char unk_1f4[0x4];
    // Bit 1: shown; DrawUnrotated draws nothing without it.
    DWORD flags;
    unsigned char unk_1fc[0x2];
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    unsigned char unk_200[0x4];
    // The ANM file whose script runs (AnmLoaded::StartScript).
    AnmLoaded *anm;
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
    // no offset, as th07's SpriteEngine::StartScriptAtOrigin. Inline, but
    // too large to expand in /Ob1 files: they call the copy at 0x40b910
    // and keep its temporaries' slots. That copy stays unnamed in
    // symbols.csv: ours comes from the /Os App object, the original's
    // from an /Od one.
    void StartScriptAtOrigin(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->unk_299 = 0xf;
        vm->unk_298 = 0xf;
        this->SetScript(vm, this->scripts[script]);
    }
    // Runs script `script` of this file on `vm`, as th07's
    // SpriteEngine::StartScript. Inline; the copy is at 0x4069f0.
    void StartScript(Sprite *vm, int script)
    {
        vm->anm = this;
        vm->scriptIndex = script;
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
    // Draws `vm` unrotated, as th07's (0x462ff0).
    int DrawUnrotated(Sprite *vm);
    // Runs `vm`'s script for a frame, as th07's (0x45ea00).
    int Step(Sprite *vm);
    // Renders printf-style text into `sprite`'s texture, as th07's
    // (0x4663b0).
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // Frees ANM slot `slot` (0x466060).
    void ReleaseAnm(int slot);
    // Releases the vertex buffer (0x446f0b, in App's object).
    void ReleaseVertexBuffer();
    // Draws the batched quads (0x462e40).
    void FlushQuadBatch();
    // Loads the image at `path` as surface `surface` (0x4667c0).
    int LoadSurface(int surface, char *path);
    // Loads the ANM file at `path` into slot `slot` (0x465800). Returns
    // it, or NULL on failure.
    AnmLoaded *LoadAnm(int slot, char *path);
    // LoadAnm for the sub thread: waits until the file is loaded or
    // g_App.closeSubThread is set, NULL then (0x465a40).
    AnmLoaded *PreloadAnm(int slot, char *path);
    // Creates the vertex buffer and the quads sprites are drawn with
    // (0x465250).
    void CreateQuad();

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

    // Forget the render state last set, as th07's, so that the next
    // sprite sets it again. Expanded under /Ob1.
    void ClearBlendMode()
    {
        this->blendMode = 3;
    }
    void ClearColorOp()
    {
        this->colorOp = 0xff;
    }
    void ClearVertexFormat()
    {
        this->vertexFormat = 0xff;
    }
    void ClearTexture()
    {
        this->currentTexture = NULL;
    }
    void ClearZWrite()
    {
        this->zWriteDisabled = 0xff;
    }
    void ClearSpriteData()
    {
        this->currentSpriteData = NULL;
    }
    void ClearUnk2e4d4()
    {
        this->lastCameraSetupFlag = 0xff;
    }
    void ClearUnk8()
    {
        this->stepCount = 0;
        this->renderStateSetCount = 0;
        this->scriptStartCount = 0;
        this->batchDrawCount = 0;
    }
    void ResetUnk0()
    {
        this->modulateColorEnabled = 0;
        this->modulateColor = 0x80808080;
    }
    // Runs `count` sprites from `vms` a frame, as th07's (0x462310).
    void StepSprites(Sprite *vms, int count);
    // Steps every ANM file still loading; returns -1 if one fails, else
    // 0 (0x466000).
    int StepAnmLoading();

    // The color sprites are modulated by while modulateColorEnabled is
    // set.
    D3DCOLOR modulateColor;
    int modulateColorEnabled;
    // The texture SetPendingCopy's copy goes to; negative for none. The
    // rest of the copy's parameters are at the end.
    int pendingCopyTexture;
    // Counters, cleared every frame.
    int scriptStartCount;
    int stepCount;
    int renderStateSetCount;
    int batchDrawCount;
    // How far the screen shakes.
    float shakeX;
    float shakeY;
    unsigned char unk_24[0x2014];
    IDirect3DSurface8 *surfaces[0x20];
    unsigned char unk_20b8[0x404];
    // The render state last set.
    IDirect3DTexture8 *currentTexture;
    BYTE blendMode;
    BYTE colorOp;
    BYTE vertexFormat;
    BYTE zWriteDisabled;
    BYTE lastCameraSetupFlag;
    unsigned char unk_24c5[0x3];
    void *currentSpriteData;
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
