// The spell card manager, from 0x4143e0. New in th08: th07 ran spell
// cards from its enemy code.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "resultscreen.h"
#include "sprite.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
int g_EasySpellCards[42] = {
    2, 6, 13, 17, 21, 25, 32, 39, 43, 47,
    54, 58, 62, 66, 70, 77, 81, 85, 89, 93,
    100, 104, 108, 112, 119, 123, 127, 131, 135, 139,
    143, 147, 151, 155, 159, 163, 167, 171, 175, 179,
    183, 187,
};
int g_NormalSpellCards[49] = {
    3, 7, 10, 14, 18, 22, 26, 29, 33, 36,
    40, 44, 48, 51, 55, 59, 63, 67, 71, 74,
    78, 82, 86, 90, 94, 97, 101, 105, 109, 113,
    116, 120, 124, 128, 132, 136, 140, 144, 148, 152,
    156, 160, 164, 168, 172, 176, 180, 184, 188,
};
int g_HardSpellCards[50] = {
    0, 4, 8, 11, 15, 19, 23, 27, 30, 34,
    37, 41, 45, 49, 52, 56, 60, 64, 68, 72,
    75, 79, 83, 87, 91, 95, 98, 102, 106, 110,
    114, 117, 121, 125, 129, 133, 137, 141, 145, 149,
    153, 157, 161, 165, 169, 173, 177, 181, 185, 189,
};
int g_LunaticSpellCards[50] = {
    1, 5, 9, 12, 16, 20, 24, 28, 31, 35,
    38, 42, 46, 50, 53, 57, 61, 65, 69, 73,
    76, 80, 84, 88, 92, 96, 99, 103, 107, 111,
    115, 118, 122, 126, 130, 134, 138, 142, 146, 150,
    154, 158, 162, 166, 170, 174, 178, 182, 186, 190,
};
int g_ExtraSpellCards[14] = {
    191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
    201, 202, 203, 204,
};
int g_AllSpellCards[222] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221,
};
int *g_SpellCardLists[6] = {
    g_EasySpellCards,
    g_NormalSpellCards,
    g_HardSpellCards,
    g_LunaticSpellCards,
    g_ExtraSpellCards,
    g_AllSpellCards,
};
int g_SpellCardListSizes[6] = {
    42, 49, 50, 50, 14, 222,
};
int g_Stage1SpellCards[13] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12,
};
int g_Stage2SpellCards[19] = {
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31,
};
int g_Stage3SpellCards[22] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 53,
};
int g_Stage4ASpellCards[23] = {
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76,
};
int g_Stage4BSpellCards[23] = {
    77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
    87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99,
};
int g_Stage5SpellCards[19] = {
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118,
};
int g_Stage6ASpellCards[28] = {
    119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
    139, 140, 141, 142, 143, 144, 145, 146,
};
int g_Stage6BSpellCards[44] = {
    147, 148, 149, 150, 151, 152, 153, 154, 155, 156,
    157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
    177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
    187, 188, 189, 190,
};
int g_StageExtraSpellCards[14] = {
    191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
    201, 202, 203, 204,
};
int g_StageLastWordSpellCards[17] = {
    205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
    215, 216, 217, 218, 219, 220, 221,
};
int g_ListedSpellCards[43] = {
    10, 11, 12, 29, 30, 31, 51, 52, 53, 74,
    75, 76, 97, 98, 99, 116, 117, 118, 143, 144,
    145, 146, 171, 172, 173, 174, 175, 176, 177, 178,
    179, 180, 181, 182, 183, 184, 185, 186, 187, 188,
    189, 190, 204,
};
int g_ListedSpellCardCount = 43;
int *g_StageSpellCards[10] = {
    g_Stage1SpellCards,
    g_Stage2SpellCards,
    g_Stage3SpellCards,
    g_Stage4ASpellCards,
    g_Stage4BSpellCards,
    g_Stage5SpellCards,
    g_Stage6ASpellCards,
    g_Stage6BSpellCards,
    g_StageExtraSpellCards,
    g_StageLastWordSpellCards,
};
int g_StageSpellCardCounts[10] = {
    13, 19, 22, 23, 23, 19, 28, 44, 14, 17,
};
#endif // TH08_EXE

