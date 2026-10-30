// The spell card manager, from 0x4143e0. New in th08: th07 ran spell
// cards from its enemy code.

#include "resultscreen.h"
#include "unsorted.h"

int GetSpellCardDifficulty(int card)
{
    int i;
    int j;

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < g_SpellCardListSizes[i]; j++)
        {
            if (g_SpellCardLists[i][j] == card)
            {
                return i;
            }
        }
    }
    return 5;
}

int IsListedSpellCard(int card)
{
    int i;

    for (i = 0; i < g_ListedSpellCardCount; i++)
    {
        if (g_ListedSpellCards[i] == card)
        {
            return 1;
        }
    }
    return 0;
}
