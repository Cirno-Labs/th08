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
#include "mainmenu.h"
#include "midi.h"
#include "replay.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

App::App()
{
    memset(this, 0, sizeof(App));
    this->stateFlags |= 0x40;
    this->stateFlags |= 0x100;
}

int App::OnDraw(App *arg)
{
    DrawFps(1);
    return 1;
}

int App::OnDrawLogo(App *arg)
{
    D3DXVECTOR3 pos;

    if (arg->loadingState >= 2 && ++arg->loadingState >= 5)
    {
        pos.x = 288.0f;
        pos.y = 454.0f;
        pos.z = 0.0f;
        g_AsciiText.scaleX = 0.5f;
        g_AsciiText.scaleY = 0.5f;
        if (arg->loadingState < 35)
        {
            g_AsciiText.SetAlpha(255 - (arg->loadingState - 5) * 128 / 30);
        }
        else
        {
            g_AsciiText.SetAlpha(255 - (65 - arg->loadingState) * 128 / 30);
        }
        g_AsciiText.AddFormat(&pos, "Press Shot Button");
        g_AsciiText.scaleX = 1.0f;
        g_AsciiText.scaleY = 1.0f;
        g_AsciiText.DrawStrings();
        g_AsciiText.stringCount = 0;
        if (arg->loadingState >= 65)
        {
            arg->loadingState = 5;
        }
    }
    if (arg->loadingState != 0)
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

                    g_App.expectedFrames += target;
                    if (target * 0.9f < fps)
                    {
                        g_App.achievedFrames += target;
                    }
                    else if (target * 0.7f < fps)
                    {
                        g_App.achievedFrames += target * 0.8f;
                    }
                    else if (target * 0.5f < fps)
                    {
                        g_App.achievedFrames += target * 0.6f;
                    }
                    else
                    {
                        g_App.achievedFrames += target * 0.5f;
                    }
                    if (!(g_GameManager.unk_3dbac >> 3 & 1))
                    {
                        g_App.roundedFps = fps + 0.5f;
                    }
                    else
                    {
                        sprintf(speedText, "%2d", g_App.roundedFps);
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
            if (g_App.fpsWarning != 0)
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
    if (g_App.stateFlags >> 5 & 1)
    {
        this->current++;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.gameSpeed > 0.99f)
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
        this->subFrame += g_App.gameSpeed * frames;
        while (this->subFrame >= 1.0f)
        {
            this->current++;
            this->subFrame -= 1.0f;
        }
    }
}

void Timer::Sub(int frames)
{
    if (g_App.stateFlags >> 5 & 1)
    {
        this->current--;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    if (g_App.gameSpeed > 0.99f)
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
        this->subFrame -= g_App.gameSpeed * frames;
        while (this->subFrame < 0.0f)
        {
            this->current--;
            this->subFrame += 1.0f;
        }
    }
}

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->gameSpeed <= 0.99f)
    {
        *subFrame += this->gameSpeed;
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

int App::SaveScreenshot(char *path)
{
    BITMAPFILEHEADER fileHdr;
    BYTE *bits;
    BITMAPINFO *bmi;
    IDirect3DSurface8 *buffer;

    bmi = NULL;
    bits = NULL;
    buffer = NULL;
    DebugPrint2("SnapShot! %s\n", path);
    this->d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &buffer);
    memset(&fileHdr, 0, sizeof(fileHdr));
    fileHdr.bfType = *(WORD *)"BM";
    fileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHdr.bfSize = fileHdr.bfOffBits;
    switch (this->presentParams.BackBufferFormat)
    {
    case D3DFMT_R5G6B5:
        DebugPrint2("16bit は取り込めない\r\n");
        LogPrintf(&g_Log, "16bit は取り込めない\r\n");
        break;
    case D3DFMT_X8R8G8B8:
    {
        int widthBytes;
        int x;
        int y;
        BYTE *dst;
        BYTE *read;
        D3DLOCKED_RECT lockRect;
        int outRow;
        HANDLE file;
        DWORD written;
        int size;

        size = sizeof(BITMAPINFO);
        bmi = (BITMAPINFO *)malloc(size);
        if (bmi == NULL)
        {
            LogPrintf(&g_Log, "snapShotScreen : 確保しくり\r\n");
            break;
        }
        memset(bmi, 0, sizeof(BITMAPINFO));
        widthBytes = 640 * 3;
        bits = (BYTE *)malloc(widthBytes * 480);
        if (bits == NULL)
        {
            LogPrintf(&g_Log, "snapShotScreen : 確保しくり\r\n");
            break;
        }
        fileHdr.bfSize += widthBytes * 480;
        bmi->bmiHeader.biBitCount = 24;
        bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi->bmiHeader.biWidth = 640;
        bmi->bmiHeader.biHeight = 480;
        bmi->bmiHeader.biPlanes = 1;
        bmi->bmiHeader.biCompression = BI_RGB;
        buffer->LockRect(&lockRect, NULL, 0);
        // Bottom-up, as a bitmap's rows go.
        outRow = 0;
        for (y = 479; y > -1; y--, outRow++)
        {
            dst = bits + widthBytes * outRow;
            read = (BYTE *)lockRect.pBits + lockRect.Pitch * y;
            for (x = 0; x < 640; x++)
            {
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read += 2;
                dst++;
            }
        }
        buffer->UnlockRect();
        file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            break;
        }
        WriteFile(file, &fileHdr, sizeof(BITMAPFILEHEADER), &written, NULL);
        WriteFile(file, bmi, sizeof(BITMAPINFOHEADER), &written, NULL);
        WriteFile(file, bits, widthBytes * 480, &written, NULL);
        CloseHandle(file);
        break;
    }
    default:
        LogPrintf(&g_Log, "error ? mother.cpp\r\n");
        return 1;
    }
    if (buffer != NULL)
    {
        buffer->Release();
        buffer = NULL;
    }
    free(bmi);
    free(bits);
    return 0;
}

