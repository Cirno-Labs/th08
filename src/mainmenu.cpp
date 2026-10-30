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

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
char *g_DemoReplayPaths[4] = {
    "demo/demorpy0.rpy",
    "demo/demorpy1.rpy",
    "demo/demorpy2.rpy",
    "demo/demorpy3.rpy",
};
char *g_PracticeStageNames[9] = {
    "Stage1 ",
    "Stage2 ",
    "Stage3 ",
    "Stage4A",
    "Stage4B",
    "Stage5 ",
    "Stage6A",
    "Stage6B",
    "StageEX",
};
char *g_SpellPracticeStageNames[10] = {
    "Stage1   ",
    "Stage2   ",
    "Stage3   ",
    "Stage4A  ",
    "Stage4B  ",
    "Stage5   ",
    "Stage6A  ",
    "Stage6B  ",
    "Extra    ",
    "Last Word",
};
char *g_FullWidthDigits[10] = {
    "０",
    "１",
    "２",
    "３",
    "４",
    "５",
    "６",
    "７",
    "８",
    "９",
};
LastSpellCondition g_LastSpellConditions[18][2] = {
    {"挑戦可能条件：望月（エキストラ）をスペルカード７枚以上取得して", 0, 0, 0, 0, 0, "　　　　　　　クリアする。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：通常モードFinalBを２キャラ以上クリアする。", 0, 0, 0, 0, 0, "　　　　　　　難易度不問。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：通常モードFinalBを３キャラ以上クリアする。", 0, 0, 0, 0, 0, "　　　　　　　難易度不問。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：スペルカードモードでスペルカードを50枚以上取得する。", 0, 0, 0, 0, 0, "　　　　　　　全キャラ合計。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：通常モードFinalBを４キャラ以上クリアする。", 0, 0, 0, 0, 0, "　　　　　　　難易度不問。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：No.%.3dを取得する。", 137, 0, 0, 0, 0, " ", 0, 0, 0, 0, 0},
    {"挑戦可能条件：スペルカードモードでラストスペルを15枚以上取得する。", 0, 0, 0, 0, 0, "　　　　　　　全キャラ合計。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：No.%.3d,%.3dを取得し", 145, 195, 0, 0, 0, "　　　　　　　かつ、No.%.3dを一度でも見る。", 204, 0, 0, 0, 0},
    {"挑戦可能条件：No.%.3d,%.3d,%.3dを一度でも見る。", 208, 209, 210, 0, 0, "　　　　　　　取得の必要はない。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：No.%.3d,%.3d,%.3d,%.3dを一度でも見る。", 205, 206, 207, 211, 0, "　　　　　　　取得の必要はない。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：魔理沙（単独使用）で三日月（ノーマル）の", 0, 0, 0, 0, 0, "　　　　　　　スペルカードを全て取得。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：霊夢（単独使用）で上つ弓張（ハード）以上をクリア。", 0, 0, 0, 0, 0, "　　　　　　　最終面はFinalBのみ。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：スペルカードモードでスペルカードを120枚以上取得する", 0, 0, 0, 0, 0, "　　　　　　　全キャラ合計。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：通常モードFinalBを６キャラ以上クリアする。", 0, 0, 0, 0, 0, "　　　　　　　", 0, 0, 0, 0, 0},
    {"挑戦可能条件：望月（エキストラ）を３キャラ以上クリアする。", 0, 0, 0, 0, 0, " ", 0, 0, 0, 0, 0},
    {"挑戦可能条件：スペルカードモードでラストスペルを30枚以上取得する。", 0, 0, 0, 0, 0, "　　　　　　　全キャラ合計。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：待宵（ルナティック）をクリアする。", 0, 0, 0, 0, 0, "　　　　　　　コンティニュー可、使用キャラ不問。", 0, 0, 0, 0, 0},
    {"挑戦可能条件：これ以外のラストワードを全て一度見る。", 0, 0, 0, 0, 0, "　　　　　　　取得の必要はない。", 0, 0, 0, 0, 0},
};
char *g_SpellPracticeDifficultyNames[6] = {
    "初月（イージー）",
    "三日月（ノーマル）",
    "上つ弓張（ハード）",
    "待宵（ルナティック）",
    "望月（エキストラ）",
    "極意（ラストワード）",
};
char *g_TitleHelpTexts[9] = {
    "ゲームを開始します",
    "エキストラステージを開始します",
    "敵にスペルカード戦をお願いします",
    "ステージを選択し、練習を開始します",
    "リプレイを鑑賞できます",
    "過去のスコアやスペルカードの取得歴を見られます",
    "音楽を聴けます",
    "各種設定できます",
    "いろいろと終了します",
};
char *g_OptionsHelpTexts[10] = {
    "プレイヤーの初期数を変更します。（初期設定　３）",
    "画面の色数を変更します。３２ＢＩＴだと最も綺麗に表示されます。",
    "ＢＧＭの再生方法を変更します。（初期設定　ＷＡＶ）",
    "ＢＧＭの音量を調整します",
    "効果音の音量を調整します",
    "ウィンドウかフルスクリーンか選択します",
    "弾が多い場面でわざと処理落ちさせます(スコア、リプレイ記録不可)",
    "全て初期設定にします",
    "パッド操作のボタン配置を変更します",
    "おいそれと終了します",
};
short g_LastPressedKey = 32;
char *g_KeyConfigHelpTexts[12] = {
    "ショット、決定ボタンを設定します",
    "ボム、キャンセルボタンを設定します",
    "低速移動ボタンを設定します",
    "メッセージスキップボタンを設定します",
    "ポーズボタンを設定します",
    "上移動ボタンを設定します",
    "下移動ボタンを設定します",
    "左移動ボタンを設定します",
    "右移動ボタンを設定します",
    "ショット押しっぱなしで低速移動になるようにします",
    "初期設定に戻します",
    "おおよそ終了します",
};
char *g_MenuShotTypeNames[12] = {
    "Rm & Yk",
    "Ms & Al",
    "Sk & Rr",
    "Ym & Yy",
    "Reimu  ",
    "Yukari ",
    "Marisa ",
    "Alice  ",
    "Sakuya ",
    "Remilia",
    "Youmu  ",
    "Yuyuko ",
};
char *g_MenuDifficultyNames[5] = {
    "Easy    ",
    "Normal  ",
    "Hard    ",
    "Lunatic ",
    "Extra   ",
};
int g_CharacterMenuSprites[12][4] = {
    {119, 111, 112, -1}, {120, 114, 113, -1}, {121, 115, 116, -1},
    {122, 118, 117, -1}, {123, 111, -1, 112}, {124, 112, -1, 111},
    {125, 114, -1, 113}, {126, 113, -1, 114}, {127, 115, -1, 116},
    {128, 116, -1, 115}, {129, 118, -1, 117}, {130, 117, -1, 118},
};
char g_FullWidthNumber[0x40];
MainMenu *g_MainMenu;
#endif // TH08_EXE

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
                    g_GameManager.stateFlags |= 0x4000;
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
            if (!g_GameManager.AnyShotHasClearedFinalA())
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
                while (!g_GameManager.AnyShotHasClearedFinalA() && this->cursor == 2)
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
            if (!g_GameManager.AnyShotHasClearedFinalA())
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
                g_GameManager.stateFlags |= 2;
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
                g_GameManager.stateFlags &= ~0x4000;
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
                g_GameManager.stateFlags &= ~1;
                g_GameManager.stateFlags &= ~0x4000;
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
                g_GameManager.stateFlags |= 1;
                g_GameManager.stateFlags &= ~0x4000;
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
                    g_GameManager.stateFlags &= ~1;
                    g_GameManager.stateFlags &= ~0x4000;
                    this->cursor = 0;
                    this->SetState(12);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    this->helpSprite->SetPendingInterrupt(2);
                    return 1;
                }
            case 2:
                if (g_GameManager.AnyShotHasClearedFinalA())
                {
                    g_GameManager.stateFlags |= 1;
                    g_GameManager.stateFlags |= 0x4000;
                    this->cursor = g_GameManager.character;
                    this->SetState(16);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    this->helpSprite->SetPendingInterrupt(2);
                    return 1;
                }
            case 4:
                g_GameManager.stateFlags &= ~1;
                g_GameManager.stateFlags &= ~0x4000;
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
                if (!(g_GameManager.stateFlags & 1))
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
                if (!(g_GameManager.stateFlags & 1))
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
            g_GameManager.stateFlags &= ~1;
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
                while (!g_GameManager.HasClearedFinalA(this->cursor))
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
                while (!g_GameManager.HasClearedFinalA(this->cursor))
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
                    if (!(g_GameManager.stateFlags & 1))
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
            g_GameManager.stateFlags &= ~1;
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
            g_GameManager.stateFlags |= 1;
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

