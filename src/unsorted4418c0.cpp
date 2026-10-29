// The file from about 0x4418c0 (WinMain): start-up, the window and the
// display, as in th07's unsorted433e90.cpp.

#include <direct.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>
#define NO_SHDOCVW_GUIDS
#include <shlguid.h>
#include <shobjidl.h>
#include <winnls32.h>

#include "chain.h"
#include "midi.h"
#include "sprite.h"
#include "unsorted.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int runResult = 0;
    MSG message;
    HRESULT testResult;
    int j;

    g_App.hInstance = hInstance;
    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfoA(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfoA(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);
    g_App.InitCriticalSections();
    LogPrintf(&g_Log, "東方動作記録 --------------------------------------------- \r\n");
    if (CheckSingleInstance(hInstance) == -1)
    {
        goto stop;
    }
    if (g_App.LoadConfig("th08.cfg") != 0)
    {
        goto stop;
    }
    ChecksumExecutable();
    QueryPerformanceFrequency((LARGE_INTEGER *)g_GameWindow.perfFrequency);
restart:
    if (CreateD3d() != 0)
    {
        goto stop;
    }
    if (CreateGameWindow(hInstance) != 0)
    {
        goto stop;
    }
    if (InitD3dDevice() != 0)
    {
        goto stop;
    }
    g_SoundPlayer.Initialize(g_GameWindow.hwnd);
    InitJoystick();
    ResetKeyboard();
    g_SpriteEngine =
        (SpriteEngine *)g_MemoryManager.Register(new SpriteEngine(), sizeof(SpriteEngine), "SprtCtrlInf");
    if (!g_App.GetWindowed())
    {
        WINNLSEnableIME(NULL, FALSE);
        ShowCursor(FALSE);
        SetCursor(NULL);
    }
    runResult = App::RegisterChain();
    if (runResult != 0)
    {
        if (runResult == -1)
        {
            goto release;
        }
        runResult = 2;
        goto release;
    }
    runResult = 0;
    g_GameWindow.curFrame = -4;
    g_GameWindow.now = g_GameWindow.lastTime = g_GameWindow.nextFrameTime = 0;
    while (!g_GameWindow.isClosing)
    {
        if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        else
        {
            testResult = g_App.d3dDevice->TestCooperativeLevel();
            if (testResult == D3D_OK)
            {
                runResult = g_GameWindow.RunFrame();
                if (runResult != 0)
                {
                    break;
                }
                g_App.stateFlags &= ~0x10;
            }
            else if (testResult == D3DERR_DEVICENOTRESET)
            {
                g_SpriteEngine->ReleaseSurfaces();
                if (g_App.d3dDevice->Reset(&g_App.presentParams) != 0)
                {
                    break;
                }
                SetDefaultRenderStates();
                g_App.redrawFrames = 3;
                g_App.stateFlags |= 0x10;
            }
        }
    }
release:
    if (g_GameManager.plst.magic != 0)
    {
        ResultScreen::RegisterChain(2);
    }
    g_Chain.Release();
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
stop:
    Sleep(1000);
    g_SoundPlayer.Release();
    g_MemoryManager.Unregister(g_SpriteEngine);
    delete g_SpriteEngine;
    g_SpriteEngine = NULL;
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Release();
        g_App.d3dDevice = NULL;
    }
    if (g_App.d3d != NULL)
    {
        g_App.d3d->Release();
        g_App.d3d = NULL;
    }
    if (g_GameWindow.hwnd != NULL)
    {
        ShowWindow(g_GameWindow.hwnd, SW_HIDE);
        MoveWindow(g_GameWindow.hwnd, 0, 0, 0, 0, FALSE);
        DestroyWindow(g_GameWindow.hwnd);
        g_GameWindow.hwnd = NULL;
    }
    ShowCursor(TRUE);
    if (runResult == 2)
    {
        g_Log.Reset();
        LogPrintf(&g_Log, "再起動を要するオプションが変更されたので再起動します\r\n");
        if (!g_App.GetWindowed())
        {
            WINNLSEnableIME(NULL, TRUE);
        }
        j = 0;
        while (j < 60)
        {
            if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            j++;
        }
        goto restart;
    }
    WriteToFile("th08.cfg", &g_App.controllerMapping, 0x3c);
    if (g_App.midi != NULL)
    {
        g_App.midi->Stop();
        g_MemoryManager.Unregister(g_App.midi);
        delete g_App.midi;
        g_App.midi = NULL;
    }
    g_Log.Flush();
    g_App.DeleteCriticalSections();
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);
    WINNLSEnableIME(NULL, TRUE);
    return 0;
}

