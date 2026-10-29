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