int MainMenu::UpdateState16()
{
    int charCount;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            int i;
            DWORD j;

            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x12);
            this->sprites[0x8b].SetPendingInterrupt(0x1b);
            this->sprites[0x8a].SetPendingInterrupt(0x1b);
            if (this->prevState != 17 && g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            this->cursor = g_GameManager.character;
            charCount = g_GameManager.AllShotsHaveExtraOpen() ? 12 : 4;
            while (!g_GameManager.HasClearedFinalA(this->cursor))
            {
                this->cursor++;
                if (this->cursor >= charCount)
                {
                    this->cursor -= charCount;
                }
            }
            g_GameManager.character = this->cursor;
            this->shownRates[0] = 0.0f;
            this->shownRates[1] = 0.0f;
            this->shownRates[2] = 0.0f;
            this->shownRates[3] = 0.0f;
            this->cursor = g_GameManager.stage;
            for (i = 0x6f; i <= 0x82; i++)
            {
                this->sprites[i].flags &= ~2;
                this->sprites[i].SetPendingInterrupt(8);
                for (j = 1; j < 3; j++)
                {
                    if (g_CharacterMenuSprites[g_GameManager.character][j] == i)
                    {
                        this->sprites[i].flags |= 2;
                        this->sprites[i].SetPendingInterrupt(9);
                    }
                }
                if (g_CharacterMenuSprites[g_GameManager.character][j] == i)
                {
                    this->sprites[i].flags |= 2;
                    this->sprites[i].SetPendingInterrupt(0x17);
                }
            }
            this->resultTextAnm->InitSprite(&this->textSprites[0], 2);
            this->textSprites[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            this->textSprites[0].flags |= 0x1800;
            this->textSprites[0].fontWidth = 15;
            this->textSprites[0].fontHeight = 15;
            this->textSprites[0].alpha = 0xff;
            this->textSprites[0].colorRgb[2] = 0xff;
            this->textSprites[0].colorRgb[1] = 0xff;
            this->textSprites[0].colorRgb[0] = 0xff;
            g_SpriteEngine->RenderText(&this->textSprites[0], 0xffffff, 0,
                                       "ステージ    取得/挑戦可能/総数（()内は全キャラ合計）");
            this->resultTextAnm->InitSprite(&this->textSprites[1], 3);
            this->textSprites[1].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            this->textSprites[1].flags |= 0x1800;
            this->textSprites[1].fontWidth = 15;
            this->textSprites[1].fontHeight = 15;
            this->textSprites[1].alpha = 0xff;
            this->textSprites[1].colorRgb[2] = 0xff;
            this->textSprites[1].colorRgb[1] = 0xff;
            this->textSprites[1].colorRgb[0] = 0xff;
            g_SpriteEngine->RenderText(&this->textSprites[1], 0xffffff, 0, "取得率　（キャラ別／総合）");
            // The flags and color go to textSprites[1] again (original).
            this->titleAnm->InitSprite(&this->textSprites[2], 0x90);
            this->textSprites[1].flags |= 0x1800;
            this->textSprites[1].alpha = 0xff;
            this->textSprites[1].colorRgb[2] = 0xff;
            this->textSprites[1].colorRgb[1] = 0xff;
            this->textSprites[1].colorRgb[0] = 0xff;
            this->subState = 0;
            this->stateFrames = 0;
            g_GameManager.stateFlags |= 1;
            g_GameManager.stateFlags |= 0x4000;
        }
        if (this->returningFromPractice != 0)
        {
            this->cursor = 0;
            this->SetState(17);
            if (g_GameManager.practiceSpellCard >= 0xcd)
            {
                g_GameManager.stage = 9;
            }
            return 2;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
    {
        int numChars;
        int stageIndex;
        int dir;

        this->MoveMenuCursor(10);
        stageIndex = this->cursor;
        this->cursor = g_GameManager.character;
        numChars = g_GameManager.AllShotsHaveExtraOpen() ? 12 : 4;
        dir = this->MoveMenuCursorLR(numChars);
        if (dir != 0)
        {
            int i;
            DWORD j;

            while (!g_GameManager.HasClearedFinalA(this->cursor))
            {
                this->cursor += dir;
                if (this->cursor >= numChars)
                {
                    this->cursor -= numChars;
                }
                if (this->cursor < 0)
                {
                    this->cursor += numChars;
                }
            }
            g_GameManager.character = this->cursor;
            for (i = 0x6f; i <= 0x82; i++)
            {
                this->sprites[i].flags &= ~2;
                this->sprites[i].SetPendingInterrupt(8);
                for (j = 1; j < 3; j++)
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
            this->inputLockout = 0;
        }
        this->cursor = stageIndex;
        if (WAS_PRESSED(0x1001))
        {
            g_SoundPlayer.QueueSound(10, 0);
            g_GameManager.stage = this->cursor;
            g_SoundPlayer.QueueSound(10, 0);
            g_SoundPlayer.PlayQueuedSounds();
            this->cursor = 0;
            this->SetState(17);
            return 2;
        }
        if (WAS_PRESSED(0xa))
        {
            g_SoundPlayer.QueueSound(11, 0);
            g_SoundPlayer.PlayQueuedSounds();
            this->subState = 3;
            this->stateFrames = 0;
            g_GameManager.stage = this->cursor;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
            break;
        }
        break;
    }
    case 3:
        if (this->stateFrames >= 20)
        {
            int state = this->state;

            this->SetState(0);
            this->cursor = 2;
            g_GameManager.stateFlags &= ~1;
            return 1;
        }
        break;
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState17()
{
    int id;
    int i;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x1a);
            this->returningFromPractice = 0;
            this->subState = 0;
            this->stateFrames = 0;
            g_GameManager.stateFlags |= 1;
            g_GameManager.stateFlags |= 0x4000;
            this->spellCardCount = g_StageSpellCardCounts[g_GameManager.stage];
            this->UnlockLastSpells();
            this->cursor = 0;
            for (i = 0; i < this->spellCardCount; i++)
            {
                if (g_StageSpellCards[g_GameManager.stage][i] == g_GameManager.practiceSpellCard)
                {
                    this->cursor = i;
                    break;
                }
            }
            this->spellPage = this->cursor / 15;
            for (i = 0; i < 15; i++)
            {
                if (i + this->spellPage * 15 >= this->spellCardCount)
                {
                    break;
                }
                id = g_StageSpellCards[g_GameManager.stage][i + this->spellPage * 15];
                this->resultTextAnm->InitSprite(&this->textSprites[i], i + 2);
                this->textSprites[i].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                this->textSprites[i].flags |= 0x1800;
                // textSprites[0]'s font width every time (original).
                this->textSprites[0].fontWidth = 15;
                this->textSprites[i].fontHeight = 15;
                if (g_GameManager.catks[id].attempts[12] == 0 && g_GameManager.catks[id].practiceAttempts[12] == 0)
                {
                    if (GetSpellCardDifficulty(id) <= 4 || !g_GameManager.IsSpellCardSeen(id))
                    {
                        g_SpriteEngine->RenderText(&this->textSprites[i], 0xffffff, 0, "？？？？？？？？");
                    }
                    else
                    {
                        g_SpriteEngine->RenderText(&this->textSprites[i], 0xffffff, 0, "挑戦者歓迎中！");
                    }
                }
                else
                {
                    g_SpriteEngine->RenderText(&this->textSprites[i], 0xffffff, 0, g_GameManager.catks[id].name);
                }
                this->textSprites[i].alpha = 0xff;
                this->textSprites[i].colorRgb[2] = 0x60;
                this->textSprites[i].colorRgb[1] = 0x60;
                this->textSprites[i].colorRgb[0] = 0x60;
            }
            i = 15;
            this->resultTextAnm->InitSprite(&this->textSprites[i], i + 2);
            this->textSprites[i].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            this->textSprites[i].flags |= 0x1800;
            this->textSprites[i].fontWidth = 15;
            this->textSprites[i].fontHeight = 15;
            g_SpriteEngine->RenderText(&this->textSprites[i], 0xffffff, 0, "取得数/挑戦数（()内は本編での集計）");
            this->textSprites[i].alpha = 0xff;
            this->textSprites[i].colorRgb[2] = 0xff;
            this->textSprites[i].colorRgb[1] = 0xff;
            this->textSprites[i].colorRgb[0] = 0xff;
            i = this->cursor - this->spellPage * 15;
            this->textSprites[i].colorRgb[2] = 0xff;
            this->textSprites[i].colorRgb[1] = 0xff;
            this->textSprites[i].colorRgb[0] = 0xff;
            for (i = 0; i < 7; i++)
            {
                g_App.textAnm->InitSprite(&this->cardDetailSprites[i], i + 0x15);
                if (i < 4)
                {
                    this->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16 + 344.0f, 0.0f);
                }
                else
                {
                    this->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16 + 344.0f + 8.0f, 0.0f);
                }
                this->cardDetailSprites[i].flags |= 0x1800;
                this->cardDetailSprites[i].fontWidth = 15;
                this->cardDetailSprites[i].fontHeight = 15;
                this->cardDetailSprites[i].color = 0xffffffff;
            }
            this->UpdateCardDetails();
            this->cardDetailsTimer = 0;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
    {
        int savedCursor;
        int cardId;
        int n;
        int page;

        page = this->spellPage;
        savedCursor = this->cursor;
        if (this->spellCardCount > 15)
        {
            if (WAS_PRESSED_OR_HELD(0x40))
            {
                this->cursor -= 15;
                g_SoundPlayer.QueueSound(12, 0);
                g_SoundPlayer.PlayQueuedSounds();
                if (this->cursor < 0)
                {
                    this->cursor = this->spellCardCount - 1;
                }
                if (this->cursor >= this->spellCardCount)
                {
                    this->cursor = 0;
                }
            }
            if (WAS_PRESSED_OR_HELD(0x80))
            {
                g_SoundPlayer.QueueSound(12, 0);
                if (this->spellCardCount - this->cursor <= this->spellCardCount % 15)
                {
                    this->cursor %= 15;
                }
                else
                {
                    this->cursor += 15;
                    if (this->cursor < 0)
                    {
                        this->cursor = this->spellCardCount - 1;
                    }
                    if (this->cursor >= this->spellCardCount)
                    {
                        this->cursor = this->spellCardCount - 1;
                    }
                }
            }
        }
        this->MoveMenuCursor(this->spellCardCount);
        this->spellPage = this->cursor / 15;
        if (page != this->spellPage)
        {
            for (n = 0; n < 15; n++)
            {
                if (n + this->spellPage * 15 >= this->spellCardCount)
                {
                    break;
                }
                this->resultTextAnm->InitSprite(&this->textSprites[n], n + 2);
                this->textSprites[n].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                this->textSprites[n].flags |= 0x1800;
                // cardDetailSprites[0]'s font width every time (original).
                this->cardDetailSprites[0].fontWidth = 15;
                this->textSprites[n].fontHeight = 15;
                cardId = g_StageSpellCards[g_GameManager.stage][n + this->spellPage * 15];
                if (g_GameManager.catks[cardId].WasAttempted(12))
                {
                    g_SpriteEngine->RenderText(&this->textSprites[n], 0xffffff, 0, g_GameManager.catks[cardId].name);
                }
                else if (GetSpellCardDifficulty(cardId) <= 4 || !g_GameManager.IsSpellCardSeen(cardId))
                {
                    g_SpriteEngine->RenderText(&this->textSprites[n], 0xffffff, 0, "？？？？？？？？");
                }
                else
                {
                    g_SpriteEngine->RenderText(&this->textSprites[n], 0xffffff, 0, "挑戦者歓迎中！");
                }
                this->textSprites[n].alpha = 0xff;
                this->textSprites[n].colorRgb[2] = 0x60;
                this->textSprites[n].colorRgb[1] = 0x60;
                this->textSprites[n].colorRgb[0] = 0x60;
            }
        }
        if (savedCursor != this->cursor)
        {
            for (n = 0; n < 15; n++)
            {
                this->textSprites[n].colorRgb[2] = 0x60;
                this->textSprites[n].colorRgb[1] = 0x60;
                this->textSprites[n].colorRgb[0] = 0x60;
            }
            n = this->cursor - this->spellPage * 15;
            this->textSprites[n].colorRgb[2] = 0xff;
            this->textSprites[n].colorRgb[1] = 0xff;
            this->textSprites[n].colorRgb[0] = 0xff;
            for (n = 0; n < 7; n++)
            {
                this->cardDetailSprites[n].alpha = 0;
            }
            this->cardDetailsTimer = 0x15;
        }
        this->UpdateCardDetails();
        if (WAS_PRESSED(0x1001))
        {
            int card;

            card = g_StageSpellCards[g_GameManager.stage][this->cursor];
            if (g_GameManager.catks[card].attempts[12] != 0 || g_GameManager.catks[card].practiceAttempts[12] != 0 ||
                (card >= 0xcd && g_GameManager.IsSpellCardSeen(card)))
            {
                g_SoundPlayer.QueueSound(10, 0);
                g_GameManager.stateFlags |= 0x4000;
                g_GameManager.practiceSpellCard = g_StageSpellCards[g_GameManager.stage][this->cursor];
                g_GameManager.difficulty = GetSpellCardDifficulty(g_GameManager.practiceSpellCard);
                if (g_GameManager.difficulty > 4)
                {
                    // Last spells play on their stage's (or a later) card
                    // list, as Normal.
                    switch (g_GameManager.practiceSpellCard)
                    {
                    case 0xcd:
                        g_GameManager.stage = 0;
                        break;
                    case 0xce:
                        g_GameManager.stage = 1;
                        break;
                    case 0xcf:
                        g_GameManager.stage = 2;
                        break;
                    case 0xd0:
                        g_GameManager.stage = 5;
                        break;
                    case 0xd1:
                        g_GameManager.stage = 6;
                        break;
                    case 0xd2:
                        g_GameManager.stage = 7;
                        break;
                    case 0xd3:
                        g_GameManager.stage = 8;
                        break;
                    case 0xd4:
                        g_GameManager.stage = 5;
                        break;
                    case 0xd5:
                        g_GameManager.stage = 8;
                        break;
                    case 0xd6:
                        g_GameManager.stage = 3;
                        break;
                    case 0xd7:
                        g_GameManager.stage = 4;
                        break;
                    default:
                        g_GameManager.stage = 3;
                    }
                    g_GameManager.difficulty = 1;
                }
                g_App.scene = 2;
                g_GameManager.SetIsReplay(0);
                g_App.StopMusic();
                return 0;
            }
            g_SoundPlayer.QueueSound(0x29, 0);
        }
        if (WAS_PRESSED(0xa))
        {
            g_GameManager.practiceSpellCard = g_StageSpellCards[g_GameManager.stage][this->cursor];
            g_SoundPlayer.QueueSound(11, 0);
            this->cursor = g_GameManager.stage;
            this->SetState(16);
            return 2;
        }
        break;
    }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

void MainMenu::UnlockLastSpells()
{
    int i;
    int captured;

    captured = 0;
    for (i = 0; i < 0xde; i++)
    {
        if (g_GameManager.catks[i].WasPracticeCaptured(12))
        {
            captured++;
        }
    }
    {
        int count;

        count = 0;
        for (i = 0; i < 12; i++)
        {
            if ((g_GameManager.clrds[i].stagesClearedNoContinue[0] & 0x4000) ||
                (g_GameManager.clrds[i].stagesClearedNoContinue[1] & 0x4000) ||
                (g_GameManager.clrds[i].stagesClearedNoContinue[2] & 0x4000) ||
                (g_GameManager.clrds[i].stagesClearedNoContinue[3] & 0x4000))
            {
                count++;
            }
        }
        if (count >= 2)
        {
            g_GameManager.flsp.Open(0xcd);
        }
    }
    {
        int count;
        int j;

        count = 0;
        for (j = 0; j < 12; j++)
        {
            if ((g_GameManager.clrds[j].stagesClearedNoContinue[0] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[1] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[2] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[3] & 0x4000))
            {
                count++;
            }
        }
        if (count >= 3)
        {
            g_GameManager.flsp.Open(0xce);
        }
    }
    if (captured >= 50)
    {
        g_GameManager.flsp.Open(0xcf);
    }
    {
        int count;

        count = 0;
        for (i = 0; i < g_ListedSpellCardCount; i++)
        {
            if (g_GameManager.catks[g_ListedSpellCards[i]].WasPracticeCaptured(12))
            {
                count++;
            }
        }
        if (count >= 15)
        {
            g_GameManager.flsp.Open(0xd2);
        }
    }
    if (g_GameManager.catks[137].WasCaptured(12))
    {
        g_GameManager.flsp.Open(0xd1);
    }
    {
        int count;
        int j;

        count = 0;
        for (j = 0; j < 12; j++)
        {
            if ((g_GameManager.clrds[j].stagesClearedNoContinue[0] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[1] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[2] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[3] & 0x4000))
            {
                count++;
            }
        }
        if (count >= 4)
        {
            g_GameManager.flsp.Open(0xd0);
        }
    }
    if (g_GameManager.catks[195].WasCaptured(12) && g_GameManager.catks[204].WasAttempted(12) &&
        g_GameManager.catks[145].WasCaptured(12))
    {
        g_GameManager.flsp.Open(0xd3);
    }
    if (g_GameManager.catks[208].WasAttempted(12) && g_GameManager.catks[209].WasAttempted(12) &&
        g_GameManager.catks[210].WasAttempted(12))
    {
        g_GameManager.flsp.Open(0xd4);
    }
    if (g_GameManager.catks[205].WasAttempted(12) && g_GameManager.catks[206].WasAttempted(12) &&
        g_GameManager.catks[207].WasAttempted(12) && g_GameManager.catks[211].WasAttempted(12))
    {
        g_GameManager.flsp.Open(0xd5);
    }
    {
        int count = 0;
        int j;

        count = 0;
        for (j = 0; j < g_SpellCardListSizes[1]; j++)
        {
            if (g_GameManager.catks[g_NormalSpellCards[j]].WasCaptured(6))
            {
                count++;
            }
        }
        if (count == g_SpellCardListSizes[1])
        {
            g_GameManager.flsp.Open(0xd6);
        }
    }
    if ((g_GameManager.clrds[4].stagesClearedNoContinue[2] & 0x4000) ||
        (g_GameManager.clrds[4].stagesClearedNoContinue[3] & 0x4000))
    {
        g_GameManager.flsp.Open(0xd7);
    }
    if (captured >= 120)
    {
        g_GameManager.flsp.Open(0xd8);
    }
    {
        int count;
        int j;

        count = 0;
        for (j = 0; j < 12; j++)
        {
            if ((g_GameManager.clrds[j].stagesClearedNoContinue[0] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[1] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[2] & 0x4000) ||
                (g_GameManager.clrds[j].stagesClearedNoContinue[3] & 0x4000))
            {
                count++;
            }
        }
        if (count >= 6)
        {
            g_GameManager.flsp.Open(0xd9);
        }
    }
    {
        int count;
        int j;

        count = 0;
        for (j = 0; j < 12; j++)
        {
            if (g_GameManager.clrds[j].stagesClearedNoContinue[4] & 0x100)
            {
                count++;
            }
        }
        if (count >= 3)
        {
            g_GameManager.flsp.Open(0xda);
        }
    }
    {
        int count;

        count = 0;
        for (i = 0; i < g_ListedSpellCardCount; i++)
        {
            if (g_GameManager.catks[g_ListedSpellCards[i]].WasPracticeCaptured(12))
            {
                count++;
            }
        }
        if (count >= 30)
        {
            g_GameManager.flsp.Open(0xdb);
        }
    }
    if (g_GameManager.clrds[12].stagesCleared[3] & 0xc000)
    {
        g_GameManager.flsp.Open(0xdc);
    }
    if (g_GameManager.catks[205].WasAttempted(12) && g_GameManager.catks[206].WasAttempted(12) &&
        g_GameManager.catks[207].WasAttempted(12) && g_GameManager.catks[208].WasAttempted(12) &&
        g_GameManager.catks[209].WasAttempted(12) && g_GameManager.catks[210].WasAttempted(12) &&
        g_GameManager.catks[211].WasAttempted(12) && g_GameManager.catks[212].WasAttempted(12) &&
        g_GameManager.catks[213].WasAttempted(12) && g_GameManager.catks[214].WasAttempted(12) &&
        g_GameManager.catks[215].WasAttempted(12) && g_GameManager.catks[216].WasAttempted(12) &&
        g_GameManager.catks[217].WasAttempted(12) && g_GameManager.catks[218].WasAttempted(12) &&
        g_GameManager.catks[219].WasAttempted(12) && g_GameManager.catks[220].WasAttempted(12))
    {
        g_GameManager.flsp.Open(0xdd);
    }
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

void MainMenu::UpdateCardDetails()
{
    int id;
    int i;
    int seen;

    if (this->subState == 1 && this->cardDetailsTimer == 0)
    {
        return;
    }
    id = g_StageSpellCards[g_GameManager.stage][this->cursor];
    seen = g_GameManager.catks[id].practiceAttempts[12] + g_GameManager.catks[id].attempts[12];
    if (this->subState == 0 || this->cardDetailsTimer == 11)
    {
        g_SpriteEngine->RenderText(&this->cardDetailSprites[0], 0xffffff, 0, "Ｎｏ．%s　　%s", FormatFullWidthNumber(id + 1, 3),
                                   seen == 0 ? "？？？？？？？？" : g_GameManager.catks[id].name);
    }
    if (this->subState == 0 || this->cardDetailsTimer == 9)
    {
        g_SpriteEngine->RenderText(&this->cardDetailSprites[1], 0xffffff, 0, "使用者  %s　　%s %s",
                                   seen == 0 ? "？？？？？？？？" : g_GameManager.catks[id].comment,
                                   g_SpellPracticeDifficultyNames[GetSpellCardDifficulty(id)],
                                   IsListedSpellCard(id) ? "Last" : " ");
    }
    if (this->subState == 0)
    {
        g_SpriteEngine->RenderText(&this->cardDetailSprites[2], 0xffffff, 0, "取得数/挑戦数[最高点]　%s　全主人公合計",
                                   GetShotName(g_GameManager.character));
    }
    if (GetSpellCardDifficulty(id) <= 4)
    {
        if (!g_GameManager.catks[id].WasAttempted(12))
        {
            if (this->subState == 0 || this->cardDetailsTimer == 7)
            {
                g_SpriteEngine->RenderText(&this->cardDetailSprites[3], 0xffffff, 0,
                                           "　　---/---(---/---)[--------]　　---/---(---/---)[--------]");
            }
        }
        else if (this->subState == 0 || this->cardDetailsTimer == 7)
        {
            g_SpriteEngine->RenderText(
                &this->cardDetailSprites[3], 0xffffff, 0, "　　%3d/%3d(%3d/%3d)[%.8d]　　%3d/%3d(%3d/%3d)[%.8d]",
                g_GameManager.catks[id].practiceCaptures[g_GameManager.character],
                g_GameManager.catks[id].practiceAttempts[g_GameManager.character],
                g_GameManager.catks[id].captures[g_GameManager.character],
                g_GameManager.catks[id].attempts[g_GameManager.character],
                g_GameManager.catks[id].practiceMaxBonus[g_GameManager.character],
                g_GameManager.catks[id].practiceCaptures[12], g_GameManager.catks[id].practiceAttempts[12],
                g_GameManager.catks[id].captures[12], g_GameManager.catks[id].attempts[12],
                g_GameManager.catks[id].practiceMaxBonus[12]);
        }
    }
    else
    {
        if (!g_GameManager.catks[id].WasAttempted(12))
        {
            if (this->subState == 0 || this->cardDetailsTimer == 7)
            {
                g_SpriteEngine->RenderText(&this->cardDetailSprites[3], 0xffffff, 0,
                                           "　　---/---(---/---)[--------]　　---/---(---/---)[--------]");
            }
        }
        else if (this->subState == 0 || this->cardDetailsTimer == 7)
        {
            g_SpriteEngine->RenderText(&this->cardDetailSprites[3], 0xffffff, 0,
                                       "　　%3d/%3d(---/---)[%.8d]　　%3d/%3d(---/---)[%.8d]",
                                       g_GameManager.catks[id].practiceCaptures[g_GameManager.character],
                                       g_GameManager.catks[id].practiceAttempts[g_GameManager.character],
                                       g_GameManager.catks[id].practiceMaxBonus[g_GameManager.character],
                                       g_GameManager.catks[id].practiceCaptures[12],
                                       g_GameManager.catks[id].practiceAttempts[12],
                                       g_GameManager.catks[id].practiceMaxBonus[12]);
        }
    }
    if (this->subState == 0)
    {
        g_SpriteEngine->RenderText(&this->cardDetailSprites[4], 0xffffff, 0, "カードの補足");
    }
    if (this->subState == 0 || this->cardDetailsTimer == 5)
    {
        // A last spell not yet open shows how to open it.
        if (!g_GameManager.catks[id].WasAttempted(12) && id >= 0xcc && id <= 0xdd && !g_GameManager.IsSpellCardSeen(id))
        {
            g_SpriteEngine->RenderText(&this->cardDetailSprites[5], 0xffffff, 0, g_LastSpellConditions[id - 0xcc][0].format,
                                       g_LastSpellConditions[id - 0xcc][0].cards[0] + 1,
                                       g_LastSpellConditions[id - 0xcc][0].cards[1] + 1,
                                       g_LastSpellConditions[id - 0xcc][0].cards[2] + 1,
                                       g_LastSpellConditions[id - 0xcc][0].cards[3] + 1,
                                       g_LastSpellConditions[id - 0xcc][0].cards[4] + 1);
        }
        else
        {
            char text[0x80];

            memset(text, 0, sizeof(text));
            memcpy(text, g_GameManager.catks[id].note, sizeof(g_GameManager.catks[id].note));
            g_SpriteEngine->RenderText(&this->cardDetailSprites[5], 0xffffff, 0,
                                       g_GameManager.catks[id].practiceCaptures[12] == 0 ? "？？？？？？？？？" : text);
        }
    }
    if (this->subState == 0 || this->cardDetailsTimer == 3)
    {
        if (!g_GameManager.catks[id].WasAttempted(12) && id >= 0xcc && id <= 0xdd && !g_GameManager.IsSpellCardSeen(id))
        {
            g_SpriteEngine->RenderText(&this->cardDetailSprites[6], 0xffffff, 0, g_LastSpellConditions[id - 0xcc][1].format,
                                       g_LastSpellConditions[id - 0xcc][1].cards[0] + 1,
                                       g_LastSpellConditions[id - 0xcc][1].cards[1] + 1,
                                       g_LastSpellConditions[id - 0xcc][1].cards[2] + 1,
                                       g_LastSpellConditions[id - 0xcc][1].cards[3] + 1,
                                       g_LastSpellConditions[id - 0xcc][1].cards[4] + 1);
        }
        else
        {
            char text[0x80];

            memset(text, 0, sizeof(text));
            memcpy(text, g_GameManager.catks[id].note2, sizeof(g_GameManager.catks[id].note2));
            g_SpriteEngine->RenderText(&this->cardDetailSprites[6], 0xffffff, 0,
                                       g_GameManager.catks[id].practiceCaptures[12] == 0 ? "？？？？？？？？？" : text);
        }
    }
    this->cardDetailSprites[5].pos.x = 96.0f;
    this->cardDetailSprites[6].pos.x = 96.0f;
    if (this->subState == 0 || this->cardDetailsTimer == 3)
    {
        for (i = 0; i < 7; i++)
        {
            this->cardDetailSprites[i].alpha = 0xff;
        }
    }
    if (this->cardDetailsTimer != 0)
    {
        this->cardDetailsTimer--;
    }
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
            this->resultTextAnm->InitSprite(&this->textSprites[0], i + 2);
            this->textSprites[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            this->textSprites[0].flags |= 0x1800;
            this->textSprites[0].fontWidth = 15;
            this->textSprites[0].fontHeight = 15;
            g_SpriteEngine->RenderText(&this->textSprites[0], 0xffffff, 0,
                                       this->replays[this->replayCursor].spellCardName);
            this->textSprites[0].alpha = 0xff;
            this->textSprites[0].colorRgb[2] = 0xff;
            this->textSprites[0].colorRgb[1] = 0xff;
            this->textSprites[0].colorRgb[0] = 0xff;
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
            g_GameManager.practiceSpellCard = this->loadedReplay->spellCardNumber;
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
            this->textSprites[0].pos = text->pos;
            this->textSprites[0].pos.y += 16.0f;
            g_SpriteEngine->DrawUnrotated(&this->textSprites[0]);
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
    for (i = 0; i < ARRAY_SIZE(g_PracticeStageNames) - 1; i++)
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

int MainMenu::DrawState16()
{
    D3DXVECTOR3 gaugePos;
    float rate;
    float mainRate;
    int stageGameCapturedAny;
    D3DXVECTOR3 pos;
    int totalGameAll;
    int j;
    int stageSeenEveryone;
    int totalPracticeAny;
    int totalSeenEveryone;
    int totalStory;
    int stageGame;
    int stagePractice;
    int totalPractice;
    int totalSeen;
    int stageKnown;
    DWORD i;
    int id;
    Sprite *title;
    int stagePracticeAll;

    totalPractice = 0;
    totalPracticeAny = 0;
    totalStory = 0;
    totalGameAll = 0;
    totalSeen = 0;
    totalSeenEveryone = 0;
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    title = &this->sprites[0x8d];
    pos = title->pos;
    pos.y -= 152.0f;
    pos.x -= 32.0f;
    this->textSprites[0].pos = pos;
    g_SpriteEngine->DrawUnrotated(&this->textSprites[0]);
    pos.y += 16.0f;
    for (i = 0; i < 10; i++)
    {
        g_AsciiText.SetHighlight(i == this->cursor);
        if (i == this->cursor)
        {
            g_AsciiText.SetColor(0xffffffff);
        }
        else
        {
            g_AsciiText.SetColor(0xffa0a0a0);
        }
        stagePractice = 0;
        stageGame = 0;
        stageGameCapturedAny = 0;
        stageKnown = 0;
        stagePracticeAll = 0;
        stageSeenEveryone = 0;
        for (j = 0; j < g_StageSpellCardCounts[i]; j++)
        {
            id = g_StageSpellCards[i][j];
            if (g_GameManager.catks[id].practiceCaptures[g_GameManager.character] > 0)
            {
                stagePractice++;
            }
            if (g_GameManager.catks[id].captures[g_GameManager.character] > 0)
            {
                stageGame++;
            }
            if (g_GameManager.catks[id].attempts[g_GameManager.character] > 0 ||
                g_GameManager.catks[id].practiceAttempts[g_GameManager.character] > 0 ||
                (i >= 9 && g_GameManager.IsSpellCardSeen(id)))
            {
                stageKnown++;
            }
            if (g_GameManager.catks[id].practiceCaptures[12] > 0)
            {
                stagePracticeAll++;
            }
            if (g_GameManager.catks[id].captures[12] > 0)
            {
                stageGameCapturedAny++;
            }
            if (g_GameManager.catks[id].attempts[12] > 0 || g_GameManager.catks[id].practiceAttempts[12] > 0 ||
                (i >= 9 && g_GameManager.IsSpellCardSeen(id)))
            {
                stageSeenEveryone++;
            }
        }
        totalPractice += stagePractice;
        totalPracticeAny += stagePracticeAll;
        totalStory += stageGame;
        totalGameAll += stageGameCapturedAny;
        totalSeen += stageKnown;
        totalSeenEveryone += stageSeenEveryone;
        g_AsciiText.AddFormat(&pos, "%s%s",
                              stagePractice >= g_StageSpellCardCounts[i]      ? "@"
                              : stagePracticeAll >= g_StageSpellCardCounts[i] ? "*"
                                                                             : " ",
                              g_SpellPracticeStageNames[i]);
        pos.x += 182.0f;
        g_AsciiText.SetScale(0.75f, 1.0f);
        g_AsciiText.AddFormat(&pos, "%3d(%3d)/%3d/%3d", stagePractice, stagePracticeAll, stageSeenEveryone,
                              g_StageSpellCardCounts[i]);
        pos.x -= 182.0f;
        pos.y += 16.0f;
        g_AsciiText.SetScale(1.0f, 1.0f);
    }
    pos.y += 5.0f;
    g_AsciiText.SetHighlight(0);
    g_AsciiText.SetColor(0xffd06060);
    g_AsciiText.AddFormat(&pos, "%sTotal", totalPractice >= 0xde ? "@" : totalPracticeAny >= 0xde ? "*" : " ");
    pos.x += 182.0f;
    g_AsciiText.SetScale(0.75f, 1.0f);
    g_AsciiText.AddFormat(&pos, "%3d(%3d)/%3d/%3d", totalPractice, totalPracticeAny, totalSeenEveryone, 0xde);
    pos.x -= 182.0f;
    g_AsciiText.SetScale(1.0f, 1.0f);
    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    this->textSprites[1].pos.x = 400.0f;
    this->textSprites[1].pos.y = 376.0f;
    this->textSprites[1].pos.z = 0.0f;
    g_SpriteEngine->DrawUnrotated(&this->textSprites[1]);
    this->textSprites[2].pos.x = 552.0f;
    this->textSprites[2].pos.y = 426.0f;
    this->textSprites[2].pos.z = 0.0f;
    g_SpriteEngine->DrawUnrotated(&this->textSprites[2]);
    gaugePos.x = 530.0f;
    gaugePos.y = 420.0f;
    rate = totalPractice / 222.0f;
    mainRate = totalStory / 205.0f;
    // The gauges fill over the first 50 frames.
    if (this->inputLockout < 50)
    {
        if (this->shownRates[0] != rate)
        {
            rate = (rate - this->shownRates[0]) * this->inputLockout / 50.0f + this->shownRates[0];
        }
        if (this->shownRates[1] != mainRate)
        {
            mainRate = (mainRate - this->shownRates[1]) * this->inputLockout / 50.0f + this->shownRates[1];
        }
    }
    this->shownRates[0] = rate;
    this->shownRates[1] = mainRate;
    gaugePos.z = 0.02f;
    DrawFan(&gaugePos, 0x60c0c0f0, rate, 48.0f);
    gaugePos.z = 0.01f;
    DrawFan(&gaugePos, 0x80c0c0f0, mainRate, 24.0f);
    gaugePos.x = 514.0f;
    gaugePos.y = 404.0f;
    g_AsciiText.DrawPercent(&gaugePos, rate * 10000.0f, 0xffffffff);
    gaugePos.y += 8.0f;
    g_AsciiText.DrawPercent(&gaugePos, mainRate * 10000.0f, 0x80c0c080);
    gaugePos.x = 530.0f;
    gaugePos.y = 420.0f;
    gaugePos.x += 32.0f;
    gaugePos.y += 18.0f;
    rate = totalPracticeAny / 222.0f;
    mainRate = totalGameAll / 205.0f;
    if (this->inputLockout < 50)
    {
        if (this->shownRates[2] != rate)
        {
            rate = (rate - this->shownRates[2]) * this->inputLockout / 50.0f + this->shownRates[2];
        }
        if (this->shownRates[3] != mainRate)
        {
            mainRate = (mainRate - this->shownRates[3]) * this->inputLockout / 50.0f + this->shownRates[3];
        }
    }
    this->shownRates[2] = rate;
    this->shownRates[3] = mainRate;
    gaugePos.z = 0.02f;
    DrawFan(&gaugePos, 0x6080c0c0, rate, 32.0f);
    gaugePos.z = 0.01f;
    DrawFan(&gaugePos, 0x8080c0c0, mainRate, 16.0f);
    gaugePos.x = 580.0f;
    gaugePos.y = 451.0f;
    g_AsciiText.DrawPercent(&gaugePos, rate * 10000.0f, 0xffffffff);
    gaugePos.y -= 8.0f;
    g_AsciiText.DrawPercent(&gaugePos, mainRate * 10000.0f, 0x80c0c080);
    return 1;
}

int MainMenu::DrawState17()
{
    int cardId;
    int i;
    WORD cleared;
    D3DXVECTOR3 pos;

    g_AsciiText.SetColor(0xffffffff);
    g_AsciiText.SetHighlight(0);
    pos = D3DXVECTOR3(16.0f, 78.0f, 0.0f);
    cleared = g_GameManager.clrds[g_GameManager.character].stagesCleared[g_App.defaultDifficulty];
    if (cleared == 0)
    {
        cleared = 1;
    }
    pos.x += 318.0f;
    this->textSprites[15].pos = pos;
    g_SpriteEngine->DrawUnrotated(&this->textSprites[15]);
    pos.x -= 318.0f;
    pos.y += 16.0f;
    for (i = 0; i < 15; i++)
    {
        if (i + this->spellPage * 15 >= this->spellCardCount)
        {
            break;
        }
        g_AsciiText.SetColor(this->textSprites[i].color);
        cardId = g_StageSpellCards[g_GameManager.stage][i + this->spellPage * 15];
        g_AsciiText.AddFormat(&pos, "%sNo.%.3d",
                              g_GameManager.catks[cardId].practiceCaptures[g_GameManager.character] > 0 ? "@"
                              : g_GameManager.catks[cardId].practiceCaptures[12] > 0                     ? "*"
                                                                                                       : " ",
                              cardId + 1);
        pos.x += 414.0f;
        g_AsciiText.SetScale(0.8f, 1.0f);
        g_AsciiText.AddFormat(&pos, "%3d/%3d(%3d/%3d)", g_GameManager.catks[cardId].practiceCaptures[g_GameManager.character],
                              g_GameManager.catks[cardId].practiceAttempts[g_GameManager.character],
                              g_GameManager.catks[cardId].captures[g_GameManager.character],
                              g_GameManager.catks[cardId].attempts[g_GameManager.character]);
        g_AsciiText.SetScale(1.0f, 1.0f);
        pos.x -= 414.0f;
        this->textSprites[i].pos = pos;
        this->textSprites[i].pos.x += 102.0f;
        g_SpriteEngine->DrawUnrotated(&this->textSprites[i]);
        pos.y += 16.0f;
    }
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[0]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[1]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[2]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[3]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[4]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[5]);
    g_SpriteEngine->DrawUnrotated(&this->cardDetailSprites[6]);
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
            this->titleAnm->InitSprite(&this->textSprites[0], 0x92);
        }
        else if (g_GameManager.HasClearedStage(7, this->cursor, g_App.defaultDifficulty))
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->textSprites[0], 0x94);
        }
        else if (g_GameManager.HasClearedStage(7, this->cursor, 0) || g_GameManager.HasClearedStage(7, this->cursor, 1) ||
                 g_GameManager.HasClearedStage(7, this->cursor, 2) || g_GameManager.HasClearedStage(7, this->cursor, 3) ||
                 this->cursor > 3)
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->textSprites[0], 0x93);
        }
        else if (g_GameManager.HasClearedStageWithContinues(6, this->cursor, 0) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 1) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 2) ||
                 g_GameManager.HasClearedStageWithContinues(6, this->cursor, 3))
        {
            shown = TRUE;
            this->titleAnm->InitSprite(&this->textSprites[0], 0x91);
        }
    }
    if (shown)
    {
        this->textSprites[0].flags |= 0x1800;
        this->textSprites[0].alpha = 0xff;
        this->textSprites[0].colorRgb[2] = 0xff;
        this->textSprites[0].colorRgb[1] = 0xff;
        this->textSprites[0].colorRgb[0] = 0xff;
        this->textSprites[0].pos.x = 400.0f;
        this->textSprites[0].pos.y = 170.0f;
        this->textSprites[0].pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(&this->textSprites[0]);
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

    g_ScreenEffectHoldFrames = 0;
    g_AsciiText.Initialize();
    g_AsciiText.StartScripts();
    if (g_GameManager.savedConfig != NULL)
    {
        delete g_GameManager.savedConfig;
        g_GameManager.savedConfig = NULL;
    }
    g_GameManager.savedConfig = new GameConfig;
    if (g_GameManager.stats != NULL)
    {
        delete g_GameManager.stats;
        g_GameManager.stats = NULL;
    }
    g_GameManager.stats = new GameManagerStats;
    g_App.gameSpeed = 1.0f;
    if (g_GameManager.stateFlags >> 3 & 1)
    {
        g_GameManager.character = g_GameManager.character = 0;
    }
    if (g_GameManager.stateFlags >> 1 & 1)
    {
        g_GameManager.stateFlags &= ~8;
    }
    if (g_GameManager.AnyShotHasExtraOpen() && !g_GameManager.anyShotHasExtraOpen)
    {
        ShowInfoScreen("title/info00.jpg");
    }
    if (g_GameManager.AnyShotHasClearedFinalA() && !g_GameManager.anyShotHasClearedFinalA)
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
    g_GameManager.anyShotHasClearedFinalA = g_GameManager.AnyShotHasClearedFinalA();
    g_GameManager.allShotsHaveExtraOpen = g_GameManager.AllShotsHaveExtraOpen();
    this->UnlockLastSpells();
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
    if (g_GameManager.stateFlags & 1)
    {
        this->cursor = (g_GameManager.stateFlags >> 14 & 1) ? 2 : 3;
        this->returningFromPractice = (g_GameManager.stateFlags >> 14 & 1) ? 2 : 1;
    }
    g_GameManager.stateFlags &= ~1;
    g_GameManager.stateFlags &= ~0x4000;
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
    g_GameManager.stateFlags &= ~2;
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
        g_App.textAnm->SetSprite(&g_MainMenu->cardDetailSprites[i], i + 0x15);
        if (i <= 4)
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f, 0.0f);
        }
        else
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f + 10.0f, 0.0f);
        }
        g_MainMenu->cardDetailSprites[i].flags |= 0x1800;
        g_MainMenu->cardDetailSprites[i].fontWidth = 15;
        g_MainMenu->cardDetailSprites[i].fontHeight = 15;
        g_MainMenu->cardDetailSprites[i].flags |= 2;
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
    if (!(g_GameManager.stateFlags >> 1 & 1))
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
        g_SpriteEngine->ClearDrawStats();
        g_SpriteEngine->ClearLastCameraSetupFlag();
        g_SpriteEngine->ResetColorModulation();
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

int SetUpTitleScene(int unused)
{
    MainMenu *menu;

    menu = new MainMenu;
    g_MainMenu = menu;

    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.SetIsPaused(0);
    menu->calcChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnUpdate);
    menu->calcChain->arg = menu;
    menu->calcChain->addedCallback = (ChainCallback)MainMenu::AddedCallback;
    menu->calcChain->deletedCallback = (ChainCallback)MainMenu::DeletedCallback;
    if (g_Chain.AddToCalcChain(menu->calcChain, 4) != 0)
    {
        return -1;
    }
    menu->drawChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnDraw);
    menu->drawChain->arg = menu;
    g_Chain.AddToDrawChain(menu->drawChain, 3);
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
