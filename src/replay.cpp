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

// As FormatAndAdvance, the result screen's copy: formats into `buf` like
// sprintf and returns the end of what it wrote. Inline; SaveReplay calls
// the copy at 0x453b80.
inline char *FormatAndAdvanceInline(char *buf, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    return buf + strlen(buf);
}

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

int ReplayManager::PlayOnUpdateWithSeeds(ReplayManager *replay)
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
    g_GameButtons = replay->seedCursor->input;
    // Compiles to nothing, but without it the registers after it
    // differ: something the original tested here was compiled out.
    if (0)
    {
    }
    replay->seedCursor++;
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

int ReplayManager::RecordAddedCallback(ReplayManager *replay)
{
    ReplayStage *cur;
    int idx;
    BYTE *extras;
    ReplayStage *earlier;

    replay->frame = 0;
    replay->frameRateBuffer = NULL;
    if (replay->data == NULL)
    {
        replay->data = (ReplayHeader *)g_MemoryManager.Register(new ReplayHeader, sizeof(ReplayHeader), "ReplayDataInf");
        memset(replay->data, 0, sizeof(ReplayHeader));
        *(DWORD *)replay->data->magic = *(DWORD *)"T8RP";
        replay->data->unk_07 = 0;
        replay->data->shotType = g_GameManager.character;
        replay->data->version = 6;
        replay->data->withSeeds = 0;
        replay->data->subVersion = 0x100;
        replay->data->versionByte = 100;
        strcpy(replay->data->exeVersion, "0100d");
        replay->data->exeSize = g_App.exeSize;
        replay->data->exeChecksum = g_App.exeChecksum;
        replay->data->isPractice = g_GameManager.IsInPractice();
        replay->data->spellCardNumber = (g_GameManager.unk_3dbac >> 14 & 1) ? g_GameManager.unk_3dbb0 : -1;
        replay->data->clearLevel =
            (g_GameManager.HasClearedStage(7, g_GameManager.character, 0) ||
             g_GameManager.HasClearedStage(7, g_GameManager.character, 1) ||
             g_GameManager.HasClearedStage(7, g_GameManager.character, 2) ||
             g_GameManager.HasClearedStage(7, g_GameManager.character, 3) || g_GameManager.character > 3)
                ? 2
                : (g_GameManager.HasClearedStage(6, g_GameManager.character, 0) ||
                           g_GameManager.HasClearedStage(6, g_GameManager.character, 1) ||
                           g_GameManager.HasClearedStage(6, g_GameManager.character, 2) ||
                           g_GameManager.HasClearedStage(6, g_GameManager.character, 3)
                       ? 1
                       : 0);
        replay->data->difficulty = g_GameManager.difficulty;
        *(DWORD *)replay->data->name = *(DWORD *)"NO NAME";
        *(GameManagerUnk4 *)replay->data->config = *g_GameManager.unk_4;
        for (idx = 0; idx < 9; idx++)
        {
            replay->data->stages[idx] = NULL;
            replay->data->extras[idx] = NULL;
        }
    }
    else
    {
        earlier = NULL;
        for (idx = 0; idx < g_GameManager.stage; idx++)
        {
            if (replay->data->stages[idx] != NULL)
            {
                earlier = replay->data->stages[idx];
            }
        }
        if (earlier != NULL)
        {
            earlier->score = g_GameManager.stats->score;
        }
    }
    idx = g_GameManager.stage;
    if (replay->data->stages[idx] != NULL)
    {
        g_MemoryManager.Free(replay->data->stages[idx]);
    }
    if (replay->data->extras[idx] != NULL)
    {
        g_MemoryManager.Free(replay->data->extras[idx]);
    }
    replay->data->stages[idx] = (ReplayStage *)g_MemoryManager.Alloc(0xd2f00, "rep data");
    replay->data->extras[idx] = (BYTE *)g_MemoryManager.Alloc(0xd2f00, "rep data");
    cur = replay->data->stages[idx];
    extras = replay->data->extras[idx];
    cur->grazeTotal = g_GameManager.stats->grazeTotal;
    cur->bombs = g_GameManager.GetBombs();
    cur->lives = g_GameManager.GetLives();
    cur->power = g_GameManager.GetPower();
    cur->rank = g_GameManager.rank;
    cur->pointItemsTotal = g_GameManager.stats->pointItemsTotal;
    cur->rngSeed = g_GameManager.rngSeed;
    cur->unk_20 = g_GameManager.unk_3dba8;
    cur->spellCardsCaptured = g_GameManager.stats->spellCardsCaptured;
    cur->unk_0c = g_GameManager.stats->unk_34;
    cur->unk_10 = g_GameManager.stats->unk_38;
    cur->unk_18 = g_GameManager.stats->unk_22;
    cur->unk_22 = g_GameManager.GetUnk28();
    cur->unk_14 = g_GameManager.stats->unk_24;
    replay->cursor = cur->frames;
    replay->seedCursor = (ReplayFrameWithSeed *)replay->cursor;
    replay->extraCursor = extras;
    *replay->cursor = 0;
    replay->seedCursor->events = 0;
    replay->seedCursor->seed = g_Rng.GetSeed();
    replay->unk_04 = 0;
    replay->clearedAtRecordStart = 0;
    return 0;
}

