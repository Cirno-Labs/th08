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
#include "midi.h"
#include "replay.h"
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

int InitDirectInput(App *game)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongA(game->hwnd, GWL_HINSTANCE);

    if (game->flags.noDirectInput)
    {
        return -1;
    }
    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID **)&game->dinput, NULL)))
    {
        game->dinput = NULL;
        LogPrintf(&g_Log, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->dinput->CreateDevice(GUID_SysKeyboard, &game->keyboard, NULL)))
    {
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->keyboard->SetDataFormat(&c_dfDIKeyboard)))
    {
        if (game->keyboard)
        {
            game->keyboard->Release();
            game->keyboard = NULL;
        }
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput SetDataFormat が使用できません\r\n");
        return -1;
    }
    if (FAILED(game->keyboard->SetCooperativeLevel(game->hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY)))
    {
        if (game->keyboard)
        {
            game->keyboard->Release();
            game->keyboard = NULL;
        }
        if (game->dinput)
        {
            game->dinput->Release();
            game->dinput = NULL;
        }
        LogPrintf(&g_Log, "DirectInput SetCooperativeLevel が使用できません\r\n");
        return -1;
    }
    game->keyboard->Acquire();
    LogPrintf(&g_Log, "DirectInput は正常に初期化されました\r\n");
    game->dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY);
    if (game->joystick)
    {
        game->joystick->SetDataFormat(&c_dfDIJoystick2);
        game->joystick->SetCooperativeLevel(game->hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        g_App.joystickCaps.dwSize = sizeof(DIDEVCAPS);
        game->joystick->GetCapabilities(&g_App.joystickCaps);
        game->joystick->EnumObjects(EnumAxesCallback, NULL, DIDFT_ALL);
        LogPrintf(&g_Log, "有効なパッドを発見しました\r\n");
    }
    return 0;
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    HRESULT hr;

    if (g_App.joystick == NULL)
    {
        hr = g_App.dinput->CreateDevice(pdidInstance->guidInstance, &g_App.joystick, NULL);
        if (FAILED(hr))
        {
            return DIENUM_CONTINUE;
        }
    }
    return DIENUM_STOP;
}

inline void FreeData(void *data);

int App::DeletedCallback(App *arg)
{
    g_App.CloseSubThread();
    if (g_App.verFile != NULL)
    {
        FreeData(g_App.verFile);
        g_App.verFile = NULL;
    }
    g_SpriteEngine->ReleaseVertexBuffer();
    g_SpriteEngine->ReleaseAnm(0);
    g_SpriteEngine->ReleaseAnm(2);
    g_SpriteEngine->ReleaseSurface(8);
    AsciiText::RemoveChain();
    g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
    if (g_App.musicMode == 2 && g_App.midi != NULL)
    {
        g_App.midi->Restart(0x1e);
    }
    SaveReplay(NULL, NULL);
    ReleaseTextSurface();
    if (arg->keyboard)
    {
        arg->keyboard->Unacquire();
    }
    if (arg->keyboard)
    {
        arg->keyboard->Release();
        arg->keyboard = NULL;
    }
    if (arg->joystick)
    {
        arg->joystick->Unacquire();
    }
    if (arg->joystick)
    {
        arg->joystick->Release();
        arg->joystick = NULL;
    }
    if (arg->dinput)
    {
        arg->dinput->Release();
        arg->dinput = NULL;
    }
    if (g_GameManager.stats != NULL)
    {
        delete g_GameManager.stats;
        g_GameManager.stats = NULL;
    }
    if (g_GameManager.unk_4 != NULL)
    {
        delete g_GameManager.unk_4;
        g_GameManager.unk_4 = NULL;
    }
    g_Archive.Release();
    if (g_App.timer != NULL)
    {
        g_App.timer->StopTimer();
        delete g_App.timer;
        g_App.timer = NULL;
    }
    return 0;
}

void SpriteEngine::ReleaseVertexBuffer()
{
    if (this->vertexBuffer != NULL)
    {
        this->vertexBuffer->Release();
        this->vertexBuffer = NULL;
    }
}

