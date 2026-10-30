// The stage background, th07's Stage, from 0x4071a0, as th07's
// stage.cpp.

#include <math.h>
#include <string.h>

#include "effect.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

Stage::Stage()
{
    memset(this, 0, sizeof(Stage));
    this->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    this->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    this->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    this->camera.fov = 0.5235988f;
    this->cameraFrom = this->camera;
    this->cameraTo = this->camera;
}

int Stage::OnUpdate(Stage *stage)
{
    StageInstr *cmd;
    D3DXVECTOR3 tmpVec;

    if (stage->data == NULL)
    {
        return 1;
    }
    if (g_GameManager.deathbombFreeze)
    {
        return 1;
    }
    if (g_GameManager.stage == 7)
    {
        if (stage->stage7Effect == NULL)
        {
            Sprite *sprite;
            D3DXVECTOR3 pos(0.0f, 0.0f, 0.0f);

            stage->stage7Effect = g_EffectManager.SpawnReservedEffect(0x40, &pos, 0xc, 1, 0xffffffff);
            sprite = &stage->stage7Effect->sprite;
            stage->anm->StartScript(sprite, 0xb);
        }
        else if (stage->waitLabel == 1)
        {
            Sprite *sprite = &stage->stage7Effect->sprite;

            stage->anm->StartScript(sprite, 0xb);
        }
        else if (stage->waitLabel == 2)
        {
            Sprite *sprite = &stage->stage7Effect->sprite;
            Sprite copy = *sprite;

            stage->anm->StartScript(sprite, 0xc);
            sprite->SetPendingInterrupt(2);
            sprite->moveEnd = copy.moveEnd;
            sprite->moveStart = copy.moveStart;
            sprite->interpDuration[0] = copy.interpDuration[0];
            sprite->interpFrame[0] = copy.interpFrame[0];
            sprite->interpEasing = copy.interpEasing;
            sprite->color = copy.color;
        }
        else if (stage->waitLabel == 3)
        {
            Sprite *sprite = &stage->stage7Effect->sprite;
            Sprite copy = *sprite;

            sprite->SetPendingInterrupt(3);
            sprite->moveEnd = copy.moveEnd;
            sprite->moveStart = copy.moveStart;
            sprite->interpDuration[0] = copy.interpDuration[0];
            sprite->interpFrame[0] = copy.interpFrame[0];
            sprite->interpEasing = copy.interpEasing;
            sprite->color = copy.color;
        }
        else if (stage->waitLabel == 4)
        {
            Sprite *sprite = &stage->stage7Effect->sprite;
            Sprite copy = *sprite;

            sprite->SetPendingInterrupt(4);
            sprite->moveEnd = copy.moveEnd;
            sprite->moveStart = copy.moveStart;
            sprite->interpDuration[0] = copy.interpDuration[0];
            sprite->interpFrame[0] = copy.interpFrame[0];
            sprite->interpEasing = copy.interpEasing;
            sprite->color = copy.color;
        }
    }
    if (stage->waitLabel != 0)
    {
        int idx;

        idx = 0;
        cmd = stage->script;
        stage->scriptIndex = 0;
        while ((cmd->opcode != 0x1f || stage->waitLabel != *(int *)&cmd->args.x) && cmd->frame != -1)
        {
            cmd++;
            idx++;
        }
        if (cmd->frame != -1)
        {
            stage->scriptIndex = idx + 1;
            stage->scriptTime = cmd->frame;
            stage->waitLabel = 0;
        }
    }
loop:
    cmd = &stage->script[stage->scriptIndex];
    if (stage->scriptTime >= cmd->frame && cmd->frame != -1)
    {
        switch (cmd->opcode)
        {
        case 0:
            if (cmd->frame == -1)
            {
                stage->keyPos = cmd->args;
                stage->cameraPos.x = stage->keyPos.x;
                stage->cameraPos.y = stage->keyPos.y;
                stage->cameraPos.z = stage->keyPos.z;
            }
            else
            {
                tmpVec = cmd->args;
                stage->cameraPos.x = tmpVec.x;
                stage->cameraPos.y = tmpVec.y;
                stage->cameraPos.z = tmpVec.z;
                stage->keyPos = tmpVec;
                stage->keyFrame = cmd->frame;
                cmd++;
                stage->nextKeyFrame = cmd->frame;
                stage->nextKeyPos = cmd->args;
            }
            break;
        case 1:
            stage->fog.color = *(D3DCOLOR *)&cmd->args.x;
            stage->fog.nearPlane = cmd->args.y;
            stage->fog.farPlane = cmd->args.z;
            stage->fogTarget = stage->fog;
            break;
        case 2:
            stage->fogStart = stage->fog;
            stage->fogFadeFrames = *(int *)&cmd->args.x;
            stage->fogFadeTime = 0;
            break;
        case 5:
            if (stage->cameraOffsetChanged)
            {
                D3DXVECTOR3 offset = cmd->args - stage->cameraFrom.eye;
                OffsetPetalEffects(&offset);
                stage->cameraOffsetChanged = 0;
            }
            stage->cameraTo.eye = stage->cameraFrom.eye;
            stage->cameraFrom.eye = cmd->args;
            if (stage->cameraFadeFrames[0] == 0)
            {
                stage->camera.eye = cmd->args;
            }
            break;
        case 6:
            stage->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[0] = 0;
            stage->cameraFadeMode[0] = *(int *)&cmd->args.y;
            break;
        case 7:
            stage->cameraTo.at = stage->cameraFrom.at;
            stage->cameraFrom.at = cmd->args;
            if (stage->cameraFadeFrames[1] == 0)
            {
                stage->camera.at = cmd->args;
            }
            break;
        case 8:
            stage->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[1] = 0;
            stage->cameraFadeMode[1] = *(int *)&cmd->args.y;
            break;
        case 9:
            stage->cameraTo.up = stage->cameraFrom.up;
            stage->cameraFrom.up = cmd->args;
            if (stage->cameraFadeFrames[2] == 0)
            {
                stage->camera.up = cmd->args;
            }
            break;
        case 10:
            stage->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            stage->cameraFadeMode[2] = *(int *)&cmd->args.y;
            stage->cameraFadeTimer[2] = 0;
            break;
        case 11:
            stage->cameraTo.fov = stage->cameraFrom.fov;
            stage->cameraFrom.fov = cmd->args.x;
            if (stage->cameraFadeFrames[3] == 0)
            {
                stage->camera.fov = cmd->args.x;
            }
            break;
        case 12:
            stage->cameraFadeFrames[3] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[3] = 0;
            stage->cameraFadeMode[3] = *(int *)&cmd->args.y;
            break;
        case 13:
            stage->clearColor = *(D3DCOLOR *)&cmd->args.x;
            break;
        case 3:
            if (stage->waitLabel != 0)
            {
                stage->waitLabel = 0;
                break;
            }
            goto done;
        case 4:
            stage->scriptIndex = *(int *)&cmd->args.x;
            stage->scriptTime = *(int *)&cmd->args.y;
            stage->cameraFadeFrames[0] = 0;
            stage->cameraOffsetChanged = 1;
            goto loop;
        case 14:
            stage->cameraTo.eye = cmd->args;
            break;
        case 15:
            stage->cameraFrom.eye = cmd->args;
            break;
        case 16:
            stage->cameraFadeStart.eye = cmd->args;
            break;
        case 17:
            stage->cameraFadeTarget.eye = cmd->args;
            break;
        case 18:
            stage->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[0] = 0;
            stage->cameraFadeMode[0] = 7;
            break;
        case 19:
            stage->cameraTo.at = cmd->args;
            break;
        case 20:
            stage->cameraFrom.at = cmd->args;
            break;
        case 21:
            stage->cameraFadeStart.at = cmd->args;
            break;
        case 22:
            stage->cameraFadeTarget.at = cmd->args;
            break;
        case 23:
            stage->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[1] = 0;
            stage->cameraFadeMode[1] = 7;
            break;
        case 24:
            stage->cameraTo.up = cmd->args;
            break;
        case 25:
            stage->cameraFrom.up = cmd->args;
            break;
        case 26:
            stage->cameraFadeStart.up = cmd->args;
            break;
        case 27:
            stage->cameraFadeTarget.up = cmd->args;
            break;
        case 28:
            stage->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            stage->cameraFadeTimer[2] = 0;
            stage->cameraFadeMode[2] = 7;
            break;
        case 29:
            if (*(int *)&cmd->args.x >= 0)
            {
                stage->anm->StartScriptAtOrigin(&stage->backgroundLayer0, *(int *)&cmd->args.x);
            }
            else
            {
                stage->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 30:
            // Hides backgroundLayer0, not backgroundLayer1, for a negative
            // script, as th07's.
            if (*(int *)&cmd->args.x >= 0)
            {
                stage->anm->StartScriptAtOrigin(&stage->backgroundLayer1, *(int *)&cmd->args.x);
            }
            else
            {
                stage->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 33:
            stage->shakeMode = *(BYTE *)&cmd->args.x;
            stage->cameraFadeFrames[4] = 0;
            stage->cameraFadeTimer[4] = 0;
            stage->cameraFadeMode[4] = 0;
            break;
        case 32:
            stage->camera.offset = cmd->args;
            break;
        case 34:
            if (*(int *)&cmd->args.x >= 0)
            {
                stage->anm->StartScriptAtOrigin(&stage->backgroundLayer2, *(int *)&cmd->args.x);
            }
            else
            {
                stage->backgroundLayer2.spriteIndex = -1;
            }
            break;
        }
        stage->scriptIndex++;
        goto loop;
    }
done:
    {
        int i;
        float delta;
        float t;

        i = 0;
        if (stage->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(stage, i, &stage->camera.eye, &stage->cameraTo.eye, &stage->cameraFrom.eye, &stage->cameraFadeStart.eye, &stage->cameraFadeTarget.eye);
        }
        i = 1;
        if (stage->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(stage, i, &stage->camera.at, &stage->cameraTo.at, &stage->cameraFrom.at, &stage->cameraFadeStart.at, &stage->cameraFadeTarget.at);
        }
        i = 2;
        if (stage->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(stage, i, &stage->camera.up, &stage->cameraTo.up, &stage->cameraFrom.up, &stage->cameraFadeStart.up, &stage->cameraFadeTarget.up);
        }
        i = 3;
        if (stage->cameraFadeFrames[i] != 0)
        {
            if (stage->cameraFadeTimer[i] < stage->cameraFadeFrames[i])
            {
                stage->cameraFadeTimer[i]++;
                t = stage->cameraFadeTimer[i].AsFloat() / stage->cameraFadeFrames[i];
            }
            else
            {
                stage->cameraFadeTimer[i] = stage->cameraFadeFrames[i];
                t = 1.0f;
                stage->cameraFadeFrames[i] = 0;
            }
            switch (stage->cameraFadeMode[i])
            {
            case 1:
                t = 1.0f - (t = 1.0f - t) * t;
                break;
            case 2:
                t = 1.0f - (t = 1.0f - t) * t * t;
                break;
            case 3:
                t = 1.0f - (t = 1.0f - t) * t * t * t;
                break;
            case 4:
                t = t * t;
                break;
            case 5:
                t = t * t * t;
                break;
            case 6:
                t = t * t * t * t;
                break;
            }
            stage->camera.fov = (delta = stage->cameraFrom.fov - stage->cameraTo.fov) * t + stage->cameraTo.fov;
        }
    }
    D3DXVec3Normalize(&stage->camera.dir, &stage->camera.at);
    if (stage->shakeMode)
    {
        switch (stage->shakeMode)
        {
        case 1:
        {
            float angle;

            angle = stage->cameraFadeTimer[4].AsFloat() * 3.1415927f * 2.0f / 480.0f - 3.1415927f;
            stage->camera.offset.x = sinf(angle) * 40.0f;
            stage->cameraFadeTimer[4]++;
            if (stage->cameraFadeTimer[4] >= 480)
            {
                stage->cameraFadeTimer[4] = 0;
            }
            break;
        }
        case 2:
        {
            float angle;

            angle = stage->cameraFadeTimer[4].AsFloat() * 3.1415927f * 2.0f / 480.0f - 3.1415927f;
            stage->camera.offset.x = sinf(angle) * 70.0f;
            stage->camera.up.x = -sinf(angle) * 0.1f;
            stage->cameraFadeTimer[4]++;
            if (stage->cameraFadeTimer[4] >= 480)
            {
                stage->cameraFadeTimer[4] = 0;
            }
            break;
        }
        case 3:
        {
            float angle;

            angle = stage->cameraFadeTimer[4].AsFloat() * 3.1415927f * 2.0f / 4800.0f - 3.1415927f;
            stage->camera.up.x = sinf(angle) * 1.0f;
            stage->camera.up.z = cosf(angle) * 1.0f;
            stage->cameraFadeTimer[4]++;
            if (stage->cameraFadeTimer[4] >= 4800)
            {
                stage->cameraFadeTimer[4] = 0;
            }
            break;
        }
        }
    }
    if (stage->fogFadeFrames != 0)
    {
        float fogT;
        int j;

        stage->fogFadeTime++;
        fogT = stage->fogFadeTime.AsFloat() / stage->fogFadeFrames;
        if (fogT >= 1.0f)
        {
            fogT = 1.0f;
        }
        for (j = 0; j < 4; j++)
        {
            ((BYTE *)&stage->fog.color)[j] =
                ((float)((BYTE *)&stage->fogTarget.color)[j] - (float)((BYTE *)&stage->fogStart.color)[j]) * fogT +
                (float)((BYTE *)&stage->fogStart.color)[j];
        }
        stage->fog.nearPlane = (stage->fogTarget.nearPlane - stage->fogStart.nearPlane) * fogT + stage->fogStart.nearPlane;
        stage->fog.farPlane = (stage->fogTarget.farPlane - stage->fogStart.farPlane) * fogT + stage->fogStart.farPlane;
        if (stage->fogFadeTime >= stage->fogFadeFrames)
        {
            stage->fogFadeFrames = 0;
        }
    }
    if (cmd->opcode != 3)
    {
        stage->scriptTime++;
    }
    stage->StepObjects();
    if (stage->spellCardState >= 1)
    {
        int k;

        if (stage->spellCardFrames == 60)
        {
            stage->spellCardState++;
        }
        stage->spellCardFrames++;
        for (k = 0; k < stage->dialogueSpriteCount; k++)
        {
            g_SpriteEngine->Step(&stage->dialogueSprites[k]);
        }
    }
    if (stage->backgroundLayer0.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer0);
    }
    if (stage->backgroundLayer1.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer1);
    }
    if (stage->backgroundLayer2.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer2);
        stage->clearColor = stage->backgroundLayer2.color;
    }
    if (stage->checksumCheckFrames % 3 == 0 &&
        (stage->checksumCheckFrames >= 700 || g_GameManager.IsInSpellCardPractice()) && stage->spellCardState < 2)
    {
        int l;
        Effect *effect;

        for (l = 0; l < 0xc; l++)
        {
            effect = g_EffectManager.SpawnEffect(0x3e, &stage->effectSpawnPos[l], 1, 0x20ffffff);
            effect->drawList = 4;
        }
    }
    stage->recordEffectSpawnPos = 1;
    if (stage->spellCardState >= 2)
    {
        stage->effectSpawnPosCount = 0;
    }
    stage->checksumCheckFrames++;
    if (stage->checksumCheckFrames % 500 == 250 && g_GameManager.ChecksumMismatch())
    {
        return 4;
    }
    return 1;
}

