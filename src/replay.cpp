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