int ReplayManager::PlayAddedCallback(ReplayManager *replay)
{
    ReplayStage *cur;
    int idx;
    int size;
    BYTE *extras;
    ReplayStage *earlier;

    replay->frame = 0;
    if (replay->data == NULL)
    {
        replay->data = (ReplayHeader *)LoadFileData(replay->path, &size, !(g_GameManager.unk_3dbac >> 1 & 1));
        replay->data = ValidateReplay(replay->data, size);
        if (replay->data == NULL)
        {
            return -1;
        }
        replay->frameRateBuffer = NULL;
        for (idx = 0; idx < 9; idx++)
        {
            if (replay->data->stages[idx] != NULL)
            {
                replay->data->stages[idx] = (ReplayStage *)((int)replay->data->stages[idx] + (int)replay->data);
            }
            if (replay->data->extras[idx] != NULL)
            {
                replay->data->extras[idx] = (BYTE *)((int)replay->data->extras[idx] + (int)replay->data);
            }
        }
    }
    idx = g_GameManager.stage;
    if (replay->data->stages[idx] == NULL)
    {
        return -1;
    }
    cur = replay->data->stages[idx];
    extras = replay->data->extras[idx];
    g_GameManager.character = replay->data->shotType;
    g_GameManager.shotType = replay->data->shotType % 1;
    g_GameManager.character = replay->data->shotType;
    g_GameManager.difficulty = replay->data->difficulty;
    g_GameManager.stats->pointItemsTotal = cur->pointItemsTotal;
    g_GameManager.rank = cur->rank;
    g_GameManager.SetLives(cur->lives);
    g_GameManager.SetBombs(cur->bombs);
    g_GameManager.SetPower(cur->power);
    g_GameManager.stats->grazeTotal = cur->grazeTotal;
    replay->cursor = cur->frames;
    replay->seedCursor = (ReplayFrameWithSeed *)replay->cursor;
    g_GameManager.unk_3dba8 = cur->unk_20;
    g_GameManager.stats->unk_24 = cur->unk_14;
    *g_GameManager.unk_4 = *(GameManagerUnk4 *)replay->data->config;
    g_Rng.SetSeed(cur->rngSeed);
    g_GameManager.stats->spellCardsCaptured = cur->spellCardsCaptured;
    g_GameManager.stats->unk_34 = cur->unk_0c;
    g_GameManager.stats->unk_38 = cur->unk_10;
    g_GameManager.stats->unk_22 = cur->unk_18;
    g_GameManager.SetUnk28(cur->unk_22);
    replay->extraCursor = extras;
    replay->unk_04 = 0;
    earlier = NULL;
    for (idx = 0; idx < g_GameManager.stage; idx++)
    {
        if (replay->data->stages[idx] != NULL)
        {
            earlier = replay->data->stages[idx];
        }
    }
    if (earlier != NULL)
    {
        g_GameManager.stats->score = earlier->score;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
    }
    return 0;
}

int ReplayManager::DeletedCallback(ReplayManager *replay)
{
    if (replay->calcChain2 != NULL)
    {
        g_Chain.Remove(replay->calcChain2);
        replay->calcChain2 = NULL;
    }
    if (replay->calcChain3 != NULL)
    {
        g_Chain.Remove(replay->calcChain3);
        replay->calcChain3 = NULL;
    }
    g_MemoryManager.Free(g_ReplayManager->data);
    if (replay->frameRateBuffer != NULL)
    {
        g_MemoryManager.Free(replay->frameRateBuffer);
    }
    g_MemoryManager.Unregister(g_ReplayManager);
    delete g_ReplayManager;
    g_ReplayManager = NULL;
    g_ReplayManager = NULL;
    return 0;
}