void UpdateEasedVector(Stage *stage, int i, D3DXVECTOR3 *out, D3DXVECTOR3 *from, D3DXVECTOR3 *to, D3DXVECTOR3 *m0, D3DXVECTOR3 *m1)
{
    float t;

    if (stage->cameraFadeTimer[i] < stage->cameraFadeFrames[i])
    {
        stage->cameraFadeTimer[i]++;
        t = stage->cameraFadeTimer[i].AsFloat() / stage->cameraFadeFrames[i];
    }
    else
    {
        stage->cameraFadeTimer[i] = stage->cameraFadeFrames[i];
        t = 1.0f;
        stage->cameraFadeFrames[i] = 0;
    }
    switch (stage->cameraFadeMode[i])
    {
    case 1:
        t = 1.0f - (t = 1.0f - t) * t;
        break;
    case 2:
        t = 1.0f - (t = 1.0f - t) * t * t;
        break;
    case 3:
        t = 1.0f - (t = 1.0f - t) * t * t * t;
        break;
    case 4:
        t = t * t;
        break;
    case 5:
        t = t * t * t;
        break;
    case 6:
        t = t * t * t * t;
        break;
    }
    if (stage->cameraFadeMode[i] != 7)
    {
        *out = *to - *from;
        *out = *out * t + *from;
    }
    else
    {
        out->x = HermiteInterpolate(from->x, to->x, m0->x, m1->x, t);
        out->y = HermiteInterpolate(from->y, to->y, m0->y, m1->y, t);
        out->z = HermiteInterpolate(from->z, to->z, m0->z, m1->z, t);
    }
}

