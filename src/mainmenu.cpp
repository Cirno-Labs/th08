// cflags: /Os /Ob1 -/Oi
// The /Os object from 0x467399 to 0x4716df, with the title menu, th07's
// mainmenu.cpp. Like App's object it expands small inlines (SetState,
// Sprite::SetPendingInterrupt); memset and strcpy are calls.

#include <direct.h>
#include <stdio.h>

#include "ascii.h"
#include "mainmenu.h"
#include "midi.h"
#include "resultscreen.h"
#include "screeneffect.h"
#include "unsorted.h"

int MainMenu::OnUpdate(MainMenu *menu)
{
    int status;

    if (menu->loadingState != 0)
    {
        if (menu->loadingState == 2)
        {
            return 4;
        }
        return 1;
    }
    switch (menu->state)
    {
    case 0:
        status = menu->StartTitle();
        break;
    case 7:
        status = menu->UpdateReplayMenu();
        break;
    case 2:
        status = menu->UpdateOptions();
        break;
    case 3:
        status = menu->UpdateKeyConfig();
        break;
    case 4:
    case 8:
    case 12:
        status = menu->UpdateDifficultyMenu();
        break;
    case 5:
    case 9:
    case 13:
    case 15:
        status = menu->UpdateCharacterMenu();
        break;
    case 11:
        status = menu->UpdateGameStartMenu();
        break;
    case 16:
        status = menu->UpdateState16();
        break;
    case 17:
        status = menu->UpdateState17();
        break;
    }
    g_SpriteEngine->StepSprites(menu->sprites, menu->spriteCount);
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->Step(menu->helpSprite);
    }
    return status;
}
int MainMenu::StartTitle()
{
    int i;
    int size;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState == 0 && g_App.prevScene != 5)
            {
                g_App.PlayMidi(8, 0);
            }
            if ((this->prevState == 0 || this->prevState == 4 || this->prevState == 7 || this->prevState == 8 ||
                 this->prevState == 12 || this->prevState == 15 || this->prevState == 16) &&
                g_SpriteEngine->LoadSurface(0, "title/title00.png") != 0)
            {
                return 0;
            }
            if (this->spriteCount == 0)
            {
                this->spriteCount = 0x8e;
                this->sprites = new Sprite[this->spriteCount];
                this->titleAnm->StartSpriteSeries(this->sprites, 0, this->spriteCount);
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 2);
            if (g_GameManager.IsReplay())
            {
                this->SetState(7);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
                this->helpSprite->SetPendingInterrupt(2);
                g_GameManager.SetIsReplay(0);
                return 1;
            }
            if (this->returningFromPractice != 0)
            {
                if (this->returningFromPractice == 2)
                {
                    g_GameManager.unk_3dbac |= 0x4000;
                }
                this->SetState(g_GameManager.IsInSpellCardPractice() ? 15 : 8);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            }
            for (i = 0; i < 9; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 1], this->sprites[i + 1].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
            if (!g_GameManager.Func43c2c5())
            {
                this->sprites[3].color = 0xff404040;
            }
            if (!g_GameManager.AnyShotHasExtraOpen())
            {
                this->sprites[2].color = 0xff404040;
            }
        }
        if (this->inputLockout < ARRAY_SIZE(g_TitleHelpTexts))
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[this->inputLockout], 0xfff0e0, 0x300000,
                                               g_TitleHelpTexts[this->inputLockout]);
            this->inputLockout++;
            return 1;
        }
        this->inputLockout = 0;
        this->stateFrames = 0;
        this->prevCursor = -1;
        this->subState = 1;
        this->idleFrames = 0;
    case 1:
        i = this->MoveMenuCursor(9);
        if (i != 0)
        {
            for (;;)
            {
                while (!g_GameManager.Func43c2c5() && this->cursor == 2)
                {
                    this->cursor += i;
                }
                if (!g_GameManager.AnyShotHasExtraOpen() && this->cursor == 1)
                {
                    this->cursor += i;
                    continue;
                }
                break;
            }
            for (i = 0; i < 9; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 1], this->sprites[i + 1].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
            if (!g_GameManager.Func43c2c5())
            {
                this->sprites[3].color = 0xff404040;
            }
            if (!g_GameManager.AnyShotHasExtraOpen())
            {
                this->sprites[2].color = 0xff404040;
            }
        }
        this->idleFrames++;
        if (g_Buttons != 0)
        {
            this->idleFrames = 0;
        }
        if (this->idleFrames > 1500)
        {
            g_GameManager.demoIndex++;
            g_GameManager.demoIndex %= 4;
            strcpy(g_GameManager.replayFile, g_DemoReplayPaths[g_GameManager.demoIndex]);
            this->loadedReplay = (ReplayHeader *)LoadFileData(g_GameManager.replayFile, &size, 0);
            this->loadedReplay = ValidateReplay(this->loadedReplay, size);
            if (this->loadedReplay == NULL)
            {
                DebugPrint2("error : Demo Play is not ready\r\n");
                this->idleFrames = 0;
            }
            else
            {
                g_GameManager.SetIsReplay(1);
                g_GameManager.unk_3dbac |= 2;
                g_GameManager.demoFrames = 0;
                g_GameManager.difficulty = this->loadedReplay->difficulty;
                g_GameManager.character = this->loadedReplay->shotType / 2;
                g_GameManager.shotType = this->loadedReplay->shotType % 2;
                g_GameManager.character = this->loadedReplay->shotType;
                i = 0;
                while (this->loadedReplay->stages[i] == NULL)
                {
                    i++;
                }
                g_GameManager.stage = i;
                MemFree(this->loadedReplay);
                this->loadedReplay = NULL;
                g_App.scene = 2;
                g_GameManager.replayMenuChoice = 0;
                g_GameManager.unk_3dbac &= ~0x4000;
                return 0;
            }
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        if (this->inputLockout < 10)
        {
            break;
        }
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            switch (this->cursor)
            {
            case 0:
                g_GameManager.unk_3dbac &= ~1;
                g_GameManager.unk_3dbac &= ~0x4000;
                this->cursor = g_App.defaultDifficulty;
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(4);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 3:
                g_GameManager.unk_3dbac |= 1;
                g_GameManager.unk_3dbac &= ~0x4000;
                this->cursor = g_App.defaultDifficulty;
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(8);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 1:
                if (g_GameManager.AnyShotHasExtraOpen())
                {
                    g_GameManager.unk_3dbac &= ~1;
                    g_GameManager.unk_3dbac &= ~0x4000;
                    this->cursor = 0;
                    this->SetState(12);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    this->helpSprite->SetPendingInterrupt(2);
                    return 1;
                }
            case 2:
                if (g_GameManager.Func43c2c5())
                {
                    g_GameManager.unk_3dbac |= 1;
                    g_GameManager.unk_3dbac |= 0x4000;
                    this->cursor = g_GameManager.character;
                    this->SetState(16);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    this->helpSprite->SetPendingInterrupt(2);
                    return 1;
                }
            case 4:
                g_GameManager.unk_3dbac &= ~1;
                g_GameManager.unk_3dbac &= ~0x4000;
                this->SetState(7);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 6:
                g_App.scene = 8;
                this->helpSprite->SetPendingInterrupt(2);
                return 0;
            case 5:
                g_App.scene = 5;
                this->helpSprite->SetPendingInterrupt(2);
                return 0;
            case 7:
                this->subState = 0;
                this->cursor = 0;
                this->inputLockout = 0;
                this->stateFrames = 0;
                this->subState = 3;
                this->stateFrames = 0;
                this->UpdateOptions();
                this->cursor = 0;
                break;
            case 8:
                this->subState = 2;
                this->stateFrames = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 1);
                if (g_App.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
            }
        }
        if (WAS_PRESSED(10))
        {
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex + 1);
            this->cursor = 8;
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
        break;
    case 2:
        if (this->stateFrames >= 60)
        {
            delete[] this->sprites;
            this->sprites = NULL;
            this->sprites = NULL;
            this->spriteCount = 0;
            this->inputLockout = 0;
            g_App.scene = -1;
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->SetState(2);
            this->cursor = 0;
            this->savedConfig = *(ConfigCopy *)&g_App.controllerMapping;
            return 1;
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateOptions()
{
    int i;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 3);
            for (i = 0; i < 10; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 10], this->sprites[i + 10].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 10],
                                      this->sprites[this->cursor + 10].baseSpriteIndex);
            this->subState = 0;
            this->stateFrames = 0;
            this->prevCursor = -1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_OptionsHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_OptionsHelpTexts[i]);
        }
    case 1:
        if (this->MoveMenuCursor(10))
        {
            for (i = 0; i < 10; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 10], this->sprites[i + 10].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 10],
                                      this->sprites[this->cursor + 10].baseSpriteIndex);
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        for (i = 0x14; i <= 0x1a; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 30)
        {
            this->sprites[0x19].flags &= ~2;
        }
        if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 60)
        {
            this->sprites[0x1a].flags &= ~2;
        }
        i = g_App.lifeCount + 0x14;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x1b; i <= 0x1c; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.colorMode16Bits + 0x1b;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x1d; i <= 0x1f; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.musicMode + 0x1d;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        if (g_App.bgmVolume >= 100)
        {
            this->titleAnm->SetSprite(&this->sprites[0x20], this->sprites[0x20].baseSpriteIndex);
            this->sprites[0x20].alpha = 0xff;
        }
        else
        {
            this->sprites[0x20].alpha = 0;
        }
        if (g_App.bgmVolume >= 10)
        {
            this->titleAnm->SetSprite(&this->sprites[0x21],
                                      this->sprites[0x21].baseSpriteIndex + g_App.bgmVolume / 10 % 10 * 2);
            this->sprites[0x21].alpha = 0xff;
        }
        else
        {
            this->sprites[0x21].alpha = 0;
        }
        this->titleAnm->SetSprite(&this->sprites[0x22], this->sprites[0x22].baseSpriteIndex + g_App.bgmVolume % 10 * 2);
        this->titleAnm->SetSprite(&this->sprites[0x23], this->sprites[0x23].baseSpriteIndex);
        if (g_App.seVolume >= 100)
        {
            this->titleAnm->SetSprite(&this->sprites[0x24], this->sprites[0x24].baseSpriteIndex);
            this->sprites[0x24].alpha = 0xff;
        }
        else
        {
            this->sprites[0x24].alpha = 0;
        }
        if (g_App.seVolume >= 10)
        {
            this->titleAnm->SetSprite(&this->sprites[0x25],
                                      this->sprites[0x25].baseSpriteIndex + g_App.seVolume / 10 % 10 * 2);
            this->sprites[0x25].alpha = 0xff;
        }
        else
        {
            this->sprites[0x25].alpha = 0;
        }
        this->titleAnm->SetSprite(&this->sprites[0x26], this->sprites[0x26].baseSpriteIndex + g_App.seVolume % 10 * 2);
        this->titleAnm->SetSprite(&this->sprites[0x27], this->sprites[0x27].baseSpriteIndex);
        for (i = 0x28; i <= 0x29; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.windowed + 0x28;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        for (i = 0x2a; i <= 0x2b; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.optionsMenuCursor + 0x2a;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        if (this->inputLockout < 4)
        {
            break;
        }
        if (WAS_PRESSED_OR_HELD(0x40))
        {
            switch (this->cursor)
            {
            case 0:
                i = 7;
                if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 30)
                {
                    i--;
                }
                if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 60)
                {
                    i--;
                }
                if (g_App.lifeCount == 0)
                {
                    g_App.lifeCount = i - 1;
                }
                else
                {
                    g_App.lifeCount--;
                }
                break;
            case 1:
                if (g_App.colorMode16Bits == 0)
                {
                    g_App.colorMode16Bits = 1;
                }
                else
                {
                    g_App.colorMode16Bits--;
                }
                break;
            case 2:
                g_App.StopMusic();
                if (g_App.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
                if (g_App.musicMode == 0)
                {
                    g_App.musicMode = 2;
                }
                else
                {
                    g_App.musicMode--;
                }
                g_App.LoadMidi(8, "bgm/th08_01.mid");
                g_App.PlayMidi(8, 0);
                break;
            case 5:
                if (g_App.windowed == 0)
                {
                    g_App.windowed = 1;
                }
                else
                {
                    g_App.windowed--;
                }
                break;
            case 6:
                if (g_App.optionsMenuCursor == 0)
                {
                    g_App.optionsMenuCursor = 1;
                }
                else
                {
                    g_App.optionsMenuCursor--;
                }
                break;
            default:
                goto volume;
            }
            g_SoundPlayer.QueueSound(12, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    volume:
        if (WAS_PRESSED(0x40))
        {
            switch (this->cursor)
            {
            case 3:
                g_App.bgmVolume -= 4;
                if (g_App.bgmVolume < 0)
                {
                    g_App.bgmVolume = 0;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                g_App.seVolume -= 4;
                if (g_App.seVolume < 0)
                {
                    g_App.seVolume = 0;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (WAS_PRESSED(0x80))
        {
            switch (this->cursor)
            {
            case 3:
                g_App.bgmVolume += 4;
                if (g_App.bgmVolume > 100)
                {
                    g_App.bgmVolume = 100;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                g_App.seVolume += 4;
                if (g_App.seVolume > 100)
                {
                    g_App.seVolume = 100;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (WAS_PRESSED(0x40) || g_ButtonFrames >= 30 && (g_Buttons & 0x40))
        {
            switch (this->cursor)
            {
            case 3:
                if (g_App.bgmVolume > 0)
                {
                    g_App.bgmVolume--;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                if (g_App.seVolume > 0)
                {
                    g_App.seVolume--;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (WAS_PRESSED(0x80) || g_ButtonFrames >= 30 && (g_Buttons & 0x80))
        {
            switch (this->cursor)
            {
            case 3:
                if (g_App.bgmVolume < 100)
                {
                    g_App.bgmVolume++;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                if (g_App.seVolume < 100)
                {
                    g_App.seVolume++;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        g_SoundPlayer.bgmVolume = g_App.bgmVolume;
        g_SoundPlayer.seVolume = g_App.seVolume;
        if (WAS_PRESSED_OR_HELD(0x80))
        {
            switch (this->cursor)
            {
            case 0:
                i = 7;
                if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 30)
                {
                    i--;
                }
                if ((DWORD)g_GameManager.plst.counts[6].plays[0] < 60)
                {
                    i--;
                }
                if (g_App.lifeCount >= i - 1)
                {
                    g_App.lifeCount = 0;
                }
                else
                {
                    g_App.lifeCount++;
                }
                break;
            case 1:
                if (g_App.colorMode16Bits >= 1)
                {
                    g_App.colorMode16Bits = 0;
                }
                else
                {
                    g_App.colorMode16Bits++;
                }
                break;
            case 2:
                g_App.StopMusic();
                if (g_App.musicMode >= 2)
                {
                    g_App.musicMode = 0;
                }
                else
                {
                    g_App.musicMode++;
                }
                g_App.LoadMidi(8, "bgm/th08_01.mid");
                g_App.PlayMidi(8, 0);
                break;
            case 5:
                if (g_App.windowed >= 1)
                {
                    g_App.windowed = 0;
                }
                else
                {
                    g_App.windowed++;
                }
                break;
            case 6:
                if (g_App.optionsMenuCursor >= 1)
                {
                    g_App.optionsMenuCursor = 0;
                }
                else
                {
                    g_App.optionsMenuCursor++;
                }
                break;
            default:
                goto idle;
            }
            g_SoundPlayer.QueueSound(12, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    idle:
        switch (this->cursor)
        {
        case 3:
        case 4:
            if (this->inputLockout % 50 == 0)
            {
                g_SoundPlayer.QueueSound(0x1d, 0);
            }
        }
        if (g_Buttons != 0)
        {
            this->subMenuIdleFrames = 0;
        }
        if (this->subMenuIdleFrames >= 3600)
        {
            goto leave;
        }
        if (WAS_PRESSED(0x1001))
        {
            switch (this->cursor)
            {
            case 7:
                g_App.lifeCount = 2;
                g_App.bombCount = 3;
                g_App.musicMode = 1;
                g_App.playSounds = 1;
                g_App.optionsMenuCursor = 0;
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                break;
            case 8:
                this->cursor = 0;
                this->SetState(3);
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                return 1;
            case 9:
            leave:
                this->cursor = 7;
                this->SetState(0);
                g_SoundPlayer.QueueSound(11, 0);
                g_SoundPlayer.PlayQueuedSounds();
                if (this->savedConfig.colorMode16Bits != g_App.colorMode16Bits ||
                    this->savedConfig.windowed != g_App.windowed)
                {
                    return 5;
                }
                return 1;
            }
        }
        if (WAS_PRESSED(10))
        {
            if (this->cursor == 9)
            {
                goto leave;
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 10],
                                      this->sprites[this->cursor + 10].baseSpriteIndex + 1);
            this->cursor = 9;
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 10],
                                      this->sprites[this->cursor + 10].baseSpriteIndex);
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

void MainMenu::ReassignButton(short button, short replacement, int unused)
{
    if (this->controllerMapping.shootButton == button)
    {
        this->controllerMapping.shootButton = replacement;
    }
    if (this->controllerMapping.bombButton == button)
    {
        this->controllerMapping.bombButton = replacement;
    }
    if (this->controllerMapping.focusButton == button)
    {
        this->controllerMapping.focusButton = replacement;
    }
    if (this->controllerMapping.upButton == button)
    {
        this->controllerMapping.upButton = replacement;
    }
    if (this->controllerMapping.downButton == button)
    {
        this->controllerMapping.downButton = replacement;
    }
    if (this->controllerMapping.leftButton == button)
    {
        this->controllerMapping.leftButton = replacement;
    }
    if (this->controllerMapping.rightButton == button)
    {
        this->controllerMapping.rightButton = replacement;
    }
    if (this->controllerMapping.menuButton == button)
    {
        this->controllerMapping.menuButton = replacement;
    }
    if (this->controllerMapping.skipButton == button)
    {
        this->controllerMapping.skipButton = replacement;
    }
}

int MainMenu::UpdateKeyConfig()
{
    Sprite *digit;
    int i;
    short key;
    BYTE *pad;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 4);
            for (i = 0; i < 12; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 0x2d], this->sprites[i + 0x2d].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 0x2d],
                                      this->sprites[this->cursor + 0x2d].baseSpriteIndex);
            this->subState = 0;
            this->stateFrames = 0;
            this->controllerMapping = g_App.controllerMapping;
            g_App.controllerMapping.upButton = -1;
            g_App.controllerMapping.downButton = -1;
            digit = &this->sprites[0x39];
            this->ShowButtonDigits(digit, this->controllerMapping.shootButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.bombButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.focusButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.skipButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.menuButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.upButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.downButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.leftButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->controllerMapping.rightButton);
            this->prevCursor = -1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_KeyConfigHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_KeyConfigHelpTexts[i]);
        }
    case 1:
        if (this->MoveMenuCursor(12))
        {
            for (i = 0; i < 12; i++)
            {
                this->titleAnm->SetSprite(&this->sprites[i + 0x2d], this->sprites[i + 0x2d].baseSpriteIndex + 1);
            }
            this->titleAnm->SetSprite(&this->sprites[this->cursor + 0x2d],
                                      this->sprites[this->cursor + 0x2d].baseSpriteIndex);
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        digit = &this->sprites[0x39];
        this->ShowButtonDigits(digit, this->controllerMapping.shootButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.bombButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.focusButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.skipButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.menuButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.upButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.downButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.leftButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->controllerMapping.rightButton);
        for (i = 0x4b; i <= 0x4c; i++)
        {
            this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        }
        i = g_App.shootFocuses + 0x4b;
        this->titleAnm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex);
        pad = GetPadButtons();
        for (key = 0; key < 0x20; key++)
        {
            if (pad[key] & 0x80)
            {
                break;
            }
        }
        if (key < 0x20 && g_LastPressedKey != key)
        {
            switch (this->cursor)
            {
            case 0:
                this->ReassignButton(key, this->controllerMapping.shootButton, 1);
                this->controllerMapping.shootButton = key;
                break;
            case 1:
                this->ReassignButton(key, this->controllerMapping.bombButton, 0);
                this->controllerMapping.bombButton = key;
                break;
            case 2:
                this->ReassignButton(key, this->controllerMapping.focusButton, 1);
                this->controllerMapping.focusButton = key;
                break;
            case 3:
                this->ReassignButton(key, this->controllerMapping.menuButton, 0);
                this->controllerMapping.menuButton = key;
                break;
            case 4:
                this->ReassignButton(key, this->controllerMapping.upButton, 0);
                this->controllerMapping.upButton = key;
                break;
            case 5:
                this->ReassignButton(key, this->controllerMapping.downButton, 0);
                this->controllerMapping.downButton = key;
                break;
            case 6:
                this->ReassignButton(key, this->controllerMapping.leftButton, 0);
                this->controllerMapping.leftButton = key;
                break;
            case 7:
                this->ReassignButton(key, this->controllerMapping.rightButton, 0);
                this->controllerMapping.rightButton = key;
                break;
            case 8:
                this->ReassignButton(key, this->controllerMapping.skipButton, 0);
                this->controllerMapping.skipButton = key;
                break;
            default:
                goto assigned;
            }
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
        }
    assigned:
        g_LastPressedKey = key;
        if (WAS_PRESSED(0x40))
        {
            switch (this->cursor)
            {
            case 9:
                g_App.shootFocuses = 1 - g_App.shootFocuses;
            }
        }
        if (WAS_PRESSED(0x80))
        {
            switch (this->cursor)
            {
            case 9:
                g_App.shootFocuses = 1 - g_App.shootFocuses;
            }
        }
        if (g_Buttons != 0)
        {
            this->subMenuIdleFrames = 0;
        }
        if (this->subMenuIdleFrames >= 3600)
        {
            goto leave;
        }
        if (WAS_PRESSED(0x1001))
        {
            switch (this->cursor)
            {
            case 10:
                g_SoundPlayer.QueueSound(10, 0);
                g_SoundPlayer.PlayQueuedSounds();
                this->controllerMapping = g_DefaultControllerMapping;
                g_App.shootFocuses = 1;
                break;
            case 11:
            leave:
                g_SoundPlayer.QueueSound(11, 0);
                g_SoundPlayer.PlayQueuedSounds();
                this->SetState(2);
                g_App.controllerMapping = this->controllerMapping;
                this->cursor = 8;
                return 1;
            }
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateDifficultyMenu()
{
    int count;
    int i;
    int state;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState != 5 && this->prevState != 9 && this->prevState != 13 &&
                g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            this->cursor = g_App.defaultDifficulty;
            if (this->state != 12)
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 7);
            }
            else if (!g_GameManager.IsPhantasmOpen())
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 12);
                this->cursor = 4;
            }
            if (this->state != 12)
            {
                if (this->cursor >= 4)
                {
                    this->cursor = 1;
                }
                for (i = 0; i < 4; i++)
                {
                    this->titleAnm->SetSprite(&this->sprites[i + 0x83], this->sprites[i + 0x83].baseSpriteIndex + 1);
                    this->sprites[i + 0x83].SetPendingInterrupt(0x19);
                }
                this->titleAnm->SetSprite(&this->sprites[this->cursor + 0x83],
                                          this->sprites[this->cursor + 0x83].baseSpriteIndex);
                this->sprites[this->cursor + 0x83].SetPendingInterrupt(0x18);
            }
            else
            {
                this->cursor -= 4;
                if (this->cursor < 0)
                {
                    this->cursor = 0;
                }
                this->titleAnm->SetSprite(&this->sprites[0x87], this->sprites[0x87].baseSpriteIndex);
            }
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->returningFromPractice != 0)
        {
            this->SetState(9);
            this->cursor = 0;
            return 1;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        count = this->state != 12 ? 4 : 1;
        if (this->MoveMenuCursor(count))
        {
            if (this->state != 12)
            {
                for (i = 0; i < 4; i++)
                {
                    this->titleAnm->SetSprite(&this->sprites[i + 0x83], this->sprites[i + 0x83].baseSpriteIndex + 1);
                    this->sprites[i + 0x83].SetPendingInterrupt(0x19);
                }
                this->titleAnm->SetSprite(&this->sprites[this->cursor + 0x83],
                                          this->sprites[this->cursor + 0x83].baseSpriteIndex);
                this->sprites[this->cursor + 0x83].SetPendingInterrupt(0x18);
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            if (this->state != 12)
            {
                g_App.defaultDifficulty = this->cursor;
            }
            else
            {
                g_App.defaultDifficulty = this->cursor + 4;
            }
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (this->state != 12)
            {
                if (!(g_GameManager.unk_3dbac & 1))
                {
                    this->SetState(5);
                }
                else
                {
                    this->SetState(9);
                }
            }
            else
            {
                this->SetState(13);
            }
            this->cursor = 0;
            return 1;
        }
        if (WAS_PRESSED(0xa))
        {
            if (this->state != 12)
            {
                g_App.defaultDifficulty = this->cursor;
            }
            else
            {
                g_App.defaultDifficulty = this->cursor + 4;
            }
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
            this->subState = 3;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            state = this->state;
            this->SetState(0);
            if (state != 12)
            {
                if (!(g_GameManager.unk_3dbac & 1))
                {
                    this->cursor = 0;
                }
                else
                {
                    this->cursor = 3;
                }
            }
            else
            {
                this->cursor = 1;
            }
            g_GameManager.unk_3dbac &= ~1;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateCharacterMenu()
{
    int charCount;
    int dir;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            int i;
            DWORD j;

            this->cursor = g_GameManager.character;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 8);
            charCount = g_GameManager.AllShotsHaveExtraOpen() ? 12 : 4;
            if (this->state == 15)
            {
                if (g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
                {
                    return 0;
                }
                while (!g_GameManager.Func43c23b(this->cursor))
                {
                    this->cursor++;
                    if (this->cursor >= charCount)
                    {
                        this->cursor -= charCount;
                    }
                }
            }
            else
            {
                this->sprites[g_App.defaultDifficulty + 0x83].SetPendingInterrupt(9);
                if (this->state == 13)
                {
                    while (!g_GameManager.HasExtraOpen(this->cursor))
                    {
                        this->cursor++;
                        if (this->cursor >= charCount)
                        {
                            this->cursor -= charCount;
                        }
                    }
                }
            }
            for (i = 0x6f; i <= 0x82; i++)
            {
                this->sprites[i].flags &= ~2;
                this->sprites[i].SetPendingInterrupt(8);
                for (j = 0; j < 3; j++)
                {
                    if (g_CharacterMenuSprites[this->cursor][j] == i)
                    {
                        this->sprites[i].flags |= 2;
                        this->sprites[i].SetPendingInterrupt(9);
                    }
                }
                if (g_CharacterMenuSprites[this->cursor][j] == i)
                {
                    this->sprites[i].flags |= 2;
                    this->sprites[i].SetPendingInterrupt(0x17);
                }
            }
            this->subState = 0;
            this->stateFrames = 0;
        }
        if (this->returningFromPractice != 0)
        {
            if (this->state != 15)
            {
                this->SetState(11);
            }
            else
            {
                this->SetState(16);
            }
            this->cursor = g_GameManager.stage;
            return 1;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        charCount = g_GameManager.AllShotsHaveExtraOpen() ? 12 : 4;
        if (this->cursor >= charCount)
        {
            this->cursor = 0;
        }
        dir = this->MoveMenuCursorLR(charCount);
        if (dir != 0)
        {
            int i;
            DWORD j;

            if (this->state == 15)
            {
                while (!g_GameManager.Func43c23b(this->cursor))
                {
                    this->cursor += dir;
                    if (this->cursor >= charCount)
                    {
                        this->cursor -= charCount;
                    }
                    if (this->cursor < 0)
                    {
                        this->cursor += charCount;
                    }
                }
            }
            else if (this->state == 13)
            {
                while (!g_GameManager.HasExtraOpen(this->cursor))
                {
                    this->cursor += dir;
                    if (this->cursor >= charCount)
                    {
                        this->cursor -= charCount;
                    }
                    if (this->cursor < 0)
                    {
                        this->cursor += charCount;
                    }
                }
            }
            for (i = 0x6f; i <= 0x82; i++)
            {
                this->sprites[i].flags |= 2;
                this->sprites[i].SetPendingInterrupt(8);
                for (j = 0; j < 3; j++)
                {
                    if (g_CharacterMenuSprites[this->cursor][j] == i)
                    {
                        this->sprites[i].SetPendingInterrupt(9);
                    }
                }
                if (g_CharacterMenuSprites[this->cursor][j] == i)
                {
                    this->sprites[i].SetPendingInterrupt(0x17);
                }
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            g_GameManager.character = this->cursor;
            g_GameManager.shotType = 0;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            if (this->state == 9)
            {
                this->cursor = g_GameManager.stage;
                this->SetState(11);
                return 2;
            }
            if (this->state == 15)
            {
                this->cursor = g_GameManager.stage;
                this->SetState(16);
                return 2;
            }
            g_GameManager.difficulty = g_App.defaultDifficulty;
            if (this->state != 13)
            {
                g_GameManager.stage = 0;
            }
            else
            {
                g_GameManager.stage = g_GameManager.difficulty + 4;
            }
            g_App.scene = 2;
            g_GameManager.SetIsReplay(0);
            g_App.StopMusic();
            return 0;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
            g_GameManager.character = this->cursor;
            if (this->state == 15)
            {
                g_SoundPlayer.QueueSound(11, 0);
                g_SoundPlayer.PlayQueuedSounds();
                this->subState = 3;
                this->stateFrames = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
                break;
            }
            else
            {
                if (this->state != 13)
                {
                    if (!(g_GameManager.unk_3dbac & 1))
                    {
                        this->SetState(4);
                    }
                    else
                    {
                        this->SetState(8);
                    }
                }
                else
                {
                    this->SetState(12);
                }
                this->cursor = 0;
                return 1;
            }
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            int state = this->state;

            this->SetState(0);
            this->cursor = 2;
            g_GameManager.unk_3dbac &= ~1;
            return 1;
        }
        break;
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateGameStartMenu()
{
    int i;
    DWORD k;
    int move;
    WORD available;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x12);
            this->returningFromPractice = 0;
            for (i = 0x6f; i <= 0x82; i++)
            {
                this->sprites[i].flags &= ~2;
                this->sprites[i].SetPendingInterrupt(8);
                for (k = 1; k < 3; k++)
                {
                    if (g_CharacterMenuSprites[g_GameManager.character][k] == i)
                    {
                        this->sprites[i].flags |= 2;
                        this->sprites[i].SetPendingInterrupt(9);
                    }
                }
                if (g_CharacterMenuSprites[g_GameManager.character][k] == i)
                {
                    this->sprites[i].flags |= 2;
                    this->sprites[i].SetPendingInterrupt(0x17);
                }
            }
            this->subState = 0;
            this->stateFrames = 0;
            g_GameManager.unk_3dbac |= 1;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        available = g_GameManager.clrds[g_GameManager.character].stagesCleared[g_App.defaultDifficulty];
        if (available == 0)
        {
            available = 1;
        }
        if (available & 0x80)
        {
            available |= 0x18;
        }
        if (!(available & 1 << this->cursor))
        {
            this->cursor = 0;
        }
        move = this->MoveMenuCursor(8);
        if (move != 0)
        {
            while (!(available & 1 << this->cursor))
            {
                this->cursor += move;
                if (this->cursor >= 8)
                {
                    this->cursor = 0;
                }
                if (this->cursor < 0)
                {
                    this->cursor = 7;
                }
            }
        }
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            g_GameManager.difficulty = g_App.defaultDifficulty;
            g_GameManager.stage = this->cursor;
            g_App.scene = 2;
            g_GameManager.SetIsReplay(0);
            g_App.StopMusic();
            return 0;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            this->cursor = g_GameManager.character;
            this->SetState(9);
            return 2;
        }
        break;
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::ShowButtonDigits(Sprite *vm, short value)
{
    if (value < 0)
    {
        vm[0].flags &= ~2;
        vm[1].flags &= ~2;
    }
    else
    {
        this->titleAnm->SetSprite(&vm[0], vm[0].baseSpriteIndex + value / 10 * 2);
        this->titleAnm->SetSprite(&vm[1], vm[1].baseSpriteIndex + value % 10 * 2);
        vm[0].flags |= 2;
        vm[1].flags |= 2;
    }
    return 0;
}

char *FormatFullWidthNumber(int value, int digits)
{
    int i;
    int divisor = 1;

    memset(g_FullWidthNumber, 0, sizeof(g_FullWidthNumber));
    for (i = 1; i < digits; i++)
    {
        divisor *= 10;
    }
    i = 0;
    while (divisor > 0)
    {
        strcpy(&g_FullWidthNumber[i], g_FullWidthDigits[value / divisor]);
        value %= divisor;
        divisor /= 10;
        i += 2;
    }
    return g_FullWidthNumber;
}

int GameManager::IsSpellCardSeen(int card)
{
    return (card < 0xcd && (this->catks[card].attempts[12] != 0 || this->catks[card].practiceAttempts[12] != 0)) ||
                   this->flsp.lastSpellsSeen[card - 0xcd] == card
               ? TRUE
               : FALSE;
}

void DrawFan(D3DXVECTOR3 *center, D3DCOLOR color, float arc, float diameter)
{
    VertexDiffuseXyzrwh vertices[0x40];
    Sprite sprite;
    D3DXVECTOR3 offset;

    sprite.blendMode = 0;
    sprite.color = 0xffffffff;
    sprite.zWriteDisabled = 1;
    sprite.cameraSetup = 0;
    vertices[0].diffuse = color;
    vertices[0].pos = *center;
    vertices[0].w = 1.0f;
    // A block: its locals sit below the vertices.
    {
        int i;
        float angle;

        angle = -1.5707964f;
        offset.x = diameter / 2.0f;
        offset.y = 0.0f;
        for (i = 1; i < 0x40; i++)
        {
            Rotate2D(&vertices[i].pos, &offset, angle);
            vertices[i].pos.x = vertices[0].pos.x + vertices[i].pos.x;
            vertices[i].pos.y = vertices[0].pos.y + vertices[i].pos.y;
            vertices[i].pos.z = vertices[0].pos.z;
            vertices[i].diffuse = color;
            vertices[i].w = 1.0f;
            angle = AddNormalizeAngle(angle, 0.1013417f * arc);
        }
    }
    g_SpriteEngine->DrawColoredFan(&sprite, vertices, 0x40);
}

int MainMenu::UpdateReplayMenu()
{
    int i;
    HANDLE findHandle;
    int loaded;
    int dataSize;
    ReplayHeader *data;
    char path[0x40];
    WIN32_FIND_DATAA fd;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState != 7 && g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 14);
            this->cursor = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            loaded = 0;
            for (i = 0; i < 15; i++)
            {
                sprintf(path, "./replay/th8_%.2d.rpy", i + 1);
                data = (ReplayHeader *)LoadFileData(path, &dataSize, 1);
                if (data == NULL)
                {
                    continue;
                }
                data = ValidateReplay(data, dataSize);
                if (data != NULL)
                {
                    this->replays[loaded] = *data;
                    strcpy(this->replayPaths[loaded], path);
                    sprintf(this->replayLabels[loaded], "No.%.2d", i + 1);
                    loaded++;
                    free(data);
                }
            }
            _mkdir("./replay");
            _chdir("./replay");
            findHandle = FindFirstFileA("th8_ud????.rpy", &fd);
            if (findHandle != INVALID_HANDLE_VALUE)
            {
                for (i = 0; i < 45; i++)
                {
                    data = (ReplayHeader *)LoadFileData(fd.cFileName, &dataSize, 1);
                    if (data == NULL)
                    {
                        continue;
                    }
                    data = ValidateReplay(data, dataSize);
                    if (data != NULL)
                    {
                        this->replays[loaded] = *data;
                        sprintf(this->replayPaths[loaded], "./replay/%s", fd.cFileName);
                        sprintf(this->replayLabels[loaded], "User ");
                        free(data);
                        loaded++;
                    }
                    if (!FindNextFileA(findHandle, &fd))
                    {
                        break;
                    }
                }
            }
            FindClose(findHandle);
            _chdir("../");
            this->replayCount = loaded;
            this->clearedInReplayMenu = 0;
        }
        if (this->inputLockout >= 8)
        {
            this->subState = 1;
            this->stateFrames = 0;
        }
        break;
    case 1:
        this->MoveMenuCursor(this->replayCount);
        if (this->replayCount > 15)
        {
            if (WAS_PRESSED_OR_HELD(0x40))
            {
                this->cursor -= 15;
                if (this->cursor < 0)
                {
                    this->cursor += this->replayCount;
                }
                g_SoundPlayer.QueueSound(12, 0);
            }
            if (WAS_PRESSED_OR_HELD(0x80))
            {
                this->cursor += 15;
                if (this->cursor >= this->replayCount)
                {
                    this->cursor -= this->replayCount;
                }
                g_SoundPlayer.QueueSound(12, 0);
            }
        }
        this->replayCursor = this->cursor;
        if (this->stateFrames < 10)
        {
            break;
        }
        if (WAS_PRESSED(0x1001))
        {
            int replaySize;

            if (this->replayCount == 0)
            {
                break;
            }
            g_SoundPlayer.QueueSound(10, 0);
            this->subState = 2;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 15);
            this->sprites[this->replayCursor % 15 + 0x50].SetPendingInterrupt(17);
            this->loadedReplay = (ReplayHeader *)LoadFileData(this->replayPaths[this->replayCursor], &replaySize, 1);
            this->loadedReplay = ValidateReplay(this->loadedReplay, replaySize);
            for (i = 0; i < 9; i++)
            {
                if (this->loadedReplay->stages[i] != NULL)
                {
                    this->loadedReplay->stages[i] =
                        (ReplayStage *)((int)this->loadedReplay + (int)this->loadedReplay->stages[i]);
                }
            }
            this->cursor = 0;
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor > 8)
                {
                    LogError(&g_Log, "リプレイデータが異常\r\n");
                    g_App.scene = -1;
                    return 0;
                }
            }
            this->resultTextAnm->InitSprite(&this->unkSprites[0], i + 2);
            this->unkSprites[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            this->unkSprites[0].flags |= 0x1800;
            this->unkSprites[0].fontWidth = 15;
            this->unkSprites[0].fontHeight = 15;
            g_SpriteEngine->RenderText(&this->unkSprites[0], 0xffffff, 0,
                                       this->replays[this->replayCursor].spellCardName);
            this->unkSprites[0].alpha = 0xff;
            this->unkSprites[0].colorRgb[2] = 0xff;
            this->unkSprites[0].colorRgb[1] = 0xff;
            this->unkSprites[0].colorRgb[0] = 0xff;
            break;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            this->subState = 4;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
        }
        break;
    case 2:
        i = this->MoveMenuCursor(9);
        if (i < 0)
        {
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor--;
                if (this->cursor < 0)
                {
                    this->cursor = 9;
                }
            }
        }
        else if (i > 0)
        {
            while (this->replays[this->replayCursor].stages[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor >= ARRAY_SIZE(this->replays[this->replayCursor].stages))
                {
                    this->cursor = 0;
                }
            }
        }
        this->replayStage = this->cursor;
        if (WAS_PRESSED(0x1001))
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 19);
            this->sprites[this->replayCursor % 15 + 0x50].SetPendingInterrupt(17);
            this->subState = 3;
            this->cursor = 0;
            this->sprites[0x6c].pendingInterrupt = 21;
            this->sprites[0x6d].pendingInterrupt = 21;
            if (this->loadedReplay->spellCardNumber < 0)
            {
                this->sprites[0x6e].pendingInterrupt = 21;
            }
            else
            {
                this->sprites[0x6e].alpha = 0;
            }
            this->sprites[this->cursor + 0x6c].pendingInterrupt = 20;
            break;
        }
        if (WAS_PRESSED(0xa))
        {
            MemFree(this->loadedReplay);
            this->loadedReplay = NULL;
            this->subState = 1;
            this->inputLockout = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 14);
            this->cursor = this->replayCursor;
            break;
        }
        break;
    case 3:
        i = this->MoveMenuCursor((this->loadedReplay->spellCardNumber < 0) + 2);
        if (i != 0)
        {
            this->sprites[0x6c].pendingInterrupt = 21;
            this->sprites[0x6d].pendingInterrupt = 21;
            if (this->loadedReplay->spellCardNumber < 0)
            {
                this->sprites[0x6e].pendingInterrupt = 21;
            }
            else
            {
                this->sprites[0x6e].alpha = 0;
            }
            this->sprites[this->cursor + 0x6c].pendingInterrupt = 20;
        }
        if (WAS_PRESSED(0x1001))
        {
            g_GameManager.SetIsReplay(1);
            strcpy(g_GameManager.replayFile, this->replayPaths[this->replayCursor]);
            g_GameManager.difficulty = this->loadedReplay->difficulty;
            g_GameManager.character = this->loadedReplay->shotType;
            g_GameManager.character = this->loadedReplay->shotType;
            g_GameManager.isInSpellCardPractice = this->loadedReplay->spellCardNumber >= 0;
            g_GameManager.unk_3dbb0 = this->loadedReplay->spellCardNumber;
            MemFree(this->loadedReplay);
            this->loadedReplay = NULL;
            g_GameManager.stage = this->replayStage;
            g_App.scene = 2;
            g_GameManager.replayMenuChoice = this->cursor;
            g_App.StopMusic();
            return 0;
        }
        if (WAS_PRESSED(0xa))
        {
            this->subState = 2;
            this->inputLockout = 0;
            this->cursor = this->replayStage;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 15);
            this->sprites[this->replayCursor % 15 + 0x50].SetPendingInterrupt(17);
            break;
        }
        break;
    case 4:
        if (this->stateFrames >= 30)
        {
            this->SetState(0);
            this->cursor = 4;
            return 1;
        }
        break;
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::DrawReplayMenu()
{
    Sprite *text;
    int top;
    int entry;
    D3DXVECTOR3 pos;

    text = &this->sprites[0x4f];
    g_AsciiText.AddFormat(&text->pos, "No.   Name       Date  Player   Rank");
    entry = this->replayCursor - this->replayCursor % 15;
    for (top = entry + 15; entry < top; entry++)
    {
        if (entry >= this->replayCount)
        {
            break;
        }
        text++;
        g_AsciiText.SetHighlight(entry == this->replayCursor);
        if (entry == this->replayCursor)
        {
            g_AsciiText.SetColor(0xffffffff);
        }
        else
        {
            g_AsciiText.SetColor(0xff808080);
        }
        if (this->replays[entry].spellCardNumber < 0)
        {
            g_AsciiText.AddFormat(&text->pos, "%s %8s  %6s %7s  %8s", this->replayLabels[entry], this->replays[entry].name,
                                  this->replays[entry].date, g_MenuShotTypeNames[this->replays[entry].shotType],
                                  g_MenuDifficultyNames[this->replays[entry].difficulty]);
        }
        else
        {
            g_AsciiText.AddFormat(&text->pos, "%s %8s  %6s %7s  Spell No.%3d", this->replayLabels[entry],
                                  this->replays[entry].name, this->replays[entry].date,
                                  g_MenuShotTypeNames[this->replays[entry].shotType],
                                  this->replays[entry].spellCardNumber + 1);
        }
    }
    if ((this->subState == 2 || this->subState == 3) && this->loadedReplay != NULL)
    {
        g_AsciiText.SetColor(0xffffffff);
        g_AsciiText.SetHighlight(0);
        text = &this->sprites[0x4e];
        g_AsciiText.AddFormat(&text->pos, "       %2.3f%%", this->loadedReplay->slowdown);
        text = &this->sprites[0x5f];
        pos = text->pos;
        g_AsciiText.AddFormat(&pos, "Stage    LastScore");
        entry = this->replayCursor - this->replayCursor % 15;
        if (this->loadedReplay->spellCardNumber < 0)
        {
            for (top = 0; top < ARRAY_SIZE(this->loadedReplay->stages); top++, entry++)
            {
                text++;
                if (this->subState != 3)
                {
                    g_AsciiText.SetHighlight(top == this->replayStage);
                    if (top == this->replayStage)
                    {
                        g_AsciiText.SetColor(0xffffffff);
                    }
                    else
                    {
                        g_AsciiText.SetColor(0xff808080);
                    }
                }
                else
                {
                    if (top == this->replayStage)
                    {
                        g_AsciiText.SetColor(0x60ffffff);
                    }
                    else
                    {
                        g_AsciiText.SetColor(0x60808080);
                    }
                }
                pos = text->pos;
                if (this->loadedReplay->stages[top] != NULL)
                {
                    g_AsciiText.AddFormat(&pos, "%s %9d0", g_PracticeStageNames[top],
                                          this->loadedReplay->stages[top]->score);
                }
                else
                {
                    g_AsciiText.AddFormat(&pos, "%s ----------", g_PracticeStageNames[top]);
                }
            }
        }
        else
        {
            text++;
            text++;
            pos = text->pos;
            g_AsciiText.AddFormat(&pos, "No.%.3d %9d0", this->loadedReplay->spellCardNumber + 1,
                                  this->loadedReplay->score);
            this->unkSprites[0].pos = text->pos;
            this->unkSprites[0].pos.y += 16.0f;
            g_SpriteEngine->DrawUnrotated(&this->unkSprites[0]);
        }
    }
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    return 1;
}

int MainMenu::DrawPracticeScores()
{
    Sprite *text;
    int i;
    WORD cleared;
    D3DXVECTOR3 pos;

    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    text = &this->sprites[0x8d];
    pos = text->pos;
    pos.y -= 96.0f;
    g_AsciiText.AddFormat(&pos, "Stage    HI-Score");
    pos.y += 16.0f;
    cleared = g_GameManager.clrds[g_GameManager.character].stagesCleared[g_App.defaultDifficulty];
    if (cleared == 0)
    {
        cleared = 1;
    }
    if (cleared & 0x80)
    {
        cleared |= 0x18;
    }
    for (i = 0; i < ARRAY_SIZE(g_PracticeStageNames); i++)
    {
        g_AsciiText.SetHighlight(i == this->cursor);
        if (i == this->cursor)
        {
            g_AsciiText.SetColor(0xffffffff);
        }
        else if (cleared & 1 << i)
        {
            g_AsciiText.SetColor(0xffa0a0a0);
        }
        else
        {
            g_AsciiText.SetColor(0xff404040);
        }
        g_AsciiText.AddFormat(&pos, "%s %9d0 (%3d)", g_PracticeStageNames[i],
                              g_GameManager.pscrs[g_GameManager.character].scores[i][g_App.defaultDifficulty],
                              g_GameManager.pscrs[g_GameManager.character].playCount[i][g_App.defaultDifficulty]);
        pos.y += 16.0f;
    }
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    return 1;
}

int MainMenu::MoveMenuCursor(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        this->cursor--;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        this->cursor++;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MoveMenuCursorLR(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::DrawCharacterMenu()
{
    BOOL shown = FALSE;

    if (this->inputLockout > 8)
    {
        if (g_GameManager.HasClearedStage(7, this->cursor, g_App.defaultDifficulty) &&
            g_GameManager.HasClearedStageWithContinues(6, this->cursor, g_App.defaultDifficulty))
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->unkSprites[0], 0x92);
        }
        else if (g_GameManager.HasClearedStage(7, this->cursor, g_App.defaultDifficulty))
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->unkSprites[0], 0x94);
        }
        else if (g_GameManager.HasClearedStage(7, this->cursor, 0) || g_GameManager.HasClearedStage(7, this->cursor, 1) ||
                 g_GameManager.HasClearedStage(7, this->cursor, 2) || g_GameManager.HasClearedStage(7, this->cursor, 3) ||
                 this->cursor > 3)
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->unkSprites[0], 0x93);
        }
        else if (g_GameManager.HasClearedStageWithContinues(6, this->cursor, 0) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 1) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 2) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 3))
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->unkSprites[0], 0x91);
        }
    }
    if (shown)
    {
        this->unkSprites[0].flags |= 0x1800;
        this->unkSprites[0].alpha = 0xff;
        this->unkSprites[0].colorRgb[2] = 0xff;
        this->unkSprites[0].colorRgb[1] = 0xff;
        this->unkSprites[0].colorRgb[0] = 0xff;
        this->unkSprites[0].pos.x = 400.0f;
        this->unkSprites[0].pos.y = 170.0f;
        this->unkSprites[0].pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&this->unkSprites[0]);
    }
    return 0;
}

