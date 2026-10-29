// cflags: /Os
// The /Os file with Gui, as th07's, from 0x4338ca to GameManager at
// 0x4399ac.

#include "unsorted.h"

int Gui::OnUpdate(Gui *gui)
{
    if (g_GameManager.isFrozen)
    {
        return 1;
    }
    gui->UpdateInterface();
    gui->impl->RunMsg();
    if ((g_Buttons & 0x100) && g_App.redrawFrames < 8)
    {
        g_App.redrawFrames = 8;
    }
    gui->frameCounter++;
    return 1;
}

int Gui::OnDraw(Gui *gui)
{
    if (gui->impl->unk_22d84 != 0)
    {
        gui->Func43826b();
    }
    gui->impl->DrawMsg();
    gui->DrawBanners();
    gui->DrawHud();
    gui->Func438a89();
    return 1;
}
