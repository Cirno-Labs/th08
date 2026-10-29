// cflags: /Os -/Oi
// The result screen's object, from about 0x453cd1: built with /Os -/Oi as
// th07's resultscreen.cpp, but without /Ob1 (the ResultScreen and ListNode
// constructors are called). The score file's loading is an /Od object of
// its own after it (scoredat.cpp).

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "lzss.h"
#include "resultscreen.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

char *GetStageName(int stage)
{
    return stage >= 9 ? "Clear" : g_StageNames[stage];
}

char *GetShotName(int shot)
{
    return g_ShotNames[shot];
}

void SaveScoreDat(ResultScreen *screen)
{
    ScoreDat *header;
    BYTE *p24;
    BYTE key26;
    int m;
    BYTE prev3;
    ScoreDat *dat10;
    void *packed7;
    Vrsm vrsm13;
    Pscr *pscr0;
    Catk *catk1;
    Clrd *clrd26;
    int s14;
    ListNode *node;
    int size0 = 0;
    BYTE *encrypted;
    BYTE *buf;
    int n;
    int i;

    buf = (BYTE *)g_MemoryManager.Alloc(0x640000, GLOBAL_H);
    memcpy(buf + size0, screen->scoreDat, sizeof(ScoreDat));
    size0 += sizeof(ScoreDat);
    screen->th8k.magic = 0x4b384854;
    screen->th8k.size2 = sizeof(Th8k);
    screen->th8k.size = sizeof(Th8k);
    screen->th8k.version = 1;
    memcpy(buf + size0, &screen->th8k, sizeof(Th8k));
    size0 += sizeof(Th8k);
    for (i = 0; i < 5; i++)
    {
        for (s14 = 0; s14 < 0xc; s14++)
        {
            node = screen->scores[i][s14].next;
            n = 0;
            while (node != NULL)
            {
                if (node->data->magic == 0x52435348)
                {
                    node->data->shot = s14;
                    node->data->difficulty = i;
                    node->data->size2 = sizeof(Hscr);
                    node->data->size = sizeof(Hscr);
                    node->data->version = 4;
                    node->data->isNew = 0;
                    memcpy(buf + size0, node->data, sizeof(Hscr));
                    size0 += sizeof(Hscr);
                }
                node = node->next;
                n++;
                if (n >= 10)
                {
                    break;
                }
            }
        }
    }
    clrd26 = g_GameManager.clrds;
    for (i = 0; i < 0xd; i++, clrd26++)
    {
        clrd26->magic = 0x44524c43;
        clrd26->size2 = sizeof(Clrd);
        clrd26->size = sizeof(Clrd);
        clrd26->version = 4;
        memcpy(buf + size0, clrd26, sizeof(Clrd));
        size0 += sizeof(Clrd);
    }
    catk1 = g_GameManager.catks;
    for (i = 0; i < 0xde; i++, catk1++)
    {
        if (catk1->magic == 0x4b544143)
        {
            catk1->index = i;
            catk1->size2 = sizeof(Catk);
            catk1->size = sizeof(Catk);
            catk1->version = 3;
            memcpy(buf + size0, catk1, sizeof(Catk));
            size0 += sizeof(Catk);
        }
    }
    pscr0 = g_GameManager.pscrs;
    for (i = 0; i < 0xc; i++, pscr0++)
    {
        if (pscr0->unk_175 != 0)
        {
            memcpy(buf + size0, pscr0, sizeof(Pscr));
            size0 += sizeof(Pscr);
        }
    }
    memcpy(buf + size0, &screen->lsnm, sizeof(Lsnm));
    size0 += sizeof(Lsnm);
    g_GameManager.flsp.magic = 0x50534c46;
    g_GameManager.flsp.version = 1;
    g_GameManager.flsp.size2 = g_GameManager.flsp.size = sizeof(Flsp);
    memcpy(buf + size0, &g_GameManager.flsp, sizeof(Flsp));
    size0 += sizeof(Flsp);
    g_App.AddPlayTime();
    memcpy(buf + size0, &g_GameManager.plst, sizeof(Plst));
    size0 += sizeof(Plst);
    vrsm13.magic = 0x4d535256;
    vrsm13.version = 1;
    vrsm13.size2 = sizeof(Vrsm);
    vrsm13.size = sizeof(Vrsm);
    vrsm13.clearedOnSave = 0;
    strcpy(vrsm13.gameVersion, "0100d");
    vrsm13.exeSize = g_App.exeSize;
    vrsm13.exeChecksum = g_App.exeChecksum;
    memcpy(buf + size0, &vrsm13, sizeof(Vrsm));
    size0 += sizeof(Vrsm);
    dat10 = (ScoreDat *)buf;
    dat10->dataSize = size0 - sizeof(ScoreDat);
    dat10->fileSize = size0;
    packed7 = LzssCompress(buf + sizeof(ScoreDat), dat10->dataSize, &dat10->packedSize);
    memcpy(buf + sizeof(ScoreDat), packed7, dat10->packedSize);
    GlobalFree(packed7);
    size0 = dat10->packedSize + sizeof(ScoreDat);
    header = (ScoreDat *)buf;
    header->dataOffset = sizeof(ScoreDat);
    header->checksum = 0;
    header->key = g_Rng.GetRandomU16InRange(256);
    header->secondaryKey = g_Rng.GetRandomU16InRange(256);
    header->version = 1;
    for (m = 4; m < size0; m++)
    {
        header->checksum += buf[m];
    }
    key26 = 0;
    prev3 = 0;
    p24 = (BYTE *)header + 1;
    m = size0;
    m -= 2;
    key26 = *p24;
    while (m > 0)
    {
        prev3 = p24[1];
        key26 = ((key26 & 0xe0) >> 5) | ((key26 & 0x1f) << 3);
        p24[1] ^= key26;
        key26 += prev3;
        p24++;
        m--;
    }
    encrypted = EncryptData(buf, size0, 0x59, 0x79, 0x100, 0xc00);
    WriteToFile("score.dat", encrypted, size0);
    g_MemoryManager.Free(buf);
    g_MemoryManager.Free(encrypted);
}