void SoundPlayer::UpdateBgmFades()
{
    if (this->bgm != NULL)
    {
        this->bgm->UpdateFade();
        this->bgm->UpdateFadeIn();
        this->bgm->UpdateFadeDown();
        this->bgm->UpdateFadeUp();
    }
}

// The start of thbgm.dat.
struct BgmDatHeader
{
    // "ZWAV".
    DWORD magic;
    // 1.
    int version;
    // 0x800 (0x700 in th07).
    int unk_8;
    int unk_c;
};

int App::LoadConfig(char *path)
{
    ConfigCopy *file;
    int configFileSize;

    memset(&g_App.controllerMapping, 0, sizeof(ConfigCopy));
    file = (ConfigCopy *)LoadFileData(path, &configFileSize, 1);
    if (file == NULL)
    {
        LogPrintf(&g_Log, "コンフィグデータが見つからないので初期化しました\r\n");
    reset:
        g_App.lifeCount = 2;
        g_App.bombCount = 3;
        g_App.colorMode16Bits = 0;
        g_App.version = 0x80001;
        g_App.padThresholdX = 600;
        g_App.padThresholdY = 600;
        // Wave music if thbgm.dat is there, with the right header.
        {
            HANDLE bgmFile;
            DWORD read;
            BgmDatHeader bgmHeader;

            bgmFile = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            if (bgmFile != INVALID_HANDLE_VALUE)
            {
                ReadFile(bgmFile, &bgmHeader, sizeof(BgmDatHeader), &read, NULL);
                CloseHandle(bgmFile);
                if (bgmHeader.magic != 'VAWZ' || bgmHeader.version != 1 || bgmHeader.unk_8 != 0x800)
                {
                    LogError(&g_Log, "BGM データのバージョンが違います\r\n");
                    return -1;
                }
                g_App.musicMode = 1;
            }
            else
            {
                g_App.musicMode = 2;
                DebugPrint2("wave データが無いので、midi にします\r\n");
            }
        }
        g_App.playSounds = 1;
        g_App.defaultDifficulty = 1;
        g_App.windowed = 0;
        g_App.frameSkip = 0;
        g_App.controllerMapping = g_DefaultControllerMapping;
        g_App.effectsQuality = 2;
        g_App.optionsMenuCursor = 0;
        g_App.shootFocuses = 0;
        g_App.bgmVolume = 100;
        g_App.seVolume = 80;
    }
    else
    {
        *(ConfigCopy *)&g_App.controllerMapping = *file;
        free(file);
        {
            HANDLE bgmFile;
            DWORD read;
            BgmDatHeader bgmHeader;

            bgmFile = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            if (bgmFile != INVALID_HANDLE_VALUE)
            {
                ReadFile(bgmFile, &bgmHeader, sizeof(BgmDatHeader), &read, NULL);
                CloseHandle(bgmFile);
                if (bgmHeader.magic != 'VAWZ' || bgmHeader.version != 1 || bgmHeader.unk_8 != 0x800)
                {
                    LogError(&g_Log, "BGM データのバージョンが違います\r\n");
                    return -1;
                }
            }
        }
        if (g_App.lifeCount >= 7 || g_App.bombCount >= 4 || g_App.colorMode16Bits >= 2 || g_App.musicMode >= 3 ||
            g_App.defaultDifficulty >= 6 || g_App.playSounds >= 2 || g_App.windowed >= 2 || g_App.frameSkip >= 3 ||
            g_App.effectsQuality >= 3 || g_App.optionsMenuCursor >= 2 || g_App.shootFocuses >= 2 || g_App.version != 0x80001 ||
            configFileSize != sizeof(ConfigCopy))
        {
            LogPrintf(&g_Log, "コンフィグデータが異常でしたので再初期化しました\r\n");
            memset(&g_App.controllerMapping, 0, sizeof(ConfigCopy));
            goto reset;
        }
        g_DefaultControllerMapping = g_App.controllerMapping;
    }
    g_App.flags.emulateColorAdd = 1;
    if (this->flags.noVertexBuffer)
    {
        LogPrintf(&g_Log, "頂点バッファの使用を抑制します\r\n");
    }
    if (this->flags.noFog)
    {
        LogPrintf(&g_Log, "フォグの使用を抑制します\r\n");
    }
    if (this->flags.force16BitTextures)
    {
        LogPrintf(&g_Log, "16Bit のテクスチャの使用を強制します\r\n");
    }
    if (this->ClearsBackBuffer())
    {
        LogPrintf(&g_Log, "バックバッファの消去を強制します\r\n");
    }
    if (this->flags.noFrameItems)
    {
        LogPrintf(&g_Log, "ゲーム周りのアイテムの描画を抑制します\r\n");
    }
    if (this->flags.noGouraudShading)
    {
        LogPrintf(&g_Log, "グーローシェーディングを抑制します\r\n");
    }
    if (this->flags.noDepthTest)
    {
        LogPrintf(&g_Log, "デプステストを抑制します\r\n");
    }
    // th06's 60 frame mode is gone: the bit is cleared.
    this->presentImmediate = 0;
    this->flags.force60Fps = 0;
    if (this->flags.noColorBlending)
    {
        LogPrintf(&g_Log, "テクスチャの色合成を抑制します\r\n");
    }
    if (this->windowed)
    {
        LogPrintf(&g_Log, "ウィンドウモードで起動します\r\n");
    }
    if (this->flags.referenceRasterizer)
    {
        LogPrintf(&g_Log, "リファレンスラスタライザを強制します\r\n");
    }
    if (this->flags.noDirectInput)
    {
        LogPrintf(&g_Log, "パッド、キーボードの入力に DirectInput を使用しません\r\n");
    }
    if (this->flags.unk12)
    {
        LogPrintf(&g_Log, "画面周りを毎回描画します\r\n");
    }
    if (this->flags.preloadBgm)
    {
        LogPrintf(&g_Log, "ＢＧＭをメモリに読み込みます\r\n");
    }
    if (this->flags.unk14)
    {
        LogPrintf(&g_Log, "垂直同期を取りません\r\n");
        g_App.presentImmediate = 1;
    }
    if (this->flags.unk15)
    {
        LogPrintf(&g_Log, "文字描画の環境を自動検出しません\r\n");
    }
    if (WriteToFile(path, &g_App.controllerMapping, sizeof(ConfigCopy)) != 0)
    {
        LogError(&g_Log, "ファイルが書き出せません %s\r\n", path);
        LogError(&g_Log, "フォルダが書込み禁止属性になっているか、ディスクがいっぱいいっぱいになってませんか？\r\n");
        return -1;
    }
    return 0;
}

