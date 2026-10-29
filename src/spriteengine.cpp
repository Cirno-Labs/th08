// The file with SpriteEngine, from 0x45e580: th07's spriteengine.cpp, in
// a different order. The script interpreter and drawing come first, then
// the constructor and the loading and copying from 0x465070.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sprite.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// that header.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

void AnmLoaded::SetScript(Sprite *vm, void *script)
{
    if (script == NULL || this->loadStep != 0)
    {
        memset(vm, 0, sizeof(Sprite));
    }
    else
    {
        vm->Initialize();
        vm->anmSlot = this->slot;
        vm->anm = this;
        vm->flags &= ~0x600;
        vm->scriptStart = (AnmInstr *)script;
        vm->scriptPos = vm->scriptStart;
        vm->scriptTime = 0;
        vm->flags &= ~1;
        g_SpriteEngine->Step(vm);
        g_SpriteEngine->scriptStartCount++;
    }
}

float Sprite::ResolveFloatArgument(float value)
{
    switch ((int)value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    }
    return value;
}

int Sprite::ResolveIntArgument(int value)
{
    switch (value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    }
    return value;
}

float *Sprite::ResolveFloatArgumentPtr(float *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch ((int)*value)
    {
    case 10004:
        return &this->scriptVarF0;
    case 10005:
        return &this->scriptVarF1;
    case 10006:
        return &this->scriptVarF2;
    case 10007:
        return &this->scriptVarF3;
    }
    return value;
}

int *Sprite::ResolveIntArgumentPtr(int *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch (*value)
    {
    case 10000:
        return &this->scriptVar0;
    case 10001:
        return &this->scriptVar1;
    case 10002:
        return &this->scriptVar2;
    case 10003:
        return &this->scriptVar3;
    case 10008:
        return &this->scriptVar8;
    case 10009:
        return &this->scriptVar9;
    }
    return value;
}

void SpriteEngine::DrawSprite(Sprite *vm)
{
    switch (vm->drawMode)
    {
    case 0:
        this->DrawUnrotated(vm);
        break;
    case 1:
        this->DrawNoRound(vm);
        break;
    case 2:
        this->Draw(vm);
        break;
    case 3:
        this->DrawRotatedNoRound(vm);
        break;
    case 4:
        this->DrawProjected(vm);
        break;
    case 5:
        this->DrawProjected3D(vm);
        break;
    }
}

void SpriteEngine::SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt)
{
    while (count != 0)
    {
        if (g_SpriteEngine->HasTexture(vms))
        {
            vms->SetPendingInterrupt(interrupt);
        }
        vms++;
        count--;
    }
}

void SpriteEngine::StepSprites(Sprite *vms, int count)
{
    while (count != 0)
    {
        if (vms->scriptIndex >= 0)
        {
            g_SpriteEngine->Step(vms);
        }
        vms++;
        count--;
    }
}

SpriteEngine::SpriteEngine()
{
    memset(this, 0, sizeof(*this));
    g_Quad[0].w = g_Quad[1].w = g_Quad[2].w = g_Quad[3].w = 1.0f;
    g_Quad[0].uv.x = 0.0f;
    g_Quad[0].uv.y = 0.0f;
    g_Quad[1].uv.x = 1.0f;
    g_Quad[1].uv.y = 0.0f;
    g_Quad[2].uv.x = 0.0f;
    g_Quad[2].uv.y = 1.0f;
    g_Quad[3].uv.x = 1.0f;
    g_Quad[3].uv.y = 1.0f;
    g_DiffuseQuad[0].w = g_DiffuseQuad[1].w = g_DiffuseQuad[2].w = g_DiffuseQuad[3].w = 1.0f;
    g_DiffuseQuad[0].uv.x = 0.0f;
    g_DiffuseQuad[0].uv.y = 0.0f;
    g_DiffuseQuad[1].uv.x = 1.0f;
    g_DiffuseQuad[1].uv.y = 0.0f;
    g_DiffuseQuad[2].uv.x = 0.0f;
    g_DiffuseQuad[2].uv.y = 1.0f;
    g_DiffuseQuad[3].uv.x = 1.0f;
    g_DiffuseQuad[3].uv.y = 1.0f;
    this->vertexBuffer = NULL;
    this->currentTexture = NULL;
    this->blendMode = 0;
    this->colorOp = 0;
    this->currentColor = 1;
    this->vertexFormat = 0;
    this->lastCameraSetupFlag = 0xff;
    this->zWriteDisabled = 0;
    this->pendingTexCopyTexture = -1;
    this->pendingCopyTexture = -1;
}