float HermiteInterpolate(float p0, float p1, float m0, float m1, float t)
{
    float f00;
    float f10;
    float f01;
    float f11;

    f00 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    f01 = t * t * (3.0f - 2.0f * t);
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    return f00 * p0 + f01 * p1 + f10 * m0 + f11 * m1;
}

void Stage::SetTintColor(D3DCOLOR color)
{
    ColorBytes c;

    if (this->tintColor.a == 0)
    {
        this->tintColor.color = color;
    }
    else
    {
        c.color = color;
        this->tintColor.r = (DWORD)(c.r + this->tintColor.r) >> 1;
        this->tintColor.g = (DWORD)(c.g + this->tintColor.g) >> 1;
        this->tintColor.b = (DWORD)(c.b + this->tintColor.b) >> 1;
        this->tintColor.a = (DWORD)(c.a + this->tintColor.a) >> 1;
    }
}

int Stage::OnDraw(Stage *stage)
{
    int i;

    stage->effectSpawnPosCount = 0;
    for (i = 0; i < 0x10; i++)
    {
        stage->effectSpawnPos[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    g_App.viewport.X = 32;
    g_App.viewport.Y = 16;
    g_App.viewport.Width = 384;
    g_App.viewport.Height = 448;
    g_SpriteEngine->BeginQuadBatch();
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_SpriteEngine->ClearDrawStats();
    g_SpriteEngine->ClearLastCameraSetupFlag();
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_SpriteEngine->FlushQuadBatch();
    if (stage->clearScreenOnce != 0)
    {
        D3DVIEWPORT8 viewport;

        viewport.X = 32;
        viewport.Y = 16;
        viewport.Width = 384;
        viewport.Height = 448;
        g_App.d3dDevice->SetViewport(&viewport);
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
        stage->clearScreenOnce = 0;
    }
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    if (stage->tintColor.a > 0)
    {
        g_SpriteEngine->SetColorModulation(stage->tintColor.color);
    }
    stage->tintColor.a = 0;
    stage->tintColor.r = 0x80;
    stage->tintColor.g = 0x80;
    stage->tintColor.b = 0x80;
    if (stage->spellCardState <= 1 && !g_Gui.IsLoadingShown())
    {
        if (stage->backgroundLayer0.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer0);
        }
        if (stage->backgroundLayer1.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer1);
        }
        if (stage->stage7Effect != NULL)
        {
            Effect *effect;

            effect = stage->stage7Effect;
            effect->draw(effect);
        }
    }
    if ((stage->clearColor & 0xff000000) == 0xff000000)
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    else if (stage->clearColor != 0)
    {
        float area[4];

        area[0] = 32.0f;
        area[1] = 16.0f;
        area[2] = 416.0f;
        area[3] = 464.0f;
        DrawColorRect(area, stage->clearColor);
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    else
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (g_SpriteEngine->modulateColorEnabled == 0)
    {
        g_App.SetRenderState(D3DRS_FOGCOLOR, stage->fog.color);
    }
    else
    {
        D3DCOLOR fogColor;

        fogColor = stage->fog.color;
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->modulateColor, 2);
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->modulateColor, 1);
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->modulateColor, 0);
        g_App.SetRenderState(D3DRS_FOGCOLOR, fogColor);
    }
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->fog.nearPlane);
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->fog.farPlane);
    if (!g_App.GetNoFog())
    {
        g_App.EnableFog();
    }
    if (stage->spellCardState <= 1 && !g_Gui.IsLoadingShown())
    {
        stage->DrawLayer(0);
        stage->DrawLayer(1);
    }
    return 1;
}

