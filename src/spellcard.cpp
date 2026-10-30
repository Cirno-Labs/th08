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

void SpellCard::Start(int card, char *name, int face, DWORD bonus, Enemy *enemy, char *text1, char *text2, char *text3)
{
    int i;
    int sum;
    int k;
    Catk *catk;
    int nameSum;

    this->unk_0_9 = 0;
    this->unk_0_0 = 1;
    this->unk_0_2 = 1;
    this->unk_0_4 = 0;
    this->unk_0_3 = 0;
    this->unk_0_6 = 0;
    this->unk_0_7 = 0;
    this->unk_0_10 = 0;
    this->card = card;
    this->enemy = enemy;
    this->enemyIndex = this->enemy->index;
    this->unk_fc = bonus;
    this->unk_2638 = bonus;
    if (this->enemy->keepSpellCard)
    {
        this->unk_2638 = 99999990;
    }
    this->unk_100 = (this->unk_fc - this->unk_fc / 7) / (this->enemy->timerCallbackTime / 60);
    this->unk_108 = this->enemy->timerCallbackTime;
    this->unk_114 = this->enemy->timerCallbackTime;
    for (i = 0; i < sizeof(this->name); i++)
    {
        this->name[i] = (BYTE)name[i] ^ 0xaa;
    }
    this->ShowName(face, this->name, 0);
    g_BulletManager.ClearAllToItems();
    g_Stage.StartSpellCard();
    for (i = 0; i < g_Stage.dialogueSpriteCount; i++)
    {
        g_Unknown577eb8->StartScript(&g_Stage.dialogueSprites[i], i + g_Stage.dialogueScriptOffset);
    }
    g_Stage.unk_625c = NULL;
    this->enemy->ResetRankSpeeds();
    this->unk_2620 = 0x80808080;
    if (this->unk_0_8)
    {
        this->unk_f8 = NULL;
        g_Gui.ShowSpellCardBonus(this->unk_104);
        g_GameManager.AddScore(this->unk_104);
        this->unk_0_8 = 0;
        if (this->unk_10 > 0)
        {
            g_GameManager.AddTimeOrbs(this->unk_10);
            this->unk_10 = 0;
        }
    }
    this->unk_0_11 = 0;
    this->effect = g_EffectManager.SpawnReservedEffect(g_GameManager.unk_3dbac_7 ? 0x34 : 0x27, &this->enemy->pos, 1, 1, 0xffffffff);
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
    this->unk_0_5 = g_GameManager.unk_3dbac_7;
    g_Gui.bombsChanged = 3;
    g_Gui.livesChanged = 3;
    this->unk_0_6 = 0;
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
            memcpy(this->unk_74, text2, sizeof(this->unk_74));
            memcpy(this->unk_b4, text3, sizeof(this->unk_b4));
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

void SpellCard::Func415d10(char *name, Enemy *enemy)
{
    this->unk_0_10 = 1;
    this->unk_0_0 = 0;
    this->unk_0_4 = 0;
    this->ShowName(-1, name, 1);
}

void SpellCard::StartBombCard(int face, char *name, int sprite)
{
    if (face == 0)
    {
        this->faceAnms[0]->StartScript(&this->unk_120, 0);
        this->faceAnms[0]->SetSprite(&this->unk_120, 0);
    }
    else if (face == 1)
    {
        this->faceAnms[1]->StartScript(&this->unk_120, 0);
        this->faceAnms[1]->SetSprite(&this->unk_120, 0);
    }
    this->backgroundAnm->StartScript(&this->unk_668, 0);
    this->backgroundAnm->StartScript(&this->unk_bb0, 2);
    this->backgroundAnm->SetSprite(&this->unk_668, sprite);
    this->backgroundAnm->SetSprite(&this->unk_bb0, sprite);
    g_App.textAnm->StartScript(&this->unk_10f8, 4);
    g_SpriteEngine->RenderText(&this->unk_10f8, 0xf0f0ff, 0, name);
    this->unk_2618 = (float)(strlen(name) * 15) / 2.0f + 16.0f;
    this->unk_1b88.SetPendingInterrupt(1);
    g_SoundPlayer.QueueSound(0xe, 0);
    g_App.redrawFrames = 2;
}

void SpellCard::ShowName(int face, char *name, int sprite)
{
    if (face >= 0)
    {
        this->faceAnms[2]->StartScript(&this->unk_3c4, 0);
        this->faceAnms[2]->SetSprite(&this->unk_3c4, face);
    }
    this->backgroundAnm->StartScript(&this->unk_668, 1);
    this->backgroundAnm->SetSprite(&this->unk_668, sprite);
    this->backgroundAnm->StartScript(&this->unk_bb0, 3);
    this->backgroundAnm->SetSprite(&this->unk_bb0, sprite);
    if (g_GameManager.IsPracticingSpellCardIn(0xcd, 0xdd))
    {
        g_App.textAnm->StartScript(&this->unk_139c, 6);
        g_App.textAnm->StartScript(&this->unk_1640, 7);
        g_App.textAnm->StartScript(&this->unk_18e4, 8);
        g_SpriteEngine->RenderTextRightAligned(&this->unk_1640, 0xfff0f0, 0, name);
        g_SpriteEngine->RenderTextRightAligned(&this->unk_18e4, 0xfff0f0, 0, name);
    }
    else
    {
        g_App.textAnm->StartScript(&this->unk_139c, 5);
    }
    g_SpriteEngine->RenderTextRightAligned(&this->unk_139c, 0xfff0f0, 0, name);
    this->unk_261c = (float)(strlen(name) * 15) / 2.0f + 16.0f;
    this->unk_1e2c.SetPendingInterrupt(1);
    if (!this->unk_0_10)
    {
        this->unk_2374.SetPendingInterrupt(1);
    }
    g_SoundPlayer.QueueSound(0xe, 0);
    g_App.redrawFrames = 2;
}

void SpellCard::Func416160()
{
    this->unk_139c.pendingInterrupt = 1;
    this->unk_1e2c.SetPendingInterrupt(2);
    if (!this->unk_0_10)
    {
        this->unk_2374.SetPendingInterrupt(2);
    }
}

void SpellCard::End()
{
    int k;
    Catk *catk;
    int captured;
    int bonus;

    g_Unknown4e3d28 = 0;
    if (this->unk_0_0)
    {
        captured = FALSE;
        this->unk_0_0 = 0;
        this->Func416160();
        if (!this->unk_0_3)
        {
            bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
            bonus = g_EnemyManager.ClearEnemiesForBonus(8000, bonus);
            if (bonus != 0)
            {
                g_GameManager.AddScore(bonus);
                g_Gui.ShowBonusPopup(bonus);
            }
            if (this->unk_0_2)
            {
                catk = &g_GameManager.catks[this->card];
                this->unk_104 = this->unk_fc;
                if (this->enemy->keepSpellCard)
                {
                    this->unk_10 = 700;
                }
                else
                {
                    k = this->unk_114.AsFrames() - this->unk_114.AsFrames() / 7;
                    if (this->unk_108.AsFrames() >= k)
                    {
                        this->unk_10 = 1000;
                    }
                    else if (this->unk_108.AsFrames() >= 180)
                    {
                        this->unk_10 = (this->unk_108.AsFrames() - 180) * 900 / (k - 180) + 100;
                    }
                    else
                    {
                        this->unk_10 = 100;
                    }
                }
                this->unk_0_9 = 1;
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
                        if (catk->maxBonus[shot] < this->unk_fc)
                        {
                            catk->maxBonus[shot] = this->unk_fc;
                        }
                        if (catk->maxBonus[12] < this->unk_fc)
                        {
                            catk->maxBonus[12] = this->unk_fc;
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
                        for (k = 0; k < sizeof(catk->unk_70); k++)
                        {
                            catk->unk_70[k] = (BYTE)this->unk_74[k] ^ 0xdd;
                        }
                        for (k = 0; k < sizeof(catk->unk_b0); k++)
                        {
                            catk->unk_b0[k] = (BYTE)this->unk_b4[k] ^ 0xee;
                        }
                        shot = g_GameManager.character;
                        if (catk->practiceMaxBonus[shot] < this->unk_fc)
                        {
                            catk->practiceMaxBonus[shot] = this->unk_fc;
                        }
                        if (catk->practiceMaxBonus[12] < this->unk_fc)
                        {
                            catk->practiceMaxBonus[12] = this->unk_fc;
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
                if (g_GameManager.unk_3dbac_7)
                {
                    g_GameManager.unk_3dbac_7 = 0;
                }
                this->effect->active = 0;
                this->effect = NULL;
                g_Gui.ShowFullPowerPopup(0, this->unk_0_5 ? 6 : 5);
            }
            else
            {
                this->unk_0_8 = 1;
                if (g_GameManager.unk_3dbac_7)
                {
                    g_GameManager.unk_3dbac_7 = 2;
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
                this->unk_f8 = this->effect;
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
    this->unk_0_11 = 0;
}

void SpellCard::Func416af0()
{
    this->unk_0_0 = 0;
    this->Func416160();
}

void SpellCard::Func416b10(int amount)
{
    if (!this->unk_0_11)
    {
        this->unk_fc += amount;
        if (this->unk_fc >= this->unk_2638)
        {
            this->unk_fc = this->unk_2638;
        }
        else
        {
            this->unk_100 += amount / 120;
        }
    }
}

int SpellCard::Update()
{
    if (g_GameManager.unk_3dbac_10 || g_GameManager.isFrozen)
    {
        return 1;
    }
    if (this->unk_0_0)
    {
        if (!this->enemy->active || this->enemyIndex != this->enemy->index)
        {
            this->Func416af0();
        }
        if (g_Player.pos.x >= 64.0f && g_Player.pos.y < 64.0f)
        {
            if (((BYTE *)&this->unk_2620)[3] > 0x20)
            {
                ((BYTE *)&this->unk_2620)[3] -= 4;
            }
        }
        else if (((BYTE *)&this->unk_2620)[3] < 0x80)
        {
            ((BYTE *)&this->unk_2620)[3] += 4;
        }
        if (this->unk_0_2)
        {
            if (!this->unk_0_11 && !this->enemy->keepSpellCard)
            {
                this->unk_fc -= (int)(this->unk_100 / 60 * g_App.gameSpeed);
                this->unk_fc -= this->unk_fc % 10;
            }
        }
        else if (this->effect->sprite.spriteIndex == 0xdd)
        {
            g_EtamaAnm->SetSprite(&this->effect->sprite, 0xde);
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
        if (!this->unk_0_6)
        {
            this->effect->origin =
                (this->enemy->pos + this->enemy->posOrigin - this->effect->origin) / 16.0f + this->effect->origin;
            this->effect->origin.z = 0.0f;
        }
        this->effect->angle = AddNormalizeAngle(this->effect->angle, this->Func417860() ? -0.031415928f : 0.015707964f);
    }
    else if (this->unk_f8 != NULL)
    {
        if (this->unk_f8->timer == 30)
        {
            this->unk_f8->sprite.interpDuration[4] = 0;
            this->unk_f8->sprite.interpFrame[4] = 20;
            this->unk_f8->sprite.scaleInterpEasing = 1;
            this->unk_f8->sprite.scaleTween.x = this->unk_f8->ringWidth;
            this->unk_f8->sprite.scaleTween.z = 64.0f;
            this->unk_f8->sprite.interpDuration[0] = 0;
            this->unk_f8->sprite.interpFrame[0] = 100;
            this->unk_f8->sprite.interpEasing = 4;
            this->unk_f8->sprite.moveStart.x = this->unk_f8->radius;
            this->unk_f8->sprite.moveEnd.x = 0.0f;
            this->unk_f8->sprite.moveStart.y = this->unk_f8->radiusY;
            this->unk_f8->sprite.moveEnd.y = 60.0f;
            this->unk_f8->sprite.pos.x = this->unk_f8->radius;
            this->unk_f8->sprite.pos.y = this->unk_f8->radiusY;
        }
        else if (this->unk_f8->timer == 60)
        {
            this->unk_f8->sprite.interpDuration[4] = 0;
            this->unk_f8->sprite.interpFrame[4] = 70;
            this->unk_f8->sprite.scaleInterpEasing = 1;
            this->unk_f8->sprite.scaleTween.x = this->unk_f8->ringWidth;
            this->unk_f8->sprite.scaleTween.z = 0.0f;
        }
        else if (this->unk_f8->timer == 130)
        {
            this->unk_f8->active = 0;
            this->unk_f8 = NULL;
            g_Gui.ShowSpellCardBonus(this->unk_104);
            g_GameManager.AddScore(this->unk_104);
            this->unk_0_8 = 0;
            if (this->unk_10 > 0)
            {
                g_GameManager.AddTimeOrbs(this->unk_10);
                g_GameManager.stats->unk_24 += this->unk_10 * 10;
                this->unk_10 = 0;
            }
        }
        if (this->unk_f8 != NULL)
        {
            if (this->unk_f8->timer <= 80)
            {
                this->unk_f8->origin = (g_Player.pos - this->unk_f8->origin) / 16.0f + this->unk_f8->origin;
                this->unk_f8->origin.z = 0.0f;
                this->unk_f8->angle = AddNormalizeAngle(this->unk_f8->angle, -0.015707964f);
            }
            else
            {
                this->unk_f8->origin = (g_Player.pos - this->unk_f8->origin) / 4.0f + this->unk_f8->origin;
                this->unk_f8->origin.z = 0.0f;
                this->unk_f8->angle = AddNormalizeAngle(this->unk_f8->angle, -0.05235988f);
            }
            this->unk_f8->radiusY = this->unk_f8->sprite.pos.y;
            if (this->unk_f8->timer > 8 && this->unk_10 > 0)
            {
                D3DXVECTOR3 itemPos;
                float angle;
                int i;
                int count;

                angle = (this->unk_f8->timer.AsFloat() - 10.0f) * 6.2831855f / 40.0f - 1.5707964f;
                angle = AddNormalizeAngle(angle, 0.0f);
                ((Vector2 *)&itemPos)->SetPolar(angle, 128.0f);
                itemPos += this->unk_f8->origin;
                itemPos.z = 0.0f;
                count = this->unk_10 > 7 ? 7 : this->unk_10;
                for (i = 0; i < count; i++)
                {
                    g_ItemManager.SpawnItem(&itemPos, 10, 0);
                }
                this->unk_10 -= count;
                angle = AddNormalizeAngle(angle, 3.1415927f);
                ((Vector2 *)&itemPos)->SetPolar(angle, 128.0f);
                itemPos += this->unk_f8->origin;
                itemPos.z = 0.0f;
                count = this->unk_10 > 7 ? 7 : this->unk_10;
                for (i = 0; i < 6; i++)
                {
                    g_ItemManager.SpawnItem(&itemPos, 10, 0);
                }
                this->unk_10 -= count;
                if (this->unk_10 < 0)
                {
                    this->unk_10 = 0;
                }
            }
        }
    }
    g_SpriteEngine->Step(&this->unk_120);
    g_SpriteEngine->Step(&this->unk_668);
    g_SpriteEngine->Step(&this->unk_bb0);
    g_SpriteEngine->Step(&this->unk_10f8);
    g_SpriteEngine->Step(&this->unk_1b88);
    g_SpriteEngine->Step(&this->unk_3c4);
    g_SpriteEngine->Step(&this->unk_90c);
    g_SpriteEngine->Step(&this->unk_e54);
    g_SpriteEngine->Step(&this->unk_139c);
    g_SpriteEngine->Step(&this->unk_1640);
    g_SpriteEngine->Step(&this->unk_18e4);
    g_SpriteEngine->Step(&this->unk_1e2c);
    g_SpriteEngine->Step(&this->unk_2374);
    this->unk_108--;
    return 1;
}

int SpellCard::Draw()
{
    if (this->unk_120.IsShown())
    {
        g_SpriteEngine->DrawUnrotated(&this->unk_120);
        g_SpriteEngine->DrawUnrotated(&this->unk_668);
        g_SpriteEngine->Draw(&this->unk_bb0);
    }
    if (this->unk_3c4.IsShown())
    {
        D3DXVECTOR3 savedPos = this->unk_3c4.pos;

        this->unk_3c4.pos += this->unk_3c4.posOffset;
        g_SpriteEngine->DrawUnrotated(&this->unk_3c4);
        this->unk_3c4.pos = savedPos;
        g_SpriteEngine->DrawUnrotated(&this->unk_90c);
        g_SpriteEngine->Draw(&this->unk_e54);
    }
    if (this->unk_10f8.IsShown())
    {
        this->unk_1b88.pos = this->unk_10f8.pos;
        this->unk_1b88.pos.x -= 32.0f;
        g_SpriteEngine->DrawUnrotated(&this->unk_1b88);
        g_SpriteEngine->Draw(&this->unk_10f8);
    }
    if (this->unk_139c.IsShown())
    {
        g_SpriteEngine->SetColorModulation(this->unk_2620);
        this->unk_1e2c.pos = this->unk_139c.pos;
        g_SpriteEngine->DrawUnrotated(&this->unk_1e2c);
        g_SpriteEngine->Draw(&this->unk_139c);
        g_SpriteEngine->Draw(&this->unk_1640);
        g_SpriteEngine->Draw(&this->unk_18e4);
        g_SpriteEngine->DrawUnrotated(&this->unk_2374);
        if (!this->unk_0_10)
        {
            Catk *spellCatk;
            int n;
            int i;
            int bonus;
            int divisor;
            int started;

            bonus = this->unk_fc;
            divisor = 10000000;
            started = 0;
            spellCatk = &g_GameManager.catks[this->card];
            if (!this->unk_0_2)
            {
                bonus = 0;
            }
            this->unk_20d0.pos = this->unk_2374.pos;
            this->unk_20d0.pos.x -= 40.0f;
            this->unk_20d0.pos.y += 1.0f;
            for (i = 0; i < 8; i++)
            {
                n = bonus / divisor;
                if (n != 0)
                {
                    started = 1;
                }
                if (started || divisor == 1)
                {
                    this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n + 0x88);
                    g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
                }
                this->unk_20d0.pos.x += 7.0f;
                bonus %= divisor;
                divisor /= 10;
            }
            n = g_GameManager.IsInSpellCardPractice() ? spellCatk->practiceCaptures[g_GameManager.character]
                                                      : spellCatk->captures[g_GameManager.character];
            if (n > 999)
            {
                n = 999;
            }
            this->unk_20d0.pos.x += 32.0f;
            started = 0;
            if (n / 100 != 0)
            {
                this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n / 100 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
                n %= 100;
                started = 1;
            }
            this->unk_20d0.pos.x += 7.0f;
            if (n / 10 != 0 || started)
            {
                this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n / 10 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
                n %= 10;
            }
            this->unk_20d0.pos.x += 7.0f;
            this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n + 0x88);
            g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
            n = g_GameManager.IsInSpellCardPractice() ? spellCatk->practiceAttempts[g_GameManager.character]
                                                      : spellCatk->attempts[g_GameManager.character];
            if (n > 999)
            {
                n = 999;
            }
            this->unk_20d0.pos.x += 13.0f;
            if (n / 100 != 0)
            {
                this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n / 100 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
                n %= 100;
                started = 1;
            }
            this->unk_20d0.pos.x += 7.0f;
            if (n / 10 != 0 || started)
            {
                this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n / 10 + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
                n %= 10;
            }
            this->unk_20d0.pos.x += 7.0f;
            this->unk_20d0.spriteData = g_AsciiText.anm->GetSprite(n % 10 + 0x88);
            g_SpriteEngine->DrawUnrotated(&this->unk_20d0);
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
