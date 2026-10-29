// cflags: /Os /Ob1
// The /Os file with GameManager, as th07's, from 0x4399ac to the Chain
// code at 0x43c710.

#include <string.h>

#include "chain.h"
#include "replay.h"
#include "unsorted.h"
BOOL GameManager::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (x - width / 2.0f > 384.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (y - height / 2.0f > 448.0f)
    {
        return FALSE;
    }
    return TRUE;
}

int GameManager::OnDraw(GameManager *mgr)
{
    if (mgr->isPaused)
    {
        mgr->isPaused = 2;
    }
    if (g_App.scene != 2)
    {
        return 3;
    }
    if ((mgr->unk_3dbac >> 5 & 3) == 1)
    {
        return 3;
    }
    if (mgr->unk_38 != 0)
    {
        return 3;
    }
    return 1;
}

int RegisterGameManagerChain()
{
    GameManager *mgr = &g_GameManager;

    g_GameManagerCalcChain.SetCallback((ChainCallback)GameManager::OnUpdate);
    g_GameManagerCalcChain.addedCallback = (ChainCallback)GameManager::AddedCallback;
    g_GameManagerCalcChain.deletedCallback = (ChainCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChain.arg = mgr;
    mgr->stageFrames = 0;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChain, 2) != 0)
    {
        return -1;
    }
    g_GameManagerDrawChain.SetCallback((ChainCallback)GameManager::OnDraw);
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain, 5);
    return 0;
}

void GameManager::SetRankForDifficulty()
{
    this->rank = g_Ranks[g_GameManager.difficulty].rank;
    this->minRank = g_Ranks[g_GameManager.difficulty].minRank;
    this->maxRank = g_Ranks[g_GameManager.difficulty].maxRank;
}

void IncrementStatCounter(DWORD *count, DWORD max)
{
    if (*count < 999999)
    {
        (*count)++;
    }
}

void RemoveGameManagerChain()
{
    g_Chain.Remove(&g_GameManagerCalcChain);
    g_Chain.Remove(&g_GameManagerDrawChain);
    if (g_GameManager.stats->score >= 1000000000)
    {
        g_GameManager.stats->score = 999999999;
    }
    g_GameManager.stats->guiScore = g_GameManager.stats->score;
    g_App.gameSpeed = 1.0f;
}

void GameManager::AddSubRank(int amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank)
    {
        this->rank = this->maxRank;
    }
}

void GameManager::SubSubRank(int amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank)
    {
        this->rank = this->minRank;
    }
}

int GameManager::HasExtraOpen(int shot)
{
    return shot > 3 || this->clrds[shot].stagesClearedNoContinue[0] & 0x4000 ||
                   this->clrds[shot].stagesClearedNoContinue[1] & 0x4000 ||
                   this->clrds[shot].stagesClearedNoContinue[2] & 0x4000 ||
                   this->clrds[shot].stagesClearedNoContinue[3] & 0x4000
               ? TRUE
               : FALSE;
}

int GameManager::AnyShotHasExtraOpen()
{
    return this->HasExtraOpen(0) || this->HasExtraOpen(1) || this->HasExtraOpen(2) || this->HasExtraOpen(3) ? TRUE
                                                                                                           : FALSE;
}

int GameManager::Func43c23b(int shot)
{
    return shot > 3 || this->clrds[shot].stagesCleared[0] & 0x8000 || this->clrds[shot].stagesCleared[1] & 0x8000 ||
                   this->clrds[shot].stagesCleared[2] & 0x8000 || this->clrds[shot].stagesCleared[3] & 0x8000
               ? TRUE
               : FALSE;
}

int GameManager::Func43c2c5()
{
    return this->Func43c23b(0) || this->Func43c23b(1) || this->Func43c23b(2) || this->Func43c23b(3) ? TRUE : FALSE;
}

int GameManager::IsPhantasmOpen()
{
    return 0;
}

int GameManager::IsPracticeReplay()
{
    return (this->unk_3dbac >> 3 & 1) && g_ReplayManager->data->isPractice ? TRUE : FALSE;
}

int GameManager::Func43c35f()
{
    switch (g_GameManager.stage)
    {
    case 0:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 1:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 2:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 3:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 4:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 5:
    {
        int value = g_GameManager.stats->unk_3c;
        int limit = g_GameManager.stats->unk_40;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 6:
        return 0;
    case 7:
        return 0;
    default:
        return 4;
    }
}
