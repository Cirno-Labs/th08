// cflags: /Os -/Oi
// The file from 0x43efef to 0x43f980, th07's unsorted431a0f.cpp: GdiSurface
// (textsurface.cpp follows). It is built with /Os, as in th07: functions
// end with `leave` instead of `mov esp, ebp; pop ebp`.

#include <d3dx8tex.h>

#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
FormatInfo g_FormatInfos[7] = {
    {22, 32, 0x00000000, 0x00ff0000, 0x0000ff00, 0x000000ff},
    {21, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff},
    {24, 16, 0x00000000, 0x00007c00, 0x000003e0, 0x0000001f},
    {23, 16, 0x00000000, 0x0000f800, 0x000007e0, 0x0000001f},
    {25, 16, 0x00008000, 0x00007c00, 0x000003e0, 0x0000001f},
    {26, 16, 0x0000f000, 0x00000f00, 0x000000f0, 0x0000000f},
    {-1, 0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
};
IDirect3DSurface8 *g_TextSurface;
#endif // TH08_EXE

#ifdef TH08_EXE
// Unused: the original's object also made a font before GdiSurface's
// first GDI call, from code /OPT:REF dropped: the hint/name table loads
// CreateFontA first of GDI32's, which decides GDI32's IAT order.
#pragma code_seg(".text$u")
HFONT UnusedCreateFont(int height)
{
    return CreateFontA(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_ROMAN, NULL);
}
#pragma code_seg()
#endif // TH08_EXE

GdiSurface::GdiSurface()
{
    this->format = -1;
    this->width = 0;
    this->height = 0;
    this->hdc = NULL;
    this->bitmap = NULL;
    this->oldObject = NULL;
    this->bits = NULL;
}

GdiSurface::~GdiSurface()
{
    this->Release();
}

bool GdiSurface::Release()
{
    if (this->hdc != NULL)
    {
        SelectObject(this->hdc, this->oldObject);
        DeleteDC(this->hdc);
        DeleteObject(this->bitmap);
        this->format = -1;
        this->width = 0;
        this->height = 0;
        this->hdc = NULL;
        this->bitmap = NULL;
        this->oldObject = NULL;
        this->bits = NULL;
        return true;
    }
    return false;
}

#ifdef TH08_EXE
// Unused: a five-byte function with one relocation, a bare jump, that
// /OPT:REF dropped. /OPT:ICF sorts it among D3DX8's identical five-byte
// codec destructors, which decides which of their copies it keeps (see
// UnusedShiftResults in resultscreen.cpp).
__declspec(naked) void UnusedJumpToTextSurface()
{
    __asm jmp CreateTextSurface
}
#endif // TH08_EXE

bool GdiSurface::Create(int width, int height, int format)
{
    if (this->CreateInFormat(width, height, format))
    {
        return true;
    }
    if (format == D3DFMT_A1R5G5B5 || format == D3DFMT_A4R4G4B4)
    {
        return this->CreateInFormat(width, height, D3DFMT_A8R8G8B8);
    }
    if (format == D3DFMT_R5G6B5)
    {
        return this->CreateInFormat(width, height, D3DFMT_X8R8G8B8);
    }
    return false;
}

bool GdiSurface::CreateInFormat(int width, int height, int format)
{
    int widthBytes;
    HDC dc;
    HGDIOBJ oldObject;
    BITMAPV4HEADER bitmapInfo;
    FormatInfo *fmt;
    HBITMAP bitmap;
    void *data;

    this->Release();
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
    fmt = this->FindFormat(format);
    if (fmt == NULL)
    {
        return false;
    }
    widthBytes = (width * fmt->bitCount / 8 + 3) / 4 * 4;
    bitmapInfo.bV4Size = sizeof(bitmapInfo);
    bitmapInfo.bV4Width = width;
    bitmapInfo.bV4Height = -(height + 1);
    bitmapInfo.bV4Planes = 1;
    bitmapInfo.bV4BitCount = fmt->bitCount;
    bitmapInfo.bV4SizeImage = height * widthBytes;
    if (format != D3DFMT_X1R5G5B5 && format != D3DFMT_X8R8G8B8)
    {
        bitmapInfo.bV4V4Compression = BI_BITFIELDS;
        bitmapInfo.bV4RedMask = fmt->redMask;
        bitmapInfo.bV4GreenMask = fmt->greenMask;
        bitmapInfo.bV4BlueMask = fmt->blueMask;
        bitmapInfo.bV4AlphaMask = fmt->alphaMask;
    }
    bitmap = CreateDIBSection(NULL, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS, &data, NULL, 0);
    if (bitmap == NULL)
    {
        return false;
    }
    memset(data, 0, bitmapInfo.bV4SizeImage);
    dc = CreateCompatibleDC(NULL);
    oldObject = SelectObject(dc, bitmap);
    this->hdc = dc;
    this->bitmap = bitmap;
    this->bits = data;
    this->imageSize = bitmapInfo.bV4SizeImage;
    this->oldObject = oldObject;
    this->width = width;
    this->height = height;
    this->format = format;
    this->widthBytes = widthBytes;
    return true;
}

FormatInfo *GdiSurface::FindFormat(int format)
{
    int i;

    for (i = 0; g_FormatInfos[i].format != -1 && g_FormatInfos[i].format != format; i++)
    {
    }
    if (format == -1)
    {
        return NULL;
    }
    return &g_FormatInfos[i];
}

bool GdiSurface::InvertAlpha(int x, int y, int width, int height, int lightShading)
{
    BYTE *line;
    int offset;
    int total;
    int val;

    total = width * height * 2;
    line = (BYTE *)this->GetBits() + y * width * 2;
    switch (this->format)
    {
    case D3DFMT_A8R8G8B8:
        for (offset = 3; offset < total; offset += 4)
        {
            line[offset] ^= 0xff;
        }
        break;
    case D3DFMT_A1R5G5B5:
        A1R5G5B5 *px;
        for (px = (A1R5G5B5 *)line, offset = 0; offset < total; offset += 2, px++)
        {
            px->a ^= 1;
            if (px->a)
            {
                if (!lightShading)
                {
                    if (px->r >= px->b)
                    {
                        val = px->r - px->r * offset * 2 / total / 3;
                        px->r = val >= 32 ? 31 : val;
                        val = px->g - px->g * offset * 2 / total / 3;
                        px->g = val >= 32 ? 31 : val;
                    }
                    else
                    {
                        val = px->b - px->b * offset / total / 2;
                        px->b = val >= 32 ? 31 : val;
                        val = px->g - px->g * offset / total / 2;
                        px->g = val >= 32 ? 31 : val;
                    }
                }
                else if (px->r >= px->b)
                {
                    val = px->r - px->r * offset / total / 4;
                    px->r = val >= 32 ? 31 : val;
                    val = px->g - px->g * offset / total / 4;
                    px->g = val >= 32 ? 31 : val;
                }
                else
                {
                    val = px->b - px->b * offset / total / 4;
                    px->b = val >= 32 ? 31 : val;
                    val = px->g - px->g * offset / total / 4;
                    px->g = val >= 32 ? 31 : val;
                }
            }
            else
            {
                px->r = 0;
                px->g = 0;
                px->b = 0;
            }
        }
        break;
    case D3DFMT_A4R4G4B4:
        for (offset = 1; offset < total; offset += 2)
        {
            line[offset] ^= 0xf0;
        }
        break;
    default:
        return false;
    }
    return true;
}

void * GdiSurface::GetBits()
{
    return this->bits;
}

int GdiSurface::GetWidthBytes()
{
    return this->widthBytes;
}

int GdiSurface::GetHeight()
{
    return this->height;
}

HDC GdiSurface::GetHdc()
{
    return this->hdc;
}

int GdiSurface::GetWidth()
{
    return this->width;
}

int GdiSurface::GetFormat()
{
    return this->format;
}

bool GdiSurface::IsCreated()
{
    return this->bitmap != NULL;
}

bool GdiSurface::CopyToSurface(IDirect3DSurface8 *surface)
{
    BYTE *dst;
    int lineSize;
    RECT region;
    int row;
    int srcPitch;
    D3DSURFACE_DESC surfaceDesc;
    BYTE *src;
    D3DLOCKED_RECT lockedSurface;

    if (!this->IsCreated())
    {
        return false;
    }
    surface->GetDesc(&surfaceDesc);
    region.left = 0;
    region.top = 0;
    region.right = this->GetWidth();
    region.bottom = this->GetHeight();
    if (surface->LockRect(&lockedSurface, &region, 0) != D3D_OK)
    {
        return false;
    }
    lineSize = lockedSurface.Pitch;
    srcPitch = this->GetWidthBytes();
    src = (BYTE *)this->GetBits();
    dst = (BYTE *)lockedSurface.pBits;
    if (surfaceDesc.Format == this->GetFormat())
    {
        for (row = 0; row < this->GetHeight(); row++)
        {
            memcpy(dst, src, srcPitch);
            src += srcPitch;
            dst += lineSize;
        }
    }
    surface->UnlockRect();
    return true;
}