EXE_INLINE SpellCard::SpellCard()
{
}

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
        this->faceAnms[0]->StartScript(&this->bombPortrait, 0);
        this->faceAnms[0]->StartScript(&this->spellPortrait, 0);
    }
    g_App.textAnm->StartScript(&this->bombName, 4);
    g_App.textAnm->StartScript(&this->spellName, 5);
    g_AsciiText.anm->StartScript(&this->bombNameBar, 1);
    g_AsciiText.anm->StartScript(&this->spellNameBar, 0);
    g_AsciiText.anm->StartScript(&this->spellScoreBar, 2);
    g_AsciiText.anm->StartScript(&this->spellScoreDigit, 4);
    this->bombPortrait.scriptPos = NULL;
    this->bombPortraitGlow.scriptPos = NULL;
    this->bombPortraitFrame.scriptPos = NULL;
    this->bombName.scriptPos = NULL;
    this->spellPortrait.scriptPos = NULL;
    this->spellPortraitGlow.scriptPos = NULL;
    this->spellPortraitFrame.scriptPos = NULL;
    this->spellName.scriptPos = NULL;
    this->bombPortrait.shown = 0;
    this->bombPortraitGlow.shown = 0;
    this->bombPortraitFrame.shown = 0;
    this->bombName.shown = 0;
    this->spellPortrait.shown = 0;
    this->spellPortraitGlow.shown = 0;
    this->spellPortraitFrame.shown = 0;
    this->spellName.shown = 0;
    this->bombName.fontWidth = 0xf;
    this->bombName.fontHeight = 0xf;
    this->spellName.fontWidth = 0xf;
    this->spellName.fontHeight = 0xf;
    this->timeOrbBonus = 0;
    return 0;
}

void SpellCard::Start(int card, char *name, int face, DWORD bonus, Enemy *enemy, char *text1, char *text2, char *text3)
{
    int i;
    int sum;
    int k;
    Catk *catk;
    int nameSum;

    this->captured = 0;
    this->active = 1;
    this->capture = 1;
    this->unusedFlag0_4 = 0;
    this->timedOut = 0;
    this->effectFixed = 0;
    this->bombed = 0;
    this->nameOnly = 0;
    this->card = card;
    this->enemy = enemy;
    this->enemyIndex = this->enemy->index;
    this->bonus = bonus;
    this->startBonus = bonus;
    if (this->enemy->keepSpellCard)
    {
        this->startBonus = 99999990;
    }
    this->bonusStep = (this->bonus - this->bonus / 7) / (this->enemy->timerCallbackTime / 60);
    this->timeLeft = this->enemy->timerCallbackTime;
    this->timeLimit = this->enemy->timerCallbackTime;
    for (i = 0; i < sizeof(this->name); i++)
    {
        this->name[i] = (BYTE)name[i] ^ 0xaa;
    }
    this->ShowName(face, this->name, 0);
    g_BulletManager.ClearAllToItems();
    g_Stage.StartSpellCard();
    for (i = 0; i < g_Stage.dialogueSpriteCount; i++)
    {
        g_EffectManager.stageAnm->StartScript(&g_Stage.dialogueSprites[i], i + g_Stage.dialogueScriptOffset);
    }
    g_Stage.spellCardDrawCallback = NULL;
    this->enemy->ResetRankSpeeds();
    this->nameColor = 0x80808080;
    if (this->bonusPending)
    {
        this->captureEffect = NULL;
        g_Gui.ShowSpellCardBonus(this->capturedBonus);
        g_GameManager.AddScore(this->capturedBonus);
        this->bonusPending = 0;
        if (this->timeOrbBonus > 0)
        {
            g_GameManager.AddTimeOrbs(this->timeOrbBonus);
            this->timeOrbBonus = 0;
        }
    }
    this->bonusFrozen = 0;
    this->effect = g_EffectManager.SpawnReservedEffect(g_GameManager.lastSpellState ? 0x34 : 0x27, &this->enemy->pos, 1, 1, 0xffffffff);
    this->effect->sprite.interpDuration[0] = 0;
    this->effect->sprite.interpFrame[0] = 100;
    this->effect->sprite.interpEasing = 6;
    this->effect->sprite.moveStart.x = 8.0f;
    this->effect->sprite.moveEnd.x = 256.0f;
    this->effect->sprite.moveStart.y = 64.0f;
    this->effect->sprite.moveEnd.y = 0.0f;
    this->effect->sprite.pos.y = 64.0f;
    this->effect->pos = this->enemy->pos;
    this->effect->segmentCount = 0x40;
    this->effect->angle = 0.0f;
    this->effect->radius = 256.0f;
    this->effect->ringWidth = 15.0f;
    this->effect->waveCount = 6.0f;
    this->lastSpell = g_GameManager.lastSpellState;
    g_Gui.bombsChanged = 3;
    g_Gui.livesChanged = 3;
    this->effectFixed = 0;
    if (!g_GameManager.IsReplay())
    {
        char buf[0x80];

        catk = &g_GameManager.catks[this->card];
        sum = 0;
        strcpy(catk->name, this->name);
        memset(buf, 0, sizeof(buf));
        for (k = 0; k < sizeof(this->name); k++)
        {
            buf[k] = (BYTE)text1[k] ^ 0xbb;
        }
        strcpy(catk->comment, buf);
        if (g_GameManager.isInSpellCardPractice)
        {
            memcpy(this->text2, text2, sizeof(this->text2));
            memcpy(this->text3, text3, sizeof(this->text3));
        }
        k = strlen(catk->name);
        while (k > 0)
        {
            k--;
            sum += catk->name[k];
        }
        nameSum = sum;
        for (k = 0; k < 13; k++)
        {
            sum += catk->captures[k];
            sum += catk->attempts[k];
            sum += catk->maxBonus[k];
            sum += catk->practiceCaptures[k];
            sum += catk->practiceAttempts[k];
            sum += catk->practiceMaxBonus[k];
        }
        if (catk->checksum != (BYTE)sum)
        {
            for (k = 0; k < 13; k++)
            {
                catk->captures[k] = 0;
                catk->attempts[k] = 0;
                catk->maxBonus[k] = 0;
                catk->practiceCaptures[k] = 0;
                catk->practiceAttempts[k] = 0;
                catk->practiceMaxBonus[k] = 0;
            }
        }
        if (!g_GameManager.isInSpellCardPractice)
        {
            if (catk->attempts[g_GameManager.character] < 9999)
            {
                catk->attempts[g_GameManager.character]++;
            }
            if (catk->attempts[12] < 9999)
            {
                catk->attempts[12]++;
            }
        }
        else
        {
            if (catk->practiceAttempts[g_GameManager.character] < 9999)
            {
                catk->practiceAttempts[g_GameManager.character]++;
            }
            if (catk->practiceAttempts[12] < 9999)
            {
                catk->practiceAttempts[12]++;
            }
        }
        for (k = 0; k < 13; k++)
        {
            nameSum += catk->captures[k];
            nameSum += catk->attempts[k];
            nameSum += catk->maxBonus[k];
            nameSum += catk->practiceCaptures[k];
            nameSum += catk->practiceAttempts[k];
            nameSum += catk->practiceMaxBonus[k];
        }
        catk->difficulty = g_GameManager.difficulty;
        catk->checksum = nameSum;
    }
}