int MainMenu::OnDraw(MainMenu *menu)
{
    int i;
    Sprite *vm;

    if (menu->loadingState != 0)
    {
        return 1;
    }
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    vm = menu->sprites;
    for (i = 0; i < menu->spriteCount; i++, vm++)
    {
        if (vm->HasTexture())
        {
            D3DXVECTOR3 savedPos = vm->pos;
            vm->pos += vm->posOffset;
            if (vm->rotation.z != 0.0f)
            {
                g_SpriteEngine->Draw(vm);
            }
            else
            {
                g_SpriteEngine->DrawUnrotated(vm);
            }
            vm->pos = savedPos;
        }
    }
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->DrawUnrotated(menu->helpSprite);
    }
    switch (menu->state)
    {
    case 5:
        menu->DrawCharacterMenu();
        break;
    case 7:
        menu->DrawReplayMenu();
        break;
    case 11:
        menu->DrawPracticeScores();
        break;
    case 16:
        menu->DrawState16();
        break;
    case 17:
        menu->DrawState17();
        break;
    }
    return 1;
}

int MainMenu::InitTitleScene()
{
    ScoreDat *dat;

    g_Unknown18b8a2c = 0;
    g_AsciiText.Initialize();
    g_AsciiText.StartScripts();
    if (g_GameManager.unk_4 != NULL)
    {
        delete g_GameManager.unk_4;
        g_GameManager.unk_4 = NULL;
    }
    g_GameManager.unk_4 = new GameManagerUnk4;
    if (g_GameManager.stats != NULL)
    {
        delete g_GameManager.stats;
        g_GameManager.stats = NULL;
    }
    g_GameManager.stats = new GameManagerStats;
    g_App.gameSpeed = 1.0f;
    if (g_GameManager.unk_3dbac >> 3 & 1)
    {
        g_GameManager.character = g_GameManager.character = 0;
    }
    if (g_GameManager.unk_3dbac >> 1 & 1)
    {
        g_GameManager.unk_3dbac &= ~8;
    }
    if (g_GameManager.AnyShotHasExtraOpen() && !g_GameManager.anyShotHasExtraOpen)
    {
        ShowInfoScreen("title/info00.jpg");
    }
    if (g_GameManager.Func43c2c5() && !g_GameManager.unk_3dbac_16)
    {
        ShowInfoScreen("title/info02.jpg");
    }
    if (g_GameManager.AllShotsHaveExtraOpen() && !g_GameManager.allShotsHaveExtraOpen)
    {
        ShowInfoScreen("title/info01.jpg");
    }
    dat = LoadScoreDat("score.dat");
    LoadClears(dat, g_GameManager.clrds);
    LoadPracticeScores(dat, g_GameManager.pscrs);
    LoadSpellCards(dat, g_GameManager.catks);
    LoadFlspChunk(dat, &g_GameManager.flsp);
    ReleaseScoreDat(dat);
    g_GameManager.anyShotHasExtraOpen = g_GameManager.AnyShotHasExtraOpen();
    g_GameManager.unk_3dbac_16 = g_GameManager.Func43c2c5();
    g_GameManager.allShotsHaveExtraOpen = g_GameManager.AllShotsHaveExtraOpen();
    this->Func46cbbb();
    this->state = 0;
    g_App.ResetClockDriftCounters();
    switch (g_App.prevScene)
    {
    case 2:
    case 3:
    case 6:
        this->cursor = g_GameManager.difficulty >= 4;
        break;
    case 5:
        this->cursor = 5;
        break;
    case 8:
        this->cursor = 6;
        break;
    default:
        this->cursor = 0;
        break;
    }
    this->returningFromPractice = 0;
    if (g_GameManager.unk_3dbac & 1)
    {
        this->cursor = (g_GameManager.unk_3dbac >> 14 & 1) ? 2 : 3;
        this->returningFromPractice = (g_GameManager.unk_3dbac >> 14 & 1) ? 2 : 1;
    }
    g_GameManager.unk_3dbac &= ~1;
    g_GameManager.unk_3dbac &= ~0x4000;
    D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
    if (g_App.prevScene == 2)
    {
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else if (g_App.prevScene != 0)
    {
        g_App.StartLoadingSprites(&pos);
    }
    g_GameManager.unk_3dbac &= ~2;
    g_GameManager.demoFrames = 0;
    this->loadingState = 1;
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)LoadingThread, NULL);
    return 0;
}