int GameManager::AllShotsHaveExtraOpen()
{
    return this->HasExtraOpen(0) && this->HasExtraOpen(1) && this->HasExtraOpen(2) && this->HasExtraOpen(3) ? TRUE
                                                                                                         : FALSE;
}

int App::LoadMidi(int slot, char *path)
{
    char *dot;
    char wavFile[0x100];

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->LoadFile(slot, path);
        }
        return 0;
    }
    if (g_App.musicMode == 1)
    {
        strcpy(wavFile, path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.QueueSoundCommand(1, slot, wavFile);
    }
    return 1;
}

int App::PlayMidi(int slot, int bgm)
{
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            MidiPlayer *midi = g_App.midi;
            midi->Stop();
            midi->ParseFile(slot);
            midi->Play();
        }
        if (!(g_GameManager.unk_3dbac >> 3 & 1) && !(g_GameManager.unk_3dbac >> 1 & 1))
        {
            g_GameManager.plst.bgmHeard[bgm] = 1;
        }
        return 0;
    }
    if (g_App.musicMode == 1)
    {
        if (g_App.flags.preloadBgm)
        {
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        g_SoundPlayer.QueueSoundCommand(2, slot, "dummy");
        if (!(g_GameManager.unk_3dbac >> 3 & 1) && !(g_GameManager.unk_3dbac >> 1 & 1))
        {
            g_GameManager.plst.bgmHeard[bgm] = 1;
        }
    }
    return 0;
}