void BulletManager::ClearAllToItems()
{
    this->ClearAll(1);
}

void SpellCard::StartNameOnly(char *name, Enemy *enemy)
{
    this->nameOnly = 1;
    this->active = 0;
    this->unusedFlag0_4 = 0;
    this->ShowName(-1, name, 1);
}

void SpellCard::StartBombCard(int face, char *name, int sprite)
{
    if (face == 0)
    {
        this->faceAnms[0]->StartScript(&this->bombPortrait, 0);
        this->faceAnms[0]->SetSprite(&this->bombPortrait, 0);
    }
    else if (face == 1)
    {
        this->faceAnms[1]->StartScript(&this->bombPortrait, 0);
        this->faceAnms[1]->SetSprite(&this->bombPortrait, 0);
    }
    this->backgroundAnm->StartScript(&this->bombPortraitGlow, 0);
    this->backgroundAnm->StartScript(&this->bombPortraitFrame, 2);
    this->backgroundAnm->SetSprite(&this->bombPortraitGlow, sprite);
    this->backgroundAnm->SetSprite(&this->bombPortraitFrame, sprite);
    g_App.textAnm->StartScript(&this->bombName, 4);
    g_SpriteEngine->RenderText(&this->bombName, 0xf0f0ff, 0, name);
    this->bombNameWidth = (float)(strlen(name) * 15) / 2.0f + 16.0f;
    this->bombNameBar.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(0xe, 0);
    g_App.redrawFrames = 2;
}