void ReplayManager::EndStage()
{
    int stageIdx;
    ReplayManager *replay = g_ReplayManager;

    if (replay != NULL)
    {
        replay->cursor++;
        *replay->cursor = 0;
        stageIdx = g_GameManager.stage;
        replay->stageEnds[stageIdx] = replay->cursor + 3;
    }
}

void SaveReplay(char *path, char *name)
{
    BYTE *dp2;
    BYTE kbVal;
    BYTE *bpCur;
    int checksum;
    char timeStrs[0x100];
    time_t now;
    struct tm *localNow2;
    int offsets;
    ReplayUserHeader user;
    HANDLE file;
    char userInfo[0x400];
    ReplayHeader hX;
    BYTE *rpyBytes;
    int chunk;
    int compSize;
    float slowdown2;
    BYTE *compressedData;
    DWORD writtenSize;
    char *p2;
    ReplayManager *replayTmp;
    int i;

    if (g_ReplayManager != NULL)
    {
        replayTmp = g_ReplayManager;
        if (replayTmp->GetMode() == 0)
        {
            if (!g_GameManager.IsInPractice() && g_GameManager.difficulty < 4)
            {
                if (memcmp(&g_App.controllerMapping, replayTmp->data->config, sizeof(replayTmp->data->config)) != 0)
                {
                    goto freeStages;
                }
            }
            if (replayTmp->data->config[0x25] != 0)
            {
                goto freeStages;
            }
            if (path != NULL)
            {
                DebugPrint("info : Replay File write %s\r\n", path);
                rpyBytes = (BYTE *)g_MemoryManager.Alloc(0x400000, "rep tmp");
                hX = *replayTmp->data;
                ReplayManager::EndStage();
                i = g_GameManager.unk_3ddc8;
                replayTmp->data->stages[i]->score = g_GameManager.stats->score;
                offsets = 0x68;
                offsets += 0xcc;
                for (i = 0; i < 9; i++)
                {
                    if (replayTmp->data->stages[i] != NULL)
                    {
                        chunk = (int)replayTmp->stageEnds[i] - (int)replayTmp->data->stages[i];
                        memcpy(rpyBytes + offsets - 0x68, replayTmp->data->stages[i], chunk);
                        hX.stages[i] = (ReplayStage *)offsets;
                        offsets += chunk;
                    }
                }
                for (i = 0; i < 9; i++)
                {
                    if (replayTmp->data->extras[i] != NULL)
                    {
                        chunk = (int)replayTmp->extraEnds[i] - (int)replayTmp->data->extras[i];
                        memcpy(rpyBytes + offsets - 0x68, replayTmp->data->extras[i], chunk);
                        hX.extras[i] = (BYTE *)offsets;
                        offsets += chunk;
                    }
                }
                hX.score = g_GameManager.stats->guiScore;
                if (hX.spellCardNumber >= 0)
                {
                    memcpy(hX.spellCardName, g_GameManager.catks[hX.spellCardNumber].name, sizeof(hX.spellCardName));
                }
                slowdown2 = (g_App.achievedFrames / g_App.expectedFrames - 0.5f) * 2.0f;
                if (slowdown2 < 0.0f)
                {
                    slowdown2 = 0.0f;
                }
                else if (slowdown2 >= 1.0f)
                {
                    slowdown2 = 1.0f;
                }
                hX.slowdown = (1.0f - slowdown2) * 100.0f;
                user.magic = *(DWORD *)"USER";
                user.type = 0;
                memset(userInfo, 0, sizeof(userInfo));
                p2 = userInfo;
                p2 = FormatAndAdvanceInline(p2, "プレイヤー名\t%s\r\n", name);
                time(&now);
                localNow2 = localtime(&now);
                strftime(timeStrs, 0x14, "%Y/%m/%d %H:%M:%S", localNow2);
                p2 = FormatAndAdvanceInline(p2, "プレイ時刻\t%s\r\n", timeStrs);
                p2 = FormatAndAdvanceInline(p2, "キャラ名\t%s\r\n", GetShotName(g_GameManager.character));
                p2 = FormatAndAdvanceInline(p2, "スコア\t\t%d0\r\n", g_GameManager.stats->guiScore);
                p2 = FormatAndAdvanceInline(p2, "難易度\t\t%s\r\n", g_ReplayDifficultyNames[g_GameManager.difficulty]);
                if (hX.spellCardNumber >= 0)
                {
                    p2 = FormatAndAdvanceInline(p2, "カード名\tNo.%3d %s\r\n", hX.spellCardNumber + 1, hX.spellCardName);
                }
                else
                {
                    p2 = FormatAndAdvanceInline(p2, "最終ステージ\t%s\r\n",
                                               (g_GameManager.unk_3dbac >> 4 & 1) ? "Clear"
                                                                                  : GetStageName(g_GameManager.stage));
                }
                p2 = FormatAndAdvanceInline(p2, "ミス回数\t%d\r\n", g_GameManager.GetDeaths());
                p2 = FormatAndAdvanceInline(p2, "ボム回数\t%d\r\n", g_GameManager.GetBombsUsed());
                p2 = FormatAndAdvanceInline(p2, "処理落ち率\t%f%%\r\n", hX.slowdown);
                g_GameManager.hscr.humanRate = (float)g_GameManager.unk_3dba0 / g_GameManager.unk_3dba4 * 10000.0f;
                p2 = FormatAndAdvanceInline(p2, "人間率\t\t%3.2f％\r\n", g_GameManager.hscr.humanRate / 100.0f);
                p2 = FormatAndAdvanceInline(p2, "ゲームのバージョン\t%d.%.2d%c\r\n", 1, 0, 'd');
                user.size = strlen(userInfo) + 0xc;
                user.size += user.size & 1;
                hX.unk_07 = 1;
                strcpy(hX.name, name);
                FormatTodayDate(hX.date);
                hX.key = g_Rng.GetRandomU16InRange(0x80) + 0x40;
                hX.randomByte = g_Rng.GetRandomU16InRange(0x100);
                hX.secondaryKey = g_Rng.GetRandomU16InRange(0x100);
                hX.slowdown2 = hX.slowdown + 1.12f;
                hX.replayFormatVersion = 30;
                memcpy(rpyBytes, &hX.randomByte, sizeof(ReplayHeader) - 0x68);
                DebugPrint("info : original size %d\r\n", offsets);
                hX.dataSize = offsets - 0x68;
                compressedData = (BYTE *)LzssCompress(rpyBytes, hX.dataSize, &hX.compressedSize);
                g_MemoryManager.Free(rpyBytes);
                compSize = hX.compressedSize;
                bpCur = &hX.key;
                checksum = 0x3f000318;
                for (i = 0; i < 0x53U; i++, bpCur++)
                {
                    checksum += *bpCur;
                }
                bpCur = compressedData;
                for (i = 0; i < compSize; i++, bpCur++)
                {
                    checksum += *bpCur;
                }
                hX.checksum = checksum;
                dp2 = (BYTE *)&hX.compressedSize;
                kbVal = hX.key;
                for (i = 0; i < 0x50U; i++, dp2++)
                {
                    *dp2 += kbVal;
                    kbVal += 7;
                }
                dp2 = compressedData;
                for (i = 0; i < compSize; i++, dp2++)
                {
                    *dp2 += kbVal;
                    kbVal += 7;
                }
                hX.size = compSize + 0x68;
                file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (file == INVALID_HANDLE_VALUE)
                {
                    goto freeStages;
                }
                WriteFile(file, &hX, 0x68, &writtenSize, NULL);
                WriteFile(file, compressedData, compSize, &writtenSize, NULL);
                WriteFile(file, &user, 0xc, &writtenSize, NULL);
                WriteFile(file, userInfo, user.size - 0xc, &writtenSize, NULL);
                CloseHandle(file);
                DebugPrint("info : Size %d -> %d\r\n", offsets, compSize + 0x68);
                GlobalFree(compressedData);
            }
        freeStages:
            for (i = 0; i < 9; i++)
            {
                if (g_ReplayManager->data->stages[i] != NULL)
                {
                    g_MemoryManager.Free(g_ReplayManager->data->stages[i]);
                }
                if (replayTmp->data->extras[i] != NULL)
                {
                    g_MemoryManager.Free(g_ReplayManager->data->extras[i]);
                }
            }
        }
        g_Chain.Remove(g_ReplayManager->calcChain);
    }
}