int Stage::OnDraw2(Stage *stage)
{
    float fog;
    int opacity;
    float area[4];
    int j;

    if (stage->spellCardState <= 1 && !g_Gui.IsLoadingShown())
    {
        stage->DrawLayer(2);
        stage->DrawLayer(3);
        if (!g_App.GetNoFog())
        {
            g_App.DisableFog();
        }
        g_EffectManager.OnDraw2();
        if (stage->spellCardState == 1)
        {
            area[0] = 32.0f;
            area[1] = 16.0f;
            area[2] = 416.0f;
            area[3] = 464.0f;
            opacity = stage->spellCardFrames * 0xff / 60;
            g_SpriteEngine->FlushQuadBatch();
            g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            if (!g_App.GetNoFog())
            {
                g_App.SetRenderState(D3DRS_FOGENABLE, FALSE);
            }
            DrawColorRect(area, opacity << 24);
        }
    }
    g_SpriteEngine->FlushQuadBatch();
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    if (stage->spellCardState >= 1)
    {
        for (j = 0; j < stage->dialogueSpriteCount; j++)
        {
            g_SpriteEngine->DrawAndFlush(&stage->dialogueSprites[j]);
        }
        if (stage->spellCardDrawCallback != NULL)
        {
            stage->spellCardDrawCallback();
        }
    }
    g_SpriteEngine->SetLastCameraSetupFlag(0);
    stage->SetupFixedCamera();
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    fog = 1000.0f;
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog);
    fog = 2000.0f;
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog);
    if (stage->keepTintColor == 0)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    stage->keepTintColor = 0;
    stage->recordEffectSpawnPos = 0;
    return 1;
}

