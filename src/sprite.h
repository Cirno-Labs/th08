// Sprites and the engine that draws them. Only what is known so far.

#pragma once

#include <stddef.h>

#include <d3d8.h>
#include <d3dx8math.h>
#include <d3dx8tex.h>

#include "common.h"
#include "timer.h"

class AnmLoaded;

// Channel `c` scaled by `factor`, 0x80 for 1, up to 0xff, as th07's.
// Inline; the copy is at 0x462750.
inline BYTE ModulateChannel(BYTE c, BYTE factor)
{
    DWORD channel = (DWORD)c * factor >> 7;
    if (channel >= 0x100)
    {
        channel = 0xff;
    }
    return channel;
}

// Scales byte `i` of `color` by byte `i` of `factor`.
#define MODULATE_CHANNEL(color, factor, i) \
    ((BYTE *)&(color))[i] = ModulateChannel(((BYTE *)&(color))[i], ((BYTE *)&(factor))[i])

// An argument of an ANM instruction.
union AnmArg
{
    int i;
    float f;
    BYTE b[4];
};

// An instruction of an ANM script, as th07's.
struct AnmInstr
{
    short opcode;
    // The size of the instruction, arguments included.
    WORD size;
    // The frame of the script it runs on.
    short time;
    // Bit n: argument n names a script variable.
    WORD paramMask;
    AnmArg args[1];
};

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

    // Flag bit 0. Inline; the copy is at 0x4066e0.
    DWORD IsShown()
    {
        return this->flags & 1;
    }
    // Whether the script waits for an interrupt. Inline; the copy at
    // 0x4396f8 is the Gui object's.
    DWORD IsStopped()
    {
        return this->stopped;
    }
    // Inline; /Os files call the copy at 0x407120.
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    // Resets the sprite's state up to pos, as th07's: a scale of 1,
    // white, the identity matrix, flags 7. Inline; the copy is at
    // 0x4068e0.
    void Initialize()
    {
        memset(this, 0, offsetof(Sprite, pos));
        this->scaleY = 1.0f;
        this->scaleX = 1.0f;
        this->color = 0xffffffff;
        D3DXMatrixIdentity(&this->matrix);
        *(WORD *)&this->flags = 7;
        this->scriptTime.Initialize();
    }

    // The value of `value` as a float argument: the variable it stands
    // for, if any, or itself.
    float ResolveFloatArgument(float value);
    // The same for an int argument; float variables are truncated.
    int ResolveIntArgument(int value);
    // Where a float script argument is read from or written to: when bit
    // `bit` of `mask` is set and `*value` is 10004 to 10007, the float
    // variable it stands for, else `value`.
    float *ResolveFloatArgumentPtr(float *value, WORD mask, int bit);
    // The same for an int argument and the int variables.
    int *ResolveIntArgumentPtr(int *value, WORD mask, int bit);

    // The sprite's rotations about x, y and z, as th07's.
    float rotationX;
    float rotationY;
    float rotationZ;
    // The rotation speeds about x, y and z, which Step adds each frame.
    float rotationSpeedX;
    float rotationSpeedY;
    float rotationSpeedZ;
    // Draw scales the sprite's height and width by these, as th07's.
    float scaleY;
    float scaleX;
    // The scale speeds, which Step adds to scaleY and scaleX each frame.
    float scaleSpeedY;
    float scaleSpeedX;
    // The size the draw functions draw the sprite at before scaling;
    // th07's used spriteData->width and height.
    float spriteWidth;
    float spriteHeight;
    // Added to the sprite's texture coordinates.
    float uvScrollX;
    float uvScrollY;
    // The script's frame.
    Timer scriptTime;
    Timer waitTimer;
    // The interpolations' frames so far and their lengths: position,
    // color, alpha, rotation, scale, as th07's, then color2 and color2's
    // alpha, new in th08.
    Timer interpDuration[7];
    Timer interpFrame[7];
    // How Step eases each of them.
    BYTE interpEasing;
    BYTE colorInterpEasing;
    BYTE alphaInterpEasing;
    BYTE rotationInterpEasing;
    BYTE scaleInterpEasing;
    BYTE color2InterpEasing;
    BYTE color2AlphaInterpEasing;
    unsigned char pad_ff[0x1];
    // The script's variables: an argument of 10000 to 10009 stands for
    // one of them. 10000-10003 and 10008-10009 are ints, 10004-10007
    // floats.
    int scriptVar0;
    int scriptVar1;
    int scriptVar2;
    int scriptVar3;
    float scriptVarF0;
    float scriptVarF1;
    float scriptVarF2;
    float scriptVarF3;
    int scriptVar8;
    int scriptVar9;
    // The texture scroll speeds, added to uvScrollX and uvScrollY.
    float uvScrollSpeedX;
    float uvScrollSpeedY;
    D3DXMATRIX matrix;
    // A copy of matrix, scaled and rotated by ProjectSprite3D.
    D3DXMATRIX drawMatrix;
    // The sprite's size relative to its texture.
    D3DXMATRIX sizeMatrix;
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            // The alpha byte of color.
            BYTE alpha;
        };
    };
    // The color drawn instead of color while flag bit 17 is set.
    union
    {
        D3DCOLOR color2;
        struct
        {
            BYTE color2Rgb[3];
            BYTE color2Alpha;
        };
    };
    // Bit 1: shown; DrawUnrotated draws nothing without it.
    union
    {
        DWORD flags;
        struct
        {
            // Bits 0 and 1, for bitfield stores.
            DWORD shown : 1;
            DWORD visible : 1;
            DWORD unusedFlag2 : 1;
            DWORD unusedFlag3 : 1;
            DWORD blendMode : 2;
            DWORD unusedFlag6 : 1;
            DWORD unusedFlag7 : 1;
            // Step's position instructions move posOffset instead of
            // pos.
            DWORD usePosOffset : 1;
            DWORD flip : 2;
            DWORD anchor : 2;
            DWORD zWriteDisabled : 1;
            // Set while the script waits for an interrupt.
            DWORD stopped : 1;
            // SetRenderStateForSprite sets the camera up again when it
            // changes; Step's instruction 31 sets it.
            DWORD cameraSetup : 1;
            DWORD freezeMatrix : 1;
            DWORD useColor2 : 1;
            DWORD unusedFlag18 : 1;
            // Step does nothing while it is set. New in th08.
            DWORD scriptPaused : 1;
        };
    };
    // Whether the bullets' draw functions turn it along their angle, as
    // th07's.
    short autoRotate;
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    // Which of SpriteEngine's draw functions DrawSprite uses. New in
    // th08.
    int drawMode;
    // The ANM file whose script runs (AnmLoaded::StartScript).
    AnmLoaded *anm;
    // Where it is drawn.
    D3DXVECTOR3 pos;
    short spriteIndex;
    // The slot of the ANM file whose script runs. New in th08.
    short anmSlot;
    // The first sprite of the script, as th07's.
    short baseSpriteIndex;
    // The script AnmLoaded::StartScript started.
    short scriptIndex;
    // The script SetScript started, and the instruction Step runs next.
    AnmInstr *scriptStart;
    AnmInstr *scriptPos;
    // The entry SetSprite chose.
    SpriteData *spriteData;
    // Where an interrupt was taken, for instruction 89 to return to. New
    // in th08.
    Timer interruptReturnTime;
    AnmInstr *interruptReturnPos;
    D3DXVECTOR3 moveStart;
    D3DXVECTOR3 moveEnd;
    D3DXVECTOR3 rotateStart;
    D3DXVECTOR3 rotateEnd;
    D3DXVECTOR3 scaleTween;
    D3DXVECTOR3 colorTween;
    // color2's interpolation, from and to. New in th08.
    D3DCOLOR color2TweenStart;
    D3DCOLOR color2TweenEnd;
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    // The script frame of the last sprite change, as th07's.
    int spriteTime;
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

