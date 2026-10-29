// cflags: /Os
// The /Os file with Gui, as th07's, from 0x4338ca to GameManager at
// 0x4399ac.

#include <string.h>

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
    if (gui->impl->unk_22d84 != 0)
    {
        gui->Func43826b();
    }
    gui->impl->DrawMsg();
    gui->DrawBanners();
    gui->DrawHud();
    gui->Func438a89();
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
    return this->impl->unk_3778.spriteIndex >= 0 && this->impl->unk_3778.IsStopped();
}

int Gui::IsMsgSkippable()
{
    return this->impl->msg.skippable;
}
