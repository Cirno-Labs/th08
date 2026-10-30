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
#include "player.h"
#include "replay.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
int g_StageClearBonuses[9] = {
    1000000, 1500000, 2000000, 2500000, 2500000, 3000000, 4000000, 6000000, 6660000,
};
D3DCOLOR g_MsgColors[12][4] = {
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
};
StageBgm g_StageBgms[9] = {
    {1, 2, 0}, {3, 4, 0}, {5, 6, 0},
    {7, 8, 0}, {7, 9, 0}, {10, 11, 0},
    {12, 13, 15}, {12, 14, 15}, {16, 17, 0},
};
D3DCOLOR g_BossTimerColors[4] = {
    0x00a0d0ff, 0x00a080ff, 0x00e080c0, 0x00ff4040,
};
char *g_AmPmNames[2] = {
    "AM",
    "PM",
};
char *g_LoadingAnmPaths[12] = {
    "loading00.anm",
    "loading01.anm",
    "loading02.anm",
    "loading03.anm",
    "loading00h.anm",
    "loading00a.anm",
    "loading01h.anm",
    "loading01a.anm",
    "loading02h.anm",
    "loading02a.anm",
    "loading03h.anm",
    "loading03a.anm",
};
ChainElem g_GuiCalcElem;
ChainElem g_GuiDrawElement;
Gui g_Gui;
#endif // TH08_EXE

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
            g_Stage.dialogueScriptOffset = 2;
            break;
        case 7:
        {
            AnmLoaded *tmp = g_SpellCard.faceAnms[2];
            g_SpellCard.faceAnms[2] = g_SpellCard.faceAnms[3];
            g_SpellCard.faceAnms[3] = tmp;
            g_Stage.dialogueScriptOffset = 2;
            CopySpriteToTexture(0x18);
            break;
        }
        case 8:
        {
            AnmLoaded *tmp = g_SpellCard.faceAnms[2];
            g_SpellCard.faceAnms[2] = g_SpellCard.faceAnms[3];
            g_SpellCard.faceAnms[3] = tmp;
            g_Stage.dialogueScriptOffset = 2;
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
            if (g_GameManager.GetClock() >= 12)
            {
                index = 5;
            }
            break;
        }
    }
    this->msg.scriptIndex = index;
    this->msg.instr = (MsgInstr *)this->msg.msgFile->scripts[index];
    this->msg.text[0].scriptIndex = -1;
    this->msg.text[1].scriptIndex = -1;
    this->msg.textBoxShown = 1;
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
    this->msg.minWaitFrames = 6;
    this->msg.speaker = 0;
    this->msg.newPage = 1;
    this->msg.nextLine = 0;
    this->msg.activePortrait = 0xff;
    g_BulletManager.ClearAllToItems();
    g_EnemyManager.ClearEnemiesForBonus(0, 0);
    g_ItemManager.CollectAll();
}