int Stage::AddedCallback(Stage *stage)
{
    int i;

    stage->scriptTime = 0;
    stage->scriptIndex = 0;
    stage->cameraPos.x = 0.0f;
    stage->cameraPos.y = 0.0f;
    stage->cameraPos.z = 0.0f;
    stage->spellCardState = 0;
    stage->fogFadeFrames = 0;
    if (!IsRestarting())
    {
        stage->anm = g_SpriteEngine->PreloadAnm(4, g_StageAnmPaths[g_GameManager.stage]);
        if (stage->anm == NULL)
        {
            return -1;
        }
    }
    else
    {
        stage->anm = g_SpriteEngine->GetAnm(4);
    }
    if (!g_GameManager.IsInSpellCardPractice())
    {
        if (stage->Load(g_StageDataFiles[g_GameManager.stage]) != 0)
        {
            return -1;
        }
    }
    else
    {
        if (stage->Load(g_SpellPracticeStageDataFiles[g_GameManager.stage]) != 0)
        {
            return -1;
        }
    }
    stage->fog.color = 0xff000000;
    stage->fog.nearPlane = 200.0f;
    stage->fog.farPlane = 500.0f;
    stage->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    stage->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    stage->camera.offset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    stage->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    stage->camera.fov = 0.5235988f;
    stage->cameraFrom = stage->camera;
    stage->cameraTo = stage->camera;
    stage->shakeMode = 0;
    for (i = 0; i < 4; i++)
    {
        stage->cameraFadeFrames[i] = 0;
        stage->cameraFadeTimer[i] = 0;
    }
    stage->waitLabel = 0;
    stage->drawDistanceSq = 1322500.0f;
    if (g_GameManager.stage == 5)
    {
        stage->drawDistanceSq = 1822500.0f;
    }
    else if (g_GameManager.stage == 6 || g_GameManager.stage == 7)
    {
        stage->drawDistanceSq = 3240000.0f;
    }
    return 0;
}

int Stage::RegisterChain(int stage)
{
    Stage *s = &g_Stage;
    StageData *prevData;

    if (IsRestarting())
    {
        prevData = s->data;
    }
    memset(s, 0, sizeof(Stage));
    if (IsRestarting())
    {
        s->data = prevData;
    }
    s->checksumCheckFrames = 0;
    s->stage = stage;
    g_StageCalcChain.SetCallback((ChainCallback)Stage::OnUpdate);
    g_StageCalcChain.addedCallback = (ChainCallback)Stage::AddedCallback;
    g_StageCalcChain.deletedCallback = (ChainCallback)Stage::DeletedCallback;
    g_StageCalcChain.arg = s;
    if (g_Chain.AddToCalcChain(&g_StageCalcChain, 8) != 0)
    {
        return -1;
    }
    g_StageDrawChain.SetCallback((ChainCallback)Stage::OnDraw);
    g_StageDrawChain.arg = s;
    g_Chain.AddToDrawChain(&g_StageDrawChain, 6);
    g_StageDrawChain2.SetCallback((ChainCallback)Stage::OnDraw2);
    g_StageDrawChain2.arg = s;
    g_Chain.AddToDrawChain(&g_StageDrawChain2, 7);
    return 0;
}

int Stage::DeletedCallback(Stage *stage)
{
    if (!IsRestarting())
    {
        g_SpriteEngine->ReleaseAnm(4);
    }
    if (stage->sprites != NULL)
    {
        g_MemoryManager.Free(stage->sprites);
        stage->sprites = NULL;
    }
    if (!IsRestarting() && stage->data != NULL)
    {
        g_MemoryManager.Free(stage->data);
        stage->data = NULL;
    }
    return 0;
}

void Stage::RemoveChain()
{
    g_Chain.Remove(&g_StageCalcChain);
    g_Chain.Remove(&g_StageDrawChain);
    g_Chain.Remove(&g_StageDrawChain2);
}

int Stage::Load(char *dataPath)
{
    int count;
    int i;
    StageObject *ob;
    StageQuad *quad;

    if (!IsRestarting())
    {
        this->data = (StageData *)LoadFileData(dataPath, NULL, 0);
        if (this->data == NULL)
        {
            LogPrintf(&g_Log, "ステージデータが見つかりません。データが壊れています\r\n");
            return -1;
        }
    }
    this->objectCount = this->data->objectCount;
    this->quadCount = this->data->quadCount;
    this->instances = (StageInstance *)(this->data->instancesOffset + (int)this->data);
    this->script = (StageInstr *)(this->data->scriptOffset + (int)this->data);
    this->objects = (StageObject **)((char *)this->data + 0x490);
    if (!IsRestarting())
    {
        for (i = 0; i < this->objectCount; i++)
        {
            this->objects[i] = (StageObject *)((int)this->objects[i] + (int)this->data);
        }
    }
    this->sprites = (Sprite *)g_MemoryManager.Alloc(this->quadCount * sizeof(Sprite), "bgscroll");
    for (i = 0, count = 0; i < this->objectCount; i++)
    {
        ob = this->objects[i];
        ob->flags = 1;
        quad = ob->quads;
        while (quad->type >= 0)
        {
            this->anm->StartScriptAtOrigin(&this->sprites[count], quad->script);
            quad->spriteIndex = count;
            count++;
            quad = (StageQuad *)((char *)quad + quad->size);
        }
    }
    switch (g_GameManager.stage)
    {
    case 2:
        g_App.textAnm->StartScript(&this->colorScaleSprite, 0x21);
        break;
    default:
        g_App.textAnm->StartScript(&this->colorScaleSprite, 0x21);
        break;
    }
    this->colorScaleSprite.SetPendingInterrupt(2);
    this->colorScaleOn = 0;
    this->colorScaleTimer = 0;
    return 0;
}

