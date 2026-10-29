// cflags: /Os
// The /Os file with Gui, as th07's, from 0x4338ca to GameManager at
// 0x4399ac.

#include <string.h>

#include "bullet.h"
#include "enemy.h"
#include "item.h"
#include "replay.h"
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
