// The score file ("score.dat"): loading it and reading its chunks. th07
// has these at the start of its /Os resultscreen.cpp; th08 compiles them
// as an /Od object of their own, right after the result screen's.

#include <string.h>

#include "lzss.h"
#include "resultscreen.h"
#include "unsorted.h"

int InsertScore(ListNode *head, Hscr *entry)
{
    int index = 0;
    ListNode *next;

    while (head->next != NULL)
    {
        if (head->next->data != NULL && head->next->data->score <= entry->score)
        {
            break;
        }
        head = head->next;
        index++;
    }
    next = head->next;
    head->next = (ListNode *)g_MemoryManager.Alloc(sizeof(ListNode), "result");
    head->next->prev = head;
    head = head->next;
    head->data = entry;
    head->next = next;
    return index;
}

void FreeListNodes(ListNode *head)
{
    ListNode *next;

    head = head->next;
    while (head != NULL)
    {
        next = head->next;
        g_MemoryManager.Free(head);
        head = next;
    }
}

ScoreDat *LoadScoreDat(char *path)
{
    Vrsm *vrsm2;
    BOOL found2;
    Th8k *th8k1;
    int left2;
    Th8k *chapterHeader;
    int remaining0;
    BYTE *p2;
    WORD checksumVal0;
    BYTE key;
    int n;
    BYTE *decoded;
    ScoreDat *dat;
    int readSize;
    ScoreDat *unpacked;

    DebugPrint("info : score load\r\n");
    dat = (ScoreDat *)LoadFileData(path, &readSize, 1);
    if (dat == NULL)
    {
    fail:
        DebugPrint("info : score recreate\r\n");
        if (dat != NULL)
        {
            g_MemoryManager.Free(dat);
        }
        dat = (ScoreDat *)g_MemoryManager.Alloc(sizeof(ScoreDat), "scorefile");
        dat->dataOffset = sizeof(ScoreDat);
        dat->fileSize = sizeof(ScoreDat);
        goto done;
    }
    if (readSize < sizeof(ScoreDat))
    {
        DebugPrint("warning : score.dat size is short\r\n");
        g_MemoryManager.Free(dat);
        goto fail;
    }
    decoded = DecryptData((BYTE *)dat, readSize, 0x59, 0x79, 0x100, 0xc00);
    g_MemoryManager.Free(dat);
    dat = (ScoreDat *)decoded;
    remaining0 = readSize - 2;
    checksumVal0 = 0;
    key = 0;
    n = 0;
    p2 = (BYTE *)dat + 1;
    while (remaining0 > 0)
    {
        key += *p2;
        key = ((key & 0xe0) >> 5) | ((key & 0x1f) << 3);
        p2[1] ^= key;
        if (n >= 2)
        {
            checksumVal0 += p2[1];
        }
        p2++;
        remaining0--;
        n++;
    }
    if (dat->checksum != checksumVal0)
    {
        DebugPrint("warning : score.dat chksum error\r\n");
        goto fail;
    }
    if (dat->dataOffset != sizeof(ScoreDat))
    {
        DebugPrint("warning : header size is mismatch\r\n");
        goto fail;
    }
    if (dat->version != 1)
    {
        DebugPrint("warning : score.dat version mismatch\r\n");
        goto fail;
    }
    unpacked = (ScoreDat *)g_MemoryManager.Alloc(sizeof(ScoreDat) + 0xa0000, "scorefile2");
    memcpy(unpacked, dat, sizeof(ScoreDat));
    LzssDecompress((BYTE *)dat + sizeof(ScoreDat), dat->packedSize, (BYTE *)unpacked + sizeof(ScoreDat),
                   dat->dataSize);
    g_MemoryManager.Free(dat);
    dat = unpacked;
    left2 = dat->fileSize;
    found2 = FALSE;
    chapterHeader = (Th8k *)((BYTE *)dat + dat->dataOffset);
    left2 -= dat->dataOffset;
    while (left2 > 0)
    {
        if (chapterHeader->magic == 0x4b384854)
        {
            found2 = TRUE;
            th8k1 = chapterHeader;
        }
        if (chapterHeader->magic == 0x4d535256 && chapterHeader->version == 1)
        {
            vrsm2 = (Vrsm *)chapterHeader;
            if (g_App.CheckVersionFile(vrsm2->gameVersion, vrsm2->exeSize, vrsm2->exeChecksum) != 0)
            {
                DebugPrint("warning : score.dat exesumcheck error\r\n");
                goto fail;
            }
        }
        if (chapterHeader->size == 0)
        {
            DebugPrint("warning : score.dat chapter size is ZERO\r\n");
            goto fail;
        }
        left2 -= chapterHeader->size;
        chapterHeader = (Th8k *)((BYTE *)chapterHeader + chapterHeader->size);
    }
    if (!found2 || th8k1->version != 1)
    {
        DebugPrint("warning : score.dat version mismatch\r\n");
        goto fail;
    }
done:
    dat->scores = (ListNode *)g_MemoryManager.Alloc(sizeof(ListNode), "result");
    dat->scores->next = NULL;
    dat->scores->data = NULL;
    dat->scores->prev = NULL;
    return dat;
}