int App::PlayMusic(char *path, int bgm)
{
    char *dot;
    char wavFile[0x100];

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            MidiPlayer *midi = g_App.midi;
            midi->Stop();
            midi->Load(path);
            midi->Play();
        }
        if (!(g_GameManager.unk_3dbac >> 3 & 1) && !(g_GameManager.unk_3dbac >> 1 & 1))
        {
            g_GameManager.plst.bgmHeard[bgm] = 1;
        }
    }
    else if (g_App.musicMode == 1)
    {
        strcpy(wavFile, path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.QueueSoundCommand(2, -1, wavFile);
        if (!(g_GameManager.unk_3dbac >> 3 & 1) && !(g_GameManager.unk_3dbac >> 1 & 1))
        {
            g_GameManager.plst.bgmHeard[bgm] = 1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::StopMusic()
{
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Stop();
        }
    }
    else if (g_App.musicMode == 1)
    {
        if (g_App.flags.preloadBgm)
        {
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        else
        {
            g_SoundPlayer.QueueSoundCommand(3, 0, "dummy");
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::FadeOutMusic(float seconds)
{
    float duration;

    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->FadeOut(1000.0f * seconds);
        }
    }
    else if (g_App.musicMode == 1)
    {
        // The wave fade counts frames, so it is stretched while the game
        // runs slow.
        if (this->gameSpeed == 0.0f)
        {
            duration = seconds;
        }
        else if (this->gameSpeed > 1.0f)
        {
            duration = seconds;
        }
        else
        {
            duration = seconds / this->gameSpeed;
        }
        g_SoundPlayer.QueueSoundCommand(5, duration, "");
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::IsScoreDisabled()
{
    return g_GameManager.unk_4 != NULL && g_GameManager.unk_4->scoreDisabled != 0;
}

int App::EnableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 1)
    {
        this->fogState = 1;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    return 0;
}

int App::DisableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 0)
    {
        this->fogState = 0;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    return 0;
}

int App::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    g_SpriteEngine->FlushQuadBatch();
    return this->d3dDevice->SetRenderState(state, value);
}

void App::AddPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastPlayTimeTick)
    {
        this->lastPlayTimeTick = now;
    }
    delta = now - this->lastPlayTimeTick;
    g_GameManager.plst.runHours += delta / 3600000;
    delta %= 3600000;
    g_GameManager.plst.runMinutes += delta / 60000;
    delta %= 60000;
    g_GameManager.plst.runSeconds += delta / 1000;
    delta %= 1000;
    g_GameManager.plst.runMilliseconds += delta;
    if (g_GameManager.plst.runMilliseconds >= 1000)
    {
        g_GameManager.plst.runSeconds += g_GameManager.plst.runMilliseconds / 1000;
        g_GameManager.plst.runMilliseconds %= 1000;
    }
    if (g_GameManager.plst.runSeconds >= 60)
    {
        g_GameManager.plst.runMinutes += g_GameManager.plst.runSeconds / 60;
        g_GameManager.plst.runSeconds %= 60;
    }
    if (g_GameManager.plst.runMinutes >= 60)
    {
        g_GameManager.plst.runHours += g_GameManager.plst.runMinutes / 60;
        g_GameManager.plst.runMinutes %= 60;
    }
    this->lastPlayTimeTick = now;
}

