// The stage background, th07's Stage, from 0x4071a0, as th07's
// stage.cpp.

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

    stage->unk_6478 = 0;
    for (i = 0; i < 0x10; i++)
    {
        stage->unk_6480[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
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
        if (stage->unk_ae8 != NULL)
        {
            Effect *effect;

            effect = stage->unk_ae8;
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
        if (stage->unk_625c != NULL)
        {
            stage->unk_625c();
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
    stage->unk_647c = 0;
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
    stage->unk_6470 = 1322500.0f;
    if (g_GameManager.stage == 5)
    {
        stage->unk_6470 = 1822500.0f;
    }
    else if (g_GameManager.stage == 6 || g_GameManager.stage == 7)
    {
        stage->unk_6470 = 3240000.0f;
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
        g_App.textAnm->StartScript(&this->unk_844, 0x21);
        break;
    default:
        g_App.textAnm->StartScript(&this->unk_844, 0x21);
        break;
    }
    this->unk_844.SetPendingInterrupt(2);
    this->unk_834 = 0;
    this->unk_838 = 0;
    return 0;
}
