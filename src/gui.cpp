// cflags: /Os
// The /Os file with Gui, as th07's, from 0x4338ca to GameManager at
// 0x4399ac.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "enemy.h"
#include "item.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

int Gui::OnUpdate(Gui *gui)
{
    if (g_GameManager.isFrozen)
    {
        return 1;
    }
    gui->UpdateInterface();
    gui->impl->RunMsg();
    if ((g_Buttons & 0x100) && g_App.redrawFrames < 8)
    {
        g_App.redrawFrames = 8;
    }
    gui->frameCounter++;
    return 1;
}

int Gui::OnDraw(Gui *gui)
{
    if (gui->impl->stageClearShown != 0)
    {
        gui->DrawStageClear();
    }
    gui->impl->DrawMsg();
    gui->DrawBanners();
    gui->DrawHud();
    gui->DrawPopups();
    return 1;
}

void GuiImpl::StartMsgScript(int index)
{
    MsgFile *msgFile;

    DebugPrint2("msg start %d\n\r", index);
    msgFile = this->msg.msgFile;
    memset(&this->msg, 0, sizeof(GuiMsg));
    this->msg.msgFile = msgFile;
    if (index == 0)
    {
        switch (g_GameManager.stage)
        {
        case 5:
            CopySpriteToTexture(0x16);
            break;
        case 6:
            g_Stage.unk_b34 = 2;
            break;
        case 7:
        {
            int tmp = g_Unknown4ecc9c;
            g_Unknown4ecc9c = g_Unknown4ecca0;
            g_Unknown4ecca0 = tmp;
            g_Stage.unk_b34 = 2;
            CopySpriteToTexture(0x18);
            break;
        }
        case 8:
        {
            int tmp = g_Unknown4ecc9c;
            g_Unknown4ecc9c = g_Unknown4ecca0;
            g_Unknown4ecca0 = tmp;
            g_Stage.unk_b34 = 2;
            CopySpriteToTexture(0x19);
            break;
        }
        }
    }
    else if (index == 10)
    {
        switch (g_GameManager.stage)
        {
        case 5:
            if (g_GameManager.stats->continuesUsed > 0)
            {
                index = 1;
                this->msg.clearLevel = 0;
            }
            else if (!g_GameManager.IsReplay())
            {
                if (g_GameManager.HasClearedStage(7, g_GameManager.character, 0) ||
                    g_GameManager.HasClearedStage(7, g_GameManager.character, 1) ||
                    g_GameManager.HasClearedStage(7, g_GameManager.character, 2) ||
                    g_GameManager.HasClearedStage(7, g_GameManager.character, 3) || g_GameManager.character > 3)
                {
                    index = 3;
                    this->msg.clearLevel = 1;
                }
                else if (g_GameManager.HasClearedStageWithContinues(6, g_GameManager.character, 0) ||
                         g_GameManager.HasClearedStageWithContinues(6, g_GameManager.character, 1) ||
                         g_GameManager.HasClearedStageWithContinues(6, g_GameManager.character, 2) ||
                         g_GameManager.HasClearedStageWithContinues(6, g_GameManager.character, 3))
                {
                    index = 2;
                    this->msg.clearLevel = 1;
                }
                else
                {
                    index = 1;
                    this->msg.clearLevel = 0;
                }
            }
            else if (g_ReplayManager->data->clearLevel == 2)
            {
                index = 3;
                this->msg.clearLevel = 1;
            }
            else if (g_ReplayManager->data->clearLevel == 1)
            {
                index = 2;
                this->msg.clearLevel = 1;
            }
            else
            {
                index = 1;
                this->msg.clearLevel = 0;
            }
            g_GameManager.clearLevel = this->msg.clearLevel;
            break;
        }
    }
    else if (index >= 6)
    {
        switch (g_GameManager.stage)
        {
        case 7:
            if (g_GameManager.GetUnk28() >= 12)
            {
                index = 5;
            }
            break;
        }
    }
    this->msg.scriptIndex = index;
    this->msg.instr = this->msg.msgFile->scripts[index];
    this->msg.text[0].scriptIndex = -1;
    this->msg.text[1].scriptIndex = -1;
    this->msg.unk_156d = 1;
    this->msg.fontSize = 15;
    this->msg.textColors[0] = g_MsgColors[g_GameManager.character][0];
    this->msg.textColors[1] = g_MsgColors[g_GameManager.character][1];
    this->msg.textColors[2] = g_MsgColors[g_GameManager.character][2];
    this->msg.textColors[3] = g_MsgColors[g_GameManager.character][3];
    this->msg.shadowColors[0] = 0;
    this->msg.shadowColors[1] = 0;
    this->msg.shadowColors[2] = 0;
    this->msg.shadowColors[3] = 0;
    this->msg.skippable = 1;
    this->msg.unk_1c = 6;
    this->msg.unk_1569 = 0;
    this->msg.unk_156a = 1;
    this->msg.unk_156b = 0;
    this->msg.unk_156c = 0xff;
    g_BulletManager.Func415c60();
    g_EnemyManager.ClearEnemiesForBonus(0, 0);
    g_ItemManager.CollectAll();
}

