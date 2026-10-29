// cflags: /Os -/Oi
// The result screen's object, from about 0x453cd1: built with /Os -/Oi as
// th07's resultscreen.cpp, but without /Ob1 (the ResultScreen and ListNode
// constructors are called). The score file's loading is an /Od object of
// its own after it (scoredat.cpp).

#include <string.h>

#include "resultscreen.h"
#include "unsorted.h"

int ResultScreen::RegisterChain(int mode)
{
    ResultScreen *screen = (ResultScreen *)g_MemoryManager.Register(new ResultScreen, sizeof(ResultScreen),
                                                                    "ResultSysInf");

    g_Unknown18b8a2c = 0;
    DebugPrint2("Stg.PlayTimeAll = %d\r\n", g_GameManager.progressFrames);
    if (mode == 1)
    {
        if (!g_GameManager.IsInPractice())
        {
            screen->state = 9;
        }
        else if (g_GameManager.unk_3dbac >> 14 & 1)
        {
            screen->state = 0x16;
        }
        else
        {
            screen->state = 0x11;
        }
    }
    else if (mode == 2)
    {
        screen->state = 0x12;
        ResultScreen::AddedCallback(screen);
        return 0;
    }
    screen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    screen->calcChain->addedCallback = (ChainCallback)ResultScreen::AddedCallback;
    screen->calcChain->deletedCallback = (ChainCallback)ResultScreen::DeletedCallback;
    screen->calcChain->arg = screen;
    if (g_Chain.AddToCalcChain(screen->calcChain, 0x10) != 0)
    {
        return -1;
    }
    screen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    screen->drawChain->arg = screen;
    g_Chain.AddToDrawChain(screen->drawChain, 0x12);
    return 0;
}

int ResultScreen::DeletedCallback(ResultScreen *screen)
{
    int i;
    int j;

    if (screen->scoreDat != NULL)
    {
        SaveScoreDat(screen);
        ReleaseScoreDat(screen->scoreDat);
    }
    screen->scoreDat = NULL;
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 0xc; j++)
        {
            screen->ReleaseScores(i, j);
        }
    }
    g_SpriteEngine->ReleaseAnm(0x15);
    g_SpriteEngine->ReleaseAnm(0x16);
    g_SpriteEngine->MoveSurface(8, 0);
    g_Chain.Remove(screen->drawChain);
    screen->drawChain = NULL;
    g_MemoryManager.Unregister(screen);
    delete screen;
    screen = NULL;
    return 0;
}

int MoveCursorVertical(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int LoadScreenPlstChunk(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->spellCursor--;
        if (screen->spellCursor < 0)
        {
            screen->spellCursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->spellCursor++;
        if (screen->spellCursor >= count)
        {
            screen->spellCursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int MoveCursorHorizontal(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}
