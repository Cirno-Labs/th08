// cflags: /Os
// The music screen's object, from 0x448a65 right after App's: built with
// /Os like it, but without /Ob1, so inlines such as
// Sprite::SetPendingInterrupt are called.

#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "musicscreen.h"
#include "unsorted.h"

int MusicScreen::UpdateIntro()
{
    int i;

    if (this->stateFrames == 0)
    {
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].SetPendingInterrupt(1);
            }
            else
            {
                this->titles[i].SetPendingInterrupt(2);
            }
        }
        for (i = 0; i < 7; i++)
        {
            this->comment[i].SetPendingInterrupt(1);
        }
    }
    if (this->stateFrames >= 8)
    {
        this->state = 1;
    }
    return 0;
}

int MusicScreen::UpdateList()
{
    int prevTop = this->listTop;
    int i;
    char buf[0x42];
    char playingStr[0x42];

    if (WAS_PRESSED_OR_HELD(0x10))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor = this->tuneCount - 1;
            this->listTop = this->tuneCount - 10;
            if (this->listTop < 0)
            {
                this->listTop = 0;
            }
        }
        else if (this->listTop > this->cursor)
        {
            this->listTop = this->cursor;
        }
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].SetPendingInterrupt(1);
            }
            else
            {
                this->titles[i].SetPendingInterrupt(2);
            }
        }
        for (i = 0; i < 7; i++)
        {
            this->comment[i].SetPendingInterrupt(2);
        }
        this->stateFrames = 0;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        this->cursor++;
        if (this->cursor >= this->tuneCount)
        {
            this->cursor = 0;
            this->listTop = 0;
        }
        else if (this->listTop <= this->cursor - 10)
        {
            this->listTop = this->cursor - 9;
        }
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].SetPendingInterrupt(1);
            }
            else
            {
                this->titles[i].SetPendingInterrupt(2);
            }
        }
        for (i = 0; i < 7; i++)
        {
            this->comment[i].SetPendingInterrupt(2);
        }
        this->stateFrames = 0;
    }
    // The comment shows a line every 2 frames from frame 10.
    if (this->stateFrames < 0x1e)
    {
        i = -1;
        if (this->stateFrames == 10)
        {
            i = 0;
        }
        else if (this->stateFrames == 12)
        {
            i = 1;
        }
        else if (this->stateFrames == 14)
        {
            i = 2;
        }
        else if (this->stateFrames == 16)
        {
            i = 3;
        }
        else if (this->stateFrames == 18)
        {
            i = 4;
        }
        else if (this->stateFrames == 20)
        {
            i = 5;
        }
        else if (this->stateFrames == 22)
        {
            i = 6;
        }
        if (i >= 0)
        {
            g_App.textAnm->StartScript(&this->comment[i], i + 10);
            this->comment[i].SetPendingInterrupt(1);
            memset(buf, 0, sizeof(buf));
            if (this->playing == this->cursor || this->bgmHeard[this->cursor] != 0)
            {
                memcpy(buf, this->tunes[this->cursor].comment[i], 0x40);
            }
            else
            {
                memcpy(buf, g_MusicNotHeardComment[i], 0x40);
            }
            if (buf[0] != '\0')
            {
                this->comment[i].flags |= 2;
                g_SpriteEngine->RenderText(&this->comment[i], 0xffe0c0, 0x300000, buf);
            }
            else
            {
                this->comment[i].flags &= ~2;
            }
        }
    }
    if (WAS_PRESSED(0x1001))
    {
        this->playing = this->cursor;
        if (g_App.GetPreloadBgm())
        {
            g_SoundPlayer.LoadBgm("thbgm.dat");
        }
        g_App.PlayMusic(this->tunes[this->playing].path, 0);
        this->stateFrames = 0;
        g_App.textAnm->StartScript(&this->comment[7], 0x11);
        this->comment[7].SetPendingInterrupt(1);
        memset(playingStr, 0, sizeof(playingStr));
        memcpy(playingStr, this->tunes[this->cursor].comment[0], 0x40);
        g_SpriteEngine->RenderText(&this->comment[7], 0xffe0c0, 0x300000, playingStr);
    }
    if (WAS_PRESSED(0xa))
    {
        g_App.scene = 1;
        g_App.StartLoadingSpritesOverScreen(&D3DXVECTOR3(500.0f, 440.0f, 0.0f));
        return 1;
    }
    if (WAS_PRESSED(0x100))
    {
        g_App.FadeOutMusic(8.0f);
    }
    // Restarts the tune playing.
    if (WAS_PRESSED(0x4000))
    {
        if (g_App.GetPreloadBgm())
        {
            g_SoundPlayer.LoadBgm("thbgm.dat");
        }
        g_App.PlayMusic(this->tunes[this->playing].path, 0);
    }
    this->stateFrames++;
    return 0;
}

int MusicScreen::RegisterChain()
{
    static MusicScreen g_MusicScreen;
    MusicScreen *screen = &g_MusicScreen;

    memset(screen, 0, sizeof(MusicScreen));
    screen->calcChain = g_Chain.CreateElem((ChainCallback)MusicScreen::OnUpdate);
    screen->calcChain->arg = screen;
    screen->calcChain->addedCallback = (ChainCallback)MusicScreen::AddedCallback;
    screen->calcChain->deletedCallback = (ChainCallback)MusicScreen::DeletedCallback;
    if (g_Chain.AddToCalcChain(screen->calcChain, 4) != 0)
    {
        return -1;
    }
    screen->drawChain = g_Chain.CreateElem((ChainCallback)MusicScreen::OnDraw);
    screen->drawChain->arg = screen;
    g_Chain.AddToDrawChain(screen->drawChain, 3);
    return 0;
}

int MusicScreen::OnUpdate(MusicScreen *screen)
{
    int oldState = screen->state;
    int i;

again:
    switch (screen->state)
    {
    case 0:
        if (screen->UpdateIntro() == 0)
        {
            break;
        }
        // The state changed: run the new one this frame.
        goto again;
    case 1:
        if (screen->UpdateList() != 0)
        {
            return 0;
        }
        break;
    }
    if (oldState != screen->state)
    {
        screen->stateFrames = 0;
    }
    else
    {
        screen->stateFrames++;
    }
    g_SpriteEngine->Step(&screen->bgSprite[0]);
    for (i = 0; i < 0x1f; i++)
    {
        g_SpriteEngine->Step(&screen->titles[i]);
    }
    for (i = 0; i < 8; i++)
    {
        g_SpriteEngine->Step(&screen->comment[i]);
    }
    return 1;
}
