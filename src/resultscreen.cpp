// cflags: /Os -/Oi
// The result screen's object, from about 0x453cd1: built with /Os -/Oi as
// th07's resultscreen.cpp, but without /Ob1 (the ResultScreen and ListNode
// constructors are called). The score file's loading is an /Od object of
// its own after it (scoredat.cpp).

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