void App::ResetClockDriftCounters()
{
    this->fastClockStreak = 0;
    this->maxFastClockStreak = 0;
    this->clockMeasurementPending = 0;
    this->fastClockCycleCount = 0;
    this->fastClockCycles = 0;
}

void MainMenu::LoadingThread(MainMenu *arg)
{
    int i;

    while (g_SpriteEngine->pendingCopyTexture >= 0)
    {
        Sleep(1);
    }
    g_MainMenu->titleAnm = g_SpriteEngine->PreloadAnm(0x14, "title01.anm");
    if (g_MainMenu->titleAnm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    g_MainMenu->resultTextAnm = g_SpriteEngine->PreloadAnm(0x16, "resulttext.anm");
    if (g_MainMenu->resultTextAnm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    for (i = 0; i < 0xe; i++)
    {
        g_App.textAnm->StartScript(&g_MainMenu->helpSprites[i], 9);
        g_App.textAnm->SetSprite(&g_MainMenu->helpSprites[i], g_MainMenu->helpSprites[i].spriteIndex + i);
        g_App.textAnm->SetSprite(&g_MainMenu->unkSprites2[i], i + 0x15);
        if (i <= 4)
        {
            g_MainMenu->unkSprites2[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f, 0.0f);
        }
        else
        {
            g_MainMenu->unkSprites2[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f + 10.0f, 0.0f);
        }
        g_MainMenu->unkSprites2[i].flags |= 0x1800;
        g_MainMenu->unkSprites2[i].fontWidth = 15;
        g_MainMenu->unkSprites2[i].fontHeight = 15;
        g_MainMenu->unkSprites2[i].flags |= 2;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    if (g_SpriteEngine->PreloadSurface(0, "title/title00.png") != 0)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (!(g_GameManager.unk_3dbac >> 1 & 1))
    {
        if (g_App.prevScene != 5)
        {
            g_App.LoadMidi(8, "bgm/th08_01.mid");
        }
        if (g_App.lastPlayTimeTick == 0)
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x15);
        }
        else
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x15);
        }
    }
    g_MainMenu->helpSprite = g_MainMenu->helpSprites;
    g_MainMenu->loadingState = 0;
    g_App.StopLoadingSprites();
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
}