void SpriteEngine::CreateQuad()
{
    VertexTex1Xyz *data;

    this->quad[0].pos.x = this->quad[2].pos.x = -128.0f;
    this->quad[1].pos.x = this->quad[3].pos.x = 128.0f;
    this->quad[0].pos.y = this->quad[1].pos.y = -128.0f;
    this->quad[2].pos.y = this->quad[3].pos.y = 128.0f;
    this->quad[0].pos.z = this->quad[1].pos.z = this->quad[2].pos.z = this->quad[3].pos.z = 0.0f;
    this->quad[0].uv.x = this->quad[2].uv.x = 0.0f;
    this->quad[1].uv.x = this->quad[3].uv.x = 1.0f;
    this->quad[0].uv.y = this->quad[1].uv.y = 0.0f;
    this->quad[2].uv.y = this->quad[3].uv.y = 1.0f;
    g_DiffuseQuadXyz[0].pos = this->quad[0].pos;
    g_DiffuseQuadXyz[1].pos = this->quad[1].pos;
    g_DiffuseQuadXyz[2].pos = this->quad[2].pos;
    g_DiffuseQuadXyz[3].pos = this->quad[3].pos;
    g_DiffuseQuadXyz[0].uv.x = this->quad[0].uv.x;
    g_DiffuseQuadXyz[0].uv.y = this->quad[0].uv.y;
    g_DiffuseQuadXyz[1].uv.x = this->quad[1].uv.x;
    g_DiffuseQuadXyz[1].uv.y = this->quad[1].uv.y;
    g_DiffuseQuadXyz[2].uv.x = this->quad[2].uv.x;
    g_DiffuseQuadXyz[2].uv.y = this->quad[2].uv.y;
    g_DiffuseQuadXyz[3].uv.x = this->quad[3].uv.x;
    g_DiffuseQuadXyz[3].uv.y = this->quad[3].uv.y;
    if (!g_App.GetNoVertexBuffer())
    {
        g_App.d3dDevice->CreateVertexBuffer(sizeof(this->quad), 0, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_MANAGED,
                                            &this->vertexBuffer);
        this->vertexBuffer->Lock(0, 0, (BYTE **)&data, 0);
        memcpy(data, this->quad, sizeof(this->quad));
        this->vertexBuffer->Unlock();
        g_App.d3dDevice->SetStreamSource(0, g_SpriteEngine->vertexBuffer, sizeof(VertexTex1Xyz));
    }
}