int GameWindow::RunFrame()
{
    int res;

    this->now = this->GetTime();
    // The clock went back: start again from now.
    if (this->lastTime > this->now)
    {
        this->nextFrameTime = this->now;
    }
    this->lastTime = this->now;
    if (this->nextFrameTime < this->now)
    {
        while (this->nextFrameTime < this->now)
        {
            this->nextFrameTime += 1.0 / 60.0;
        }
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = 0;
        g_App.viewport.Y = 0;
        g_App.viewport.Width = 640;
        g_App.viewport.Height = 480;
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.PlayQueuedSounds();
        if (res == 0)
        {
            g_App.CloseSubThread();
            return 1;
        }
        if (res == -1)
        {
            g_App.CloseSubThread();
            return 2;
        }
        this->curFrame++;
        if (g_App.frameSkip <= this->curFrame)
        {
            g_App.d3dDevice->BeginScene();
            g_SpriteEngine->BeginQuadBatch();
            g_App.fogState = 0xff;
            g_App.DisableFog();
            g_Chain.RunDrawChain();
            g_SpriteEngine->FlushQuadBatch();
            g_App.d3dDevice->SetTexture(0, NULL);
            g_App.d3dDevice->EndScene();
            this->curFrame = 0;
        }
        this->now = this->GetTime();
        PresentFrame();
    }
    else
    {
        Sleep(0);
    }
    return 0;
}

void PresentFrame()
{
    int i;
    char path[0x100];

    if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_SpriteEngine->ReleaseSurfaces();
        g_App.d3dDevice->Reset(&g_App.presentParams);
        SetDefaultRenderStates();
        g_App.redrawFrames = 2;
    }
    g_SpriteEngine->DoPendingCopy();
    if (WAS_PRESSED(0x800))
    {
        _mkdir("snapshot");
        for (i = 0; i < 1000; i++)
        {
            sprintf(path, "snapshot/th%.3d.bmp", i);
            if (!FileExists(path))
            {
                break;
            }
        }
        if (i < 1000)
        {
            g_App.SaveScreenshot(path);
        }
    }
    if (g_App.redrawFrames != 0 && !g_GameManager.unk_3dbb2)
    {
        g_App.redrawFrames--;
    }
}

