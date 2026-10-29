// cflags: /Os /Ob1
// The file from 0x445453: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th07's
// unsorted437903.cpp. Built with /Os, and with /Ob1 unlike the rest of
// th08: OnDrawLogo expands inlines. Its functions are laid out without
// alignment.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "sprite.h"
#include "unsorted.h"

App::App()
{
    memset(this, 0, sizeof(App));
    this->unk_1a4 |= 0x40;
    this->unk_1a4 |= 0x100;
}

int App::OnDraw(App *arg)
{
    DrawFps(1);
    return 1;
}

int App::OnDrawLogo(App *arg)
{
    D3DXVECTOR3 pos;

    if (arg->logoFrames >= 2 && ++arg->logoFrames >= 5)
    {
        pos.x = 288.0f;
        pos.y = 454.0f;
        pos.z = 0.0f;
        g_AsciiText.scaleX = 0.5f;
        g_AsciiText.scaleY = 0.5f;
        if (arg->logoFrames < 35)
        {
            g_AsciiText.SetAlpha(255 - (arg->logoFrames - 5) * 128 / 30);
        }
        else
        {
            g_AsciiText.SetAlpha(255 - (65 - arg->logoFrames) * 128 / 30);
        }
        g_AsciiText.AddFormat(&pos, "Press Shot Button");
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.DrawStrings();
        g_AsciiText.stringCount = 0;
        if (arg->logoFrames >= 65)
        {
            arg->logoFrames = 5;
        }
    }
    if (arg->logoFrames != 0)
    {
        g_SpriteEngine->CopySurfaceToBackBuffer(8, 0, 0, 0, 0);
    }
    else if (g_SpriteEngine->GetSurface(8) != NULL)
    {
        g_SpriteEngine->ReleaseSurface(8);
    }
    return 1;
}

int App::OnDrawLoading(App *arg)
{
    DWORD i;

    for (i = 0; i < 3; i++)
    {
        g_LoadingSprites[i].pos += g_LoadingSprites[i].posOffset;
        g_SpriteEngine->Draw(&g_LoadingSprites[i]);
        g_LoadingSprites[i].pos -= g_LoadingSprites[i].posOffset;
    }
    if (arg->unk_294 != 0)
    {
        return 1;
    }
    return 1;
}