char *FormatAndAdvance(char *buf, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    return buf + strlen(buf);
}

int ResultScreen::AddScore(Hscr *entry, int difficulty, int shot)
{
    return InsertScore(&this->scores[difficulty][shot], entry);
}

void ResultScreen::ReleaseScores(int difficulty, int shot)
{
    FreeListNodes(&this->scores[difficulty][shot]);
}

void ResultScreen::SetState(int state)
{
    this->prevState = this->state;
    this->state = state;
    this->unk_0c = state;
    this->subState = 0;
    this->unk_18 = 0;
    this->frame = 0;
    this->unk_54 = 0;
}

int ResultScreen::UpdateScoreShotSelect()
{
    int i;
    Sprite *sprite;

    switch (this->subState)
    {
    case 0:
        if (this->unk_18 == 0)
        {
            this->cursor = this->unk_3c;
            for (i = 0xf; i <= 0x1a; i++)
            {
                this->sprites[i].pendingInterrupt = 0x16;
                g_SpriteEngine->Step(&this->sprites[i]);
                if (i - 0xf == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (this->unk_18 < 6)
        {
            break;
        }
        this->subState++;
        this->unk_18 = 0;
    case 1:
        i = MoveCursorVertical(this, 0xc);
        if (i != 0)
        {
            for (i = 0xf; i <= 0x1a; i++)
            {
                if (i - 0xf == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (WAS_PRESSED(0xa))
        {
            this->SetState(3);
            g_SoundPlayer.QueueSound(0xb, 0);
            this->unk_3c = this->cursor;
            for (i = 0xf; i <= 0x1a; i++)
            {
                this->sprites[i].pendingInterrupt = 1;
            }
            return 1;
        }
        if (WAS_PRESSED(0x1001))
        {
            sprite = this->sprites;
            for (i = 0xf; i <= 0x1a; i++)
            {
                if (i - 0xf == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x17;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
            }
            this->sprites[0x28].pendingInterrupt = 3;
            this->unk_3c = -1;
            this->SetState(5);
            g_SoundPlayer.QueueSound(0xa, 0);
            return 1;
        }
        break;
    }
    this->unk_18++;
    return 0;
}

int ResultScreen::UpdateScoreList()
{
    int prevCursor;

    if (this->unk_3c != this->cursor && this->frame == 10)
    {
        this->unk_3c = this->cursor;
    }
    if (this->frame < 6)
    {
        return 0;
    }
    prevCursor = this->cursor;
    if (MoveCursorHorizontal(this, 0xc) != 0)
    {
        this->frame = 0;
        this->sprites[0x28].pendingInterrupt = this->unk_44 + 3;
        this->sprites[prevCursor + 0xf].pendingInterrupt = 0x18;
        this->sprites[this->cursor + 0xf].pendingInterrupt = 0x19;
    }
    if (WAS_PRESSED(0xa))
    {
        this->unk_3c = this->cursor;
        this->SetState(4);
        this->sprites[0x28].pendingInterrupt = 1;
        g_SoundPlayer.QueueSound(0xb, 0);
        return 1;
    }
    this->unk_18++;
    return 0;
}

int ResultScreen::UpdateSpellDifficultySelect()
{
    int i;
    Sprite *sprite;

    switch (this->subState)
    {
    case 0:
        if (this->unk_18 == 0)
        {
            this->cursor = this->unk_48;
            for (i = 9; i <= 0xe; i++)
            {
                this->sprites[i].pendingInterrupt = 0x16;
                g_SpriteEngine->Step(&this->sprites[i]);
                if (i - 9 == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (this->unk_18 < 6)
        {
            break;
        }
        this->subState++;
        this->unk_18 = 0;
    case 1:
        i = MoveCursorVertical(this, 6);
        if (i != 0)
        {
            for (i = 9; i <= 0xe; i++)
            {
                if (i - 9 == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (WAS_PRESSED(0xa))
        {
            this->SetState(1);
            g_SoundPlayer.QueueSound(0xb, 0);
            this->unk_48 = this->cursor;
            this->cursor = 1;
            return 1;
        }
        if (WAS_PRESSED(0x1001))
        {
            sprite = this->sprites;
            for (i = 9; i <= 0xe; i++)
            {
                if (i - 9 == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x17;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
            }
            this->unk_48 = this->cursor;
            this->SetState(7);
            g_SoundPlayer.QueueSound(0xa, 0);
            return 1;
        }
        break;
    }
    this->unk_18++;
    return 0;
}

int ResultScreen::UpdateSpellShotSelect()
{
    int i;
    Sprite *sprite;

    switch (this->subState)
    {
    case 0:
        if (this->unk_18 == 0)
        {
            this->cursor = this->spellCursor;
            for (i = 0x1b; i <= 0x27; i++)
            {
                this->sprites[i].pendingInterrupt = 0x16;
                g_SpriteEngine->Step(&this->sprites[i]);
                if (i - 0x1b == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (this->unk_18 < 6)
        {
            break;
        }
        this->subState++;
        this->unk_18 = 0;
    case 1:
        i = MoveCursorVertical(this, 0xd);
        if (i != 0)
        {
            for (i = 0x1b; i <= 0x27; i++)
            {
                if (i - 0x1b == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x14;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 0x15;
                }
            }
        }
        if (WAS_PRESSED(0xa))
        {
            this->SetState(6);
            g_SoundPlayer.QueueSound(0xb, 0);
            this->spellCursor = this->cursor;
            for (i = 0x1b; i <= 0x27; i++)
            {
                this->sprites[i].pendingInterrupt = 1;
            }
            return 1;
        }
        if (WAS_PRESSED(0x1001))
        {
            sprite = this->sprites;
            for (i = 0x1b; i <= 0x27; i++)
            {
                if (i - 0x1b == this->cursor)
                {
                    this->sprites[i].pendingInterrupt = 0x17;
                }
                else
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
            }
            this->spellCursor = this->cursor;
            this->SetState(8);
            g_SoundPlayer.QueueSound(0xa, 0);
            this->sprites[0x28].pendingInterrupt = 3;
            this->cursor = 0;
            this->unk_40 = -1;
            return 1;
        }
        break;
    }
    this->unk_18++;
    return 0;
}

int ResultScreen::UpdateSpellList()
{
    int id;
    int i;
    int total;

    if (this->unk_54 != 0 && this->frame >= 10)
    {
        this->SetState(7);
    }
    total = g_SpellCardListSizes[this->unk_48];
    if ((this->unk_40 != this->cursor || this->unk_34 != this->spellCursor) && this->frame == 10)
    {
        this->unk_40 = this->cursor;
        this->unk_34 = this->spellCursor;
        for (i = this->unk_40 * 10; i < this->unk_40 * 10 + 10; i++)
        {
            if (i >= total)
            {
                break;
            }
            id = g_SpellCardLists[this->unk_48][i];
            if (g_GameManager.catks[id].unk_124[12] == 0)
            {
                g_SpriteEngine->RenderText(&this->textSprites[i % 10], 0xffffff, 0, "？？？？？");
            }
            else
            {
                g_SpriteEngine->RenderText(&this->textSprites[i % 10], 0xffffff, 0, g_GameManager.catks[id].name);
            }
            this->textSprites[i % 10].alpha = 0xff;
        }
        g_SpriteEngine->RenderText(&this->textSprites[10], 0xffffff, 0, "%3d/%3d（キャラ切り替え↓↑）",
                                   this->capturedSpellCounts[this->unk_48][this->spellCursor], total);
        this->textSprites[10].alpha = 0xff;
    }
    if (this->frame < 6)
    {
        return 0;
    }
    if (MoveCursorHorizontal(this, (total + 9) / 10) != 0)
    {
        this->frame = 0;
        this->sprites[0x28].pendingInterrupt = 10;
    }
    else if (LoadScreenPlstChunk(this, 0xd) != 0)
    {
        this->frame = 0;
        this->unk_38 = 1;
        this->sprites[this->unk_34 + 0x1b].pendingInterrupt = 0x18;
        this->sprites[this->spellCursor + 0x1b].pendingInterrupt = 0x19;
    }
    if (WAS_PRESSED(0xa))
    {
        this->unk_54 = 1;
        this->frame = 0;
        g_SoundPlayer.QueueSound(0xb, 0);
        this->sprites[0x28].pendingInterrupt = 1;
        return 1;
    }
    this->unk_18++;
    return 0;
}

void FormatTodayDate(char *date)
{
    time_t t;
    struct tm *tm;

    time(&t);
    tm = localtime(&t);
    strftime(date, 6, "%m/%d", tm);
}

int ResultScreen::WaitAfterNameEntry()
{
    Sprite *sprite;

    switch (this->state)
    {
    case 0xf:
        if (this->frame <= 30)
        {
            sprite = &this->sprites[0x47];
            sprite->pendingInterrupt = 0x12;
        }
        if (this->frame >= 90 && WAS_PRESSED(0x1001))
        {
            sprite = &this->sprites[0x47];
            sprite->pendingInterrupt = 2;
            this->frame = 0;
            this->state = 0x10;
        }
        break;
    case 0x10:
        if (this->frame >= 30)
        {
            this->frame = 9;
            this->state = 0xa;
        }
        break;
    }
    return 0;
}

int ResultScreen::RegisterChain(int mode)
{
    ResultScreen *screen = (ResultScreen *)g_MemoryManager.Register(new ResultScreen, sizeof(ResultScreen),
                                                                    "ResultSysInf");

    g_Unknown18b8a2c = 0;
    DebugPrint2("Stg.PlayTimeAll = %d\r\n", g_GameManager.progressFrames);
    if (mode == 1)
    {
        if (!g_GameManager.IsInPractice())
        {
            screen->state = 9;
        }
        else if (g_GameManager.unk_3dbac >> 14 & 1)
        {
            screen->state = 0x16;
        }
        else
        {
            screen->state = 0x11;
        }
    }
    else if (mode == 2)
    {
        screen->state = 0x12;
        ResultScreen::AddedCallback(screen);
        return 0;
    }
    screen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    screen->calcChain->addedCallback = (ChainCallback)ResultScreen::AddedCallback;
    screen->calcChain->deletedCallback = (ChainCallback)ResultScreen::DeletedCallback;
    screen->calcChain->arg = screen;
    if (g_Chain.AddToCalcChain(screen->calcChain, 0x10) != 0)
    {
        return -1;
    }
    screen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    screen->drawChain->arg = screen;
    g_Chain.AddToDrawChain(screen->drawChain, 0x12);
    return 0;
}

int ResultScreen::OnUpdate(ResultScreen *screen)
{
    Sprite *vm;
    int i;
    Sprite *sprite0;

    switch (screen->state)
    {
    case 0x11:
        g_App.scene = 1;
        return 0;
    case 0x16:
        g_App.scene = 1;
        return 0;
    case 0x12:
        return 0;
    case 0:
    menu:
        screen->SetState(1);
    case 1:
        screen->UpdateMenu();
        break;
    case 2:
        if (screen->frame == 1)
        {
            D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);

            g_App.StartLoadingSprites(&pos);
            sprite0 = screen->sprites;
            for (i = 0; i < 0x48; i++, sprite0++)
            {
                sprite0->pendingInterrupt = 2;
            }
        }
        if (screen->frame < 20)
        {
            break;
        }
        g_App.scene = 1;
        return 0;
    case 4:
        screen->UpdateScoreShotSelect();
        break;
    case 3:
        screen->UpdateScoreDifficultySelect();
        break;
    case 5:
        screen->UpdateScoreList();
        break;
    case 7:
        screen->UpdateSpellShotSelect();
        break;
    case 6:
        screen->UpdateSpellDifficultySelect();
        break;
    case 8:
        screen->UpdateSpellList();
        break;
    case 9:
        screen->HandleNameEntry();
        break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
        screen->HandleReplaySaving();
        break;
    case 15:
    case 16:
        screen->WaitAfterNameEntry();
        break;
    case 0x13:
    case 0x14:
    case 0x15:
        if (screen->RunStatisticsMenu())
        {
            goto menu;
        }
        break;
    }
    vm = screen->sprites;
    for (i = 0; i < 0x48; i++, vm++)
    {
        g_SpriteEngine->Step(vm);
    }
    screen->frame++;
    return 1;
}

int ResultScreen::DeletedCallback(ResultScreen *screen)
{
    int i;
    int j;

    if (screen->scoreDat != NULL)
    {
        SaveScoreDat(screen);
        ReleaseScoreDat(screen->scoreDat);
    }
    screen->scoreDat = NULL;
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 0xc; j++)
        {
            screen->ReleaseScores(i, j);
        }
    }
    g_SpriteEngine->ReleaseAnm(0x15);
    g_SpriteEngine->ReleaseAnm(0x16);
    g_SpriteEngine->MoveSurface(8, 0);
    g_Chain.Remove(screen->drawChain);
    screen->drawChain = NULL;
    g_MemoryManager.Unregister(screen);
    delete screen;
    screen = NULL;
    return 0;
}

int MoveCursorVertical(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int LoadScreenPlstChunk(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x10))
    {
        screen->spellCursor--;
        if (screen->spellCursor < 0)
        {
            screen->spellCursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x20))
    {
        screen->spellCursor++;
        if (screen->spellCursor >= count)
        {
            screen->spellCursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}

int MoveCursorHorizontal(ResultScreen *screen, int count)
{
    if (WAS_PRESSED_OR_HELD(0x40))
    {
        screen->cursor--;
        if (screen->cursor < 0)
        {
            screen->cursor += count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return -1;
    }
    if (WAS_PRESSED_OR_HELD(0x80))
    {
        screen->cursor++;
        if (screen->cursor >= count)
        {
            screen->cursor -= count;
        }
        g_SoundPlayer.QueueSound(0xc, 0);
        return 1;
    }
    return 0;
}