void DrawFps(int draw)
{
    static DWORD frames;
    static char fpsText[0x100];
    static LARGE_INTEGER lastCounter;
    static DWORD measures;
    static char speedText[0x100];
    float fps;
    float duration;

    if (g_GameManager.unk_2d == 0)
    {
        frames += 1 + g_App.frameSkip;
        if (g_App.perfFrequency[0] == 0)
        {
            static DWORD lastTime = timeGetTime();
            DWORD now = timeGetTime();

            if (now < lastTime)
            {
                lastTime = now;
                frames = 0;
            }
            if (now - lastTime >= 500)
            {
                duration = (now - lastTime) / 1000.0f;
                lastTime = now;
            measured:
                fps = frames / duration;
                frames = 0;
                sprintf(fpsText, "%.02ffps", fps);
                if ((g_GameManager.unk_3dbac >> 2 & 1) && draw)
                {
                    float target = 60.0f;

                    g_App.unk_194 += target;
                    if (target * 0.9f < fps)
                    {
                        g_App.unk_190 += target;
                    }
                    else if (target * 0.7f < fps)
                    {
                        g_App.unk_190 += target * 0.8f;
                    }
                    else if (target * 0.5f < fps)
                    {
                        g_App.unk_190 += target * 0.6f;
                    }
                    else
                    {
                        g_App.unk_190 += target * 0.5f;
                    }
                    if (!(g_GameManager.unk_3dbac >> 3 & 1))
                    {
                        g_App.unk_198 = fps + 0.5f;
                    }
                    else
                    {
                        sprintf(speedText, "%2d", g_App.unk_198);
                    }
                }
            }
        }
        else
        {
            LARGE_INTEGER counter;

            if (lastCounter.LowPart == 0)
            {
                QueryPerformanceCounter(&lastCounter);
            }
            QueryPerformanceCounter(&counter);
            if (counter.LowPart < lastCounter.LowPart)
            {
                lastCounter = counter;
                frames = 0;
            }
            if (counter.LowPart >= lastCounter.LowPart + (g_App.perfFrequency[0] >> 1))
            {
                duration = (float)(counter.LowPart - lastCounter.LowPart) / (float)g_App.perfFrequency[0];
                lastCounter = counter;
                measures++;
                goto measured;
            }
        }
    }
    if (g_App.hideFps == 0 && draw)
    {
        D3DXVECTOR3 pos;

        pos.x = 512.0f;
        pos.y = 464.0f;
        pos.z = 0.0f;
        g_AsciiText.AddString(&pos, fpsText);
        if ((g_GameManager.unk_3dbac >> 3 & 1) && (g_GameManager.unk_3dbac >> 2 & 1))
        {
            D3DXVECTOR3 speedPos;

            speedPos.x = 384.0f;
            speedPos.y = 448.0f;
            speedPos.z = 0.0f;
            if (g_App.unk_33c != 0)
            {
                g_AsciiText.color = 0xffff4040;
            }
            else
            {
                g_AsciiText.color = 0xffffffd0;
            }
            g_AsciiText.AddString(&speedPos, speedText);
            g_AsciiText.color = 0xffffffff;
        }
    }
}

void Timer::Add(int frames)
{
    if (g_App.unk_1a4 >> 5 & 1)
    {
        this->current++;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.unk_188 > 0.99f)
    {
        this->current += frames;
    }
    else if (frames < 0)
    {
        this->Sub(-frames);
    }
    else
    {
        this->previous = this->current;
        this->subFrame += g_App.unk_188 * frames;
        while (this->subFrame >= 1.0f)
        {
            this->current++;
            this->subFrame -= 1.0f;
        }
    }
}

void Timer::Sub(int frames)
{
    if (g_App.unk_1a4 >> 5 & 1)
    {
        this->current--;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.unk_188 > 0.99f)
    {
        this->current -= frames;
    }
    else if (frames < 0)
    {
        this->Add(-frames);
    }
    else
    {
        this->previous = this->current;
        this->subFrame -= g_App.unk_188 * frames;
        while (this->subFrame < 0.0f)
        {
            this->current--;
            this->subFrame += 1.0f;
        }
    }
}

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->unk_188 <= 0.99f)
    {
        *subFrame += this->unk_188;
        if (*subFrame >= 1.0f)
        {
            (*frames)++;
            *subFrame -= 1.0f;
        }
    }
    else
    {
        (*frames)++;
    }
}

inline void FreeData(void *data)
{
    free(data);
}
