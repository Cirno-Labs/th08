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
#include "chain.h"
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

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext)
{
    HWND hDlg = (HWND)pContext;
    DIPROPRANGE diprg;

    if (pdidoi->dwType & DIDFT_AXIS)
    {
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        if (FAILED(g_App.joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
        {
            return DIENUM_STOP;
        }
    }
    return DIENUM_CONTINUE;
}

int App::RegisterChain()
{
    App *game = &g_App;
    ChainElem *elem;
    int ret;

    game->curScene = 0;
    game->scene = -1;
    game->frames = 0;
    elem = g_Chain.CreateElem((ChainCallback)App::OnUpdate);
    elem->arg = game;
    elem->addedCallback = (ChainCallback)App::AddedCallback;
    elem->deletedCallback = (ChainCallback)App::DeletedCallback;
    ret = g_Chain.AddToCalcChain(elem, 0);
    if (ret != 0)
    {
        return ret;
    }
    elem = g_Chain.CreateElem((ChainCallback)App::OnDraw);
    elem->arg = game;
    g_Chain.AddToDrawChain(elem, 0x10);
    elem = g_Chain.CreateElem((ChainCallback)App::OnDrawLogo);
    elem->arg = game;
    g_Chain.AddToDrawChain(elem, 0);
    elem = g_Chain.CreateElem((ChainCallback)App::OnDrawLoading);
    elem->arg = game;
    g_Chain.AddToDrawChain(elem, 2);
    return 0;
}

int OpenGameArchive()
{
    char path[0x80];
    int size;

    if (g_Archive.Open("th08.dat"))
    {
        sprintf(path, "th08_%.4x%c.ver", 0x100, 'd');
        g_App.verFile = LoadFileData(path, &size, 0);
        g_App.verFileSize = size;
        if (g_App.verFile == NULL)
        {
            LogError(&g_Log, "error : データのバージョンが違います\r\n");
            return -1;
        }
    }
    else
    {
        LogError(&g_Log, "error : データファイルが存在しません\r\n");
        return -1;
    }
    return 0;
}

int MeasureRefreshRate()
{
    int count;
    int before;
    int ticks;
    float values[30];
    int validCount;
    float fpsRate;
    int interval;
    int nowTime;
    float seconds;
    float sum;
    int k;

    count = 0;
    ticks = 0;
    validCount = 0;
    before = 0;
    timeBeginPeriod(1);
    before = timeGetTime();
    timeEndPeriod(1);
    while (count < 1800 && validCount < 8)
    {
        g_App.d3dDevice->BeginScene();
        g_SpriteEngine->CopySurfaceToBackBuffer(8, 0, 0, 0, 0);
        g_App.d3dDevice->EndScene();
        if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
        {
            g_App.d3dDevice->Reset(&g_App.presentParams);
        }
        count++;
        timeBeginPeriod(1);
        nowTime = timeGetTime();
        timeEndPeriod(1);
        ticks++;
        interval = nowTime - before;
        if (interval >= 700)
        {
            before = nowTime;
            ticks = 0;
        }
        else if (interval >= 500)
        {
            seconds = interval / 1000.0f;
            fpsRate = ticks * 1000.0f / interval;
            if (fpsRate >= 57.0f)
            {
                values[validCount] = fpsRate;
                validCount++;
            }
            before = nowTime;
            ticks = 0;
        }
    }
    if (!g_App.flags.unk14)
    {
        sum = 0.0f;
        if (validCount >= 2)
        {
            for (k = 0; k < validCount; k++)
            {
                sum += values[k];
            }
            sum /= k;
        }
        else
        {
            sum = 1000.0f;
        }
        if (sum > 160.0f)
        {
            LogPrintf(&g_Log, "垂直同期が取れてないか、リフレッシュレートが高すぎます\r\n");
            LogPrintf(&g_Log, "強制６０フレームモードで動作します\r\n");
            g_App.presentImmediate = 1;
            return -2;
        }
        else if (sum >= 65.0f)
        {
            LogPrintf(&g_Log, "垂直同期が取れてないか、リフレッシュレートが高すぎます。\r\n");
            LogPrintf(&g_Log, "強制６０フレームモードで動作します\r\n");
            g_App.presentImmediate = 1;
            return -2;
        }
    }
    return 0;
}