void DecodeMsgString(char *dst, char *src)
{
    int i;
    char c;

    i = 0;
    do
    {
        c = *src ^ 0x77;
        dst[i] = c;
        i++;
        src++;
    } while (c != 0);
}

int GuiImpl::DrawMsg()
{
    float height;

    if (this->msg.scriptIndex < 0)
    {
        return -1;
    }
    if (this->msg.timer < 60)
    {
        height = this->msg.timer.AsFloat() * 48.0f / 60.0f;
    }
    else
    {
        height = 48.0f;
    }
    VertexDiffuseXyzrwh vertices[4];
    vertices[0].pos = D3DXVECTOR3(g_GameManager.arena[0] + 16.0f, 384.0f, 0.0f);
    vertices[1].pos = D3DXVECTOR3(g_GameManager.arena[0] + 384.0f - 16.0f, 384.0f, 0.0f);
    vertices[2].pos = D3DXVECTOR3(g_GameManager.arena[0] + 16.0f, 384.0f + height, 0.0f);
    vertices[3].pos = D3DXVECTOR3(g_GameManager.arena[0] + 384.0f - 16.0f, 384.0f + height, 0.0f);
    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    if (this->msg.portraits[0].pos.z >= this->msg.portraits[1].pos.z)
    {
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[0]);
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[1]);
    }
    else
    {
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[1]);
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[0]);
    }
    if (this->msg.portraits[2].pos.z >= this->msg.portraits[3].pos.z)
    {
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[2]);
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[3]);
    }
    else
    {
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[3]);
        g_SpriteEngine->DrawUnrotated(&this->msg.portraits[2]);
    }
    g_SpriteEngine->FlushQuadBatch();
    if (this->msg.unk_156d)
    {
        if (!g_App.GetNoColorBlending())
        {
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        }
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        if (!g_App.GetNoDepthTest())
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
        g_SpriteEngine->ClearVertexFormat();
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
    g_SpriteEngine->DrawUnrotated(&this->msg.text[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.text[1]);
    g_SpriteEngine->DrawUnrotated(&this->msg.nameplates[0]);
    g_SpriteEngine->DrawUnrotated(&this->msg.nameplates[1]);
    return 0;
}

BOOL Gui::MsgHoldsTimeline()
{
    if (this->impl == NULL)
    {
        return FALSE;
    }
    if (this->impl->msg.timelineRelease > 0)
    {
        return FALSE;
    }
    return this->impl->msg.scriptIndex >= 0;
}

int Gui::IsMsgRunning()
{
    if (this->impl == NULL)
    {
        return 0;
    }
    return this->impl->msg.scriptIndex >= 0 || this->impl->msg.scriptIndex == -2 ? 1 : 0;
}

void Gui::DrawBanners()
{
    int i;
    D3DCOLOR colorA2;
    D3DCOLOR dark;
    float r2[4];
    D3DCOLOR timerColor;
    float end;
    int index2;
    int total;
    int shrink;

    for (i = 0; i < 4; i++)
    {
        g_SpriteEngine->Draw(&this->impl->unk_2a44[i]);
    }
    g_SpriteEngine->Draw(&this->impl->unk_34d4);
    g_SpriteEngine->Draw(&this->impl->unk_2156c);
    if (this->impl->loadingIcon.spriteIndex >= 0)
    {
        g_SpriteEngine->DrawUnrotated(&this->impl->loadingIcon);
        g_SpriteEngine->DrawProjected3D(&this->impl->loadingBar);
        for (i = 0; i < ARRAY_SIZE(this->impl->unk_3f64); i++)
        {
            g_SpriteEngine->DrawProjected3D(&this->impl->unk_3f64[i]);
        }
        if (this->impl->loadingCaption.spriteIndex >= 0)
        {
            this->impl->loadingCaption.pos = D3DXVECTOR3(304.0f, 448.0f, 0.0f);
            g_SpriteEngine->DrawUnrotated(&this->impl->loadingCaption);
        }
    }
    if (this->impl->backgroundTileCount != 0)
    {
        for (i = 0; i < 0xa8; i++)
        {
            g_SpriteEngine->DrawProjected3D(&this->impl->backgroundTiles[i]);
            g_SpriteEngine->ClearSpriteData();
        }
    }
    if (this->impl->msg.scriptIndex < 0 && this->bossPresent + this->impl->bossBarState > 0)
    {
        r2[0] = 64.0f;
        r2[1] = 19.0f;
        r2[2] = this->bossHealthShown * 320.0f + 64.0f;
        r2[3] = 23.0f;
        colorA2 = this->bossBarAlpha << 24 | 0xffffff;
        dark = this->bossBarAlpha << 24 | 0x202060;
        D3DXVECTOR3 pos(48.0f, 16.0f, 0.0f);
        DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        for (index2 = 0; index2 < 8; index2++)
        {
            if (this->sectionStarts[index2] == 0.0f)
            {
                continue;
            }
            if (this->sectionEnds[index2] >= this->bossHealthShown)
            {
                continue;
            }
            end = this->sectionStarts[index2];
            if (this->bossHealthShown < end)
            {
                end = this->bossHealthShown;
            }
            r2[0] = this->sectionEnds[index2] * 320.0f + 64.0f;
            r2[1] = 19.0f;
            r2[2] = end * 320.0f + 64.0f;
            r2[3] = 23.0f;
            colorA2 = this->bossBarAlpha << 24 | (this->sectionColors[index2] & 0xffffff);
            dark = this->bossBarAlpha << 24 | (this->sectionColors[index2] >> 2 & 0x3f3f3f);
            DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        }
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[12]);
        r2[0] = 33.0f;
        r2[1] = 19.0f;
        r2[2] = r2[0] + 3.0f;
        r2[3] = r2[1] + 4.0f;
        total = this->bossLives;
        shrink = this->bossLives <= 5 ? 2 : 1;
        for (index2 = 0; index2 < total; index2++)
        {
            r2[0] = index2 * 26.0f / total + 35.0f;
            r2[2] = (index2 + 1) * 26.0f / total + 35.0f - shrink;
            colorA2 = this->bossBarAlpha << 24 | (0xffffff - index2 * 0xff / 9);
            dark = this->bossBarAlpha << 24 | 0x202020;
            DrawGradientRect(r2, colorA2, colorA2, dark, dark);
        }
        pos = D3DXVECTOR3(384.0f, 16.0f, 0.0f);
        if (this->bossTimer >= 20)
        {
            timerColor = g_BossTimerColors[0];
        }
        else if (this->bossTimer >= 10)
        {
            timerColor = g_BossTimerColors[1];
        }
        else if (this->bossTimer >= 5)
        {
            timerColor = g_BossTimerColors[2];
        }
        else
        {
            timerColor = g_BossTimerColors[3];
        }
        g_AsciiText.SetColor(this->bossBarAlpha << 24 | timerColor);
        total = this->bossTimer > 99 ? 99 : this->bossTimer;
        if (this->lastDrawnBossTimer != this->bossTimer)
        {
            if (total < 3)
            {
                g_SoundPlayer.QueueSound(0x26, 0);
            }
            else if (total < 10)
            {
                g_SoundPlayer.QueueSound(0x1d, 0);
            }
        }
        g_AsciiText.AddFormat(&pos, "%.2d", total);
        g_AsciiText.SetColor(0xffffffff);
        this->lastDrawnBossTimer = this->bossTimer;
        if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu && !g_GameManager.unk_3dbac_10 &&
            g_EnemyManager.bosses[0] != NULL)
        {
            pos = D3DXVECTOR3(2.0f, 29.0f, 0.0f);
            g_AsciiText.SetScale(1.0f, 1.0f);
            g_AsciiText.Func403600(&pos, g_EnemyManager.bosses[0]->Func41fd40(), g_EnemyManager.bosses[0]->unk_3380,
                                   0xfff0f00f);
        }
    }
    g_SpriteEngine->DrawUnrotated(&this->impl->unk_212c8);
}

