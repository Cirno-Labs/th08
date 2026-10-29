// cflags: /Os -/Oi
// The file starting at 0x43efef, th07's unsorted431a0f.cpp. It is built
// with /Os, as in th07: functions end with `leave` instead of
// `mov esp, ebp; pop ebp`.

#include <d3dx8tex.h>

#include "unsorted.h"

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