void MainMenu::ShowInfoScreen(char *path)
{
    int i;

    i = 0;
    g_SpriteEngine->LoadSurface(0, path);
    while (i < 6000)
    {
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearSpriteData();
        g_SpriteEngine->ClearTexture();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearZWrite();
        g_SpriteEngine->ClearUnk8();
        g_SpriteEngine->ClearUnk2e4d4();
        g_SpriteEngine->ResetUnk0();
        g_App.d3dDevice->BeginScene();
        g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
        if (i < 20)
        {
            float rect[4];
            ColorBytes fade;

            rect[0] = 0.0f;
            rect[1] = 0.0f;
            rect[2] = 639.0f;
            rect[3] = 479.0f;
            fade.a = (60 - i) * 255 / 20;
            fade.r = fade.g = fade.b = 0;
            DrawColorRect(rect, fade.color);
        }
        else if (i > 5940)
        {
            ColorBytes color;
            float area[4];

            area[0] = 0.0f;
            area[1] = 0.0f;
            area[2] = 639.0f;
            area[3] = 479.0f;
            color.a = (i - 5980) * 255 / 20;
            color.r = color.g = color.b = 0;
            DrawColorRect(area, color.color);
        }
        g_Buttons = ReadButtons();
        g_App.d3dDevice->EndScene();
        if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
        {
            g_App.d3dDevice->Reset(&g_App.presentParams);
        }
        if (i >= 180 && i < 5980 && WAS_PRESSED(0x1003))
        {
            i = 5980;
            g_SoundPlayer.QueueSound(10, 0);
        }
        i++;
        g_SoundPlayer.PlayQueuedSounds();
    }
    g_SpriteEngine->ReleaseSurface(0);
}

int MainMenu::ReleaseTitleScene()
{
    if (this->loadedReplay != NULL)
    {
        MemFree(this->loadedReplay);
        this->loadedReplay = NULL;
    }
    if (this->sprites != NULL)
    {
        delete[] this->sprites;
        this->sprites = NULL;
        this->sprites = NULL;
    }
    return 0;
}

int MainMenu::AddedCallback(MainMenu *menu)
{
    return menu->InitTitleScene();
}

int MainMenu::DeletedCallback(MainMenu *menu)
{
    g_App.d3dDevice->ResourceManagerDiscardBytes(0);
    g_SpriteEngine->ReleaseAnm(0x14);
    g_SpriteEngine->ReleaseAnm(0x16);
    g_SpriteEngine->ReleaseSurface(0);
    g_Chain.Remove(menu->drawChain);
    menu->drawChain = NULL;
    menu->ReleaseTitleScene();
    delete menu;
    menu = NULL;
    return 0;
}