int Gui::AddedCallback(Gui *gui)
{
    return gui->LoadResources();
}

int Gui::DeletedCallback(Gui *gui)
{
    if (!Func438ffd())
    {
        g_SpriteEngine->ReleaseAnm(0xd);
    }
    gui->FreeMsgFile();
    if (Func438ff3())
    {
        g_SpriteEngine->ReleaseAnm(0xa);
        g_SpriteEngine->ReleaseAnm(0xc);
        g_SpriteEngine->ReleaseAnm(0xb);
        g_SpriteEngine->ReleaseAnm(0xe);
        g_MemoryManager.Unregister(gui->impl);
        delete gui->impl;
        gui->impl = NULL;
    }
    return 0;
}

int Gui::RegisterChain()
{
    Gui *gui = &g_Gui;

    if (Func438fe9())
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = (GuiImpl *)g_MemoryManager.Register(new GuiImpl, sizeof(GuiImpl), "FRScreenImplInf");
    }
    g_GuiCalcChain.SetCallback((ChainCallback)Gui::OnUpdate);
    g_GuiCalcChain.addedCallback = (ChainCallback)Gui::AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainCallback)Gui::DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, 15) != 0)
    {
        return -1;
    }
    g_GuiDrawChain.SetCallback((ChainCallback)Gui::OnDraw);
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, 17);
    return 0;
}

