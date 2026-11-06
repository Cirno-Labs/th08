// cflags: /Os -/Oi
// The text helpers, 0x43f980 to 0x440010, as th06's textsurface.cpp: the
// text surface and rendering text into a texture (g_TextSurface is
// defined in unsorted43efef.cpp). Built with /Os. Split at a plausible
// seam (nothing in the binary shows it).

#include <d3dx8tex.h>

#include "unsorted.h"

void CreateTextSurface()
{
    g_App.d3dDevice->CreateImageSurface(1024, 64, D3DFMT_A1R5G5B5, &g_TextSurface);
}

void ReleaseTextSurface()
{
    if (g_TextSurface != NULL)
    {
        g_TextSurface->Release();
        g_TextSurface = NULL;
    }
}

#ifdef TH08_EXE
// Unused: the original's object also drew text with DrawTextA, setting
// the DC's background mode and then its text color before its first
// TextOutA, from code /OPT:REF dropped, as th06's does. A function's
// references come out in reverse order, so SetTextColor loads first;
// DrawTextA and ReleaseDC are USER32 import objects the original loaded
// and dropped.
#pragma code_seg(".text$u")
void UnusedDrawText(HWND window, HDC hdc, char *text, RECT *rect, COLORREF color)
{
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    DrawTextA(hdc, text, -1, rect, DT_LEFT);
    ReleaseDC(window, hdc);
}
#pragma code_seg()
#endif // TH08_EXE

void RenderTextToTextureLarge(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                             COLORREF shadowColor, char *text, IDirect3DTexture8 *texture)
{
    HDC hdcMem;
    HFONT hNewFont;
    D3DSURFACE_DESC sd;
    HGDIOBJ oldObject;
    RECT rcSrc;
    RECT rcDst;
    IDirect3DSurface8 *dstSurface;

    hNewFont = CreateFontA(fontHeight * 2 - 2, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH,
                           "ＭＳ ゴシック");
    GdiSurface dib;
    g_TextSurface->GetDesc(&sd);
    dib.Create(sd.Width, sd.Height, sd.Format);
    hdcMem = dib.GetHdc();
    oldObject = SelectObject(hdcMem, hNewFont);
    dib.InvertAlpha(0, 0, width * 2, fontHeight * 2 + 6, 0);
    SetBkMode(hdcMem, TRANSPARENT);
    if (shadowColor != (COLORREF)-1)
    {
        SetTextColor(hdcMem, 0);
        TextOutA(hdcMem, x * 2 + 4, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 0, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 4, text, strlen(text));
    }
    else
    {
        SetTextColor(hdcMem, 0);
        TextOutA(hdcMem, x * 2 + 3, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 1, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 1, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 3, text, strlen(text));
    }
    SetTextColor(hdcMem, textColor);
    TextOutA(hdcMem, x * 2 + 2, 2, text, strlen(text));
    SelectObject(hdcMem, oldObject);
    dib.InvertAlpha(0, 0, width * 2, fontHeight * 2 + 6, shadowColor == (COLORREF)-1);
    dib.CopyToSurface(g_TextSurface);
    SelectObject(hdcMem, oldObject);
    DeleteObject(hNewFont);
    rcDst.left = 0;
    rcDst.top = y;
    rcDst.right = width;
    rcDst.bottom = y + destHeight;
    rcSrc.left = 0;
    rcSrc.top = 0;
    rcSrc.right = width * 2;
    rcSrc.bottom = fontHeight * 2;
    if (rcSrc.right > 1024)
    {
        rcSrc.right = 1024;
    }
    texture->GetSurfaceLevel(0, &dstSurface);
    D3DXLoadSurfaceFromSurface(dstSurface, NULL, &rcDst, g_TextSurface, NULL, &rcSrc, D3DX_FILTER_TRIANGLE, 0);
    if (dstSurface != NULL)
    {
        dstSurface->Release();
        dstSurface = NULL;
    }
}

void RenderTextToTexture(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture)
{
    HDC hdcMem;
    HFONT hNewFont;
    D3DSURFACE_DESC sd;
    HGDIOBJ oldObject;
    RECT rcSrc;
    RECT rcDst;
    IDirect3DSurface8 *dstSurface;

    hNewFont = CreateFontA(fontHeight * 2, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH,
                           "ＭＳ ゴシック");
    GdiSurface dib;
    g_TextSurface->GetDesc(&sd);
    dib.Create(sd.Width, sd.Height, sd.Format);
    hdcMem = dib.GetHdc();
    oldObject = SelectObject(hdcMem, hNewFont);
    dib.InvertAlpha(0, 0, sd.Width, fontHeight * 2 + 6, 0);
    SetBkMode(hdcMem, TRANSPARENT);
    if (shadowColor != (COLORREF)-1)
    {
        SetTextColor(hdcMem, 0);
        TextOutA(hdcMem, x * 2 + 4, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 0, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 4, text, strlen(text));
    }
    else
    {
        SetTextColor(hdcMem, 0);
        TextOutA(hdcMem, x * 2 + 3, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 1, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 1, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 3, text, strlen(text));
    }
    SetTextColor(hdcMem, textColor);
    TextOutA(hdcMem, x * 2 + 2, 2, text, strlen(text));
    SelectObject(hdcMem, oldObject);
    dib.InvertAlpha(0, 0, sd.Width, fontHeight * 2 + 6, shadowColor == (COLORREF)-1);
    dib.CopyToSurface(g_TextSurface);
    SelectObject(hdcMem, oldObject);
    DeleteObject(hNewFont);
    rcDst.left = 0;
    rcDst.top = y;
    rcDst.right = width;
    rcDst.bottom = y + destHeight;
    rcSrc.left = 0;
    rcSrc.top = 0;
    rcSrc.right = width * 2;
    rcSrc.bottom = fontHeight * 2;
    if (rcSrc.right > 1024)
    {
        rcSrc.right = 1024;
    }
    texture->GetSurfaceLevel(0, &dstSurface);
    D3DXLoadSurfaceFromSurface(dstSurface, NULL, &rcDst, g_TextSurface, NULL, &rcSrc, D3DX_FILTER_TRIANGLE, 0);
    if (dstSurface != NULL)
    {
        dstSurface->Release();
        dstSurface = NULL;
    }
}

#ifdef TH08_EXE
// Unused: the original's object also moved memory with memmove, from code
// /OPT:REF dropped, as th06's does. libcmt's memmove.obj then loads, and
// lies, right after strlen.obj.
#pragma code_seg(".text$u")
void UnusedMoveBits(void *dst, void *src, size_t size)
{
    memmove(dst, src, size);
}
#pragma code_seg()
#endif // TH08_EXE