DWORD LoadHighScores(void *scoreDat, ListNode *list, int shot, int difficulty, BYTE *bestContinues)
{
    Hscr *chunk;
    int left;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    if (list == NULL)
    {
        FreeListNodes(dat1->scores);
        dat1->scores->next = NULL;
        dat1->scores->data = NULL;
        dat1->scores->prev = NULL;
    }
    left = dat1->fileSize;
    chunk = (Hscr *)((BYTE *)dat1 + dat1->dataOffset);
    left -= dat1->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x52435348 && chunk->version == 4 && chunk->shot == shot &&
            chunk->difficulty == difficulty)
        {
            if (list != NULL)
            {
                InsertScore(list, chunk);
            }
            else
            {
                InsertScore(dat1->scores, chunk);
            }
        }
        left -= chunk->size;
        chunk = (Hscr *)((BYTE *)chunk + chunk->size);
    }
    if (bestContinues != NULL)
    {
        *bestContinues = dat1->scores->next != NULL ? (char)dat1->scores->next->data->continuesUsed : 0;
    }
    return dat1->scores->next != NULL
               ? (dat1->scores->next->data->score > 100000 ? dat1->scores->next->data->score : 100000)
               : 100000;
}

int LoadSpellCards(void *scoreDat, Catk *catks)
{
    Catk *chunk;
    int left;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    if (catks == NULL)
    {
        return -1;
    }
    chunk = (Catk *)((BYTE *)dat1 + dat1->dataOffset);
    left = dat1->fileSize - dat1->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4b544143 && chunk->version == 3)
        {
            if (chunk->index >= 0xde)
            {
                break;
            }
            catks[chunk->index] = *chunk;
        }
        left -= chunk->size;
        chunk = (Catk *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadLsnmChunk(void *scoreDat, Lsnm *lsnm)
{
    Lsnm *chunk;
    int left;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    chunk = (Lsnm *)((BYTE *)dat1 + dat1->dataOffset);
    left = dat1->fileSize - dat1->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4d4e534c && chunk->version == 1)
        {
            *lsnm = *chunk;
            return 1;
        }
        left -= chunk->size;
        chunk = (Lsnm *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadFlspChunk(void *scoreDat, Flsp *flsp)
{
    Flsp *chunk;
    int left;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    chunk = (Flsp *)((BYTE *)dat1 + dat1->dataOffset);
    left = dat1->fileSize - dat1->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x50534c46 && chunk->version == 1)
        {
            *flsp = *chunk;
            return 1;
        }
        left -= chunk->size;
        chunk = (Flsp *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadClears(void *scoreDat, Clrd *clrds)
{
    Clrd *p;
    int i;
    int left;
    int k;
    ScoreDat *dat1 = (ScoreDat *)scoreDat;

    if (clrds == NULL)
    {
        return -1;
    }
    for (i = 0; i < 13; i++)
    {
        memset(&clrds[i], 0, sizeof(Clrd));
        clrds[i].magic = 0x44524c43;
        clrds[i].size2 = sizeof(Clrd);
        clrds[i].size = sizeof(Clrd);
        clrds[i].version = 4;
        clrds[i].shot = i;
        for (k = 0; k < 5; k++)
        {
            clrds[i].stagesClearedNoContinue[k] = 1;
            clrds[i].stagesCleared[k] = 1;
        }
    }
    p = (Clrd *)((BYTE *)dat1 + dat1->dataOffset);
    left = dat1->fileSize - dat1->dataOffset;
    while (left > 0)
    {
        if (p->magic == 0x44524c43 && p->version == 4)
        {
            if (p->shot >= 13)
            {
                break;
            }
            clrds[p->shot] = *p;
        }
        left -= p->size;
        p = (Clrd *)((BYTE *)p + p->size);
    }
    return 0;
}

int LoadPracticeScores(void *scoreDat, Pscr *pscrs)
{
    Pscr *p;
    Pscr *chunk;
    int i3;
    int rest3;
    ScoreDat *dat10 = (ScoreDat *)scoreDat;

    if (pscrs == NULL)
    {
        return -1;
    }
    p = pscrs;
    for (i3 = 0; i3 < 12; i3++, p++)
    {
        memset(p, 0, sizeof(Pscr));
        p->magic = 0x52435350;
        p->size2 = sizeof(Pscr);
        p->size = sizeof(Pscr);
        p->version = 2;
        p->shot = i3;
        p->practised = 0;
    }
    chunk = (Pscr *)((BYTE *)dat10 + dat10->dataOffset);
    rest3 = dat10->fileSize - dat10->dataOffset;
    while (rest3 > 0)
    {
        if (chunk->magic == 0x52435350 && chunk->version == 2)
        {
            p = chunk;
            if (p->shot >= 12)
            {
                break;
            }
            pscrs[p->shot] = *p;
        }
        rest3 -= chunk->size;
        chunk = (Pscr *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadPlstChunk(void *scoreDat, Plst *plst)
{
    Plst *chunk;
    int rest3;
    ScoreDat *dat10 = (ScoreDat *)scoreDat;

    chunk = (Plst *)((BYTE *)dat10 + dat10->dataOffset);
    rest3 = dat10->fileSize - dat10->dataOffset;
    while (rest3 > 0)
    {
        if (chunk->magic == 0x54534c50 && chunk->version == 2)
        {
            *plst = *chunk;
        }
        rest3 -= chunk->size;
        chunk = (Plst *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

void ReleaseScoreDat(ScoreDat *dat)
{
    FreeListNodes(dat->scores);
    g_MemoryManager.Free(dat->scores);
    g_MemoryManager.Free(dat);
}