void DebugPrint2(const char *fmt, ...)
{
}

void Gui::RemoveChain()
{
    g_Chain.Remove(&g_GuiCalcChain);
    g_Chain.Remove(&g_GuiDrawChain);
}

BOOL Gui::IsLoadingShown()
{
    return this->impl->loadingIcon.spriteIndex >= 0 && this->impl->loadingIcon.IsStopped();
}

int Gui::IsMsgSkippable()
{
    return this->impl->msg.skippable;
}

void Gui::ShowBonusPopup(int bonus)
{
    this->impl->bonusPopup.pos = D3DXVECTOR3(416.0f, 48.0f, 0.0f);
    this->impl->bonusPopup.shown = 1;
    this->impl->bonusPopup.timer = 0;
    this->impl->bonusPopup.value = bonus;
    g_App.redrawFrames = 2;
}

void Gui::ShowFullPowerPopup(int points, int kind)
{
    this->impl->fullPowerPopup.pos = D3DXVECTOR3(416.0f, 168.0f, 0.0f);
    this->impl->fullPowerPopup.shown = kind;
    this->impl->fullPowerPopup.timer = 0;
    this->impl->fullPowerPopup.value = points;
    g_App.redrawFrames = 2;
}

void Gui::ShowSpellCardBonus(int value)
{
    this->impl->spellBonusPopup.pos = D3DXVECTOR3(224.0f, 16.0f, 0.0f);
    this->impl->spellBonusPopup.shown = 1;
    this->impl->spellBonusPopup.timer = 0;
    this->impl->spellBonusPopup.value = value;
    g_App.redrawFrames = 2;
}

void CopySpriteToTexture(int sprite)
{
    RECT dstRect;
    RECT spriteRect;

    dstRect.left = g_Gui.anm->GetSprite(10)->startX;
    dstRect.top = g_Gui.anm->GetSprite(10)->startY;
    dstRect.right = g_Gui.anm->GetSprite(10)->endX;
    dstRect.bottom = g_Gui.anm->GetSprite(10)->endY;
    spriteRect.left = g_Gui.anm->GetSprite(sprite)->startX;
    spriteRect.top = g_Gui.anm->GetSprite(sprite)->startY;
    spriteRect.right = g_Gui.anm->GetSprite(sprite)->endX;
    spriteRect.bottom = g_Gui.anm->GetSprite(sprite)->endY;
    g_SpriteEngine->CopyTextureRect(10, 0, 10, 1, &dstRect, &spriteRect);
}

void Func438046()
{
    switch (g_GameManager.stage)
    {
    default:
        CopySpriteToTexture(0x10);
        break;
    case 1:
        CopySpriteToTexture(0x11);
        break;
    case 2:
        CopySpriteToTexture(0x12);
        break;
    case 3:
        if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.IsPracticingSpellCard(0xd6))
        {
            CopySpriteToTexture(0x13);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xd8))
        {
            CopySpriteToTexture(0x1a);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xd9))
        {
            CopySpriteToTexture(0x1b);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xda))
        {
            CopySpriteToTexture(0x1c);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xdb))
        {
            CopySpriteToTexture(0x1d);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xdc))
        {
            CopySpriteToTexture(0x1e);
        }
        else if (g_GameManager.IsPracticingSpellCard(0xdd))
        {
            CopySpriteToTexture(0x1f);
        }
        break;
    case 4:
        CopySpriteToTexture(0x14);
        break;
    case 5:
        if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.IsPracticingSpellCard(0xd4))
        {
            CopySpriteToTexture(0x15);
        }
        else
        {
            CopySpriteToTexture(0x16);
        }
        break;
    case 6:
        CopySpriteToTexture(0x17);
        break;
    case 7:
        if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.IsPracticingSpellCardIn(0x93, 0x96))
        {
            CopySpriteToTexture(0x17);
        }
        else
        {
            CopySpriteToTexture(0x18);
        }
        break;
    case 8:
        if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.IsPracticingSpellCardIn(0xbf, 0xc1) ||
            g_GameManager.IsPracticingSpellCard(0xd5))
        {
            CopySpriteToTexture(0x20);
        }
        else
        {
            CopySpriteToTexture(0x19);
        }
        break;
    }
}

