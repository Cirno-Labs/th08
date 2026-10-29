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