void FormatCapsReport(D3DCAPS8 *caps, char *buffer)
{
    char *p = buffer;

    p += sprintf(p, "現在のビデオカード、及びドライバの能力詳細\r\n");
    p = AppendCapsFlagText("　走査線取得能力 : ", caps->Caps, 0x20000, p);
    p = AppendCapsFlagText("　ウィンドウモードのレンダリング : ", caps->Caps2, 0x80000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（直接）: ", caps->PresentationIntervals, 0x80000000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（垂直同期）: ", caps->PresentationIntervals, 0x1, p);
    p += sprintf(p, "　-- デバイス能力 ------------------------------\r\n");
    p = AppendCapsFlagText("　System -> 非ローカルVRAMブリット : ", caps->DevCaps, 0x20000, p);
    p = AppendCapsFlagText("　ハードウェア T&L : ", caps->DevCaps, 0x10000, p);
    p = AppendCapsFlagText("　非ローカルVRAMからテクスチャ取得 : ", caps->DevCaps, 0x1000, p);
    p = AppendCapsFlagText("　システムメモリからテクスチャ取得 : ", caps->DevCaps, 0x100, p);
    p = AppendCapsFlagText("　VRAM からテクスチャ取得 : ", caps->DevCaps, 0x200, p);
    p = AppendCapsFlagText("　頂点バッファにシステムメモリを使用 : ", caps->DevCaps, 0x40, p);
    p = AppendCapsFlagText("　頂点バッファにビデオメモリを使用 : ", caps->DevCaps, 0x80, p);
    p += sprintf(p, "　-- プリミティブ能力 ---------------------------\r\n");
    p = AppendCapsFlagText("　半透明処理 : ", caps->PrimitiveMiscCaps, 0x800, p);
    p = AppendCapsFlagText("　ポイントのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x100, p);
    p = AppendCapsFlagText("　プリミティブのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x200, p);
    p = AppendCapsFlagText("　法線クリップ（反時計周り） : ", caps->PrimitiveMiscCaps, 0x40, p);
    p = AppendCapsFlagText("　法線クリップ（時計周り） : ", caps->PrimitiveMiscCaps, 0x20, p);
    p = AppendCapsFlagText("　法線クリップ無し : ", caps->PrimitiveMiscCaps, 0x10, p);
    p = AppendCapsFlagText("　デプステストON/OFF切り替え : ", caps->PrimitiveMiscCaps, 0x2, p);
    p += sprintf(p, "　-- ラスタ能力 --------------------------------\r\n");
    p = AppendCapsFlagText("　異方性フィルタリング : ", caps->RasterCaps, 0x20000, p);
    p = AppendCapsFlagText("　アンチエイリアシング : ", caps->RasterCaps, 0x1000, p);
    p = AppendCapsFlagText("　ディザ処理 : ", caps->RasterCaps, 0x1, p);
    p = AppendCapsFlagText("　範囲ベースのフォグ : ", caps->RasterCaps, 0x10000, p);
    p = AppendCapsFlagText("　Zベースのフォグ : ", caps->RasterCaps, 0x200000, p);
    p = AppendCapsFlagText("　テーブルフォグ : ", caps->RasterCaps, 0x100, p);
    p = AppendCapsFlagText("　頂点フォグ : ", caps->RasterCaps, 0x80, p);
    p = AppendCapsFlagText("　デプステスト : ", caps->RasterCaps, 0x10, p);
    p += sprintf(p, "　-- シェーディング能力 -----------------------\r\n");
    p = AppendCapsFlagText("　グーローシェーディング : ", caps->ShadeCaps, 0x8, p);
    p = AppendCapsFlagText("　α成分のグーローシェーディング : ", caps->ShadeCaps, 0x4000, p);
    p = AppendCapsFlagText("　グーローシェーディングでフォグ : ", caps->ShadeCaps, 0x80000, p);
    p += sprintf(p, "　-- テクスチャ能力 ---------------------------\r\n");
    p += sprintf(p, "　最大テクスチャサイズ : (%d, %d)\r\n", caps->MaxTextureWidth, caps->MaxTextureHeight);
    p = AppendCapsFlagText("　α付きテクスチャ : ", caps->TextureCaps, 0x4, p);
    p = AppendCapsFlagText("　テクスチャトランスフォーム : ", caps->TextureCaps, 0x400, p);
    p = AppendCapsFlagText("　バイリニア補間（拡大） : ", caps->TextureFilterCaps, 0x2000000, p);
    p = AppendCapsFlagText("　バイリニア補間（縮小） : ", caps->TextureFilterCaps, 0x200, p);
    p += sprintf(p, "--------------------------------------------\r\n");
}

char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out)
{
    out += sprintf(out, name);
    if ((caps & flag) == 0)
    {
        out += sprintf(out, "不可\r\n");
    }
    else
    {
        out += sprintf(out, "可\r\n");
    }
    return out;
}

int CheckSingleInstance(HINSTANCE instance)
{
    char exePath[0x105];
    char consoleTitle[0x105];
    char *extension;

    g_AppMutex = CreateMutexA(NULL, TRUE, "Touhou 08 App");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        LogError(&g_Log, "二つは起動できません\r\n");
        return -1;
    }
    STARTUPINFOA startup = {sizeof(STARTUPINFOA)};
    GetModuleFileNameA(NULL, exePath, 0x105);
    GetConsoleTitleA(consoleTitle, 0x105);
    GetStartupInfoA(&startup);
    if (startup.lpTitle != NULL)
    {
        extension = strrchr(startup.lpTitle, '.');
        if (FileExists(startup.lpTitle) && extension != NULL)
        {
            if (_stricmp(extension, ".lnk") == 0)
            {
                do
                {
                    ResolveShortcut(startup.lpTitle, consoleTitle, 0x104);
                    extension = strrchr(consoleTitle, '.');
                } while (_stricmp(extension, ".lnk") == 0);
            }
            else
            {
                strcpy(consoleTitle, startup.lpTitle);
            }
            if (strcmp(exePath, consoleTitle) != 0)
            {
                g_GameWindow.unk_1c = 1;
            }
        }
        g_App.stateFlags &= ~0x40;
    }
    else
    {
        g_App.stateFlags |= 0x40;
    }
    if (g_AppMutex == NULL)
    {
        return -1;
    }
    return 0;
}