void Gui::DrawStageClear()
{
    D3DXVECTOR3 cursor(120.0f, 96.0f, 0.0f);

    g_AsciiText.SetColor(0xffffff40);
    if (g_GameManager.stage < 6)
    {
        g_AsciiText.AddFormat(&cursor, "Stage Clear");
    }
    else
    {
        if (g_GameManager.stage >= 7)
        {
            cursor.y -= 16.0f;
        }
        g_AsciiText.AddFormat(&cursor, "All Clear!");
    }
    cursor.y += 32.0f;
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.AddFormat(&cursor, "Clear = %8d0", this->impl->stageClearBonus);
    cursor.y += 16.0f;
    g_AsciiText.SetColor(0xffe0e0ff);
    g_AsciiText.AddFormat(&cursor, "Point = %8d0", this->impl->stageClearPoints * 5000);
    cursor.y += 16.0f;
    g_AsciiText.SetColor(0xffd0d0ff);
    g_AsciiText.AddFormat(&cursor, "Graze = %8d0", this->impl->stageClearGraze * 50);
    cursor.y += 16.0f;
    g_AsciiText.SetColor(0xffd0d0ff);
    g_AsciiText.AddFormat(&cursor, "Time  = %8d0", this->impl->stageClearTime * 100);
    cursor.y += 16.0f;
    g_AsciiText.SetColor(0xffd0d0ff);
    cursor.y += 16.0f;
    g_AsciiText.AddFormat(&cursor, "over-80%% = %3d.%.2d%%", g_GameManager.unk_3de24 * 100 / g_GameManager.unk_3de14,
                          g_GameManager.unk_3de24 * 10000 / g_GameManager.unk_3de14 % 100);
    cursor.y += 16.0f;
    g_AsciiText.AddFormat(&cursor, "over 80%% = %3d.%.2d%%", g_GameManager.unk_3de20 * 100 / g_GameManager.unk_3de14,
                          g_GameManager.unk_3de20 * 10000 / g_GameManager.unk_3de14 % 100);
    if (g_GameManager.stage >= 6 && !g_GameManager.IsInPractice() && !g_GameManager.IsPracticeReplay())
    {
        cursor.y += 16.0f;
        g_AsciiText.SetColor(0xffffff80);
        g_AsciiText.AddFormat(&cursor, "Player =%8d0", g_GameManager.GetLives() * 2500000);
        cursor.y += 16.0f;
        g_AsciiText.SetColor(0xffffff80);
        g_AsciiText.AddFormat(&cursor, "Bomb   = %7d0", g_GameManager.GetBombs() * 500000);
        if (g_GameManager.stage == 7 && !g_GameManager.IsInPractice() && !g_GameManager.IsPracticeReplay())
        {
            cursor.y += 16.0f;
            g_AsciiText.SetColor(0xffffff80);
            g_AsciiText.AddFormat(&cursor, "Last Time = %2d:%.2d", this->impl->stageClearLastTime / 60 % 12,
                                  this->impl->stageClearLastTime % 60);
            cursor.y += 16.0f;
            g_AsciiText.AddFormat(&cursor, "Night Bonus");
            cursor.y += 16.0f;
            g_AsciiText.AddFormat(&cursor, "        %8d0", (12 - g_GameManager.GetUnk28()) * 2000000);
        }
    }
    cursor.y += 32.0f;
    switch (g_GameManager.difficulty)
    {
    case 0:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Easy Rank    *0.5");
        break;
    case 1:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Normal Rank  *1.0");
        break;
    case 2:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Hard Rank    *1.2");
        break;
    case 3:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Lunatic Rank *1.5");
        break;
    case 4:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Extra Rank   *2.0");
        break;
    case 5:
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddFormat(&cursor, "Phantasm Rank*2.0");
        break;
    }
    if (g_GameManager.difficulty < 4 && !(g_GameManager.unk_3dbac & 1))
    {
        cursor.y += 16.0f;
        switch (g_GameManager.unk_4->lifeCount)
        {
        case 3:
            g_AsciiText.SetColor(0xffff8080);
            g_AsciiText.AddFormat(&cursor, "Player Penalty*0.5");
            cursor.y += 16.0f;
            break;
        case 4:
            g_AsciiText.SetColor(0xffff8080);
            g_AsciiText.AddFormat(&cursor, "Player Penalty*0.2");
            cursor.y += 16.0f;
            break;
        case 5:
            g_AsciiText.SetColor(0xffff8080);
            g_AsciiText.AddFormat(&cursor, "Player Penalty*0.1");
            cursor.y += 16.0f;
            break;
        case 6:
            g_AsciiText.SetColor(0xffff8080);
            g_AsciiText.AddFormat(&cursor, "Player Penalty*0.05");
            cursor.y += 16.0f;
            break;
        }
    }
    cursor.y += 16.0f;
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.AddFormat(&cursor, "Total = %8d0", this->impl->stageClearTotal);
    g_AsciiText.SetColor(0xffffffff);
    if (g_GameManager.stage <= 5)
    {
        cursor.y += 40.0f;
        cursor.x = 120.0f;
        g_AsciiText.SetColor(0xffdfdfdf);
        g_AsciiText.AddFormat(&cursor, "%s%2d:%.2d", g_AmPmNames[this->impl->stageClearClock / 60 < 12],
                              this->impl->stageClearClock / 60 % 12, this->impl->stageClearClock % 60);
        cursor.x += 99.0f;
        g_AsciiText.SetColor(0xffafafaf);
        g_AsciiText.AddFormat(&cursor, ">>");
        cursor.x += 34.0f;
        g_AsciiText.SetColor(0xffff8f8f);
        g_AsciiText.AddFormat(&cursor, "%s%2d:%.2d", g_AmPmNames[this->impl->stageClearClock2 / 60 < 12],
                              this->impl->stageClearClock2 / 60 % 12, this->impl->stageClearClock2 % 60);
        g_AsciiText.SetColor(0xffffffff);
    }
}

