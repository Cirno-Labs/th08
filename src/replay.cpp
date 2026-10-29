// The file with ReplayManager, around 0x451d90.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "enemy.h"
#include "lzss.h"
#include "replay.h"
#include "resultscreen.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

ReplayHeader *ValidateReplay(void *data, int size)
{
    BYTE *ptr2;
    int sum;
    BYTE *bytes;
    BYTE b;
    ReplayHeader *file;
    int i;
    ReplayHeader *decoded;

    file = (ReplayHeader *)data;
    if (file == NULL)
    {
        goto fail;
    }
    if (*(DWORD *)file->magic != *(DWORD *)"T8RP")
    {
        goto fail;
    }
    if (file->version != 6)
    {
        goto fail;
    }
    bytes = (BYTE *)&file->compressedSize;
    b = file->key;
    for (i = 0; i < file->size - 0x18; i++, bytes++)
    {
        *bytes -= b;
        b += 7;
    }
    ptr2 = &file->key;
    sum = 0x3f000318;
    for (i = 0; i < file->size - 0x15; i++, ptr2++)
    {
        sum += *ptr2;
    }
    if (sum != file->checksum)
    {
        goto fail;
    }
    decoded = (ReplayHeader *)g_MemoryManager.Alloc(file->dataSize + 0x68 + (size - file->size), GLOBAL_H);
    memcpy(decoded, data, 0x68);
    LzssDecompress((BYTE *)file + 0x68, file->compressedSize, (BYTE *)decoded + 0x68, file->dataSize);
    memcpy((BYTE *)decoded + file->dataSize + 0x68, (BYTE *)data + file->size, size - file->size);
    file = decoded;
    if (file->config[0x25] != 0)
    {
        goto failDecoded;
    }
    if (g_App.CheckVersionFile(file->exeVersion, file->exeSize, file->exeChecksum) != 0)
    {
        goto failDecoded;
    }
    g_MemoryManager.Free(data);
    return decoded;

fail:
    g_MemoryManager.Free(data);
    return NULL;

failDecoded:
    g_MemoryManager.Free(data);
    g_MemoryManager.Free(decoded);
    return NULL;
}

int ReplayManager::RegisterChain(int mode, char *path)
{
    ReplayManager *replay;

    g_PrevGameButtons = 0;
    g_GameButtons = 0;
    if (g_ReplayManager == NULL)
    {
        replay = (ReplayManager *)g_MemoryManager.Register(new ReplayManager, sizeof(ReplayManager), "ReplayInf");
        g_ReplayManager = replay;
        memset(replay, 0, sizeof(ReplayManager));
        replay->data = NULL;
        replay->mode = mode;
        replay->path = path;
        switch (mode)
        {
        case 0:
            replay->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate);
            replay->calcChain->addedCallback = (ChainCallback)ReplayManager::RecordAddedCallback;
            replay->calcChain->deletedCallback = (ChainCallback)ReplayManager::DeletedCallback;
            replay->calcChain->arg = replay;
            if (g_Chain.AddToCalcChain(replay->calcChain, 0x11) != 0)
            {
                return -1;
            }
            replay->calcChain2 = NULL;
            replay->calcChain3 = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate2);
            replay->calcChain3->arg = replay;
            g_Chain.AddToCalcChain(replay->calcChain3, 7);
            ReplayManager::RecordOnUpdate2(replay);
            break;
        case 1:
            replay->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate);
            replay->calcChain->addedCallback = (ChainCallback)ReplayManager::PlayAddedCallback;
            replay->calcChain->deletedCallback = (ChainCallback)ReplayManager::DeletedCallback;
            replay->calcChain->arg = replay;
            if (g_Chain.AddToCalcChain(replay->calcChain, 6) != 0)
            {
                return -1;
            }
            if (replay->data->withSeeds)
            {
                replay->calcChain->callback = (ChainCallback)ReplayManager::PlayOnUpdateWithSeeds;
            }
            replay->calcChain2 = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate2);
            replay->calcChain2->arg = replay;
            g_Chain.AddToCalcChain(replay->calcChain2, 0x12);
            replay->calcChain3 = NULL;
            if (replay->data->withSeeds)
            {
                replay->calcChain3 = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate2);
                replay->calcChain3->arg = replay;
                g_Chain.AddToCalcChain(replay->calcChain3, 7);
                ReplayManager::RecordOnUpdate2(replay);
            }
            break;
        }
    }
    else
    {
        switch (mode)
        {
        case 0:
            ReplayManager::RecordAddedCallback(g_ReplayManager);
            break;
        case 1:
            ReplayManager::PlayAddedCallback(g_ReplayManager);
            break;
        }
    }
    return 0;
}

