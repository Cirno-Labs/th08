// The spell card manager, from 0x4143e0. New in th08: th07 ran spell
// cards from its enemy code.

#include <string.h>

#include "ascii.h"
#include "resultscreen.h"
#include "sprite.h"
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

// Loads boss face `path` into slot `slot` as faceAnms[`idx`], returning -1
// from the caller if it fails.
#define LOAD_FACE(idx, slot, path)                                      \
    this->faceAnms[idx] = g_SpriteEngine->PreloadAnm(slot, path);       \
    if (this->faceAnms[idx] == NULL)                                    \
    {                                                                   \
        return -1;                                                      \
    }

int SpellCard::Initialize()
{
    if (IsLoadingResources())
    {
        memset(this, 0, sizeof(SpellCard));
        this->backgroundAnm = g_SpriteEngine->PreloadAnm(0xf, "face_cdbg.anm");
        if (this->backgroundAnm == NULL)
        {
            return -1;
        }
        if (!g_GameManager.IsInSpellCardPractice())
        {
            switch (g_GameManager.character)
            {
            default:
                LOAD_FACE(0, 0x10, "face_rm00.anm");
                LOAD_FACE(1, 0x11, "face_yk00.anm");
                break;
            case 1:
            case 6:
            case 7:
                LOAD_FACE(0, 0x10, "face_mr00.anm");
                LOAD_FACE(1, 0x11, "face_al00.anm");
                break;
            case 2:
            case 8:
            case 9:
                LOAD_FACE(0, 0x10, "face_sk00.anm");
                LOAD_FACE(1, 0x11, "face_rs00.anm");
                break;
            case 3:
            case 10:
            case 11:
                LOAD_FACE(0, 0x10, "face_ym00.anm");
                LOAD_FACE(1, 0x11, "face_yy00.anm");
                break;
            }
        }
    }
    else
    {
        memset(this, 0, sizeof(SpellCard));
        this->backgroundAnm = g_SpriteEngine->GetAnm(0xf);
        if (!g_GameManager.IsInSpellCardPractice())
        {
            this->faceAnms[0] = g_SpriteEngine->GetAnm(0x10);
            this->faceAnms[1] = g_SpriteEngine->GetAnm(0x11);
        }
    }
    if (!IsRestarting())
    {
        if (!g_GameManager.IsInSpellCardPractice())
        {
            switch (g_GameManager.stage)
            {
            case 0:
                LOAD_FACE(2, 0x12, "face_st01.anm");
                break;
            case 1:
                LOAD_FACE(2, 0x12, "face_st02.anm");
                break;
            case 2:
                LOAD_FACE(2, 0x12, "face_st03.anm");
                break;
            case 3:
                LOAD_FACE(2, 0x12, "face_st04a.anm");
                break;
            case 4:
                LOAD_FACE(2, 0x12, "face_st04b.anm");
                break;
            case 5:
                LOAD_FACE(2, 0x12, "face_st05.anm");
                LOAD_FACE(3, 0x13, "face_st05b.anm");
                break;
            case 6:
                LOAD_FACE(2, 0x12, "face_st06.anm");
                break;
            case 7:
                LOAD_FACE(2, 0x12, "face_st06.anm");
                LOAD_FACE(3, 0x13, "face_st07.anm");
                break;
            case 8:
                LOAD_FACE(2, 0x12, "face_st08m.anm");
                LOAD_FACE(3, 0x13, "face_st08.anm");
                break;
            default:
                LOAD_FACE(2, 0x12, "face_st03.anm");
            }
        }
        else
        {
            switch (g_GameManager.stage)
            {
            case 0:
                LOAD_FACE(2, 0x12, "face_st01sp.anm");
                break;
            case 1:
                LOAD_FACE(2, 0x12, "face_st02sp.anm");
                break;
            case 2:
                LOAD_FACE(2, 0x12, "face_st03sp.anm");
                break;
            case 3:
                if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.IsPracticingSpellCard(0xd6))
                {
                    LOAD_FACE(2, 0x12, "face_st04asp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xd8))
                {
                    LOAD_FACE(2, 0x12, "face_sksp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xd9))
                {
                    LOAD_FACE(2, 0x12, "face_ymsp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xda))
                {
                    LOAD_FACE(2, 0x12, "face_alsp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xdb))
                {
                    LOAD_FACE(2, 0x12, "face_rssp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xdc))
                {
                    LOAD_FACE(2, 0x12, "face_yysp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCard(0xdd))
                {
                    LOAD_FACE(2, 0x12, "face_yksp.anm");
                }
                else
                {
                    LOAD_FACE(2, 0x12, "face_st04asp.anm");
                }
                break;
            case 4:
                LOAD_FACE(2, 0x12, "face_st04bsp.anm");
                break;
            case 5:
                if (g_GameManager.IsPracticingSpellCard(0xd4))
                {
                    LOAD_FACE(2, 0x12, "face_st05msp.anm");
                }
                else
                {
                    LOAD_FACE(2, 0x12, "face_st05sp.anm");
                }
                break;
            case 6:
                LOAD_FACE(2, 0x12, "face_st06sp.anm");
                break;
            case 7:
                if (g_GameManager.IsPracticingSpellCardIn(0x93, 0x96))
                {
                    LOAD_FACE(2, 0x12, "face_st06sp.anm");
                }
                else
                {
                    LOAD_FACE(2, 0x12, "face_st07sp.anm");
                }
                break;
            case 8:
                if (g_GameManager.IsPracticingSpellCardIn(0xbf, 0xc1) || g_GameManager.IsPracticingSpellCard(0xd5))
                {
                    LOAD_FACE(2, 0x12, "face_st08msp.anm");
                }
                else if (g_GameManager.IsPracticingSpellCardIn(0xc2, 0xcc) || g_GameManager.IsPracticingSpellCard(0xd3))
                {
                    LOAD_FACE(2, 0x12, "face_st08sp.anm");
                }
                break;
            default:
                LOAD_FACE(2, 0x12, "face_st03.anm");
            }
        }
    }
    else
    {
        this->faceAnms[2] = g_SpriteEngine->GetAnm(0x12);
        this->faceAnms[3] = g_SpriteEngine->GetAnm(0x13);
    }
    if (this->faceAnms[0] != NULL)
    {
        this->faceAnms[0]->StartScript(&this->unk_120, 0);
        this->faceAnms[0]->StartScript(&this->unk_3c4, 0);
    }
    g_App.textAnm->StartScript(&this->unk_10f8, 4);
    g_App.textAnm->StartScript(&this->unk_139c, 5);
    g_AsciiText.anm->StartScript(&this->unk_1b88, 1);
    g_AsciiText.anm->StartScript(&this->unk_1e2c, 0);
    g_AsciiText.anm->StartScript(&this->unk_2374, 2);
    g_AsciiText.anm->StartScript(&this->unk_20d0, 4);
    this->unk_120.scriptPos = NULL;
    this->unk_668.scriptPos = NULL;
    this->unk_bb0.scriptPos = NULL;
    this->unk_10f8.scriptPos = NULL;
    this->unk_3c4.scriptPos = NULL;
    this->unk_90c.scriptPos = NULL;
    this->unk_e54.scriptPos = NULL;
    this->unk_139c.scriptPos = NULL;
    this->unk_120.shown = 0;
    this->unk_668.shown = 0;
    this->unk_bb0.shown = 0;
    this->unk_10f8.shown = 0;
    this->unk_3c4.shown = 0;
    this->unk_90c.shown = 0;
    this->unk_e54.shown = 0;
    this->unk_139c.shown = 0;
    this->unk_10f8.fontWidth = 0xf;
    this->unk_10f8.fontHeight = 0xf;
    this->unk_139c.fontWidth = 0xf;
    this->unk_139c.fontHeight = 0xf;
    this->unk_10 = 0;
    return 0;
}