void SpellCard::ShowName(int face, char *name, int sprite)
{
    if (face >= 0)
    {
        this->faceAnms[2]->StartScript(&this->spellPortrait, 0);
        this->faceAnms[2]->SetSprite(&this->spellPortrait, face);
    }
    this->backgroundAnm->StartScript(&this->bombPortraitGlow, 1);
    this->backgroundAnm->SetSprite(&this->bombPortraitGlow, sprite);
    this->backgroundAnm->StartScript(&this->bombPortraitFrame, 3);
    this->backgroundAnm->SetSprite(&this->bombPortraitFrame, sprite);
    if (g_GameManager.IsPracticingSpellCardIn(0xcd, 0xdd))
    {
        g_App.textAnm->StartScript(&this->spellName, 6);
        g_App.textAnm->StartScript(&this->lastWordName1, 7);
        g_App.textAnm->StartScript(&this->lastWordName2, 8);
        g_SpriteEngine->RenderTextRightAligned(&this->lastWordName1, 0xfff0f0, 0, name);
        g_SpriteEngine->RenderTextRightAligned(&this->lastWordName2, 0xfff0f0, 0, name);
    }
    else
    {
        g_App.textAnm->StartScript(&this->spellName, 5);
    }
    g_SpriteEngine->RenderTextRightAligned(&this->spellName, 0xfff0f0, 0, name);
    this->spellNameWidth = (float)(strlen(name) * 15) / 2.0f + 16.0f;
    this->spellNameBar.SetPendingInterrupt(1);
    if (!this->nameOnly)
    {
        this->spellScoreBar.SetPendingInterrupt(1);
    }
    g_SoundPlayer.QueueSound(0xe, 0);
    g_App.redrawFrames = 2;
}

void SpellCard::EndSpellCardName()
{
    this->spellName.pendingInterrupt = 1;
    this->spellNameBar.SetPendingInterrupt(2);
    if (!this->nameOnly)
    {
        this->spellScoreBar.SetPendingInterrupt(2);
    }
}