int GuiImpl::RunMsg()
{
    MsgArgs *args;

    if (this->msg.scriptIndex < 0)
    {
        return -1;
    }
    if (this->msg.timelineRelease > 0)
    {
        this->msg.timelineRelease--;
    }
    if (this->msg.skippable && (g_GameButtons & 0x100))
    {
        this->msg.timer = this->msg.instr->time;
    }
    if (g_Player.playerState != 2)
    {
        g_ItemManager.CollectAll();
    }
    while (this->msg.timer >= this->msg.instr->time)
    {
        switch (this->msg.instr->opcode)
        {
        case 0:
            this->msg.scriptIndex = -1;
            return -1;
        case 15:
        {
            MsgPortraitArgs *portraitArgs;
            int i;

            portraitArgs = &this->msg.instr->portraitArgs;
            if (this->msg.activePortrait != portraitArgs->who)
            {
                for (i = 0; i < ARRAY_SIZE(this->msg.portraits); i++)
                {
                    if (this->msg.activePortrait == i)
                    {
                        if (this->msg.activePortrait / 2 != portraitArgs->who / 2)
                        {
                            this->msg.portraits[i].pendingInterrupt = 6;
                        }
                        else
                        {
                            this->msg.portraits[i].pendingInterrupt = 4;
                        }
                    }
                    else
                    {
                        this->msg.portraits[i].pendingInterrupt = 4;
                    }
                }
            }
            this->msg.portraits[portraitArgs->who].pendingInterrupt = 3;
            this->msg.activePortrait = portraitArgs->who;
            if (portraitArgs->sprites[0] >= 0)
            {
                g_SpellCard.faceAnms[0]->SetSprite(&this->msg.portraits[0], portraitArgs->sprites[0]);
            }
            if (portraitArgs->sprites[1] >= 0)
            {
                g_SpellCard.faceAnms[1]->SetSprite(&this->msg.portraits[1], portraitArgs->sprites[1]);
            }
            if (portraitArgs->sprites[2] >= 0)
            {
                g_SpellCard.faceAnms[2]->SetSprite(&this->msg.portraits[2], portraitArgs->sprites[2]);
            }
            if (portraitArgs->sprites[3] >= 0)
            {
                g_SpellCard.faceAnms[3]->SetSprite(&this->msg.portraits[3], portraitArgs->sprites[3]);
            }
            this->msg.speaker = portraitArgs->who;
            this->msg.newPage = 1;
            break;
        }
        case 17:
        {
            MsgPortraitArgs *portraitArgs2;
            int i;

            portraitArgs2 = &this->msg.instr->portraitArgs;
            if (this->msg.activePortrait != portraitArgs2->who)
            {
                for (i = 0; i < ARRAY_SIZE(this->msg.portraits); i++)
                {
                    if (this->msg.activePortrait == i)
                    {
                        if (this->msg.activePortrait / 2 != portraitArgs2->who / 2)
                        {
                            this->msg.portraits[i].pendingInterrupt = 6;
                        }
                        else
                        {
                            this->msg.portraits[i].pendingInterrupt = 4;
                        }
                    }
                    else
                    {
                        this->msg.portraits[i].pendingInterrupt = 4;
                    }
                }
            }
            this->msg.portraits[portraitArgs2->who].pendingInterrupt = 3;
            this->msg.activePortrait = portraitArgs2->who;
            if (portraitArgs2->sprites[0] >= 0)
            {
                switch (portraitArgs2->who)
                {
                case 0:
                    g_SpellCard.faceAnms[0]->SetSprite(&this->msg.portraits[0], portraitArgs2->sprites[0]);
                    break;
                case 1:
                    g_SpellCard.faceAnms[1]->SetSprite(&this->msg.portraits[1], portraitArgs2->sprites[0]);
                    break;
                case 2:
                    g_SpellCard.faceAnms[2]->SetSprite(&this->msg.portraits[2], portraitArgs2->sprites[0]);
                    break;
                case 3:
                    g_SpellCard.faceAnms[3]->SetSprite(&this->msg.portraits[3], portraitArgs2->sprites[0]);
                    break;
                }
            }
            this->msg.speaker = portraitArgs2->who;
            this->msg.newPage = 1;
            break;
        }
        case 1:
            args = &this->msg.instr->args;
            switch (args->who)
            {
            case 0:
                g_SpellCard.faceAnms[0]->StartScript(&this->msg.portraits[0], args->index);
                break;
            case 1:
                g_SpellCard.faceAnms[1]->StartScript(&this->msg.portraits[1], args->index);
                break;
            case 2:
                g_SpellCard.faceAnms[2]->StartScript(&this->msg.portraits[2], args->index);
                break;
            case 3:
                g_SpellCard.faceAnms[3]->StartScript(&this->msg.portraits[3], args->index);
                break;
            }
            if (this->msg.portraits[args->who].spriteData->width > 128.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -112.0f;
            }
            else
            {
                this->msg.portraits[args->who].posOffset.x = 0.0f;
            }
            break;
        case 2:
            args = &this->msg.instr->args;
            switch (args->who)
            {
            case 0:
                g_SpellCard.faceAnms[0]->SetSprite(&this->msg.portraits[0], args->index);
                break;
            case 1:
                g_SpellCard.faceAnms[1]->SetSprite(&this->msg.portraits[1], args->index);
                break;
            case 2:
                g_SpellCard.faceAnms[2]->SetSprite(&this->msg.portraits[2], args->index);
                break;
            case 3:
                g_SpellCard.faceAnms[3]->SetSprite(&this->msg.portraits[3], args->index);
                break;
            }
            if (this->msg.portraits[args->who].spriteData->width > 256.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -208.0f;
                this->msg.portraits[args->who].posOffset.y = -50.0f;
            }
            else if (this->msg.portraits[args->who].spriteData->width > 128.0f)
            {
                this->msg.portraits[args->who].posOffset.x = -80.0f;
            }
            else
            {
                this->msg.portraits[args->who].posOffset.x = 0.0f;
            }
            break;
        case 3:
        {
            char text[0x40];

            args = &this->msg.instr->args;
            if (args->index == 0 && this->msg.text[1].scriptIndex >= 0)
            {
                g_SpriteEngine->RenderText(&this->msg.text[1], this->msg.textColors[args->who],
                                           this->msg.shadowColors[args->who], " ");
            }
            g_App.textAnm->StartScript(&this->msg.text[args->index], args->index);
            this->msg.text[args->index].fontHeight = this->msg.fontSize;
            this->msg.text[args->index].fontWidth = this->msg.text[args->index].fontHeight;
            DecodeMsgString(text, args->text);
            g_SpriteEngine->RenderText(&this->msg.text[args->index], this->msg.textColors[args->who],
                                       this->msg.shadowColors[args->who], text);
            this->msg.waitFrames = 0;
            break;
        }
        case 16:
        {
            char text2[0x40];

            args = &this->msg.instr->args;
            if (this->msg.newPage)
            {
                if (this->msg.text[1].scriptIndex >= 0)
                {
                    g_SpriteEngine->RenderText(&this->msg.text[1], this->msg.textColors[this->msg.speaker],
                                               this->msg.shadowColors[this->msg.speaker], " ");
                }
                this->msg.nextLine = 0;
            }
            g_App.textAnm->StartScript(&this->msg.text[this->msg.nextLine], this->msg.nextLine);
            this->msg.text[this->msg.nextLine].fontHeight = this->msg.fontSize;
            this->msg.text[this->msg.nextLine].fontWidth = this->msg.text[this->msg.nextLine].fontHeight;
            DecodeMsgString(text2, (char *)args);
            g_SpriteEngine->RenderText(&this->msg.text[this->msg.nextLine], this->msg.textColors[this->msg.speaker],
                                       this->msg.shadowColors[this->msg.speaker], text2);
            this->msg.waitFrames = 0;
            this->msg.newPage = 0;
            this->msg.nextLine++;
            break;
        }
        case 19:
        {
            char text3[0x40];

            args = &this->msg.instr->args;
            g_App.textAnm->StartScript(&this->msg.text[0], 0);
            this->msg.text[0].fontHeight = this->msg.fontSize;
            this->msg.text[0].fontWidth = this->msg.text[0].fontHeight;
            DecodeMsgString(text3, (char *)args);
            g_SpriteEngine->RenderText(&this->msg.text[0], this->msg.textColors[0], this->msg.shadowColors[0], text3);
            this->msg.waitFrames = 0;
            break;
        }
        case 20:
        {
            char text4[0x40];

            args = &this->msg.instr->args;
            g_App.textAnm->StartScript(&this->msg.text[1], 1);
            this->msg.text[1].fontHeight = this->msg.fontSize;
            this->msg.text[1].fontWidth = this->msg.text[1].fontHeight;
            DecodeMsgString(text4, (char *)args);
            g_SpriteEngine->RenderText(&this->msg.text[1], this->msg.textColors[0], this->msg.shadowColors[0], text4);
            this->msg.waitFrames = 0;
            break;
        }
        case 21:
            if ((g_GameButtons & 0x10) && (g_GameButtons & 0x10) != (g_PrevGameButtons & 0x10))
            {
                if (this->msg.clearLevel == 1)
                {
                    g_SoundPlayer.QueueSound(0xc, 0);
                }
                this->msg.clearLevel = 0;
            }
            if ((g_GameButtons & 0x20) && (g_GameButtons & 0x20) != (g_PrevGameButtons & 0x20))
            {
                if (this->msg.clearLevel == 0)
                {
                    g_SoundPlayer.QueueSound(0xc, 0);
                }
                this->msg.clearLevel = 1;
            }
            this->msg.text[this->msg.clearLevel].color = 0xffffffff;
            this->msg.text[1 - this->msg.clearLevel].color = 0xe0606060;
            if (!(g_GameButtons & 1) || (g_GameButtons & 1) == (g_PrevGameButtons & 1) || this->msg.waitFrames < 60)
            {
                if (this->msg.waitFrames >= this->msg.instr->i)
                {
                    this->msg.newPage = 1;
                    this->msg.minWaitFrames = 30;
                    break;
                }
                this->msg.waitFrames++;
                goto stepSprites;
            }
            g_SoundPlayer.QueueSound(10, 0);
            break;
        case 22:
            g_GameManager.clearLevel = this->msg.clearLevel;
            g_Gui.StartMsg(this->msg.clearLevel + 1);
            continue;
        case 4:
            if (!this->msg.skippable || !(g_GameButtons & 0x100))
            {
                if (!(g_GameButtons & 1) || (g_GameButtons & 1) == (g_PrevGameButtons & 1) ||
                    this->msg.waitFrames < this->msg.minWaitFrames)
                {
                    if (this->msg.waitFrames >= this->msg.instr->i)
                    {
                        this->msg.newPage = 1;
                        this->msg.minWaitFrames = 30;
                        break;
                    }
                    this->msg.waitFrames++;
                    goto stepSprites;
                }
                this->msg.newPage = 1;
                this->msg.minWaitFrames = 8;
            }
            break;
        case 5:
            args = &this->msg.instr->args;
            this->msg.portraits[args->who].pendingInterrupt = args->value;
            break;
        case 6:
            this->msg.timelineRelease++;
            break;
        case 7:
            if (this->msg.instr->i < 0)
            {
                g_App.StopMusic();
            }
            else
            {
                g_Gui.textAnm->StartScript(&this->trackNumberSprites[3], 3);
                g_Gui.textAnm->SetSprite(&this->trackNumberSprites[3], this->msg.instr->i + 3);
                if (g_App.PlayMidi(this->msg.instr->i,
                                   (&g_StageBgms[g_GameManager.stage].stageBgm)[this->msg.instr->i]) != 0)
                {
                    g_App.PlayMusic(g_Stage.data->songPaths[this->msg.instr->i],
                                    (&g_StageBgms[g_GameManager.stage].stageBgm)[this->msg.instr->i]);
                }
            }
            break;
        case 8:
            args = &this->msg.instr->args;
            g_SpellCard.faceAnms[2]->StartScript(&this->msg.nameplates[0], 1);
            this->msg.waitFrames = 0;
            break;
        case 9:
        {
            int k;

            this->stageClearPower = g_GameManager.GetPower();
            this->stageClearPoints = g_GameManager.stats->stageClearPoints;
            this->stageClearTime = g_GameManager.GetTimeOrbs();
            this->stageClearGraze = g_GameManager.stats->graze;
            this->stageClearClock = g_GameManager.GetClock() * 30 + 660;
            this->stageClearTimePassed = g_GameManager.GetStageClearClockStep();
            g_GameManager.AddClock(this->stageClearTimePassed);
            this->stageClearBonus = g_StageClearBonuses[g_GameManager.stage];
            this->stageClearLastTime = g_GameManager.GetClock() * 30 + 660;
            this->stageClearClock2 = this->stageClearClock;
            this->stageClearClockTimer = 0;
            this->stageClearShown = 1;
            g_GameManager.stageClearStarted = 1;
            if (g_GameManager.stage != 6 && g_GameManager.stage != 7 && g_GameManager.stage != 8)
            {
                g_AsciiText.anm->StartScript(&this->timePassedDigit, 3);
                g_AsciiText.anm->SetSprite(&this->timePassedDigit, this->stageClearTimePassed + 0x80);
            }
            else
            {
                this->timePassedDigit.scriptPos = NULL;
            }
            this->timePassedDigit.SetPendingInterrupt(1);
            if (g_GameManager.stage != 6 && g_GameManager.stage != 7 && g_GameManager.stage != 8)
            {
                g_Gui.characterAnm->StartScript(&this->loadingIcon, 0);
                g_AsciiText.anm2->StartScript(&this->loadingBar, 1);
                g_SpriteEngine->SetPendingCopy(3, 0x20, 0x10, 0x180, 0x1c0, this->loadingBar.spriteData->startX,
                                               this->loadingBar.spriteData->startY,
                                               this->loadingBar.spriteData->width,
                                               this->loadingBar.spriteData->height);
                for (k = 0; k < ARRAY_SIZE(this->loadingBarTrail); k++)
                {
                    g_AsciiText.anm2->StartScript(&this->loadingBarTrail[k], 2);
                    this->loadingBarTrail[k].scriptVar8 = k * 4 + 3;
                    this->loadingBarTrail[k].alpha = 0x40 - k * 2;
                }
            }
            else
            {
                g_GameManager.stats->checksumKeyStage = -1;
            }
            if (g_GameManager.stage != 7 && g_GameManager.stage != 6 && g_GameManager.stage != 8 &&
                g_GameManager.GetBombs() < 3 &&
                (g_GameManager.character == 3 || g_GameManager.character == 10 || g_GameManager.character == 11))
            {
                g_GameManager.AddBombs(1);
                g_SoundPlayer.QueueSound(0x23, 0);
                g_Gui.bombsChanged = 2;
            }
            break;
        }
        case 10:
            goto stepSprites;
        case 12:
            g_App.FadeOutMusic(4.0f);
            break;
        case 14:
            ScreenEffect::StartEffect(4, 0x1ba, 0xffffff, 0, 0, 0x15);
            g_App.redrawFrames = 0x1ba;
            break;
        case 11:
            if (g_GameManager.stage == 6 || g_GameManager.stage == 7 || g_GameManager.stage == 8)
            {
                g_GameManager.stageEndState = 2;
            }
            goto stepSprites;
        case 13:
            this->msg.skippable = this->msg.instr->b;
            break;
        case 18:
            this->msg.textBoxShown = this->msg.instr->b;
            break;
        }
        this->msg.instr = (MsgInstr *)((BYTE *)this->msg.instr + 4 + this->msg.instr->argsSize);
    }
    this->msg.timer++;
stepSprites:
    g_SpriteEngine->Step(&this->msg.portraits[0]);
    g_SpriteEngine->Step(&this->msg.portraits[1]);
    g_SpriteEngine->Step(&this->msg.portraits[2]);
    g_SpriteEngine->Step(&this->msg.portraits[3]);
    g_SpriteEngine->Step(&this->msg.text[0]);
    g_SpriteEngine->Step(&this->msg.text[1]);
    g_SpriteEngine->Step(&this->msg.nameplates[0]);
    g_SpriteEngine->Step(&this->msg.nameplates[1]);
    if (this->msg.timer < 60 && this->msg.skippable && (g_GameButtons & 0x100))
    {
        this->msg.timer = 60;
    }
    return 0;
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
    if (this->msg.textBoxShown)
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

void Gui::UpdateInterface()
{
    int m;
    int clearBonus;
    int j;
    int remaining;
    int i;

    if (this->impl->msg.scriptIndex < 0)
    {
        if (this->bossPresent)
        {
            if (this->impl->bossBarState == 0)
            {
                this->impl->sprites[12].SetPendingInterrupt(1);
                this->impl->bossBarState = 1;
                this->bossBarAlpha = 0;
            }
            else
            {
                if (this->impl->sprites[12].IsStopped())
                {
                    this->impl->bossBarState = 2;
                }
                if (this->bossBarAlpha < 0xfc)
                {
                    this->bossBarAlpha += 4;
                }
                else
                {
                    this->bossBarAlpha = 0xff;
                }
            }
        }
        else if (this->impl->bossBarState != 0)
        {
            if (this->impl->bossBarState <= 2)
            {
                this->impl->sprites[12].SetPendingInterrupt(2);
                this->impl->bossBarState = 3;
            }
            if (this->bossBarAlpha > 0)
            {
                this->bossBarAlpha -= 4;
            }
            else
            {
                this->bossBarAlpha = 0;
            }
            if (this->impl->sprites[12].IsStopped())
            {
                this->impl->bossBarState = 0;
                this->bossHealthShown = 0.0f;
                this->bossBarAlpha = 0;
            }
        }
        if (this->impl->bossBarState >= 2)
        {
            if (this->bossHealth > this->bossHealthShown)
            {
                this->bossHealthShown += 0.01f;
                if (this->bossHealth < this->bossHealthShown)
                {
                    this->bossHealthShown = this->bossHealth;
                }
            }
            else if (this->bossHealth < this->bossHealthShown)
            {
                this->bossHealthShown -= 0.02f;
                if (this->bossHealth > this->bossHealthShown)
                {
                    this->bossHealthShown = this->bossHealth;
                }
            }
        }
    }
    g_SpriteEngine->StepSprites(this->impl->sprites, 0x10);
    g_SpriteEngine->StepSprites(this->impl->trackNumberSprites, 4);
    if (!(g_GameManager.stateFlags >> 14 & 1) && this->impl->trackNumberSprites[0].alpha != 0)
    {
        g_SpriteEngine->StepSprites(&this->impl->stageTitleTime, 1);
    }
    g_SpriteEngine->Step(&this->impl->timePassedDigit);
    g_SpriteEngine->Step(&this->impl->clock);
    // The clock fades to 0x40 while the player is near it.
    if (this->impl->clock.alpha != 0)
    {
        if (g_Player.pos.x >= 64.0f && g_Player.pos.y < 128.0f)
        {
            if (this->impl->clock.alpha > 0x40)
            {
                this->impl->clock.alpha -= 4;
            }
        }
        else if (this->impl->clock.alpha < 0xff)
        {
            if (this->impl->clock.alpha <= 0xfb)
            {
                this->impl->clock.alpha += 4;
            }
            else
            {
                this->impl->clock.alpha = 0xff;
            }
        }
    }
    g_SpriteEngine->Step(&this->impl->livesBombsOverlay);
    g_SpriteEngine->Step(&this->impl->difficultyLabel);
    if (this->impl->loadingIcon.spriteIndex >= 0)
    {
        if (g_SpriteEngine->Step(&this->impl->loadingIcon) != 0)
        {
            this->impl->loadingIcon.spriteIndex = -1;
        }
        if (g_SpriteEngine->Step(&this->impl->loadingBar) != 0)
        {
            this->impl->loadingBar.spriteIndex = -1;
        }
        for (i = 0; i < ARRAY_SIZE(this->impl->loadingBarTrail); i++)
        {
            g_SpriteEngine->Step(&this->impl->loadingBarTrail[i]);
        }
    }
    if (this->impl->backgroundTileCount != 0)
    {
        remaining = 0xa8;
        for (j = 0; j < 0xa8; j++)
        {
            if (g_SpriteEngine->Step(&this->impl->backgroundTiles[j]) != 0)
            {
                remaining--;
            }
        }
        this->impl->backgroundTileCount = remaining;
    }
    if (this->impl->bonusPopup.shown)
    {
        if (this->impl->bonusPopup.timer < 30)
        {
            this->impl->bonusPopup.pos.x = this->impl->bonusPopup.timer.AsFloat() * -312.0f / 30.0f + 416.0f;
        }
        else
        {
            this->impl->bonusPopup.pos.x = 104.0f;
        }
        if (this->impl->bonusPopup.timer >= 250)
        {
            this->impl->bonusPopup.shown = 0;
        }
        this->impl->bonusPopup.timer++;
    }
    if (this->impl->fullPowerPopup.shown)
    {
        if (this->impl->fullPowerPopup.timer < 30)
        {
            this->impl->fullPowerPopup.pos.x = this->impl->fullPowerPopup.timer.AsFloat() * -312.0f / 30.0f + 416.0f;
        }
        else
        {
            this->impl->fullPowerPopup.pos.x = 104.0f;
        }
        if (this->impl->fullPowerPopup.timer >= 180)
        {
            this->impl->fullPowerPopup.shown = 0;
        }
        this->impl->fullPowerPopup.timer++;
    }
    if (this->impl->spellBonusPopup.shown)
    {
        if (this->impl->spellBonusPopup.timer >= 280)
        {
            this->impl->spellBonusPopup.shown = 0;
        }
        this->impl->spellBonusPopup.timer++;
    }
    if (this->impl->stageClearShown == 1)
    {
        clearBonus = 0;
        clearBonus += this->impl->stageClearBonus;
        clearBonus += this->impl->stageClearGraze * 50;
        clearBonus += this->impl->stageClearPoints * 5000;
        clearBonus += this->impl->stageClearTime * 100;
        if (g_GameManager.stage >= 6 && !g_GameManager.IsInPractice())
        {
            clearBonus += g_GameManager.GetLives() * 2500000;
            clearBonus += g_GameManager.GetBombs() * 500000;
        }
        if (g_GameManager.stage == 7)
        {
            clearBonus += (12 - g_GameManager.GetClock()) * 2000000;
        }
        switch (g_GameManager.difficulty)
        {
        case 0:
            clearBonus /= 2;
            break;
        case 2:
            clearBonus = clearBonus * 12 / 10;
            break;
        case 3:
            clearBonus = clearBonus * 15 / 10;
            break;
        case 4:
            clearBonus *= 2;
            break;
        }
        switch (g_GameManager.savedConfig->lifeCount)
        {
        case 3:
            clearBonus = clearBonus * 5 / 10;
            break;
        case 4:
            clearBonus = clearBonus * 2 / 10;
            break;
        case 5:
            clearBonus = clearBonus / 10;
            break;
        case 6:
            clearBonus = clearBonus / 20;
            break;
        }
        this->impl->stageClearTotal = clearBonus;
        for (m = 0; m < 10; m++)
        {
            g_GameManager.AddScore(clearBonus);
        }
        this->impl->stageClearShown++;
    }
    if (g_GameManager.stage < 6 && this->impl->stageClearClock2 != 0 &&
        this->impl->stageClearClock2 >= this->impl->stageClearLastTime && g_GameManager.stageEndState == 0)
    {
        g_GameManager.stageEndState = 2;
    }
    if (this->impl->stageClearClock2 != 0 && this->impl->stageClearClock2 != this->impl->stageClearLastTime)
    {
        if (this->impl->stageClearClockTimer >= 60)
        {
            if (this->impl->stageClearClock2 < this->impl->stageClearLastTime)
            {
                this->impl->stageClearClock2++;
                if ((g_GameButtons & 1) || (g_GameButtons & 0x100))
                {
                    this->impl->stageClearClock2 += 3;
                }
                if (this->impl->stageClearClock2 > this->impl->stageClearLastTime)
                {
                    this->impl->stageClearClock2 = this->impl->stageClearLastTime;
                }
            }
            else
            {
                this->impl->stageClearClockTimer++;
            }
        }
        else
        {
            this->impl->stageClearClockTimer++;
        }
    }
}

void Gui::DrawHud()
{
    float y;
    float xx;
    int idx;
    Sprite *vm;

    g_SpriteEngine->FlushQuadBatch();
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    if (!g_App.GetNoFrameItems())
    {
        vm = &this->impl->sprites[15];
        xx = 480.0f;
        vm->pos = D3DXVECTOR3(xx, 40.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
        vm->pos = D3DXVECTOR3(xx, 56.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
        if (this->livesChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 88.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->bombsChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 104.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->powerChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 136.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->grazeChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 152.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->frameItemsHideTimer)
        {
            vm->pos = D3DXVECTOR3(xx, 168.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        if (this->timeOrbsChanged)
        {
            vm->pos = D3DXVECTOR3(xx, 184.0f, 0.48f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        vm->pos = D3DXVECTOR3(512.0f, 464.0f, 0.48f);
        g_SpriteEngine->DrawUnrotated(vm);
    }
    vm = &this->impl->sprites[13];
    if (g_App.GetRedrawHudEveryFrame() || vm->scriptPos != NULL || g_App.redrawFrames != 0)
    {
        for (y = 0.0f; y < 464.0f; y += 32.0f)
        {
            vm->pos = D3DXVECTOR3(0.0f, y, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        for (xx = 416.0f; xx < 624.0f; xx += 32.0f)
        {
            for (y = 16.0f; y < 464.0f; y += 32.0f)
            {
                vm->pos = D3DXVECTOR3(xx, y, 0.49f);
                g_SpriteEngine->DrawUnrotated(vm);
            }
        }
        vm = &this->impl->sprites[14];
        for (xx = 0.0f; xx < 624.0f; xx += 128.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 0.0f, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
            vm->pos = D3DXVECTOR3(xx, 464.0f, 0.49f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[0]);
        g_SpriteEngine->Draw(&this->impl->sprites[1]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[2]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[3]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[4]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[5]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[6]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[7]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[8]);
        g_SpriteEngine->DrawUnrotated(&this->impl->sprites[9]);
        g_SpriteEngine->DrawUnrotated(&this->impl->difficultyLabel);
        this->livesChanged = 2;
        this->bombsChanged = 2;
        this->grazeChanged = 2;
        this->frameItemsHideTimer = 2;
        this->powerChanged = 2;
        this->timeOrbsChanged = 2;
    }
    if (this->livesChanged)
    {
        vm = &this->impl->sprites[10];
        for (idx = 0, xx = 488.0f; idx < g_GameManager.GetLives(); idx++, xx += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 88.0f, 0.46f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
    }
    if (this->bombsChanged)
    {
        vm = &this->impl->sprites[11];
        for (idx = 0, xx = 488.0f; idx < g_GameManager.GetBombs(); idx++, xx += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xx, 104.0f, 0.46f);
            g_SpriteEngine->DrawUnrotated(vm);
        }
    }
    if ((this->bombsChanged || this->livesChanged) && g_GameManager.lastSpellState == 1 && g_SpellCard.IsActive())
    {
        g_SpriteEngine->DrawUnrotated(&this->impl->livesBombsOverlay);
    }
    vm = &this->impl->sprites[14];
    for (xx = 32.0f; xx < 368.0f; xx += 128.0f)
    {
        vm->pos = D3DXVECTOR3(xx, 464.0f, 0.49f);
        g_SpriteEngine->DrawUnrotated(vm);
    }
    {
        D3DXVECTOR3 scorePos(488.0f, 56.0f, 0.0f);

        g_AsciiText.AddFormat(&scorePos, "%.9d", g_GameManager.stats->guiScore);
        scorePos.x += 117.0f;
        g_AsciiText.AddFormat(&scorePos, "%1d",
                              g_GameManager.stats->continuesUsed > 9 ? 9 : g_GameManager.stats->continuesUsed);
        g_AsciiText.SetScale(1.0f, 1.0f);
        scorePos = D3DXVECTOR3(488.0f, 40.0f, 0.0f);
        g_AsciiText.AddFormat(&scorePos, "%.9d", g_GameManager.stats->highScore);
        scorePos.x += 117.0f;
        g_AsciiText.AddFormat(&scorePos, "%1d",
                              g_GameManager.stats->bestContinuesUsed > 9 ? 9
                                                                         : g_GameManager.stats->bestContinuesUsed);
        g_AsciiText.SetScale(1.0f, 1.0f);
        if (this->grazeChanged || g_App.GetNoFrameItems())
        {
            scorePos = D3DXVECTOR3(488.0f, 152.0f, 0.0f);
            g_AsciiText.AddFormat(&scorePos, "%d", g_GameManager.stats->grazeTotal);
        }
        if (this->frameItemsHideTimer || g_App.GetNoFrameItems())
        {
            scorePos = D3DXVECTOR3(488.0f, 168.0f, 0.0f);
            scorePos.x += g_AsciiText.AddFormatLen(&scorePos, "%d", g_GameManager.stats->pointItemsTotal) * 13;
            g_AsciiText.SetScale(0.5f, 1.0f);
            g_AsciiText.AddFormat(&scorePos, "/");
            g_AsciiText.SetScale(1.0f, 1.0f);
            scorePos.x += 6.0f;
            g_AsciiText.AddFormat(&scorePos, "%d", g_GameManager.stats->checksumKeyPoints);
        }
        if (this->timeOrbsChanged || g_App.GetNoFrameItems())
        {
            if (g_GameManager.GetTimeOrbs() >= g_GameManager.GetTimeOrbsNeeded())
            {
                g_AsciiText.SetColor(0xfffff0c0);
            }
            scorePos = D3DXVECTOR3(488.0f, 184.0f, 0.0f);
            scorePos.x += g_AsciiText.AddFormatLen(&scorePos, "%d", g_GameManager.GetTimeOrbs()) * 13;
            g_AsciiText.SetScale(0.5f, 1.0f);
            g_AsciiText.AddFormat(&scorePos, "/");
            g_AsciiText.SetScale(1.0f, 1.0f);
            scorePos.x += 6.0f;
            g_AsciiText.AddFormat(&scorePos, "%d", g_GameManager.GetTimeOrbsNeeded());
            g_AsciiText.SetColor(0xffffffff);
        }
    }
    g_SpriteEngine->FlushQuadBatch();
    if (this->powerChanged || g_App.GetNoFrameItems())
    {
        VertexDiffuseXyzrwh vertices[4];

        if (g_GameManager.GetPower() > 0)
        {
            vertices[0].pos = D3DXVECTOR3(488.0f, 136.0f, 0.1f);
            vertices[1].pos = D3DXVECTOR3(g_GameManager.GetPower() + 488 + 0.0f, 136.0f, 0.1f);
            vertices[2].pos = D3DXVECTOR3(488.0f, 152.0f, 0.1f);
            vertices[3].pos = D3DXVECTOR3(g_GameManager.GetPower() + 488 + 0.0f, 152.0f, 0.1f);
            vertices[0].diffuse = vertices[2].diffuse = 0xe0e0e0ff;
            vertices[1].diffuse = vertices[3].diffuse = 0x80e0e0ff;
            vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
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
        if (g_GameManager.GetPower() < 128)
        {
            g_AsciiText.AddFormat(&D3DXVECTOR3(488.0f, 136.0f, 0.0f), "%d", g_GameManager.GetPower());
        }
        else
        {
            g_AsciiText.AddFormat(&D3DXVECTOR3(488.0f, 136.0f, 0.0f), "MAX");
        }
    }
    if (this->livesChanged)
    {
        this->livesChanged--;
    }
    if (this->powerChanged)
    {
        this->powerChanged--;
    }
    if (this->bombsChanged)
    {
        this->bombsChanged--;
    }
    if (this->grazeChanged)
    {
        this->grazeChanged--;
    }
    if (this->frameItemsHideTimer)
    {
        this->frameItemsHideTimer--;
    }
    if (this->timeOrbsChanged)
    {
        this->timeOrbsChanged--;
    }
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
        g_SpriteEngine->Draw(&this->impl->trackNumberSprites[i]);
    }
    g_SpriteEngine->Draw(&this->impl->stageTitleTime);
    g_SpriteEngine->Draw(&this->impl->clock);
    if (this->impl->loadingIcon.spriteIndex >= 0)
    {
        g_SpriteEngine->DrawUnrotated(&this->impl->loadingIcon);
        g_SpriteEngine->DrawProjected3D(&this->impl->loadingBar);
        for (i = 0; i < ARRAY_SIZE(this->impl->loadingBarTrail); i++)
        {
            g_SpriteEngine->DrawProjected3D(&this->impl->loadingBarTrail[i]);
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
        if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu && !g_GameManager.deathbombFreeze &&
            g_EnemyManager.bosses[0] != NULL)
        {
            pos = D3DXVECTOR3(2.0f, 29.0f, 0.0f);
            g_AsciiText.SetScale(1.0f, 1.0f);
            g_AsciiText.AddNumberPopupBrief(&pos, g_EnemyManager.bosses[0]->CountFollowers(), g_EnemyManager.bosses[0]->followerCount,
                                   0xfff0f00f);
        }
    }
    g_SpriteEngine->DrawUnrotated(&this->impl->timePassedDigit);
}

int Gui::AddedCallback(Gui *gui)
{
    return gui->LoadResources();
}

int Gui::DeletedCallback(Gui *gui)
{
    if (!GuiIsRestarting())
    {
        g_SpriteEngine->ReleaseAnm(0xd);
    }
    gui->FreeMsgFile();
    if (GuiIsFreeingResources())
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

    if (GuiIsLoadingResources())
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = (GuiImpl *)g_MemoryManager.Register(new GuiImpl, sizeof(GuiImpl), "FRScreenImplInf");
    }
    g_GuiCalcElem.SetCallback((ChainCallback)Gui::OnUpdate);
    g_GuiCalcElem.addedCallback = (ChainCallback)Gui::AddedCallback;
    g_GuiCalcElem.deletedCallback = (ChainCallback)Gui::DeletedCallback;
    g_GuiCalcElem.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcElem, 15) != 0)
    {
        return -1;
    }
    g_GuiDrawElement.SetCallback((ChainCallback)Gui::OnDraw);
    g_GuiDrawElement.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawElement, 17);
    return 0;
}

void DebugPrint2(const char *fmt, ...)
{
}

void Gui::RemoveChain()
{
    g_Chain.Remove(&g_GuiCalcElem);
    g_Chain.Remove(&g_GuiDrawElement);
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

void CopyStageSprite()
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
    g_AsciiText.AddFormat(&cursor, "over-80%% = %3d.%.2d%%", g_GameManager.lowGaugeFrames * 100 / g_GameManager.gaugeFrames,
                          g_GameManager.lowGaugeFrames * 10000 / g_GameManager.gaugeFrames % 100);
    cursor.y += 16.0f;
    g_AsciiText.AddFormat(&cursor, "over 80%% = %3d.%.2d%%", g_GameManager.highGaugeFrames * 100 / g_GameManager.gaugeFrames,
                          g_GameManager.highGaugeFrames * 10000 / g_GameManager.gaugeFrames % 100);
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
            g_AsciiText.AddFormat(&cursor, "        %8d0", (12 - g_GameManager.GetClock()) * 2000000);
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
    if (g_GameManager.difficulty < 4 && !(g_GameManager.stateFlags & 1))
    {
        cursor.y += 16.0f;
        switch (g_GameManager.savedConfig->lifeCount)
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

int App::GetRedrawHudEveryFrame()
{
    return this->flags.redrawHudEveryFrame;
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

void Gui::StartLoadingBar()
{
    g_AsciiText.anm2->StartScript(&this->impl->loadingBar, 1);
    g_SpriteEngine->SetPendingCopy(3, 32, 16, 384, 448, this->impl->loadingBar.spriteData->startX,
                                   this->impl->loadingBar.spriteData->startY, this->impl->loadingBar.spriteData->width,
                                   this->impl->loadingBar.spriteData->height);
}

int GuiIsLoadingResources()
{
    return g_App.loadsResources;
}

int GuiIsFreeingResources()
{
    return g_App.freesResources;
}

int GuiIsRestarting()
{
    return g_App.isRestarting;
}

int Gui::StartClock()
{
    this->timesAnm->StartScriptAtOrigin(&this->impl->clock, 2);
    this->timesAnm->SetSprite(&this->impl->clock, g_GameManager.GetClock());
    return 0;
}

int Gui::AdvanceClock()
{
    this->timesAnm->SetSprite(&this->impl->clock, g_GameManager.GetClock());
    this->impl->clock.SetPendingInterrupt(1);
    return 0;
}

int Gui::AdvanceClockToEnd()
{
    this->timesAnm->SetSprite(&this->impl->clock, g_GameManager.GetClock());
    this->impl->clock.SetPendingInterrupt(2);
    return 0;
}

int Gui::HideClock()
{
    this->impl->clock.alpha = 0;
    return 0;
}

int Gui::LoadResources()
{
    int x;
    int y;
    DWORD i;

    if (GuiIsLoadingResources())
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
        g_AsciiText.anm->StartScript(&this->impl->livesBombsOverlay, 0x1a);
        g_AsciiText.anm->StartScript(&this->impl->difficultyLabel, 0x19);
        if (g_GameManager.IsInSpellCardPractice() && g_GameManager.practiceSpellCard >= 0xcd)
        {
            g_AsciiText.anm->SetSprite(&this->impl->difficultyLabel, 0x120);
        }
        else
        {
            g_AsciiText.anm->SetSprite(&this->impl->difficultyLabel, g_GameManager.difficulty + 0x11b);
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
    g_Gui.HideClock();
    this->timesAnm->StartScriptAtOrigin(&this->impl->stageTitleTime, 0);
    this->timesAnm->SetSprite(&this->impl->stageTitleTime, g_GameManager.GetClock());
    if (!g_GameManager.IsInSpellCardPractice() &&
        this->LoadMsg(g_MsgPaths[g_GameManager.stage][g_GameManager.character]) != 0)
    {
        return -1;
    }
    if (!GuiIsRestarting())
    {
        if (!(g_GameManager.stateFlags >> 14 & 1) || g_GameManager.practiceSpellCard < 0xcd)
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
    if (GuiIsLoadingResources())
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
    if (!(g_GameManager.stateFlags >> 14 & 1))
    {
        this->textAnm->StartSpriteSeries(this->impl->trackNumberSprites, 0, 4);
    }
    else if (!GuiIsRestarting() || GetPracticeBgmRestarts(g_GameManager.practiceSpellCard))
    {
        this->textAnm->StartSpriteSeries(this->impl->trackNumberSprites, 3, 1);
        this->textAnm->SetSprite(this->impl->trackNumberSprites, GetPracticeBgmTitleIndex(g_GameManager.practiceSpellCard) + 3);
    }
    this->impl->msg.scriptIndex = -1;
    this->impl->stageClearShown = 0;
    this->impl->bonusPopup.shown = 0;
    this->impl->fullPowerPopup.shown = 0;
    this->impl->spellBonusPopup.shown = 0;
    this->livesChanged = 2;
    this->bombsChanged = 2;
    this->grazeChanged = 2;
    this->frameItemsHideTimer = 2;
    this->powerChanged = 2;
    this->timeOrbsChanged = 2;
    g_AsciiText.anm->StartScript(&this->impl->timePassedDigit, 3);
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

int GetPracticeBgmRestarts(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].restartsMusic;
        }
        i++;
    }
    return 0;
}

int GetPracticeBgmTitleIndex(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].titleIndex;
        }
        i++;
    }
    return 0;
}