int App::GetNoFrameItems()
{
    return this->flags.noFrameItems;
}

int App::GetNoDepthTest()
{
    return this->flags.noDepthTest;
}

int App::GetNoColorBlending()
{
    return this->flags.noColorBlending;
}

int App::GetUnk12()
{
    return this->flags.unk12;
}

void Gui::DrawPopups()
{
    char buf[32];

    g_AsciiText.SetInArena(1);
    if (this->impl->bonusPopup.shown)
    {
        g_AsciiText.SetColor(0xffffff80);
        g_AsciiText.AddFormat(&this->impl->bonusPopup.pos, " BONUS %8d", this->impl->bonusPopup.value);
        g_AsciiText.SetColor(0xffffffff);
    }
    switch (this->impl->fullPowerPopup.shown)
    {
    case 1:
        g_AsciiText.SetColor(0xffc0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "Full Power Mode!");
        g_AsciiText.SetColor(0xffffffff);
        break;
    case 2:
        g_AsciiText.SetScale(0.9f, 1.0f);
        g_AsciiText.SetGlyphWidth(11);
        g_AsciiText.SetColor(0xffe0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "Supernatural Border!!");
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.SetGlyphWidth(13);
        break;
    case 3:
        g_AsciiText.SetColor(0xffc0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "CherryPoint Max!");
        g_AsciiText.SetColor(0xffffffff);
        break;
    case 4:
        g_AsciiText.SetScale(0.9f, 1.0f);
        g_AsciiText.SetGlyphWidth(11);
        g_AsciiText.SetColor(0xffe0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "Border Bonus %7d", this->impl->fullPowerPopup.value);
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.SetGlyphWidth(13);
        break;
    case 5:
        g_AsciiText.SetScale(0.9f, 1.0f);
        g_AsciiText.SetGlyphWidth(11);
        g_AsciiText.SetColor(0xffe0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "Spell Bonus Failed");
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.SetGlyphWidth(13);
        break;
    case 6:
        g_AsciiText.SetScale(0.9f, 1.0f);
        g_AsciiText.SetGlyphWidth(11);
        g_AsciiText.SetColor(0xffe0b0ff);
        g_AsciiText.AddFormat(&this->impl->fullPowerPopup.pos, "Last Spell Failed");
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.SetGlyphWidth(13);
        break;
    }
    if (this->impl->spellBonusPopup.shown)
    {
        g_AsciiText.SetColor(0xffff0000);
        this->impl->spellBonusPopup.pos.x = (384.0f - strlen("Spell Card Bonus!") * 14.0f) / 2.0f + 32.0f;
        this->impl->spellBonusPopup.pos.y = 80.0f;
        g_AsciiText.AddFormat(&this->impl->spellBonusPopup.pos, "Spell Card Bonus!");
        this->impl->spellBonusPopup.pos.y += 16.0f;
        sprintf(buf, "+%d", this->impl->spellBonusPopup.value);
        this->impl->spellBonusPopup.pos.x = (384.0f - strlen(buf) * 28.0f) / 2.0f + 32.0f;
        g_AsciiText.SetScale(2.0f, 2.0f);
        g_AsciiText.SetColor(0xffff8080);
        g_AsciiText.AddString(&this->impl->spellBonusPopup.pos, buf);
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.SetColor(0xffffffff);
    }
    g_AsciiText.SetInArena(0);
}