void SpellCard::End()
{
    int k;
    Catk *catk;
    int captured;
    int bonus;

    g_AsciiText.ringAlpha = 0;
    if (this->active)
    {
        captured = FALSE;
        this->active = 0;
        this->EndSpellCardName();
        if (!this->timedOut)
        {
            bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
            bonus = g_EnemyManager.ClearEnemiesForBonus(8000, bonus);
            if (bonus != 0)
            {
                g_GameManager.AddScore(bonus);
                g_Gui.ShowBonusPopup(bonus);
            }
            if (this->capture)
            {
                catk = &g_GameManager.catks[this->card];
                this->capturedBonus = this->bonus;
                if (this->enemy->keepSpellCard)
                {
                    this->timeOrbBonus = 700;
                }
                else
                {
                    k = this->timeLimit.AsFrames() - this->timeLimit.AsFrames() / 7;
                    if (this->timeLeft.AsFrames() >= k)
                    {
                        this->timeOrbBonus = 1000;
                    }
                    else if (this->timeLeft.AsFrames() >= 180)
                    {
                        this->timeOrbBonus = (this->timeLeft.AsFrames() - 180) * 900 / (k - 180) + 100;
                    }
                    else
                    {
                        this->timeOrbBonus = 100;
                    }
                }
                this->captured = 1;
                if (!g_GameManager.IsReplay())
                {
                    int nameSum;
                    int sum;
                    int shot;

                    sum = 0;
                    k = strlen(catk->name);
                    while (k > 0)
                    {
                        k--;
                        sum += catk->name[k];
                    }
                    nameSum = sum;
                    for (k = 0; k < 13; k++)
                    {
                        sum += catk->captures[k];
                        sum += catk->attempts[k];
                        sum += catk->maxBonus[k];
                        sum += catk->practiceCaptures[k];
                        sum += catk->practiceAttempts[k];
                        sum += catk->practiceMaxBonus[k];
                    }
                    if (catk->checksum != (BYTE)sum)
                    {
                        for (k = 0; k < 13; k++)
                        {
                            catk->captures[k] = 0;
                            catk->attempts[k] = 0;
                            catk->maxBonus[k] = 0;
                            catk->practiceCaptures[k] = 0;
                            catk->practiceAttempts[k] = 0;
                            catk->practiceMaxBonus[k] = 0;
                        }
                    }
                    catk->difficulty = g_GameManager.difficulty;
                    if (!g_GameManager.IsInSpellCardPractice())
                    {
                        shot = g_GameManager.character;
                        if (catk->maxBonus[shot] < this->bonus)
                        {
                            catk->maxBonus[shot] = this->bonus;
                        }
                        if (catk->maxBonus[12] < this->bonus)
                        {
                            catk->maxBonus[12] = this->bonus;
                        }
                        if (catk->captures[shot] < 9999)
                        {
                            catk->captures[shot]++;
                        }
                        if (catk->captures[12] < 9999)
                        {
                            catk->captures[12]++;
                        }
                    }
                    else
                    {
                        for (k = 0; k < sizeof(catk->note); k++)
                        {
                            catk->note[k] = (BYTE)this->text2[k] ^ 0xdd;
                        }
                        for (k = 0; k < sizeof(catk->note2); k++)
                        {
                            catk->note2[k] = (BYTE)this->text3[k] ^ 0xee;
                        }
                        shot = g_GameManager.character;
                        if (catk->practiceMaxBonus[shot] < this->bonus)
                        {
                            catk->practiceMaxBonus[shot] = this->bonus;
                        }
                        if (catk->practiceMaxBonus[12] < this->bonus)
                        {
                            catk->practiceMaxBonus[12] = this->bonus;
                        }
                        if (catk->practiceCaptures[shot] < 9999)
                        {
                            catk->practiceCaptures[shot]++;
                        }
                        if (catk->practiceCaptures[12] < 9999)
                        {
                            catk->practiceCaptures[12]++;
                        }
                    }
                    for (k = 0; k < 13; k++)
                    {
                        nameSum += catk->captures[k];
                        nameSum += catk->attempts[k];
                        nameSum += catk->maxBonus[k];
                        nameSum += catk->practiceCaptures[k];
                        nameSum += catk->practiceAttempts[k];
                        nameSum += catk->practiceMaxBonus[k];
                    }
                    catk->checksum = nameSum;
                    g_GameManager.hscr.spellCards[this->card]++;
                }
                g_GameManager.stats->spellCardsCaptured++;
                captured = TRUE;
                g_GameManager.RerollChecksumKeys();
            }
        }
        if (this->effect != NULL)
        {
            if (!captured)
            {
                if (g_GameManager.lastSpellState)
                {
                    g_GameManager.lastSpellState = 0;
                }
                this->effect->active = 0;
                this->effect = NULL;
                g_Gui.ShowFullPowerPopup(0, this->lastSpell ? 6 : 5);
            }
            else
            {
                this->bonusPending = 1;
                if (g_GameManager.lastSpellState)
                {
                    g_GameManager.lastSpellState = 2;
                }
                this->effect->sprite.interpDuration[0] = 0;
                this->effect->sprite.interpFrame[0] = 30;
                this->effect->sprite.interpEasing = 6;
                this->effect->sprite.moveStart.x = this->effect->radius;
                this->effect->sprite.moveEnd.x = 256.0f;
                this->effect->sprite.moveStart.y = this->effect->radiusY;
                this->effect->sprite.moveEnd.y = 0.0f;
                this->effect->sprite.pos.x = this->effect->radius;
                this->effect->sprite.pos.y = this->effect->radiusY;
                this->effect->sprite.interpDuration[1] = 0;
                this->effect->sprite.interpFrame[1] = 60;
                this->effect->sprite.colorInterpEasing = 3;
                *(ColorBytes *)&this->effect->sprite.colorTween.y = *(ColorBytes *)&this->effect->sprite.color;
                ((BYTE *)&this->effect->sprite.colorTween.z)[2] = 0xd0;
                ((BYTE *)&this->effect->sprite.colorTween.z)[1] = 0x80;
                ((BYTE *)&this->effect->sprite.colorTween.z)[0] = 0xa0;
                ((BYTE *)&this->effect->sprite.colorTween.z)[3] = 0x20;
                this->effect->waveCount = 6.0f;
                this->effect->timer = 0;
                this->captureEffect = this->effect;
                this->effect = NULL;
                g_SoundPlayer.QueueSound(0x23, 0);
            }
        }
        g_Player.bombCooldown = 0x10;
        if (g_Player.playerState == 0)
        {
            g_Player.stateTimer = 0x46;
            g_Player.playerState = 3;
        }
        g_Gui.bombsChanged = 3;
        g_Gui.livesChanged = 3;
        g_SoundPlayer.QueueSound(0xf, 0);
        g_Stage.EndSpellCard();
    }
    if (this->enemy != NULL)
    {
        this->enemy->keepSpellCard = 0;
    }
    this->enemy = NULL;
    this->bonusFrozen = 0;
}

void SpellCard::Abort()
{
    this->active = 0;
    this->EndSpellCardName();
}

void SpellCard::AddBonus(int amount)
{
    if (!this->bonusFrozen)
    {
        this->bonus += amount;
        if (this->bonus >= this->startBonus)
        {
            this->bonus = this->startBonus;
        }
        else
        {
            this->bonusStep += amount / 120;
        }
    }
}

