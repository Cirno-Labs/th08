// The spell card manager, from 0x4143e0. New in th08: th07 ran spell
// cards from its enemy code.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "effect.h"
#include "enemy.h"
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
        this->unk_f8 = 0;
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
    this->effect->unk_334 = 6.0f;
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
            sum += catk->unk_18c[k];
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
                catk->unk_18c[k] = 0;
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
            nameSum += catk->unk_18c[k];
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