void Gui::Func438f58()
{
    g_AsciiText.anm2->StartScript(&this->impl->loadingBar, 1);
    g_SpriteEngine->SetPendingCopy(3, 32, 16, 384, 448, this->impl->loadingBar.spriteData->startX,
                                   this->impl->loadingBar.spriteData->startY, this->impl->loadingBar.spriteData->width,
                                   this->impl->loadingBar.spriteData->height);
}

int Func438fe9()
{
    return g_App.unk_164;
}

int Func438ff3()
{
    return g_App.unk_168;
}

int Func438ffd()
{
    return g_App.unk_16c;
}

int Gui::Func439007()
{
    this->timesAnm->StartScriptAtOrigin(&this->impl->unk_2156c, 2);
    this->timesAnm->SetSprite(&this->impl->unk_2156c, g_GameManager.GetUnk28());
    return 0;
}

int Gui::Func439050()
{
    this->timesAnm->SetSprite(&this->impl->unk_2156c, g_GameManager.GetUnk28());
    this->impl->unk_2156c.SetPendingInterrupt(1);
    return 0;
}

int Gui::Func439093()
{
    this->timesAnm->SetSprite(&this->impl->unk_2156c, g_GameManager.GetUnk28());
    this->impl->unk_2156c.SetPendingInterrupt(2);
    return 0;
}

int Gui::Func4390d6()
{
    this->impl->unk_2156c.alpha = 0;
    return 0;
}