int SpellCard::Update()
{
    if (g_GameManager.deathbombFreeze || g_GameManager.isFrozen)
    {
        return 1;
    }
    if (this->active)
    {
        if (!this->enemy->active || this->enemyIndex != this->enemy->index)
        {
            this->Abort();
        }
        if (g_Player.pos.x >= 64.0f && g_Player.pos.y < 64.0f)
        {
            if (((BYTE *)&this->nameColor)[3] > 0x20)
            {
                ((BYTE *)&this->nameColor)[3] -= 4;
            }
        }
        else if (((BYTE *)&this->nameColor)[3] < 0x80)
        {
            ((BYTE *)&this->nameColor)[3] += 4;
        }
        if (this->capture)
        {
            if (!this->bonusFrozen && !this->enemy->keepSpellCard)
            {
                this->bonus -= (int)(this->bonusStep / 60 * g_App.gameSpeed);
                this->bonus -= this->bonus % 10;
            }
        }
        else if (this->effect->sprite.spriteIndex == 0xdd)
        {
            g_EffectManager.anm->SetSprite(&this->effect->sprite, 0xde);
            this->effect->sprite.scaleTween.z = 4.0f;
            this->effect->sprite.scaleY = 4.0f;
        }
        if (this->effect->waveCount != 0.0f)
        {
            this->effect->radiusY = this->effect->sprite.pos.y;
            if (this->effect->radiusY == 0.0f)
            {
                this->effect->waveCount = 0.0f;
            }
        }
        if (this->effect->sprite.interpFrame[0].AsFrames() == 0)
        {
            this->effect->sprite.interpDuration[0] = 0;
            this->effect->sprite.interpFrame[0] = this->enemy->timerCallbackTime - 100;
            this->effect->sprite.interpEasing = 0;
            this->effect->sprite.moveStart.x = 256.0f;
            this->effect->sprite.moveEnd.x = 8.0f;
            this->effect->sprite.moveStart.y = 0.0f;
            this->effect->sprite.moveEnd.y = 0.0f;
        }
        if (!this->effectFixed)
        {
            this->effect->origin =
                (this->enemy->pos + this->enemy->posOrigin - this->effect->origin) / 16.0f + this->effect->origin;
            this->effect->origin.z = 0.0f;
        }
        this->effect->angle = AddNormalizeAngle(this->effect->angle, this->IsLastSpellActive() ? -0.031415928f : 0.015707964f);
    }
    else if (this->captureEffect != NULL)
    {
        if (this->captureEffect->timer == 30)
        {
            this->captureEffect->sprite.interpDuration[4] = 0;
            this->captureEffect->sprite.interpFrame[4] = 20;
            this->captureEffect->sprite.scaleInterpEasing = 1;
            this->captureEffect->sprite.scaleTween.x = this->captureEffect->ringWidth;
            this->captureEffect->sprite.scaleTween.z = 64.0f;
            this->captureEffect->sprite.interpDuration[0] = 0;
            this->captureEffect->sprite.interpFrame[0] = 100;
            this->captureEffect->sprite.interpEasing = 4;
            this->captureEffect->sprite.moveStart.x = this->captureEffect->radius;
            this->captureEffect->sprite.moveEnd.x = 0.0f;
            this->captureEffect->sprite.moveStart.y = this->captureEffect->radiusY;
            this->captureEffect->sprite.moveEnd.y = 60.0f;
            this->captureEffect->sprite.pos.x = this->captureEffect->radius;
            this->captureEffect->sprite.pos.y = this->captureEffect->radiusY;
        }
        else if (this->captureEffect->timer == 60)
        {
            this->captureEffect->sprite.interpDuration[4] = 0;
            this->captureEffect->sprite.interpFrame[4] = 70;
            this->captureEffect->sprite.scaleInterpEasing = 1;
            this->captureEffect->sprite.scaleTween.x = this->captureEffect->ringWidth;
            this->captureEffect->sprite.scaleTween.z = 0.0f;
        }
        else if (this->captureEffect->timer == 130)
        {
            this->captureEffect->active = 0;
            this->captureEffect = NULL;
            g_Gui.ShowSpellCardBonus(this->capturedBonus);
            g_GameManager.AddScore(this->capturedBonus);
            this->bonusPending = 0;
            if (this->timeOrbBonus > 0)
            {
                g_GameManager.AddTimeOrbs(this->timeOrbBonus);
                g_GameManager.stats->maxPointValue += this->timeOrbBonus * 10;
                this->timeOrbBonus = 0;
            }
        }
        if (this->captureEffect != NULL)
        {
            if (this->captureEffect->timer <= 80)
            {
                this->captureEffect->origin = (g_Player.pos - this->captureEffect->origin) / 16.0f + this->captureEffect->origin;
                this->captureEffect->origin.z = 0.0f;
                this->captureEffect->angle = AddNormalizeAngle(this->captureEffect->angle, -0.015707964f);
            }
            else
            {
                this->captureEffect->origin = (g_Player.pos - this->captureEffect->origin) / 4.0f + this->captureEffect->origin;
                this->captureEffect->origin.z = 0.0f;
                this->captureEffect->angle = AddNormalizeAngle(this->captureEffect->angle, -0.05235988f);
            }
            this->captureEffect->radiusY = this->captureEffect->sprite.pos.y;
            if (this->captureEffect->timer > 8 && this->timeOrbBonus > 0)
            {
                D3DXVECTOR3 itemPos;
                float angle;
                int i;
                int count;

                angle = (this->captureEffect->timer.AsFloat() - 10.0f) * 6.2831855f / 40.0f - 1.5707964f;
                angle = AddNormalizeAngle(angle, 0.0f);
                ((Vector2 *)&itemPos)->SetPolar(angle, 128.0f);
                itemPos += this->captureEffect->origin;
                itemPos.z = 0.0f;
                count = this->timeOrbBonus > 7 ? 7 : this->timeOrbBonus;
                for (i = 0; i < count; i++)
                {
                    g_ItemManager.SpawnItem(&itemPos, 10, 0);
                }
                this->timeOrbBonus -= count;
                angle = AddNormalizeAngle(angle, 3.1415927f);
                ((Vector2 *)&itemPos)->SetPolar(angle, 128.0f);
                itemPos += this->captureEffect->origin;
                itemPos.z = 0.0f;
                count = this->timeOrbBonus > 7 ? 7 : this->timeOrbBonus;
                for (i = 0; i < 6; i++)
                {
                    g_ItemManager.SpawnItem(&itemPos, 10, 0);
                }
                this->timeOrbBonus -= count;
                if (this->timeOrbBonus < 0)
                {
                    this->timeOrbBonus = 0;
                }
            }
        }
    }
    g_SpriteEngine->Step(&this->bombPortrait);
    g_SpriteEngine->Step(&this->bombPortraitGlow);
    g_SpriteEngine->Step(&this->bombPortraitFrame);
    g_SpriteEngine->Step(&this->bombName);
    g_SpriteEngine->Step(&this->bombNameBar);
    g_SpriteEngine->Step(&this->spellPortrait);
    g_SpriteEngine->Step(&this->spellPortraitGlow);
    g_SpriteEngine->Step(&this->spellPortraitFrame);
    g_SpriteEngine->Step(&this->spellName);
    g_SpriteEngine->Step(&this->lastWordName1);
    g_SpriteEngine->Step(&this->lastWordName2);
    g_SpriteEngine->Step(&this->spellNameBar);
    g_SpriteEngine->Step(&this->spellScoreBar);
    this->timeLeft--;
    return 1;
}