int Stage::StepObjects()
{
    StageQuad *altQuad;
    int busy;
    int i;
    Sprite *sprite;
    StageObject *obj;
    StageQuad *piece;

    if (this->colorScaleOn)
    {
        if (g_Player.IsUnk5Clear())
        {
            this->colorScaleOn = 0;
            this->colorScaleTimer = 0;
            this->colorScaleSprite.SetPendingInterrupt(2);
        }
    }
    else
    {
        if (g_Player.IsUnk5Set())
        {
            this->colorScaleOn = 1;
            this->colorScaleTimer = 0;
            this->colorScaleSprite.SetPendingInterrupt(1);
        }
    }
    this->colorScaleTimer++;
    g_SpriteEngine->Step(&this->colorScaleSprite);
    for (i = 0; i < this->objectCount; i++)
    {
        obj = this->objects[i];
        if (obj->flags & 1)
        {
            busy = 0;
            piece = obj->quads;
            while (piece->type >= 0)
            {
                sprite = &this->sprites[piece->spriteIndex];
                switch (piece->type)
                {
                case 0:
                    g_SpriteEngine->Step(sprite);
                    break;
                case 1:
                    altQuad = piece;
                    g_SpriteEngine->Step(sprite);
                    break;
                }
                if (sprite->scriptPos != NULL)
                {
                    busy++;
                }
                piece = (StageQuad *)((char *)piece + piece->size);
            }
            if (sprite->autoRotate == 1)
            {
                sprite->flags |= 0x20000;
                sprite->color2Rgb[2] = (DWORD)sprite->colorRgb[2] * this->colorScaleSprite.colorRgb[2] >> 8;
                sprite->color2Rgb[1] = (DWORD)sprite->colorRgb[1] * this->colorScaleSprite.colorRgb[1] >> 8;
                sprite->color2Rgb[0] = (DWORD)sprite->colorRgb[0] * this->colorScaleSprite.colorRgb[0] >> 8;
                sprite->color2Alpha = (DWORD)sprite->alpha * this->colorScaleSprite.alpha >> 8;
            }
            if (busy == 0)
            {
                obj->flags &= ~1;
            }
        }
    }
    return 0;
}

