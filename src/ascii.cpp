// Text drawn with the ASCII font, the pause and continue menus and the
// popups, as th07's ascii.cpp. The first object of the game, from
// 0x402000; its inline copies run to about 0x4071a0.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "chain.h"
#include "sprite.h"
#include "unsorted.h"

AsciiText::AsciiText()
{
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    AsciiPopup *entry;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        entry = text->popups;
        if (!g_GameManager.unk_3dbac_10)
        {
            for (i = 0; i < 0x2d3; i++, entry++)
            {
                if (!entry->active)
                {
                    continue;
                }
                entry->pos.y -= 0.5f * g_App.gameSpeed;
                entry->timer++;
                if (entry->timer > 60)
                {
                    entry->active = 0;
                }
            }
            entry = text->numberPopups;
            for (i = 0; i < 0x80; i++, entry++)
            {
                if (!entry->active)
                {
                    continue;
                }
                entry->timer++;
                if (entry->timer > 90)
                {
                    entry->active = 0;
                }
            }
        }
    }
    else if (g_GameManager.isPaused)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_GameManager.isInContinueMenu)
    {
        text->continueMenu.UpdateContinueMenu();
    }
    text->RunScripts();
    if (g_GameManager.IsDemo())
    {
        if (text->demoIcon.scriptIndex == 0)
        {
            text->anm->StartScript(&text->demoIcon, 11);
        }
        g_SpriteEngine->Step(&text->demoIcon);
    }
    else
    {
        text->demoIcon.scriptIndex = 0;
    }
    text->frameCounter++;
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->ClearStrings();
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    if (text->demoIcon.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->demoIcon);
    }
    return 1;
}
