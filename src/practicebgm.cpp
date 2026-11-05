// cflags: /Os
// The spell card practice's BGM lookups, 0x439916 to 0x4399ac, in
// g_SpellCardBgms (gamemanager.cpp). Built with /Os. In the original they
// follow gui.cpp's last inline copies (from 0x439829), which may end
// gui.cpp's object: then these start their own, as here, or
// gamemanager.cpp's. A plausible seam.

#include "unsorted.h"

int GetPracticeBgmRestarts(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].restartsMusic;
        }
        i++;
    }
    return 0;
}

int GetPracticeBgmTitleIndex(int card)
{
    int i;

    i = 0;
    while (g_SpellCardBgms[i].lastCard >= 0)
    {
        if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[i].lastCard)
        {
            return g_SpellCardBgms[i].titleIndex;
        }
        i++;
    }
    return 0;
}