int SpellCard::Draw()
{
    if (this->bombPortrait.IsShown())
    {
        g_SpriteEngine->DrawUnrotated(&this->bombPortrait);
        g_SpriteEngine->DrawUnrotated(&this->bombPortraitGlow);
        g_SpriteEngine->Draw(&this->bombPortraitFrame);
    }
    if (this->spellPortrait.IsShown())
    {
        D3DXVECTOR3 savedPos = this->spellPortrait.pos;

        this->spellPortrait.pos += this->spellPortrait.posOffset;
        g_SpriteEngine->DrawUnrotated(&this->spellPortrait);
        this->spellPortrait.pos = savedPos;
        g_SpriteEngine->DrawUnrotated(&this->spellPortraitGlow);
        g_SpriteEngine->Draw(&this->spellPortraitFrame);
    }
    if (this->bombName.IsShown())
    {
        this->bombNameBar.pos = this->bombName.pos;
        this->bombNameBar.pos.x -= 32.0f;
        g_SpriteEngine->DrawUnrotated(&this->bombNameBar);
        g_SpriteEngine->Draw(&this->bombName);
    }
    if (this->spellName.IsShown())
    {
        g_SpriteEngine->SetColorModulation(this->nameColor);
        this->spellNameBar.pos = this->spellName.pos;
        g_SpriteEngine->DrawUnrotated(&this->spellNameBar);
        g_SpriteEngine->Draw(&this->spellName);
        g_SpriteEngine->Draw(&this->lastWordName1);
        g_SpriteEngine->Draw(&this->lastWordName2);
        g_SpriteEngine->DrawUnrotated(&this->spellScoreBar);
        if (!this->nameOnly)
        {
            Catk *spellCatk;
            int n;
            int i;
            int bonus;
            int divisor;
            int started;

            bonus = this->bonus;
            divisor = 10000000;
            started = 0;
            spellCatk = &g_GameManager.catks[this->card];
            if (!this->capture)
            {
                bonus = 0;
            }
            this->spellScoreDigit.pos = this->spellScoreBar.pos;
            this->spellScoreDigit.pos.x -= 40.0f;
            this->spellScoreDigit.pos.y += 1.0f;
            for (i = 0; i < 8; i++)
            {
                n = bonus / divisor;
                if (n != 0)
                {
                    started = 1;
                }
                if (started || divisor == 1)
                {
                    this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n + 0x88);
                    g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
                }
                this->spellScoreDigit.pos.x += 7.0f;
                bonus %= divisor;
                divisor /= 10;
            }
            n = g_GameManager.IsInSpellCardPractice() ? spellCatk->practiceCaptures[g_GameManager.character]
                                                      : spellCatk->captures[g_GameManager.character];
            if (n > 999)
            {
                n = 999;
            }
            this->spellScoreDigit.pos.x += 32.0f;
            started = 0;
            if (n / 100 != 0)
            {
                this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n / 100 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
                n %= 100;
                started = 1;
            }
            this->spellScoreDigit.pos.x += 7.0f;
            if (n / 10 != 0 || started)
            {
                this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n / 10 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
                n %= 10;
            }
            this->spellScoreDigit.pos.x += 7.0f;
            this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n + 0x88);
            g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
            n = g_GameManager.IsInSpellCardPractice() ? spellCatk->practiceAttempts[g_GameManager.character]
                                                      : spellCatk->attempts[g_GameManager.character];
            if (n > 999)
            {
                n = 999;
            }
            this->spellScoreDigit.pos.x += 13.0f;
            if (n / 100 != 0)
            {
                this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n / 100 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
                n %= 100;
                started = 1;
            }
            this->spellScoreDigit.pos.x += 7.0f;
            if (n / 10 != 0 || started)
            {
                this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n / 10 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
                n %= 10;
            }
            this->spellScoreDigit.pos.x += 7.0f;
            this->spellScoreDigit.spriteData = g_AsciiText.anm->GetSprite(n % 10 + 0x88);
            g_SpriteEngine->DrawUnrotated(&this->spellScoreDigit);
        }
        g_SpriteEngine->ResetColorModulation();
    }
    return 1;
}