void App::AddTotalPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastTotalPlayTimeTick)
    {
        this->lastTotalPlayTimeTick = 0;
    }
    delta = now - this->lastTotalPlayTimeTick;
    g_GameManager.plst.playHours += delta / 3600000;
    delta %= 3600000;
    g_GameManager.plst.playMinutes += delta / 60000;
    delta %= 60000;
    g_GameManager.plst.playSeconds += delta / 1000;
    delta %= 1000;
    g_GameManager.plst.playMilliseconds += delta;
    if (g_GameManager.plst.playMilliseconds >= 1000)
    {
        g_GameManager.plst.playSeconds += g_GameManager.plst.playMilliseconds / 1000;
        g_GameManager.plst.playMilliseconds %= 1000;
    }
    if (g_GameManager.plst.playSeconds >= 60)
    {
        g_GameManager.plst.playMinutes += g_GameManager.plst.playSeconds / 60;
        g_GameManager.plst.playSeconds %= 60;
    }
    if (g_GameManager.plst.playMinutes >= 60)
    {
        g_GameManager.plst.playHours += g_GameManager.plst.playMinutes / 60;
        g_GameManager.plst.playMinutes %= 60;
    }
    this->lastTotalPlayTimeTick = now;
}

inline void FreeData(void *data)
{
    free(data);
}

int App::CheckVersionFile(char *version, DWORD size, DWORD checksum)
{
    DWORD size2;
    char *start;
    DWORD left;
    DWORD checksum2;
    char *p;

    if (this->verFile == NULL)
    {
        return 0;
    }
    p = (char *)this->verFile;
    left = this->verFileSize;
    if (strncmp(version, "debug", 5) == 0)
    {
        return 0;
    }
    if (strcmp("0100d", "debug") == 0)
    {
        return 0;
    }
    while (left > 0)
    {
        if (strncmp(version, p, 5) == 0)
        {
            p += 6;
            sscanf(p, "%d %d", &size2, &checksum2);
            if (size2 == size && checksum2 == checksum)
            {
                return 0;
            }
        }
        start = p;
        p = strchr(p, '\n') + 1;
        left -= p - start;
    }
    return -1;
}

int App::StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg)
{
    this->CloseSubThread();
    DebugPrint2("info : Sub Thread Start Request\n");
    this->subThread = CreateThread(NULL, 0, proc, arg, 0, &this->subThreadId);
    this->subThreadRunning = 1;
    return this->subThread != NULL ? 0 : -1;
}

void App::CloseSubThread()
{
    if (this->subThread != NULL)
    {
        DebugPrint2("info : Sub Thread Close Request\n");
        this->closeSubThread = 1;
        while (WaitForSingleObject(this->subThread, 1000) == WAIT_TIMEOUT)
        {
            Sleep(1);
        }
        CloseHandle(this->subThread);
        this->subThread = NULL;
        this->closeSubThread = 0;
    }
}

void App::StartLoadingSprites(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScript(&g_LoadingSprites[0], 0);
        this->loadingAnm->StartScript(&g_LoadingSprites[1], 1);
        this->loadingAnm->StartScript(&g_LoadingSprites[2], 2);
        this->loadingState = 1;
        g_LoadingSprites[0].pos = *pos;
        g_LoadingSprites[1].pos = *pos;
        g_LoadingSprites[2].pos = *pos;
    }
}

void App::StopLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingSprites[0].pendingInterrupt = 1;
        g_LoadingSprites[1].pendingInterrupt = 1;
        g_LoadingSprites[2].pendingInterrupt = 1;
        this->loadingState = 0;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}

void App::StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScript(&g_LoadingSprites[0], 0);
        this->loadingAnm->StartScript(&g_LoadingSprites[1], 1);
        this->loadingAnm->StartScript(&g_LoadingSprites[2], 2);
        this->loadingState = 1;
        g_LoadingSprites[0].pos = *pos;
        g_LoadingSprites[1].pos = *pos;
        g_LoadingSprites[2].pos = *pos;
    }
    g_SpriteEngine->SetPendingCopy(8, 0, 0, 640, 480, 0, 0, 640, 480);
}

void App::FinishLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingSprites[0].pendingInterrupt = 2;
        g_LoadingSprites[1].pendingInterrupt = 2;
        g_LoadingSprites[2].pendingInterrupt = 2;
        this->loadingState = 2;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}
