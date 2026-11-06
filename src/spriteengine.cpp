// The file with SpriteEngine, from 0x45e430 to 0x462790: th07's
// spriteengine.cpp, in a different order. The script interpreter comes
// first; the drawing, the constructor and the loading and copying from
// 0x465070 are spritedraw.cpp's.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sprite.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
// MSVC emits these COMDATs in reverse declaration order. Keeping
// g_Half first in the image also folds the compiler's 0.5f pool into it.
__declspec(selectany) const int g_TextureFormatSizes[6] = {
    4, 4, 2, 2, 3, 2,
};
__declspec(selectany) const D3DFORMAT g_TextureFormats[6] = {
    D3DFMT_UNKNOWN,
    D3DFMT_A8R8G8B8,
    D3DFMT_A1R5G5B5,
    D3DFMT_R5G6B5,
    D3DFMT_R8G8B8,
    D3DFMT_A4R4G4B4,
};
__declspec(selectany) extern const float g_Half = 0.5f;
SpriteEngine *g_SpriteEngine;
VertexTex1Xyzrwh g_TexQuad[4];
VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];
VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];
#endif // TH08_EXE

int AnmLoaded::SetSprite(Sprite *vm, int sprite)
{
    if (this->file == NULL || this->loadStep != 0)
    {
        return -1;
    }
    vm->anm = this;
    vm->spriteIndex = sprite;
    vm->spriteData = &this->sprites[sprite];
    vm->spriteWidth = vm->spriteData->width;
    vm->spriteHeight = vm->spriteData->height;
    D3DXMatrixIdentity(&vm->matrix);
    D3DXMatrixIdentity(&vm->sizeMatrix);
    if (vm->spriteData->scaleX < 1.0f)
    {
        sprite = 0;
    }
    vm->matrix.m[0][0] = vm->spriteWidth / 256.0f;
    vm->matrix.m[1][1] = vm->spriteHeight / 256.0f;
    vm->sizeMatrix.m[0][0] = vm->spriteWidth / vm->spriteData->textureWidth * vm->spriteData->scaleX;
    vm->sizeMatrix.m[1][1] = vm->spriteHeight / vm->spriteData->textureHeight * vm->spriteData->scaleY;
    vm->drawMatrix = vm->matrix;
    return 0;
}

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

#define ANM_INT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveIntArgument(curInstr->args[n].i) : curInstr->args[n].i)
#define ANM_FLOAT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveFloatArgument(curInstr->args[n].f) : curInstr->args[n].f)
#define ANM_INT_VAR(n) (*vm->ResolveIntArgumentPtr(&curInstr->args[n].i, curInstr->paramMask, (n)))
#define ANM_FLOAT_VAR(n) (*vm->ResolveFloatArgumentPtr(&curInstr->args[n].f, curInstr->paramMask, (n)))