int SpriteEngine::AdjustTextureFormat(int format)
{
    if (g_App.GetForce16BitTextures())
    {
        if (g_TextureFormats[format] == D3DFMT_A8R8G8B8 || g_TextureFormats[format] == D3DFMT_UNKNOWN)
        {
            format = 5;
        }
        else if (g_TextureFormats[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }
    return format;
}

int SpriteEngine::LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey)
{
    format = AdjustTextureFormat(format);
    if (D3DXCreateTextureFromFileInMemoryEx(g_App.d3dDevice, tex->data, tex->size, 0, 0, 0, 0, g_TextureFormats[format],
                                            D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &tex->texture) != D3D_OK)
    {
        return -1;
    }
    return 0;
}

int SpriteEngine::CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format)
{
    BYTE *dstRow;
    BYTE *srcRow;
    int y;
    D3DLOCKED_RECT rect;
    AnmImage *img;
    IDirect3DSurface8 *level;
    IDirect3DSurface8 *srcSurface;

    srcSurface = NULL;
    level = NULL;
    format = AdjustTextureFormat(format);
    img = image;
    g_App.d3dDevice->CreateImageSurface(img->width, img->height, g_TextureFormats[img->format], &srcSurface);
    srcSurface->LockRect(&rect, NULL, 0);
    for (y = 0; y < img->height; y++)
    {
        dstRow = (BYTE *)rect.pBits + y * rect.Pitch;
        srcRow = &image->data[y * img->width * g_TextureFormatSizes[img->format]];
        memcpy(dstRow, srcRow, img->width * g_TextureFormatSizes[img->format]);
    }
    srcSurface->UnlockRect();
    if (D3DXCreateTexture(g_App.d3dDevice, img->width, img->height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED,
                          &tex->texture) != D3D_OK)
    {
        goto fail;
    }
    tex->texture->GetSurfaceLevel(0, &level);
    if (D3DXLoadSurfaceFromSurface(level, NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_LINEAR, 0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return -1;
}

int SpriteEngine::CreateTexture(AnmTexture *tex, int width, int height, int format)
{
    D3DXCreateTexture(g_App.d3dDevice, width, height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED, &tex->texture);
    return 0;
}

AnmLoaded *SpriteEngine::LoadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    DebugPrint("::loadAnim : %s\n", path);
    anm = this->PreloadAnmFile(slot, path);
    if (anm != NULL)
    {
        anm->loadStep = 1;
        while (anm->loadStep != 0)
        {
            anm = this->PostloadAnm(anm);
        }
    }
    return anm;
}

AnmLoaded *SpriteEngine::PreloadAnmFile(int slot, char *path)
{
    AnmFile *header;
    int numScripts;
    int result;
    AnmFile *data;
    AnmLoaded *anm;
    int anmCount;
    int spriteCount;
    int i;

    DebugPrint("::preloadAnim : %s\n", path);
    if (slot >= 0x19)
    {
        LogError(&g_Log, "テクスチャ格納先が足りません\r\n");
        return NULL;
    }
    this->ReleaseAnm(slot);
    data = (AnmFile *)LoadFileData(path, NULL, 0);
    anmCount = 0;
    numScripts = 0;
    spriteCount = 0;
    i = 0;
    anm = &this->anms[slot];
    if (data == NULL)
    {
        return NULL;
    }
    anm->slot = slot;
    anm->file = data;
    header = data;
    while (1)
    {
        anmCount++;
        numScripts += header->numScripts;
        spriteCount += header->numSprites;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    anm->anmCount = anmCount;
    anm->textures = (AnmTexture *)g_MemoryManager.Alloc(anmCount * sizeof(AnmTexture), GLOBAL_H);
    memset(anm->textures, 0, anmCount * sizeof(AnmTexture));
    anm->sprites = (SpriteData *)g_MemoryManager.Alloc(spriteCount * sizeof(SpriteData), GLOBAL_H);
    anm->scripts = (void **)g_MemoryManager.Alloc(numScripts * sizeof(void *), GLOBAL_H);
    header = data;
    anmCount = 0;
    spriteCount = 0;
    numScripts = 0;
    while (1)
    {
        result = this->PreloadAnmTexture(anm, i, &spriteCount, &numScripts, header);
        if (result < 0)
        {
            return NULL;
        }
        i++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    return anm;
}

AnmLoaded *SpriteEngine::PreloadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    anm = this->PreloadAnmFile(slot, path);
    if (anm == NULL)
    {
        return NULL;
    }
    anm->loadStep = 1;
    while (anm->loadStep != 0 && !g_App.closeSubThread)
    {
        Sleep(1);
    }
    DebugPrint("::preloadAnimEnd : %s\n", path);
    return g_App.closeSubThread ? NULL : anm;
}

int SpriteEngine::PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file)
{
    void *data;
    int size;
    char *name;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Log, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name != '@')
        {
            data = LoadFileData(name, &size, 1);
            if (data == NULL)
            {
                LogError(&g_Log, "テクスチャ %s が読み込めません。データが失われてるか壊れています\r\n", name);
                return -1;
            }
            anm->textures[index].size = size;
            anm->textures[index].data = data;
        }
    }
    return highest + 1;
}

