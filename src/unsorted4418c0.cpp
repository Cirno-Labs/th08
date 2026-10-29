// The file from about 0x4418c0 (WinMain): start-up, the window and the
// display, as in th07's unsorted433e90.cpp.

#include <stdio.h>
#include <string.h>

#include "unsorted.h"

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
        g_App.unk_1a4 &= ~0x40;
    }
    else
    {
        g_App.unk_1a4 |= 0x40;
    }
    if (g_AppMutex == NULL)
    {
        return -1;
    }
    return 0;
}