int Gui::LoadResources()
{
    int x;
    int y;
    DWORD i;

    if (Func438fe9())
    {
        memset(this->impl, 0, sizeof(GuiImpl));
        this->anm = g_SpriteEngine->PreloadAnm(10, "front.anm");
        if (this->anm == NULL)
        {
            return -1;
        }
        this->HideInterface();
        this->timesAnm = g_SpriteEngine->PreloadAnm(0xe, "times.anm");
        if (this->timesAnm == NULL)
        {
            return -1;
        }
        this->characterAnm = g_SpriteEngine->PreloadAnm(0xc, g_LoadingAnmPaths[g_GameManager.character]);
        if (this->characterAnm == NULL)
        {
            return -1;
        }
        g_AsciiText.anm->StartScript(&this->impl->unk_5484, 0x1a);
        g_AsciiText.anm->StartScript(&this->impl->unk_22e14, 0x19);
        if (g_GameManager.IsInSpellCardPractice() && g_GameManager.unk_3dbb0 >= 0xcd)
        {
            g_AsciiText.anm->SetSprite(&this->impl->unk_22e14, 0x120);
        }
        else
        {
            g_AsciiText.anm->SetSprite(&this->impl->unk_22e14, g_GameManager.difficulty + 0x11b);
        }
    }
    else
    {
        this->HideInterface();
        g_AsciiText.anm2->StartScript(&this->impl->loadingBar, 1);
        this->impl->loadingBar.pendingInterrupt = 1;
        for (y = 0; y < 0xe; y++)
        {
            for (x = 0; x < 0xc; x++)
            {
                g_AsciiText.anm2->StartScript(&this->impl->backgroundTiles[y * 0xc + x], ((y + x) & 1) + 3);
                this->impl->backgroundTiles[y * 0xc + x].scriptVar8 = y + x * 2;
                this->impl->backgroundTiles[y * 0xc + x].pos.x = x * 32.0f - 0.5f + 16.0f;
                this->impl->backgroundTiles[y * 0xc + x].pos.y = y * 32.0f - 0.5f + 16.0f;
                this->impl->backgroundTiles[y * 0xc + x].pos.z = 0.0f;
                this->impl->backgroundTiles[y * 0xc + x].uvScrollX = x * 32.0f / 512.0f;
                this->impl->backgroundTiles[y * 0xc + x].uvScrollY = y * 32.0f / 512.0f;
            }
        }
        this->impl->backgroundTileCount = 0xa8;
    }
    g_Gui.Func4390d6();
    this->timesAnm->StartScriptAtOrigin(&this->impl->unk_34d4, 0);
    this->timesAnm->SetSprite(&this->impl->unk_34d4, g_GameManager.GetUnk28());
    if (!g_GameManager.IsInSpellCardPractice() &&
        this->LoadMsg(g_MsgPaths[g_GameManager.stage][g_GameManager.character]) != 0)
    {
        return -1;
    }
    if (!Func438ffd())
    {
        if (!(g_GameManager.unk_3dbac >> 14 & 1) || g_GameManager.unk_3dbb0 < 0xcd)
        {
            this->textAnm = g_SpriteEngine->PreloadAnm(0xd, g_StageTextAnmPaths[g_GameManager.stage]);
            if (this->textAnm == NULL)
            {
                return -1;
            }
        }
        else
        {
            this->textAnm = g_SpriteEngine->PreloadAnm(0xd, g_StageTextAnmPaths[8]);
            if (this->textAnm == NULL)
            {
                return -1;
            }
        }
    }
    if (Func438fe9())
    {
        for (i = 0; i < 0x10; i++)
        {
            this->anm->StartScript(&this->impl->sprites[i], i);
        }
    }
    this->frameCounter = 0;
    this->bossPresent = 0;
    this->impl->bossBarState = 0;
    this->bossHealth = 0.0f;
    this->bossHealthShown = 0.0f;
    if (!(g_GameManager.unk_3dbac >> 14 & 1))
    {
        this->textAnm->StartSpriteSeries(this->impl->unk_2a44, 0, 4);
    }
    else if (!Func438ffd() || Func439916(g_GameManager.unk_3dbb0))
    {
        this->textAnm->StartSpriteSeries(this->impl->unk_2a44, 3, 1);
        this->textAnm->SetSprite(this->impl->unk_2a44, Func439961(g_GameManager.unk_3dbb0) + 3);
    }
    this->impl->msg.scriptIndex = -1;
    this->impl->stageClearShown = 0;
    this->impl->bonusPopup.shown = 0;
    this->impl->fullPowerPopup.shown = 0;
    this->impl->spellBonusPopup.shown = 0;
    this->livesChanged = 2;
    this->bombsChanged = 2;
    this->grazeChanged = 2;
    this->unk_4_8 = 2;
    this->powerChanged = 2;
    this->unk_4_10 = 2;
    g_AsciiText.anm->StartScript(&this->impl->unk_212c8, 3);
    g_App.redrawFrames = 0x10;
    this->impl->stageClearClock2 = 0;
    return 0;
}

void Gui::HideInterface()
{
    this->impl->loadingIcon.spriteIndex = -1;
    this->impl->loadingCaption.spriteIndex = -1;
    this->impl->loadingBar.spriteIndex = -1;
    this->impl->backgroundTileCount = 0;
}

int Gui::LoadMsg(char *path)
{
    int i;

    this->FreeMsgFile();
    this->impl->msg.msgFile = (MsgFile *)LoadFileData(path, NULL, 0);
    if (this->impl->msg.msgFile == NULL)
    {
        LogPrintf(&g_Log, "error : メッセージファイル %s が読み込めませんでした\r\n", path);
        return -1;
    }
    this->impl->msg.scriptIndex = -1;
    this->impl->msg.instr = NULL;
    for (i = 0; i < this->impl->msg.msgFile->count; i++)
    {
        this->impl->msg.msgFile->scripts[i] = (char *)this->impl->msg.msgFile->scripts[i] + (int)this->impl->msg.msgFile;
    }
    return 0;
}

void Gui::FreeMsgFile()
{
    if (this->impl->msg.msgFile != NULL)
    {
        g_MemoryManager.Free(this->impl->msg.msgFile);
        this->impl->msg.msgFile = NULL;
    }
}

void Gui::StartMsg(int index)
{
    this->impl->StartMsgScript(index);
}

int Func439916(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.unk_3dbb0 <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].unk_10;
        }
        i++;
    }
    return 0;
}

int Func439961(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.unk_3dbb0 <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].unk_c;
        }
        i++;
    }
    return 0;
}