// A vertex with a transformed position and a color, as
// D3DFVF_XYZRHW | D3DFVF_DIFFUSE.
struct VertexDiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
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
    // Shows sprite `sprite` of this file on `vm`, as th07's
    // SpriteEngine::SetSprite (0x45e430). Returns 0, or -1 while the
    // file is not loaded.
    int SetSprite(Sprite *vm, int sprite);
    // Starts `script` on `vm` (0x45e580).
    void SetScript(Sprite *vm, void *script);
    // Starts script `script` + n of this file on sprite n of the
    // `count` sprites from `vms`, and makes their sprites their base
    // sprites, as th07's SpriteEngine::StartSpriteSeries.
    void StartSpriteSeries(Sprite *vms, int script, int count);
    // Copies `sprite` into sprite `index` and works out its texture
    // coordinates and size, as th07's SpriteEngine::LoadSprite.
    void LoadSprite(int index, SpriteData *sprite);
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
    // Initializes `vm` and shows sprite `sprite` of this file on it, as
    // th07's SpriteEngine::InitSprite. Inline; the copy is at 0x4068a0.
    void InitSprite(Sprite *vm, int sprite)
    {
        vm->Initialize();
        vm->anm = this;
        this->SetSprite(vm, sprite);
    }
    // Runs script `script` of this file on `vm`, as th07's
    // SpriteEngine::StartScript. Inline; the copy is at 0x4069f0.
    void StartScript(Sprite *vm, int script)
    {
        vm->anm = this;
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }

    // Sprite `sprite` of this file. Inline; the copy at 0x406880 is an
    // /Od object's.
    SpriteData *GetSprite(int sprite)
    {
        return &this->sprites[sprite];
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
    // Copies the `width` by `height` rectangle at (`left`, `top`) of
    // surface `surface` to the back buffer at (`x`, `y`), as th07's.
    void CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height);
    // Releases surface `surface` (0x466ae0).
    void ReleaseSurface(int surface);
    // Draws `vm` at its position (0x463210).
    int Draw(Sprite *vm);
    // Rotates (`x`, `y`) by the angle with `sine` and `cosine` and moves
    // it to (`cx`, `cy`), into `out`, as th07's.
    void RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy);
    // Draws `vm` unrotated, as th07's (0x462ff0).
    int DrawUnrotated(Sprite *vm);
    // DrawUnrotated without rounding to pixels, as th07's.
    int DrawNoRound(Sprite *vm);
    // Draw without rounding to pixels when it is not rotated. New in
    // th08.
    int DrawRotatedNoRound(Sprite *vm);
    // Draws `vm` at its 3D position projected to the screen, as th07's.
    // Sets g_DiffuseQuad to `vm` projected from its 3D position, rotated,
    // its size scaled by how far g_CameraSizeOffset from it reaches on
    // the screen, as th07's. Returns 0, or -1 when it is outside the
    // depth range.
    int ProjectSprite(Sprite *vm);
    int DrawProjected(Sprite *vm);
    // Draws `vm` in 3D, projected by ProjectSprite3D, as th07's.
    // Sets g_DiffuseQuad to the vertex buffer's quad transformed by
    // `vm`'s drawMatrix and its position, projected to the screen, as
    // th07's. Remakes drawMatrix first from matrix, the scale and the
    // rotations while flag 3 or 2 is set and flag 16 is not.
    void ProjectSprite3D(Sprite *vm);
    int DrawProjected3D(Sprite *vm);
    // ProjectSprite, calling `callback` with `vm` and its projected
    // position, if not NULL, before the quad is placed there: it can
    // move it. New in th08.
    int ProjectSpriteWithCallback(Sprite *vm, void (*callback)(Sprite *vm, D3DXVECTOR3 *pos));
    // Draws `vm` in 3D with the vertex buffer's quad, after the pending
    // batch, as th07's. The texture matrix is also set again while the
    // texture scrolls in x.
    int Draw3D(Sprite *vm);
    // DrawProjected with ProjectSpriteWithCallback. New in th08.
    int DrawProjectedWithCallback(Sprite *vm, void (*callback)(Sprite *vm, D3DXVECTOR3 *pos));
    // Draws `vm` with the draw function its drawMode picks: 0
    // DrawUnrotated, 1 DrawNoRound, 2 Draw, 3 DrawRotatedNoRound, 4
    // DrawProjected, 5 DrawProjected3D. New in th08.
    void DrawSprite(Sprite *vm);
    // Runs `vm`'s script for a frame, as th07's (0x45ea00).
    int Step(Sprite *vm);
    // Renders `text` into `texture`, its font sizes 15 when not
    // positive and scaled by `scaleX` and `scaleY`, as th07's; fonts of
    // 8 pixels or less are drawn 8 by 8 with RenderTextToTexture. Takes
    // the texture rather than its index in th08.
    void RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused, int fontHeight,
                                  int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor, char *text, float scaleX,
                                  float scaleY);
    // Renders printf-style text into `sprite`'s texture, as th07's
    // (0x4663b0).
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, aligned to the right of the sprite.
    void RenderTextRightAligned(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, centered on the sprite.
    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // Frees ANM slot `slot` (0x466060).
    void ReleaseAnm(int slot);
    // Releases `tex`'s texture and frees its image file.
    void ReleaseTexture(AnmTexture *tex);
    // Releases the surfaces before the device is reset. Inline, defined
    // in the WinMain file.
    void ReleaseSurfaces();
    // Copies the back buffer's rectangle into texture `texture`, as
    // th07's (0x466f20).
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Copies rectangle `srcRect` of texture `srcTexture` of ANM slot
    // `srcAnm` into rectangle `dstRect` of texture `dstTexture` of slot
    // `dstAnm`, as th07's.
    void CopyTextureRect(int dstAnm, int dstTexture, int srcAnm, int srcTexture, RECT *dstRect, RECT *srcRect);
    // Copies the back buffer's rectangle into surface `surface`,
    // creating it at the destination size (0x467180). New in th08.
    void CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Does the pending copies, if any: the texture copy, then the surface
    // copy. Inline, defined in the WinMain file.
    void DoPendingCopy();
    // Releases the vertex buffer (0x446f0b, in App's object).
    void ReleaseVertexBuffer();
    // Sets the render states `vm` wants on the device, as th07's.
    void SetRenderStateForSprite(Sprite *vm);
    // Sets the blend mode and z writes `vm` wants, as th07's; the z
    // writes through App::SetRenderState, which flushes the batch first.
    void SetBlendModeForSprite(Sprite *vm);
    // Adds g_DiffuseQuad to the batch with `vm`'s texture coordinates,
    // shaken, rounded to pixels with flag 1, with `vm`'s color unless
    // flag 2 is set, unless it is outside the viewport, as th07's.
    // Returns 0.
    int DrawQuad(Sprite *vm, int flags);
    // Adds `quad`, a triangle strip, to the batch as two triangles, as
    // th07's. Returns 0.
    int AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad);
    // Starts batching quads for a frame, as th07's (0x462e00).
    void BeginQuadBatch();
    // Draws the batched quads (0x462e40).
    void FlushQuadBatch();
    // Loads the image at `path` as surface `surface` (0x4667c0).
    int LoadSurface(int surface, char *path);
    // Reads the image file at `path` for LoadSurface to load surface
    // `surface` from later. Returns 0, or -1 on failure. New in th08.
    int PreloadSurface(int surface, char *path);
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
    // Reads the ANM file at `path` into slot `slot`, sets up its tables
    // and reads its image files ("::preloadAnim"). The textures are
    // created later, by PostloadAnm. Returns the slot's AnmLoaded, or
    // NULL on failure.
    AnmLoaded *PreloadAnmFile(int slot, char *path);
    // Checks `file`, ANM `index` of `anm`, and reads its image file
    // unless the image is in the ANM or made blank. Returns 1, or -1 on
    // failure.
    int PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file);
    // Creates the textures and sprites of the next ANM of `anm`
    // ("::postloadAnim"): one a call, counted by anm->loadStep, which
    // drops to 0 after the last. Returns `anm`, or NULL on failure.
    AnmLoaded *PostloadAnm(AnmLoaded *anm);
    // Creates the texture of `file`, ANM `index` of `anm`, and loads its
    // sprites from `spriteOffset` and scripts from `scriptOffset` on,
    // as th07's LoadAnmSprites. Returns 1, or -1 on failure.
    int LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file);

    IDirect3DSurface8 *GetSurface(int surface)
    {
        return this->surfaces[surface];
    }
    // Moves surface `src`, its backup and size to slot `dst`, releasing
    // what `dst` held, if `src` is loaded. New in th08; inline, the copy
    // is at 0x45a3fd.
    void MoveSurface(int dst, int src)
    {
        if (this->surfaces[src] != NULL)
        {
            this->ReleaseSurface(dst);
            this->surfaces[dst] = this->surfaces[src];
            this->surfacesBackup[dst] = this->surfacesBackup[src];
            this->surfaces[src] = NULL;
            this->surfacesBackup[src] = NULL;
            this->surfaceInfos[dst] = this->surfaceInfos[src];
        }
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
    // Whether `vm` shows a sprite of an ANM file whose textures are
    // loaded: th07's Sprite::HasTexture. Inline; the copy is at
    // 0x4622c0.
    BOOL HasTexture(Sprite *vm)
    {
        if (vm->spriteData == NULL)
        {
            return FALSE;
        }
        if (vm->spriteData->sourceFileIndex < 0)
        {
            return FALSE;
        }
        return this->anms[vm->spriteData->sourceFileIndex].textures != NULL;
    }
    // Sets the texture coordinates, color and w of the `count` vertices
    // from `vertices`, a strip across `vm`'s sprite from right to left:
    // even ones along its top, odd ones along its bottom, as th07's.
    // Returns 0, or -1 for fewer than 3.
    int FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // The same for a strip down the sprite from bottom to top: even
    // vertices along its left side, odd ones along its right. New in
    // th08.
    int FillVerticalStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Draws the `count` vertices from `vertices` as a triangle strip with
    // `vm`'s texture and blend mode, after the pending batch, as th07's;
    // the vertices' colors stand in for the texture factor until
    // SetRenderStateForSprite resets it. Returns 0, or -1 when `vm` is
    // hidden or transparent.
    int DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Adds `quad`, set up by the caller, to the batch with `vm`'s
    // texture and blend mode, as DrawQuad's end. Returns 0, or -1 when
    // `vm` is hidden or transparent. New in th08.
    int DrawVertexQuad(Sprite *vm, VertexTex1DiffuseXyzrwh *quad);
    // Draws the `count` vertices from `vertices` as an untextured
    // triangle fan in their own colors, with `vm`'s blend mode and no z
    // writes, after the pending batch, then has the next sprite set its
    // states again. Returns 0. New in th08.
    int DrawColoredFan(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count);
    // Sends interrupt `interrupt` to the `count` sprites from `vms` that
    // show a loaded sprite, as th07's.
    void SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt);
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
    // Set to have SetRenderStateForSprite set the texture factor as
    // texture stage 0's second color and alpha arguments again.
    BYTE resetTextureArgs;
    unsigned char unk_24c6[0x2];
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
// 0.5, for DrawQuad's rounding.
extern float g_Half;
// The offset from a sprite's position whose projection sets the size
// ProjectSprite draws it at.
extern D3DXVECTOR3 g_CameraSizeOffset;
// CreateQuad's quad with a color, for drawing without the vertex
// buffer.
extern VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];
// The texture formats ANM files name by number.
extern D3DFORMAT g_TextureFormats[];
// The bytes per pixel of each of g_TextureFormats.
extern int g_TextureFormatSizes[];

// The loading screen's sprites, right after g_App (0x17ceac0).
extern Sprite g_LoadingSprites[3];