int SpriteEngine::Step(Sprite *vm)
{
    AnmInstr *curInstr;
    AnmInstr *fallback;
    int k;
    float t;

    if (vm->scriptPos == NULL)
    {
        return 1;
    }
    if (vm->scriptPaused)
    {
        return 0;
    }
    if (vm->pendingInterrupt != 0)
    {
        goto interrupt;
    }
    while (curInstr = vm->scriptPos, curInstr->time <= vm->scriptTime.AsFrames())
    {
        switch (curInstr->opcode)
        {
        case -1:
        case 1:
            vm->shown = 0;
        case 2:
            vm->scriptPos = NULL;
            return 1;
        case 3:
            vm->shown = 1;
            vm->anm->SetSprite(vm, ANM_INT_ARG(0));
            vm->spriteTime = vm->scriptTime.AsFrames();
            break;
        case 7:
            vm->scaleX = ANM_FLOAT_ARG(0);
            vm->scaleY = ANM_FLOAT_ARG(1);
            vm->flags |= 8;
            break;
        case 8:
            vm->alpha = ANM_INT_ARG(0);
            break;
        case 9:
            vm->colorRgb[2] = ANM_INT_ARG(0);
            vm->colorRgb[1] = ANM_INT_ARG(1);
            vm->colorRgb[0] = ANM_INT_ARG(2);
            break;
        case 85:
            vm->color2Alpha = ANM_INT_ARG(0);
            break;
        case 84:
            vm->color2Rgb[2] = ANM_INT_ARG(0);
            vm->color2Rgb[1] = ANM_INT_ARG(1);
            vm->color2Rgb[0] = ANM_INT_ARG(2);
            break;
        case 4:
            vm->scriptTime = curInstr->args[1].i;
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[0].i);
            continue;
        case 5:
            ANM_INT_VAR(0)--;
            if (ANM_INT_ARG(0) > 0)
            {
                vm->scriptTime = curInstr->args[2].i;
                vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[1].i);
                continue;
            }
            break;
        case 10:
            vm->flip ^= 1;
            vm->scaleX *= -1.0f;
            vm->flags |= 8;
            break;
        case 24:
            vm->usePosOffset = curInstr->args[0].i;
            break;
        case 11:
            vm->flip ^= 2;
            vm->scaleY *= -1.0f;
            vm->flags |= 8;
            break;
        case 12:
            vm->rotation.x = ANM_FLOAT_ARG(0);
            vm->rotation.y = ANM_FLOAT_ARG(1);
            vm->rotation.z = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 13:
            vm->rotationSpeed.x = ANM_FLOAT_ARG(0);
            vm->rotationSpeed.y = ANM_FLOAT_ARG(1);
            vm->rotationSpeed.z = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 14:
            vm->scaleSpeedX = ANM_FLOAT_ARG(0);
            vm->scaleSpeedY = ANM_FLOAT_ARG(1);
            break;
        case 29:
            vm->interpDuration[4] = 0;
            vm->interpFrame[4] = ANM_INT_ARG(2);
            vm->scaleInterpEasing = 0;
            *(D3DXVECTOR2 *)&vm->scaleTween = *(D3DXVECTOR2 *)&vm->scaleX;
            vm->scaleTween.z = ANM_FLOAT_ARG(0);
            vm->colorTween.x = ANM_FLOAT_ARG(1);
            break;
        case 15:
            ((BYTE *)&vm->colorTween.y)[3] = vm->alpha;
            ((BYTE *)&vm->colorTween.z)[3] = curInstr->args[0].i;
            vm->interpDuration[2] = 0;
            vm->interpFrame[2] = ANM_INT_ARG(1);
            vm->alphaInterpEasing = 0;
            break;
        case 16:
            vm->blendMode = curInstr->args[0].i != 0;
            break;
        case 82:
            vm->blendMode = curInstr->args[0].i;
            break;
        case 6:
            if (!vm->usePosOffset)
            {
                vm->pos = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            else
            {
                vm->posOffset = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            break;
        case 19:
            vm->interpEasing = 6;
            goto posInterp;
        case 18:
            vm->interpEasing = 4;
            goto posInterp;
        case 17:
            vm->interpEasing = 0;
        posInterp:
            if (!vm->usePosOffset)
            {
                vm->moveStart = vm->pos;
            }
            else
            {
                vm->moveStart = vm->posOffset;
            }
            vm->moveEnd = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            vm->interpFrame[0] = ANM_INT_ARG(3);
            vm->interpDuration[0] = 0;
            break;
        case 79:
            if (vm->waitTimer == 0)
            {
                vm->waitTimer = ANM_INT_ARG(0);
            }
            else
            {
                vm->waitTimer--;
            }
            if (vm->waitTimer <= 0)
            {
                vm->waitTimer = 0;
                break;
            }
            vm->scriptTime--;
            goto done;
        case 23:
            vm->shown = 0;
        case 20:
            if (vm->pendingInterrupt == 0)
            {
                vm->stopped = 1;
                vm->scriptTime--;
                goto done;
            }
        interrupt:
            fallback = NULL;
            curInstr = vm->scriptStart;
            while ((curInstr->opcode != 21 || vm->pendingInterrupt != curInstr->args[0].i) &&
                   curInstr->opcode != -1)
            {
                if (curInstr->opcode == 21 && curInstr->args[0].i == -1)
                {
                    fallback = curInstr;
                }
                curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            }
            vm->pendingInterrupt = 0;
            vm->stopped = 0;
            if (curInstr->opcode != 21)
            {
                if (fallback == NULL)
                {
                    vm->scriptTime--;
                    goto done;
                }
                curInstr = fallback;
            }
            vm->interruptReturnTime = vm->scriptTime;
            vm->interruptReturnPos = vm->scriptPos;
            curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            vm->scriptPos = curInstr;
            vm->scriptTime = vm->scriptPos->time;
            vm->shown = 1;
            continue;
        case 89:
            vm->scriptTime = vm->interruptReturnTime;
            vm->scriptPos = vm->interruptReturnPos;
            continue;
        case 28:
            vm->shown = curInstr->args[0].i;
            break;
        case 22:
            vm->anchor = 3;
            break;
        case 25:
            vm->autoRotate = curInstr->args[0].i;
            break;
        case 26:
            vm->uvScrollX += ANM_FLOAT_ARG(0);
            if (vm->uvScrollX >= 1.0f)
            {
                vm->uvScrollX -= 1.0f;
            }
            else if (vm->uvScrollX < 0.0f)
            {
                vm->uvScrollX += 1.0f;
            }
            break;
        case 27:
            vm->uvScrollY += ANM_FLOAT_ARG(0);
            if (vm->uvScrollY >= 1.0f)
            {
                vm->uvScrollY -= 1.0f;
            }
            else if (vm->uvScrollY < 0.0f)
            {
                vm->uvScrollY += 1.0f;
            }
            break;
        case 80:
            vm->uvScrollSpeedX = ANM_FLOAT_ARG(0);
            break;
        case 81:
            vm->uvScrollSpeedY = ANM_FLOAT_ARG(0);
            break;
        case 30:
            vm->zWriteDisabled = curInstr->args[0].i;
            break;
        case 31:
            vm->cameraSetup = curInstr->args[0].i;
            break;
        case 32:
            vm->interpDuration[0] = 0;
            vm->interpFrame[0] = ANM_INT_ARG(0);
            vm->interpEasing = curInstr->args[1].i;
            if (!vm->usePosOffset)
            {
                vm->moveStart = vm->pos;
            }
            else
            {
                vm->moveStart = vm->posOffset;
            }
            vm->moveEnd.x = ANM_FLOAT_ARG(2);
            vm->moveEnd.y = ANM_FLOAT_ARG(3);
            vm->moveEnd.z = ANM_FLOAT_ARG(4);
            break;
        case 33:
            vm->interpDuration[1] = 0;
            vm->interpFrame[1] = ANM_INT_ARG(0);
            vm->colorInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->colorTween.y)[2] = vm->colorRgb[2];
            ((BYTE *)&vm->colorTween.y)[1] = vm->colorRgb[1];
            ((BYTE *)&vm->colorTween.y)[0] = vm->colorRgb[0];
            ((BYTE *)&vm->colorTween.z)[2] = ANM_INT_ARG(2);
            ((BYTE *)&vm->colorTween.z)[1] = ANM_INT_ARG(3);
            ((BYTE *)&vm->colorTween.z)[0] = ANM_INT_ARG(4);
            break;
        case 34:
            vm->interpDuration[2] = 0;
            vm->interpFrame[2] = ANM_INT_ARG(0);
            vm->alphaInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->colorTween.y)[3] = vm->alpha;
            ((BYTE *)&vm->colorTween.z)[3] = ANM_INT_ARG(2);
            break;
        case 86:
            vm->interpDuration[5] = 0;
            vm->interpFrame[5] = ANM_INT_ARG(0);
            vm->color2InterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->color2TweenStart)[2] = vm->color2Rgb[2];
            ((BYTE *)&vm->color2TweenStart)[1] = vm->color2Rgb[1];
            ((BYTE *)&vm->color2TweenStart)[0] = vm->color2Rgb[0];
            ((BYTE *)&vm->color2TweenEnd)[2] = ANM_INT_ARG(2);
            ((BYTE *)&vm->color2TweenEnd)[1] = ANM_INT_ARG(3);
            ((BYTE *)&vm->color2TweenEnd)[0] = ANM_INT_ARG(4);
            break;
        case 87:
            vm->interpDuration[6] = 0;
            vm->interpFrame[6] = ANM_INT_ARG(0);
            vm->color2AlphaInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->color2TweenStart)[3] = vm->color2Alpha;
            ((BYTE *)&vm->color2TweenEnd)[3] = ANM_INT_ARG(2);
            break;
        case 35:
            vm->interpDuration[3] = 0;
            vm->interpFrame[3] = ANM_INT_ARG(0);
            vm->rotationInterpEasing = curInstr->args[1].i;
            vm->rotateStart = vm->rotation;
            vm->rotateEnd.x = ANM_FLOAT_ARG(2);
            vm->rotateEnd.y = ANM_FLOAT_ARG(3);
            vm->rotateEnd.z = ANM_FLOAT_ARG(4);
            vm->flags |= 4;
            break;
        case 36:
            vm->interpDuration[4] = 0;
            vm->interpFrame[4] = ANM_INT_ARG(0);
            vm->scaleInterpEasing = curInstr->args[1].i;
            *(D3DXVECTOR2 *)&vm->scaleTween = *(D3DXVECTOR2 *)&vm->scaleX;
            vm->scaleTween.z = ANM_FLOAT_ARG(2);
            vm->colorTween.x = ANM_FLOAT_ARG(3);
            vm->flags |= 8;
            break;
        case 83:
            vm->drawMode = curInstr->args[0].i;
            break;
        case 37:
            ANM_INT_VAR(0) = ANM_INT_ARG(1);
            break;
        case 38:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1);
            break;
        case 49:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) + ANM_INT_ARG(2);
            break;
        case 50:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) + ANM_FLOAT_ARG(2);
            break;
        case 51:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) - ANM_INT_ARG(2);
            break;
        case 52:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) - ANM_FLOAT_ARG(2);
            break;
        case 53:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) * ANM_INT_ARG(2);
            break;
        case 54:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) * ANM_FLOAT_ARG(2);
            break;
        case 55:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) / ANM_INT_ARG(2);
            break;
        case 56:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) / ANM_FLOAT_ARG(2);
            break;
        case 57:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) % ANM_INT_ARG(2);
            break;
        case 58:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            break;
        case 39:
            ANM_INT_VAR(0) += ANM_INT_ARG(1);
            break;
        case 40:
            ANM_FLOAT_VAR(0) += ANM_FLOAT_ARG(1);
            break;
        case 41:
            ANM_INT_VAR(0) -= ANM_INT_ARG(1);
            break;
        case 42:
            ANM_FLOAT_VAR(0) -= ANM_FLOAT_ARG(1);
            break;
        case 43:
            ANM_INT_VAR(0) *= ANM_INT_ARG(1);
            break;
        case 44:
            ANM_FLOAT_VAR(0) *= ANM_FLOAT_ARG(1);
            break;
        case 45:
            ANM_INT_VAR(0) /= ANM_INT_ARG(1);
            break;
        case 46:
            ANM_FLOAT_VAR(0) /= ANM_FLOAT_ARG(1);
            break;
        case 47:
            ANM_INT_VAR(0) %= ANM_INT_ARG(1);
            break;
        case 48:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1));
            break;
        case 59:
            ANM_INT_VAR(0) = g_Random.GetRandomU32InRange(ANM_INT_ARG(1));
            break;
        case 60:
            ANM_FLOAT_VAR(0) = g_Random.GetRandomF32InRange(ANM_FLOAT_ARG(1));
            break;
        case 61:
            ANM_FLOAT_VAR(0) = sinf(ANM_FLOAT_ARG(1));
            break;
        case 62:
            ANM_FLOAT_VAR(0) = cosf(ANM_FLOAT_ARG(1));
            break;
        case 63:
            ANM_FLOAT_VAR(0) = tanf(ANM_FLOAT_ARG(1));
            break;
        case 64:
            ANM_FLOAT_VAR(0) = acosf(ANM_FLOAT_ARG(1));
            break;
        case 65:
            ANM_FLOAT_VAR(0) = atanf(ANM_FLOAT_ARG(1));
            break;
        case 66:
            ANM_FLOAT_VAR(0) = AddNormalizeAngle(ANM_FLOAT_ARG(0), 0.0f);
            break;
        case 67:
            if (ANM_INT_ARG(0) == ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 68:
            if (ANM_FLOAT_ARG(0) == ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 69:
            if (ANM_INT_ARG(0) != ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 70:
            if (ANM_FLOAT_ARG(0) != ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 71:
            if (ANM_INT_ARG(0) < ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 72:
            if (ANM_FLOAT_ARG(0) < ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 73:
            if (ANM_INT_ARG(0) <= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 74:
            if (ANM_FLOAT_ARG(0) <= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 75:
            if (ANM_INT_ARG(0) > ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 76:
            if (ANM_FLOAT_ARG(0) > ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 77:
            if (ANM_INT_ARG(0) >= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 78:
            if (ANM_FLOAT_ARG(0) >= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 88:
            vm->useColor2 = curInstr->args[0].b[1];
            break;
        jump:
            vm->scriptTime = curInstr->args[3].i;
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[2].i);
            continue;
        }
        vm->scriptPos = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
    }

done:
    if (vm->rotationSpeed.x != 0.0f)
    {
        vm->rotation.x = AddNormalizeAngle(vm->rotation.x, g_App.gameSpeed * vm->rotationSpeed.x);
        vm->flags |= 4;
    }
    if (vm->rotationSpeed.y != 0.0f)
    {
        vm->rotation.y = AddNormalizeAngle(vm->rotation.y, g_App.gameSpeed * vm->rotationSpeed.y);
        vm->flags |= 4;
    }
    if (vm->rotationSpeed.z != 0.0f)
    {
        vm->rotation.z = AddNormalizeAngle(vm->rotation.z, g_App.gameSpeed * vm->rotationSpeed.z);
        vm->flags |= 4;
    }
    for (k = 0; k < 7; k++)
    {
        if (vm->interpFrame[k] > 0)
        {
            vm->interpDuration[k]++;
            if (vm->interpDuration[k] >= vm->interpFrame[k].AsFrames())
            {
                t = 1.0f;
                vm->interpFrame[k] = 0;
            }
            else
            {
                t = vm->interpDuration[k].AsFloat() / vm->interpFrame[k].AsFloat();
            }
            switch ((&vm->interpEasing)[k])
            {
            case 1:
                t = t * t;
                break;
            case 2:
                t = t * t * t;
                break;
            case 3:
                t = (t = t * t) * t;
                break;
            case 4:
                t = (t = 1.0f - t) * t;
                t = 1.0f - t;
                break;
            case 5:
                t = (t = 1.0f - t) * t * t;
                t = 1.0f - t;
                break;
            case 6:
                t = (t = (t = 1.0f - t) * t) * t;
                t = 1.0f - t;
                break;
            }
            switch (k)
            {
            case 0:
                if (!vm->usePosOffset)
                {
                    vm->pos.x = (vm->moveEnd.x - vm->moveStart.x) * t + vm->moveStart.x;
                    vm->pos.y = (vm->moveEnd.y - vm->moveStart.y) * t + vm->moveStart.y;
                    vm->pos.z = (vm->moveEnd.z - vm->moveStart.z) * t + vm->moveStart.z;
                }
                else
                {
                    vm->posOffset.x = (vm->moveEnd.x - vm->moveStart.x) * t + vm->moveStart.x;
                    vm->posOffset.y = (vm->moveEnd.y - vm->moveStart.y) * t + vm->moveStart.y;
                    vm->posOffset.z = (vm->moveEnd.z - vm->moveStart.z) * t + vm->moveStart.z;
                }
                break;
            case 1:
                vm->colorRgb[2] = ((float)((BYTE *)&vm->colorTween.z)[2] - ((BYTE *)&vm->colorTween.y)[2]) * t +
                                 ((BYTE *)&vm->colorTween.y)[2];
                vm->colorRgb[1] = ((float)((BYTE *)&vm->colorTween.z)[1] - ((BYTE *)&vm->colorTween.y)[1]) * t +
                                 ((BYTE *)&vm->colorTween.y)[1];
                vm->colorRgb[0] = ((float)((BYTE *)&vm->colorTween.z)[0] - ((BYTE *)&vm->colorTween.y)[0]) * t +
                                 ((BYTE *)&vm->colorTween.y)[0];
                break;
            case 2:
                vm->alpha = ((float)((BYTE *)&vm->colorTween.z)[3] - ((BYTE *)&vm->colorTween.y)[3]) * t +
                            ((BYTE *)&vm->colorTween.y)[3];
                break;
            case 5:
                vm->color2Rgb[2] = ((float)((BYTE *)&vm->color2TweenEnd)[2] - ((BYTE *)&vm->color2TweenStart)[2]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[2];
                vm->color2Rgb[1] = ((float)((BYTE *)&vm->color2TweenEnd)[1] - ((BYTE *)&vm->color2TweenStart)[1]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[1];
                vm->color2Rgb[0] = ((float)((BYTE *)&vm->color2TweenEnd)[0] - ((BYTE *)&vm->color2TweenStart)[0]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[0];
                break;
            case 6:
                vm->color2Alpha = ((float)((BYTE *)&vm->color2TweenEnd)[3] - ((BYTE *)&vm->color2TweenStart)[3]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[3];
                break;
            case 3:
                vm->rotation.x = AddNormalizeAngle((vm->rotateEnd.x - vm->rotateStart.x) * t, vm->rotateStart.x);
                vm->rotation.y = AddNormalizeAngle((vm->rotateEnd.y - vm->rotateStart.y) * t, vm->rotateStart.y);
                vm->rotation.z = AddNormalizeAngle((vm->rotateEnd.z - vm->rotateStart.z) * t, vm->rotateStart.z);
                vm->flags |= 4;
                break;
            case 4:
                vm->scaleX = (vm->scaleTween.z - vm->scaleTween.x) * t + vm->scaleTween.x;
                vm->scaleY = (vm->colorTween.x - vm->scaleTween.y) * t + vm->scaleTween.y;
                vm->flags |= 8;
                break;
            }
        }
    }
    if (vm->scaleSpeedY != 0.0f)
    {
        vm->scaleY += g_App.gameSpeed * vm->scaleSpeedY;
        vm->flags |= 8;
    }
    if (vm->scaleSpeedX != 0.0f)
    {
        vm->scaleX += g_App.gameSpeed * vm->scaleSpeedX;
        vm->flags |= 8;
        vm->flags |= 4;
    }
    vm->uvScrollX = vm->uvScrollX + vm->uvScrollSpeedX;
    if (vm->uvScrollX >= 1.0f)
    {
        vm->uvScrollX -= 1.0f;
    }
    else if (vm->uvScrollX < 0.0f)
    {
        vm->uvScrollX += 1.0f;
    }
    vm->uvScrollY = vm->uvScrollY + vm->uvScrollSpeedY;
    if (vm->uvScrollY >= 1.0f)
    {
        vm->uvScrollY -= 1.0f;
    }
    else if (vm->uvScrollY < 0.0f)
    {
        vm->uvScrollY += 1.0f;
    }
    vm->scriptTime++;
    this->stepCount++;
    return 0;
}

#undef ANM_INT_ARG
#undef ANM_FLOAT_ARG
#undef ANM_INT_VAR
#undef ANM_FLOAT_VAR

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

void AnmLoaded::StartSpriteSeries(Sprite *vms, int script, int count)
{
    while (count != 0)
    {
        this->StartScriptAtOrigin(vms, script);
        vms->baseSpriteIndex = vms->spriteIndex;
        script++;
        vms++;
        count--;
    }
}

void SpriteEngine::SetRenderStateForSprite(Sprite *vm)
{
    D3DCOLOR color;

    if (this->blendMode != (vm->flags >> 4 & 3))
    {
        this->FlushQuadBatch();
        this->blendMode = vm->flags >> 4 & 3;
        switch (this->blendMode)
        {
        case 0:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case 1:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }
    color = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
    if (this->resetTextureArgs != 0)
    {
        this->resetTextureArgs = 0;
        if (!g_App.GetNoVertexBuffer())
        {
            this->FlushQuadBatch();
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        }
    }
    if (!g_App.GetNoVertexBuffer())
    {
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(color, this->modulateColor, 2);
            MODULATE_CHANNEL(color, this->modulateColor, 1);
            MODULATE_CHANNEL(color, this->modulateColor, 0);
            MODULATE_CHANNEL(color, this->modulateColor, 3);
        }
        if (this->currentColor != color)
        {
            this->FlushQuadBatch();
            this->currentColor = color;
            g_App.d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->currentColor);
        }
    }
    else
    {
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(color, this->modulateColor, 2);
            MODULATE_CHANNEL(color, this->modulateColor, 1);
            MODULATE_CHANNEL(color, this->modulateColor, 0);
            MODULATE_CHANNEL(color, this->modulateColor, 3);
        }
        g_DiffuseQuad[0].diffuse = color;
        g_DiffuseQuad[1].diffuse = color;
        g_DiffuseQuad[2].diffuse = color;
        g_DiffuseQuad[3].diffuse = color;
        g_DiffuseQuadXyz[0].diffuse = color;
        g_DiffuseQuadXyz[1].diffuse = color;
        g_DiffuseQuadXyz[2].diffuse = color;
        g_DiffuseQuadXyz[3].diffuse = color;
    }
    if (!g_App.GetNoDepthTest() && this->zWriteDisabled != (vm->flags >> 13 & 1))
    {
        this->FlushQuadBatch();
        this->zWriteDisabled = vm->flags >> 13 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    if (this->lastCameraSetupFlag != (vm->flags >> 15 & 1))
    {
        this->FlushQuadBatch();
        this->lastCameraSetupFlag = vm->flags >> 15 & 1;
        if (this->lastCameraSetupFlag == 0)
        {
            g_StageManager.SetupFixedCamera();
            g_App.d3dDevice->SetViewport(&g_App.viewport);
        }
        else
        {
            g_StageManager.SetupCamera();
            g_App.d3dDevice->SetViewport(&g_App.viewport);
        }
    }
    this->renderStateSetCount++;
}