void ForceForegroundWindow(HWND hwnd)
{
    DWORD activeThread;
    DWORD thread;
    DWORD lockTime;

    activeThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    thread = GetWindowThreadProcessId(hwnd, NULL);
    AttachThreadInput(thread, activeThread, TRUE);
    SystemParametersInfoA(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);
    SetActiveWindow(hwnd);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    AttachThreadInput(thread, activeThread, FALSE);
}

int ChecksumExecutable()
{
    char exePath[0x105];
    int sum;
    int *data;
    int idx;
    int *words;
    int fileSize;

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) != 0)
    {
        sum = 0;
        data = words = (int *)LoadFileData(exePath, &fileSize, 1);
        if (words == NULL)
        {
            return -1;
        }
        for (idx = 0; idx < fileSize / 4 - 1; idx++, words++)
        {
            sum += *words;
        }
        DebugPrint("main sum %d\r\n", sum);
        g_MemoryManager.Free(data);
        g_App.exeChecksum = sum;
        g_App.exeSize = fileSize;
        return sum;
    }
    return -1;
}

int ResolveShortcut(char *path, char *out, int size)
{
    HRESULT res;
    int ok;
    IShellLinkA *shellLink;
    IPersistFile *storage;
    WCHAR *buf;
    WIN32_FIND_DATAA findData;

    if (out == NULL)
    {
        return 0;
    }
    ok = 0;
    CoInitialize(NULL);
    res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void **)&shellLink);
    if (SUCCEEDED(res))
    {
        res = shellLink->QueryInterface(IID_IPersistFile, (void **)&storage);
        if (SUCCEEDED(res))
        {
            buf = new WCHAR[size];
            if (SUCCEEDED(res))
            {
                MultiByteToWideChar(CP_ACP, 0, path, -1, buf, size);
                res = storage->Load(buf, STGM_READ);
                if (SUCCEEDED(res))
                {
                    res = shellLink->GetPath(out, size, &findData, 0);
                    if (SUCCEEDED(res))
                    {
                        ok = 1;
                    }
                }
            }
            delete buf;
            storage->Release();
        }
        shellLink->Release();
    }
    CoUninitialize();
    return ok;
}

void Log::Reset()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
}

void Log::Flush()
{
    if (this->cursor != this->buffer)
    {
        LogPrintf(this, "---------------------------------------------------------- \r\n");
        if (this->hasError)
        {
            MessageBoxA(NULL, this->buffer, "log", MB_ICONERROR);
        }
        WriteToFile("./log.txt", this->buffer, strlen(this->buffer));
    }
}

int App::GetEmulateColorAdd()
{
    return this->flags.emulateColorAdd;
}

int App::GetNoVertexBuffer()
{
    return this->flags.noVertexBuffer;
}

int App::GetForce16BitTextures()
{
    return this->flags.force16BitTextures;
}

int App::GetReferenceRasterizer()
{
    return this->flags.referenceRasterizer;
}

int App::GetWindowed()
{
    return this->windowed;
}