int Stage::DrawLayer(int layer)
{
    int fog;
    StageQuad *quad;
    float range;
    StageInstance *placement = this->instances;
    int objCount = 0;
    int any = 0;
    D3DXVECTOR3 centre;
    D3DXVECTOR3 rightValue;
    float dotVal;
    StageObject *objData;
    D3DXVECTOR3 diff2;
    Sprite *spr;
    D3DXVECTOR3 origin(0.0f, 0.0f, 0.0f);
    D3DCOLOR colVal;
    float w2;
    D3DXMATRIX mat;
    StageLineQuad *altQuad;

    fog = 0xff;
    this->SetupCamera();
    g_SpriteEngine->SetLastCameraSetupFlag(1);
    D3DXMatrixIdentity(&mat);
    rightValue.x = g_App.viewMatrix._11;
    rightValue.y = g_App.viewMatrix._12;
    rightValue.z = g_App.viewMatrix._13;
    D3DXVec3Normalize(&rightValue, &rightValue);
    while (placement->id >= 0)
    {
        objData = this->objects[placement->id];
        if (objData->layer == layer)
        {
            quad = objData->quads;
            centre.x = objData->pos.x + placement->pos.x - this->cameraPos.x + objData->size.x / 2.0f;
            centre.y = objData->pos.y + placement->pos.y - this->cameraPos.y + objData->size.y / 2.0f;
            centre.z = objData->pos.z + placement->pos.z - this->cameraPos.z + objData->size.z / 2.0f;
            centre = centre - (this->camera.eye + this->camera.offset);
            if (D3DXVec3LengthSq(&centre) > this->drawDistanceSq)
            {
                goto next;
            }
            dotVal = D3DXVec3Dot(&centre, &this->camera.dir);
            range = D3DXVec3Length(&objData->size) / 2.0f + 960.0f;
            if (dotVal > range || dotVal < 80.0f)
            {
                goto next;
            }
            objData->flags |= 2;
            any = 1;
            while (quad->type >= 0)
            {
                spr = &this->sprites[quad->spriteIndex];
                switch (quad->type)
                {
                case 0:
                    spr->pos.x = spr->posOffset.x + quad->pos.x + placement->pos.x - this->cameraPos.x;
                    spr->pos.y = spr->posOffset.y + quad->pos.y + placement->pos.y - this->cameraPos.y;
                    spr->pos.z = spr->posOffset.z + quad->pos.z + placement->pos.z - this->cameraPos.z;
                    // scaleY and scaleX are SpriteBase's +0x18 and +0x1c,
                    // really the horizontal and vertical scales.
                    if (quad->width != 0.0f)
                    {
                        spr->scaleY = quad->width / spr->spriteData->width;
                    }
                    if (quad->height != 0.0f)
                    {
                        spr->scaleX = quad->height / spr->spriteData->height;
                    }
                    if ((spr->autoRotate & 0xf) == 2)
                    {
                        mat._41 = spr->pos[0];
                        mat._42 = spr->pos[1];
                        mat._43 = spr->pos[2];
                        D3DXVec3Project(&centre, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                        (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                        if (quad->width != 0.0f)
                        {
                            w2 = quad->width;
                        }
                        else
                        {
                            w2 = spr->spriteData->width;
                        }
                        mat._41 += rightValue.x * w2 * spr->scaleY;
                        mat._42 += rightValue.y * w2 * spr->scaleY;
                        mat._43 += rightValue.z * w2 * spr->scaleY;
                        D3DXVec3Project(&diff2, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                        (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                        diff2 = diff2 - centre;
                        spr->scaleY = D3DXVec3Length(&diff2) / w2;
                        spr->scaleX = spr->scaleY;
                        if (w2 < 0.0f)
                        {
                            spr->scaleX = -spr->scaleX;
                        }
                        diff2 = spr->pos - (this->camera.eye + this->camera.offset);
                        w2 = D3DXVec3Length(&diff2);
                        colVal = spr->color;
                        if (this->fog.nearPlane < w2)
                        {
                            w2 = (this->fog.nearPlane - w2) / (this->fog.nearPlane - this->fog.farPlane);
                            if (w2 >= 1.0f)
                            {
                                goto nextQuad;
                            }
                            ((BYTE *)&spr->color)[0] = ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * w2);
                            ((BYTE *)&spr->color)[1] = ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * w2);
                            ((BYTE *)&spr->color)[2] = ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * w2);
                            ((BYTE *)&spr->color)[3] = ((BYTE *)&spr->color)[3] * (1.0f - w2);
                        }
                        spr->pos = centre;
                        if (spr->pos.z < 0.0f || spr->pos.z > 1.0f)
                        {
                        }
                        else
                        {
                            if (fog != 0)
                            {
                                if (!g_App.GetNoFog())
                                {
                                    g_App.DisableFog();
                                }
                                fog = 0;
                            }
                            g_SpriteEngine->DrawNoRound(spr);
                            if ((spr->autoRotate & 0xf0) == 0x10 && this->fog.nearPlane > w2 && this->recordEffectSpawnPos != 0)
                            {
                                this->effectSpawnPos[this->effectSpawnPosCount] = centre;
                                this->effectSpawnPos[this->effectSpawnPosCount].z = 0.0f;
                                this->effectSpawnPosCount++;
                            }
                        }
                        spr->color = colVal;
                    }
                    else
                    {
                        if (!g_App.GetNoFog() && fog != 1)
                        {
                            if (!g_App.GetNoFog())
                            {
                                g_App.EnableFog();
                            }
                            fog = 1;
                        }
                        g_SpriteEngine->Draw3D(spr);
                    }
                    break;
                case 1:
                {
                    altQuad = (StageLineQuad *)quad;
                    D3DXVECTOR3 begin;
                    D3DXVECTOR3 endPos;
                    VertexTex1DiffuseXyzrwh vertices[4];
                    float halfW;
                    float halfW2;
                    float dist;

                    begin.x = altQuad->start.x + placement->pos.x - this->cameraPos.x;
                    begin.y = altQuad->start.y + placement->pos.y - this->cameraPos.y;
                    begin.z = altQuad->start.z + placement->pos.z - this->cameraPos.z;
                    mat._41 = begin.x;
                    mat._42 = begin.y;
                    mat._43 = begin.z;
                    D3DXVec3Project(&centre, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                    (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                    if (altQuad->width != 0.0f)
                    {
                        dist = altQuad->width;
                    }
                    else
                    {
                        dist = spr->spriteData->width;
                    }
                    mat._41 += rightValue.x * dist;
                    mat._42 += rightValue.y * dist;
                    mat._43 += rightValue.z * dist;
                    D3DXVec3Project(&diff2, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                    (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                    diff2 = diff2 - centre;
                    halfW = D3DXVec3Length(&diff2) / 2.0f;
                    diff2 = begin - (this->camera.eye + this->camera.offset);
                    dist = D3DXVec3Length(&diff2);
                    if (this->fog.nearPlane < dist)
                    {
                        dist = (this->fog.nearPlane - dist) / (this->fog.nearPlane - this->fog.farPlane);
                        if (dist < 1.0f)
                        {
                            ((BYTE *)&vertices[0].diffuse)[0] = ((BYTE *)&vertices[1].diffuse)[0] =
                                ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[1] = ((BYTE *)&vertices[1].diffuse)[1] =
                                ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[2] = ((BYTE *)&vertices[1].diffuse)[2] =
                                ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[3] = ((BYTE *)&vertices[1].diffuse)[3] =
                                ((BYTE *)&spr->color)[3] * (1.0f - dist);
                        }
                        else
                        {
                            ((BYTE *)&vertices[0].diffuse)[3] = ((BYTE *)&vertices[1].diffuse)[3] = 0;
                        }
                    }
                    else
                    {
                        vertices[0].diffuse = vertices[1].diffuse = spr->color;
                    }
                    begin.x = altQuad->end.x + placement->pos.x - this->cameraPos.x;
                    begin.y = altQuad->end.y + placement->pos.y - this->cameraPos.y;
                    begin.z = altQuad->end.z + placement->pos.z - this->cameraPos.z;
                    mat._41 = begin.x;
                    mat._42 = begin.y;
                    mat._43 = begin.z;
                    D3DXVec3Project(&endPos, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                    (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                    if (altQuad->width != 0.0f)
                    {
                        dist = altQuad->width;
                    }
                    else
                    {
                        dist = spr->spriteData->width;
                    }
                    mat._41 += rightValue.x * dist;
                    mat._42 += rightValue.y * dist;
                    mat._43 += rightValue.z * dist;
                    D3DXVec3Project(&diff2, &origin, &g_App.viewport, (D3DXMATRIX *)&g_App.projectionMatrix,
                                    (D3DXMATRIX *)&g_App.viewMatrix, &mat);
                    diff2 = diff2 - endPos;
                    halfW2 = D3DXVec3Length(&diff2) / 2.0f;
                    diff2 = begin - (this->camera.eye + this->camera.offset);
                    dist = D3DXVec3Length(&diff2);
                    if (this->fog.nearPlane < dist)
                    {
                        dist = (this->fog.nearPlane - dist) / (this->fog.nearPlane - this->fog.farPlane);
                        if (dist < 1.0f)
                        {
                            ((BYTE *)&vertices[2].diffuse)[0] = ((BYTE *)&vertices[3].diffuse)[0] =
                                ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[1] = ((BYTE *)&vertices[3].diffuse)[1] =
                                ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[2] = ((BYTE *)&vertices[3].diffuse)[2] =
                                ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[3] = ((BYTE *)&vertices[3].diffuse)[3] =
                                ((BYTE *)&spr->color)[3] * (1.0f - dist);
                        }
                        else
                        {
                            ((BYTE *)&vertices[2].diffuse)[3] = ((BYTE *)&vertices[3].diffuse)[3] = 0;
                        }
                    }
                    else
                    {
                        vertices[2].diffuse = vertices[3].diffuse = spr->color;
                    }
                    origin = endPos - centre;
                    dist = sqrtf(origin.x * origin.x + origin.y * origin.y);
                    if (dist < 0.00001f)
                    {
                        goto nextQuad;
                    }
                    origin /= dist;
                    if (centre.z < 0.0f || centre.z > 1.0f)
                    {
                        goto nextQuad;
                    }
                    if (endPos.z < 0.0f || endPos.z > 1.0f)
                    {
                        goto nextQuad;
                    }
                    vertices[0].pos.x = origin.y * halfW + centre.x;
                    vertices[0].pos.y = centre.y - origin.x * halfW;
                    vertices[0].pos.z = centre.z;
                    vertices[1].pos.x = centre.x - origin.y * halfW;
                    vertices[1].pos.y = origin.x * halfW + centre.y;
                    vertices[1].pos.z = centre.z;
                    vertices[2].pos.x = origin.y * halfW2 + endPos.x;
                    vertices[2].pos.y = endPos.y - origin.x * halfW2;
                    vertices[2].pos.z = endPos.z;
                    vertices[3].pos.x = endPos.x - origin.y * halfW2;
                    vertices[3].pos.y = origin.x * halfW2 + endPos.y;
                    vertices[3].pos.z = endPos.z;
                    vertices[0].uv.x = vertices[2].uv.x = spr->spriteData->uvStartX + spr->uvScrollX;
                    vertices[1].uv.x = vertices[3].uv.x = spr->spriteData->uvEndX + spr->uvScrollX;
                    vertices[0].uv.y = vertices[1].uv.y = spr->spriteData->uvStartY + spr->uvScrollY;
                    vertices[2].uv.y = vertices[3].uv.y = spr->spriteData->uvEndY + spr->uvScrollY;
                    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
                    if (fog != 0)
                    {
                        if (!g_App.GetNoFog())
                        {
                            g_App.DisableFog();
                        }
                        fog = 0;
                    }
                    g_SpriteEngine->DrawVertexQuad(spr, vertices);
                    break;
                }
                }
            nextQuad:
                quad = (StageQuad *)((char *)quad + quad->size);
            }
            objCount++;
        }
    next:
        placement++;
    }
    return 0;
}

void Stage::SetupFixedCamera()
{
    float midX;
    float halfHeight;
    float aspectXY;
    float fovAngle;
    float zDist;

    midX = (float)g_App.viewport.Width / 2.0f;
    halfHeight = (float)g_App.viewport.Height / 2.0f;
    aspectXY = (float)g_App.viewport.Width / (float)g_App.viewport.Height;
    fovAngle = 0.31415927f;
    zDist = halfHeight / (float)tan(fovAngle / 2.0f);
    D3DXMatrixLookAtLH((D3DXMATRIX *)&g_App.viewMatrix, &D3DXVECTOR3(midX, halfHeight, zDist), &D3DXVECTOR3(midX, halfHeight, 0.0f),
                       &D3DXVECTOR3(0.0f, -1.0f, 0.0f));
    D3DXMatrixPerspectiveFovLH((D3DXMATRIX *)&g_App.projectionMatrix, fovAngle, aspectXY, 1.0f, 10000.0f);
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &g_App.viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_App.projectionMatrix);
}

void Stage::SetupCamera()
{
    D3DXVECTOR3 at = this->camera.at + this->camera.eye;
    D3DXVECTOR3 eye = this->camera.offset + this->camera.eye;

    D3DXMatrixLookAtLH((D3DXMATRIX *)&g_App.viewMatrix, &eye, &at, &this->camera.up);
    D3DXMatrixPerspectiveFovLH((D3DXMATRIX *)&g_App.projectionMatrix, this->camera.fov,
                               (float)g_App.viewport.Width / (float)g_App.viewport.Height, 30.0f, 1800.0f);
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &g_App.viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_App.projectionMatrix);
    D3DXVec3Cross(&this->camera.cameraRight, &this->camera.at, &this->camera.up);
    D3DXVec3Normalize(&this->camera.cameraRight, &this->camera.cameraRight);
}

// An inline of unsorted.h, probably, whose out-of-line copy the original
// keeps with this file's inline copies; defined here so that the other
// files' calls stay calls, as th07's. checksum has to be
// lifeChecksumKeys[2] plus checksumKeysE[2] for each byte ComputeChecksum
// sums (0x1f4 past checksum), computedChecksum plus checksumKeysD[3]
// checksumValue.
int GameManager::ChecksumMismatch()
{
    return this->stats->checksum == this->stats->lifeChecksumKeys[2] + this->stats->checksumKeysE[2] * ((BYTE *)&this->stats->checksum - (BYTE *)this->stats->lifeChecksumKeys + 0x1f4) &&
                   this->stats->computedChecksum + this->stats->checksumKeysD[3] == (int)this->checksumValue
               ? FALSE
               : TRUE;
}