int ReplayManager::RecordOnUpdate2(ReplayManager *replay)
{
    replay->frameEvents = 0;
    replay->frameSeed = g_Rng.GetSeed();
    g_Rng.ResetGenerated();
    if (g_GameManager.pauseRequested)
    {
        replay->frameEvents |= 0x100;
    }
    g_GameManager.pauseRequested = 0;
    return 1;
}

int ReplayManager::RecordOnUpdate(ReplayManager *replay)
{
    int idx;
    WORD buttons;

    if (!(g_GameManager.unk_3dbac >> 2 & 1))
    {
        return 1;
    }
    g_PrevGameButtons = g_GameButtons;
    g_GameButtons = g_Buttons;
    if (g_GameManager.unk_4->scoreDisabled != 0)
    {
        return 1;
    }
    if (g_App.GetNoScore())
    {
        return 1;
    }
    if (g_GameManager.unk_3dbac >> 9 & 1)
    {
        if (replay->unk_04 >= 3)
        {
            return 1;
        }
        replay->unk_04++;
    }
    idx = g_GameManager.unk_3ddc8;
    buttons = g_Buttons;
    g_GameButtons = buttons;
    replay->cursor++;
    replay->stageEnds[idx] = replay->cursor + 1;
    *replay->cursor = buttons;
    if (replay->frame % 30 == 0)
    {
        *replay->extraCursor = (BYTE)g_App.roundedFps | (g_App.fastClockStreak != 0 ? 0x80 : 0);
        replay->extraCursor[1] = (BYTE)g_App.roundedFps;
        replay->extraEnds[idx] = replay->extraCursor + 2;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}

int ReplayManager::PlayOnUpdate2(ReplayManager *replay)
{
    if (!(g_GameManager.unk_3dbac >> 2 & 1))
    {
        return 1;
    }
    if (g_GameManager.unk_3dbac >> 9 & 1)
    {
        return 1;
    }
    if (g_Gui.IsMsgRunning() && g_Gui.IsMsgSkippable() && replay->frame % 3 != 2)
    {
        return 6;
    }
    if (!(g_GameManager.unk_3dbac >> 9 & 1) && g_GameManager.replayMenuChoice == 2 &&
        !g_EnemyManager.AnyBossActive() && replay->frame % 5 != 4)
    {
        return 6;
    }
    return 1;
}

int ReplayManager::PlayOnUpdate(ReplayManager *replay)
{
    int unused;

    if (!(g_GameManager.unk_3dbac >> 2 & 1))
    {
        return 1;
    }
    if (g_GameManager.unk_4->scoreDisabled != 0)
    {
        return 1;
    }
    if (g_GameManager.unk_3dbac >> 9 & 1)
    {
        return 1;
    }
    unused = 0;
    g_PrevGameButtons = g_GameButtons;
    g_GameButtons = *replay->cursor;
    replay->cursor++;
    g_ButtonRepeat = 0;
    if (g_PrevGameButtons == g_GameButtons)
    {
        if (g_ButtonFrames >= 30)
        {
            if (g_ButtonFrames % 8 == 0)
            {
                g_ButtonRepeat = 1;
            }
            if (g_ButtonFrames >= 38)
            {
                g_ButtonFrames = 30;
            }
        }
        g_ButtonFrames++;
    }
    else
    {
        g_ButtonFrames = 0;
    }
    if (replay->frame % 30 == 0)
    {
        g_App.roundedFps = (char)replay->extraCursor[1] & 0x7f;
        g_App.fpsWarning = (char)replay->extraCursor[1] >> 7;
        replay->extraCursor++;
    }
    replay->frame++;
    return 1;
}
