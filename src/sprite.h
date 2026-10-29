// Sprites and the engine that draws them. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>
#include <d3dx8tex.h>

#include "common.h"

class AnmLoaded;

// An entry of a loaded ANM file's sprite list, th07's with the texture
// pointer added. Only the fields known so far.
struct SpriteData
{
    // The slot of the ANM file the sprite comes from.
    int sourceFileIndex;
    // The texture it is drawn from. New in th08.
    IDirect3DTexture8 *texture;
    // The sprite's rectangle in the texture, in pixels.
    float startX;
    float startY;
    float endX;
    float endY;
    // The texture's size.
    float textureHeight;
    float textureWidth;
    // The rectangle in texture coordinates.
    float uvStartX;
    float uvStartY;
    float uvEndX;
    float uvEndY;
    // Its size in pixels.
    float height;
    float width;
    // How much larger the texture is than the sprite's size: the
    // rectangle's size is divided by these.
    float scaleX;
    float scaleY;
    int unk_40;
};
ASSERT_SIZE(SpriteData, 0x44);

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
    unsigned char unk_21c[0x8];
    // The entry SetSprite chose.
    SpriteData *spriteData;
    unsigned char unk_228[0x60];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    unsigned char unk_294[0x4];
    // The size of the characters RenderText draws; 15 from
    // AnmLoaded::StartScriptAtOrigin.
    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char unk_29a[0xa];
};
ASSERT_SIZE(Sprite, 0x2a4);

// A vertex with a transformed position, a color and texture
// coordinates.
struct VertexTex1DiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};

// A vertex with a transformed position and texture coordinates.
struct VertexTex1Xyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DXVECTOR2 uv;
};

// A vertex with a position and texture coordinates.
struct VertexTex1Xyz
{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 uv;
};

// A vertex with a position, a color and texture coordinates.
struct VertexTex1DiffuseXyz
{
    D3DXVECTOR3 pos;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};

// An image stored in an ANM file, its pixels following the header, as
// th07's.
struct AnmImage
{
    unsigned char pad_0[0x6];
    // The pixels' format, an index into g_TextureFormats.
    short format;
    short width;
    short height;
    unsigned char pad_c[0x4];
    BYTE data[1];
};

// A sprite in an ANM file: its number and rectangle, in pixels.
struct AnmSprite
{
    int id;
    float x;
    float y;
    float width;
    float height;
};

// An ANM file's header, as th07's. The sprites' and then the scripts'
// offsets follow it.
struct AnmFile
{
    int numSprites;
    int numScripts;
    int textureIndex;
    int width;
    int height;
    int format;
    D3DCOLOR colorKey;
    // The image's file name, or "@" for a texture created blank.
    int nameOffset;
    int spriteOffset;
    int alphaNameOffset;
    // 3 in th08's files.
    int version;
    // The texture's resource priority.
    DWORD priority;
    // The image stored in the file, when hasImage is set.
    int imageOffset;
    BYTE hasImage;
    BYTE ownsData;
    unsigned char pad_36[0x2];
    // The offset of the next ANM in the same file, 0 for the last.
    int nextOffset;
    unsigned char pad_3c[0x4];
    int spriteOffsets[1];
};

// The texture of one ANM of a loaded file, and the image file it is
// created from. New in th08.
struct AnmTexture
{
    IDirect3DTexture8 *texture;
    void *data;
    int size;
};

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
        vm->fontHeight = 15;
        vm->fontWidth = 15;
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

    // Its slot in SpriteEngine::anms.
    int slot;
    // The file's data: the first of its chained ANMs.
    AnmFile *file;
    // How many ANMs the file chains.
    int anmCount;
    // The sprites and scripts of all of them, numbered from the first.
    SpriteData *sprites;
    void **scripts;
    // One texture per ANM.
    AnmTexture *textures;
    // While the textures are created a frame at a time: one more than
    // the number of ANMs done, 0 when all are.
    int loadStep;
};

class ScreenEffect;

