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