AnmLoaded *SpriteEngine::PostloadAnm(AnmLoaded *anm)
{
    AnmFile *header;
    int scriptOffset;
    int result;
    AnmFile *data;
    int done;
    int spriteOffset;
    int index;

    DebugPrint("::postloadAnim : %d, %d\n", anm->slot, anm->loadStep);
    data = anm->file;
    done = 0;
    scriptOffset = 0;
    spriteOffset = 0;
    index = 0;
    anm->file = data;
    header = data;
    while (1)
    {
        if (done == anm->loadStep - 1)
        {
            result = this->LoadAnmSprites(anm, index, spriteOffset, scriptOffset, header);
            if (result < 0)
            {
                anm->loadStep = 0;
                return NULL;
            }
        }
        spriteOffset += header->numSprites;
        scriptOffset += header->numScripts;
        index++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
        done++;
        if (done == anm->loadStep)
        {
            anm->loadStep++;
            return anm;
        }
    }
    anm->loadStep = 0;
    return anm;
}

int SpriteEngine::LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file)
{
    SpriteData data;
    int *entries;
    int k;
    AnmSprite *rawSprite;
    char *name;
    D3DSURFACE_DESC desc;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Log, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name == '@')
        {
            this->CreateTexture(&anm->textures[index], anmHeader->width, anmHeader->height, anmHeader->format);
        }
        else if (this->LoadTexture(&anm->textures[index], anmHeader->format, anmHeader->colorKey) != 0)
        {
            LogError(&g_Log, "テクスチャ %s が作成できません。データが失われてるか壊れています\r\n", name);
            return -1;
        }
    }
    else if (this->CreateTextureFromImage(&anm->textures[index],
                                          (AnmImage *)((BYTE *)anmHeader + anmHeader->imageOffset),
                                          anmHeader->format) != 0)
    {
        LogError(&g_Log, "テクスチャが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anm->textures[index].texture->SetPriority(anmHeader->priority);
    anm->textures[index].texture->PreLoad();
    anm->textures[index].texture->GetLevelDesc(0, &desc);
    entries = anmHeader->spriteOffsets;
    for (k = 0; k < anmHeader->numSprites; k++, entries++)
    {
        rawSprite = (AnmSprite *)((char *)anmHeader + *entries);
        data.sourceFileIndex = anm->slot;
        data.texture = anm->textures[index].texture;
        data.scaleX = (float)desc.Width / anmHeader->width;
        data.scaleY = (float)desc.Height / anmHeader->height;
        data.startX = data.scaleX * rawSprite->x;
        data.startY = data.scaleY * rawSprite->y;
        data.endX = (rawSprite->x + rawSprite->width) * data.scaleX;
        data.endY = (rawSprite->y + rawSprite->height) * data.scaleY;
        data.textureWidth = desc.Width;
        data.textureHeight = desc.Height;
        anm->LoadSprite(spriteOffset, &data);
        spriteOffset++;
    }
    for (k = 0; k < anmHeader->numScripts; k++, entries += 2)
    {
        anm->scripts[scriptOffset] = (char *)anmHeader + entries[1];
        scriptOffset++;
    }
    return highest + 1;
}

int SpriteEngine::StepAnmLoading()
{
    int i;

    for (i = 0; i < ARRAY_SIZE(this->anms); i++)
    {
        if (this->anms[i].loadStep != 0)
        {
            if (this->PostloadAnm(&this->anms[i]) == NULL)
            {
                return -1;
            }
        }
    }
    return 0;
}

void SpriteEngine::ReleaseAnm(int slot)
{
    int i;

    if (slot < 0 || slot >= ARRAY_SIZE(this->anms))
    {
        return;
    }
    if (this->anms[slot].file != NULL)
    {
        for (i = 0; i < this->anms[slot].anmCount; i++)
        {
            this->ReleaseTexture(&this->anms[slot].textures[i]);
        }
        g_MemoryManager.Free(this->anms[slot].textures);
        g_MemoryManager.Free(this->anms[slot].sprites);
        g_MemoryManager.Free(this->anms[slot].scripts);
        g_MemoryManager.Free(this->anms[slot].file);
        memset(&this->anms[slot], 0, sizeof(AnmLoaded));
    }
}

void SpriteEngine::ReleaseTexture(AnmTexture *tex)
{
    if (tex->texture != NULL)
    {
        tex->texture->Release();
        tex->texture = NULL;
    }
    if (tex->data != NULL)
    {
        g_MemoryManager.Free(tex->data);
    }
}

void AnmLoaded::LoadSprite(int index, SpriteData *sprite)
{
    this->sprites[index] = *sprite;
    // The comma operators reproduce the original's operand order: it
    // loads the texture size first (fdivr).
    this->sprites[index].uvStartX = this->sprites[index].startX / (0, this->sprites[index].textureWidth);
    this->sprites[index].uvEndX = this->sprites[index].endX / (0, this->sprites[index].textureWidth);
    this->sprites[index].uvStartY = this->sprites[index].startY / (0, this->sprites[index].textureHeight);
    this->sprites[index].uvEndY = this->sprites[index].endY / (0, this->sprites[index].textureHeight);
    this->sprites[index].width = (this->sprites[index].endX - this->sprites[index].startX) / sprite->scaleX;
    this->sprites[index].height = (this->sprites[index].endY - this->sprites[index].startY) / sprite->scaleY;
}

void SpriteEngine::RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused,
                                            int fontHeight, int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor,
                                            char *text, float scaleX, float scaleY)
{
    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }
    if (destHeight <= 0)
    {
        destHeight = 15;
    }
    if (fontHeight > 8)
    {
        RenderTextToTextureBold(x, y, width, unused, fontHeight * scaleX, destHeight * scaleY, textColor, shadowColor,
                                text, texture);
    }
    else
    {
        RenderTextToTexture(x, y, width, unused, 8, 8, textColor, shadowColor, text, texture);
    }
}