// The sprite engine, th07's SpriteEngine. Only the fields known so far.
class SpriteEngine
{
public:
    // 0x465070.
    SpriteEngine();
    // Empty; folded with MmTimer::OnTimer (0x43ef40).
    ~SpriteEngine();

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
    // Releases the surfaces before the device is reset. Inline, defined
    // in the WinMain file.
    void ReleaseSurfaces();
    // Copies the back buffer's rectangle into texture `texture`, as
    // th07's (0x466f20).
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Copies the back buffer's rectangle into surface `surface`,
    // creating it at the destination size (0x467180). New in th08.
    void CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Does the pending copies, if any: the texture copy, then the surface
    // copy. Inline, defined in the WinMain file.
    void DoPendingCopy();
    // Releases the vertex buffer (0x446f0b, in App's object).
    void ReleaseVertexBuffer();
    // Starts batching quads for a frame, as th07's (0x462e00).
    void BeginQuadBatch();
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
    // `format`, or the 16-bit format to use instead of it when
    // g_App forces 16-bit textures. th07 did this in each texture loader.
    static int AdjustTextureFormat(int format);
    // Creates `tex`'s texture from its image file, in the format
    // g_TextureFormats[`format`], with `colorKey` transparent. Returns 0,
    // or -1 on failure.
    int LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey);
    // Creates `tex`'s texture from the pixels of `image`, in the format
    // g_TextureFormats[`format`]. Returns 0, or -1 on failure.
    int CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format);
    // Creates `tex`'s texture at `width` by `height` in the format
    // g_TextureFormats[`format`]. Returns 0.
    int CreateTexture(AnmTexture *tex, int width, int height, int format);

    IDirect3DSurface8 *GetSurface(int surface)
    {
        return this->surfaces[surface];
    }

    // Has a copy of the back buffer done after the frame is drawn, as
    // th07's, unless one is already pending: then returns -1, else 0. In
    // th08 this copy goes to surface `texture` (CopyBackBufferToSurface).
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
    // The surface SetPendingCopy's copy goes to; negative for none. The
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
    // The loaded ANM files, by slot.
    AnmLoaded anms[0x100];
    // The world matrix ProjectSprite3D last projected with.
    D3DXMATRIX lastWorldMatrix;
    Sprite unusedSprite1c64;
    unsigned char unk_1f08[0x130];
    // Images loaded as surfaces to copy to the back buffer from.
    IDirect3DSurface8 *surfaces[0x20];
    // System memory copies of them, to recreate them from.
    IDirect3DSurface8 *surfacesBackup[0x20];
    // The image files they are loaded from, and their sizes. New in th08.
    void *surfaceFiles[0x20];
    int surfaceFileSizes[0x20];
    // The images' sizes and formats.
    D3DXIMAGE_INFO surfaceInfos[0x20];
    // The texture factor last set on the device.
    D3DCOLOR currentColor;
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
    // The quad the vertex buffer holds, 256 pixels across.
    VertexTex1Xyz quad[4];
    // The number of quads batched to draw with FlushQuadBatch.
    int batchQuadCount;
    // The batched quads, two triangles each.
    VertexTex1DiffuseXyzrwh batchVertices[0x18000];
    // Where the next quad goes, and the first one not drawn yet.
    VertexTex1DiffuseXyzrwh *batchCursor;
    VertexTex1DiffuseXyzrwh *batchStart;
    // A pending CopyBackBufferToTexture, as th07's pendingCopy: the
    // texture, negative for none, then its other arguments. New in th08
    // next to the surface copy.
    int pendingTexCopyTexture;
    int pendingTexCopySrcLeft;
    int pendingTexCopySrcTop;
    int pendingTexCopySrcWidth;
    int pendingTexCopySrcHeight;
    int pendingTexCopyDstLeft;
    int pendingTexCopyDstTop;
    int pendingTexCopyDstWidth;
    int pendingTexCopyDstHeight;
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
// The quad Draw draws with a color, without the vertex buffer.
extern VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];
// A transformed quad without a color.
extern VertexTex1Xyzrwh g_Quad[4];
// CreateQuad's quad with a color, for drawing without the vertex
// buffer.
extern VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];
// The texture formats ANM files name by number.
extern D3DFORMAT g_TextureFormats[];
// The bytes per pixel of each of g_TextureFormats.
extern int g_TextureFormatSizes[];

// The loading screen's sprites, right after g_App (0x17ceac0).
extern Sprite g_LoadingSprites[3];