int SpellCard::RegisterChain()
{
    SpellCard *sc = &g_SpellCard;

    if (sc->Initialize() != 0)
    {
        return -1;
    }
    sc->calcChain = g_Chain.CreateElem((ChainCallback)SpellCard::OnUpdate);
    sc->drawChain = g_Chain.CreateElem((ChainCallback)SpellCard::OnDraw);
    sc->calcChain->deletedCallback = (ChainCallback)SpellCard::DeletedCallback;
    sc->calcChain->arg = sc;
    sc->drawChain->arg = sc;
    g_Chain.AddToCalcChain(sc->calcChain, 0xc);
    g_Chain.AddToDrawChain(sc->drawChain, 0xf);
    return 0;
}

int SpellCard::OnUpdate(SpellCard *sc)
{
    return sc->Update();
}

int SpellCard::OnDraw(SpellCard *sc)
{
    return sc->Draw();
}

int SpellCard::DeletedCallback(SpellCard *sc)
{
    if (!IsRestarting())
    {
        g_SpriteEngine->ReleaseAnm(0x12);
        g_SpriteEngine->ReleaseAnm(0x13);
    }
    if (IsFreeingResources())
    {
        g_SpriteEngine->ReleaseAnm(0xf);
        g_SpriteEngine->ReleaseAnm(0x10);
        g_SpriteEngine->ReleaseAnm(0x11);
    }
    if (sc->calcChain != NULL)
    {
        sc->calcChain->deletedCallback = NULL;
    }
    g_Chain.Remove(sc->drawChain);
    sc->drawChain = NULL;
    return 0;
}

void SpellCard::RemoveChain()
{
    if (g_SpellCard.calcChain != NULL)
    {
        g_Chain.Remove(g_SpellCard.calcChain);
    }
}