void SpriteEngine::RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    char buf[0x80];
    int fontW;

    fontW = sprite->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    this->RenderTextToTextureIndex(sprite->spriteData->texture, sprite->spriteData->startX,
                                   sprite->spriteData->startY, sprite->spriteData->textureWidth,
                                   sprite->spriteData->textureHeight, fontW, sprite->fontHeight, textColor, shadowColor,
                                   buf, sprite->spriteData->scaleX, sprite->spriteData->scaleY);
    sprite->flags |= 1;
}

void SpriteEngine::RenderTextRightAligned(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x80];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 2.0f;
    this->RenderTextToTextureIndex(vm->spriteData->texture, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, buf, vm->spriteData->scaleX, vm->spriteData->scaleY);
    vm->flags |= 1;
}

void SpriteEngine::RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x48];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX / 2.0f -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 4.0f;
    this->RenderTextToTextureIndex(vm->spriteData->texture, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW, vm->fontHeight,
                                   color, shadowColor, buf, vm->spriteData->scaleX, vm->spriteData->scaleY);
    vm->flags |= 1;
}

int SpriteEngine::LoadSurface(int surface, char *path)
{
    void *data;
    int size;
    IDirect3DSurface8 *srcSurface;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    if (this->surfaceFiles[surface] == NULL)
    {
        data = LoadFileData(path, &size, 0);
        if (data == NULL)
        {
            LogError(&g_Log, "%sが読み込めないです。\r\n", path);
            return -1;
        }
    }
    else
    {
        data = this->surfaceFiles[surface];
        size = this->surfaceFileSizes[surface];
        this->surfaceFiles[surface] = NULL;
    }
    if (g_App.d3dDevice->CreateImageSurface(640, 1024, g_App.presentParams.BackBufferFormat, &srcSurface) != D3D_OK)
    {
        return -1;
    }
    if (D3DXLoadSurfaceFromFileInMemory(srcSurface, NULL, NULL, data, size, NULL, D3DX_FILTER_NONE, 0,
                                        &this->surfaceInfos[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryManager.Free(data);
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryManager.Free(data);
    return -1;
}

int SpriteEngine::PreloadSurface(int surface, char *path)
{
    void *data;
    int size;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    data = LoadFileData(path, &size, 0);
    if (data == NULL)
    {
        LogError(&g_Log, "%sが読み込めないです。\r\n", path);
        return -1;
    }
    this->surfaceFiles[surface] = data;
    this->surfaceFileSizes[surface] = size;
    return 0;
}

void SpriteEngine::ReleaseSurface(int surface)
{
    if (this->surfaces[surface] != NULL)
    {
        this->surfaces[surface]->Release();
        this->surfaces[surface] = NULL;
    }
    if (this->surfacesBackup[surface] != NULL)
    {
        this->surfacesBackup[surface]->Release();
        this->surfacesBackup[surface] = NULL;
    }
    if (this->surfaceFiles[surface] != NULL)
    {
        g_MemoryManager.Free(this->surfaceFiles[surface]);
    }
    this->surfaceFiles[surface] = NULL;
}

void SpriteEngine::CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = this->surfaceInfos[surface].Width;
    rect.bottom = this->surfaceInfos[surface].Height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    IDirect3DSurface8 *dest;
    RECT rect;
    RECT destinationRect;

    if (this->anms[texture].textures[0].texture == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->anms[texture].textures[0].texture->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        backBuffer->Release();
        return;
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (D3DXLoadSurfaceFromSurface(dest, NULL, &destinationRect, backBuffer, NULL, &rect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        backBuffer->Release();
        return;
    }
    dest->Release();
    backBuffer->Release();
}

void SpriteEngine::CopyTextureRect(int dstAnm, int dstTexture, int srcAnm, int srcTexture, RECT *dstRect, RECT *srcRect)
{
    IDirect3DSurface8 *dest;
    IDirect3DSurface8 *src;

    if (this->anms[dstAnm].textures[dstTexture].texture == NULL)
    {
        return;
    }
    if (this->anms[srcAnm].textures[srcTexture].texture == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (this->anms[dstAnm].textures[dstTexture].texture->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        return;
    }
    if (this->anms[srcAnm].textures[srcTexture].texture->GetSurfaceLevel(0, &src) != D3D_OK)
    {
        dest->Release();
        return;
    }
    if (D3DXLoadSurfaceFromSurface(dest, NULL, dstRect, src, NULL, srcRect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        src->Release();
        return;
    }
    dest->Release();
    src->Release();
}

void SpriteEngine::CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    RECT destinationRect;

    this->FlushQuadBatch();
    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    this->surfaceInfos[surface].Width = dstWidth;
    this->surfaceInfos[surface].Height = dstHeight;
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto done;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto done;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, &destinationRect, backBuffer, NULL, &rect,
                                   D3DX_DEFAULT, 0) != D3D_OK)
    {
        goto done;
    }
    D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, this->surfaces[surface], NULL, NULL,
                               D3DX_DEFAULT, 0);

done:
    if (backBuffer != NULL)
    {
        backBuffer->Release();
        backBuffer = NULL;
    }
}
