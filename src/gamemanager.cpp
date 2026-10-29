// cflags: /Os /Ob1
// The /Os file with GameManager, as th07's, from 0x4399ac to the Chain
// code at 0x43c710.

#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "effect.h"
#include "enemy.h"
#include "midi.h"
#include "player.h"
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

int GameManager::ComputeChecksum()
{
    int sum;

    sum = ChecksumBytes((BYTE *)g_GameManager.stats->lifeChecksumKeys,
                        (BYTE *)&this->stats->checksum - (BYTE *)this->stats->lifeChecksumKeys);
    sum += ChecksumBytes((BYTE *)g_GameManager.stats->checksumKeysE, sizeof(g_GameManager.stats->checksumKeysE));
    sum += ChecksumBytes((BYTE *)g_GameManager.unk_4, 0x3c);
    sum += ChecksumBytes((BYTE *)&g_App.controllerMapping, 0x3c);
    sum += ChecksumBytes((BYTE *)&this->hscr, sizeof(this->hscr));
    return sum;
}

int ChecksumBytes(BYTE *data, int size)
{
    int i;
    int sum;

    sum = 0;
    for (i = 0; i < size; i++, data++)
    {
        sum += *data;
        g_GameManager.stats->checksum += g_GameManager.stats->checksumKeysE[2];
    }
    return sum;
}

void GameManager::AddLifeOrBomb()
{
    if ((int)this->stats->lives < 8)
    {
        this->AddLives(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Gui.livesChanged = 2;
    }
    else if ((int)this->stats->bombs < 8)
    {
        this->AddBombs(1);
        g_SoundPlayer.QueueSound(0x1c, 0);
        this->AddSubRank(200);
        g_Gui.bombsChanged = 2;
    }
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

void RerollStatsChecksumKeys()
{
    int sum;
    int j;

    g_GameManager.stats->grazeCount = g_Rng.GetRandomU32InRange(100000) + 6543;
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->lifeChecksumKeys); j++)
    {
        g_GameManager.stats->lifeChecksumKeys[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->checksumKeysD); j++)
    {
        g_GameManager.stats->checksumKeysD[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->statsChecksumKeysA); j++)
    {
        g_GameManager.stats->statsChecksumKeysA[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->statsChecksumKeysB); j++)
    {
        g_GameManager.stats->statsChecksumKeysB[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->lifeChecksumKeysF); j++)
    {
        g_GameManager.stats->lifeChecksumKeysF[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->statsChecksumKeysC); j++)
    {
        g_GameManager.stats->statsChecksumKeysC[j] = g_Rng.GetRandomF32ZeroToOne() * 100000.0f + 6543.0f;
    }
    for (j = 0; j < ARRAY_SIZE(g_GameManager.stats->checksumKeysE); j++)
    {
        g_GameManager.stats->checksumKeysE[j] = g_Rng.GetRandomU32InRange(100000) + 6543;
    }
    g_GameManager.stats->checksum = g_GameManager.stats->lifeChecksumKeys[2];
    sum = g_GameManager.ComputeChecksum();
    g_GameManager.stats->computedChecksum = sum;
    g_GameManager.checksumValue = (float)sum + g_GameManager.stats->checksumKeysD[3];
}

int LoadScoreFile()
{
    Catk *catks;
    int i;
    ScoreDat *data;
    int j;

    catks = g_GameManager.catks;
    ResultScreen::RegisterChain(2);
    memset(g_GameManager.catks, 0, sizeof(g_GameManager.catks));
    for (i = 0; i < 0xde; i++, catks++)
    {
        catks->magic = 0x4b544143;
        catks->size2 = sizeof(Catk);
        catks->size = sizeof(Catk);
        catks->version = 3;
        catks->index = i;
        for (j = 0; j < 7; j++)
        {
            catks->attempts[j] = 0;
            catks->captures[j] = 0;
            catks->maxBonus[j] = 0;
        }
    }
    data = LoadScoreDat("score.dat");
    if (data == NULL)
    {
        LogPrintf(&g_Log, "error : スコアファイルの読み取りに失敗しました\r\n");
        return -1;
    }
    g_GameManager.stats->highScore = LoadHighScores(data, NULL, g_GameManager.character, g_GameManager.difficulty,
                                                    &g_GameManager.stats->bestContinuesUsed);
    LoadSpellCards(data, g_GameManager.catks);
    LoadClears(data, g_GameManager.clrds);
    LoadPracticeScores(data, g_GameManager.pscrs);
    if (g_GameManager.unk_3dbac & 1)
    {
        g_GameManager.stats->highScore =
            g_GameManager.pscrs[g_GameManager.character].scores[g_GameManager.stage][g_GameManager.difficulty];
        g_GameManager.pscrs[g_GameManager.character].playCount[g_GameManager.stage][g_GameManager.difficulty]++;
        g_GameManager.pscrs[g_GameManager.character].unk_175 = 1;
    }
    ReleaseScoreDat(data);
    memcpy(g_GameManager.savedCatks, g_GameManager.catks, sizeof(g_GameManager.catks));
    memset(&g_GameManager.hscr, 0, sizeof(g_GameManager.hscr));
    g_GameManager.hscr.shot = g_GameManager.character;
    g_GameManager.hscr.difficulty = g_GameManager.difficulty;
    memcpy(g_GameManager.hscr.config, &g_App.controllerMapping, sizeof(g_GameManager.hscr.config));
    g_GameManager.unk_3db94 = 0;
    return 0;
}

void IncrementStatCounter(DWORD *count, DWORD max)
{
    if (*count < 999999)
    {
        (*count)++;
    }
}

int GameManager::DeletedCallback(GameManager *mgr)
{
    g_Unknown18b8a2c = 1;
    g_Unknown4e3d28 = 0;
    if (g_App.scene != 3 && g_App.scene != 11 && g_App.scene != 12)
    {
        g_App.unk_168 = 1;
    }
    else
    {
        g_App.unk_168 = 0;
    }
    if (!(g_GameManager.unk_3dbac >> 14 & 1) || g_App.unk_168 != 0)
    {
        g_App.StopMusic();
        if (g_App.musicMode == 2 && g_App.midi != NULL)
        {
            g_App.midi->Restart(0x1e);
        }
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    Func4180f0();
    Stage::RemoveChain();
    BulletManager::RemoveChain();
    Player::RemoveChain();
    EnemyManager::RemoveChain();
    EffectManager::RemoveChain();
    Gui::RemoveChain();
    if (!(g_GameManager.unk_3dbac >> 3 & 1))
    {
        ReplayManager::EndStage();
    }
    if (!(g_GameManager.unk_3dbac >> 3 & 1))
    {
        g_App.AddTotalPlayTime();
    }
    g_App.lastTotalPlayTimeTick = 0;
    g_App.AddPlayTime();
    mgr->unk_3dbac &= ~4;
    g_AsciiText.Initialize();
    g_GameManager.unk_2d = 0;
    g_GameManager.stageFrames = 0;
    return 0;
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

void GameManager::Func43c0bb(int amount, int force)
{
    if (g_Unknown17d6ed4 != 0 && force == 0)
    {
        return;
    }
    this->stats->unk_22 += amount;
    if (this->stats->unk_22 < this->unk_3ddf8)
    {
        this->stats->unk_22 = this->unk_3ddf8;
    }
    else if (this->stats->unk_22 > this->unk_3ddfa)
    {
        this->stats->unk_22 = this->unk_3ddfa;
    }
    this->stats->unk_20 = this->stats->unk_22;
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

void GameManager::AdvanceStage()
{
    switch (this->stage)
    {
    case 0:
        this->stage = 1;
        break;
    case 1:
        this->stage = 2;
        break;
    case 2:
        switch (g_GameManager.character)
        {
        case 0:
        case 4:
        case 5:
            this->stage = 4;
            break;
        case 1:
        case 6:
        case 7:
            this->stage = 3;
            break;
        case 2:
        case 8:
        case 9:
            this->stage = 3;
            break;
        case 3:
        case 10:
        case 11:
            this->stage = 4;
            break;
        }
        break;
    case 3:
    case 4:
        this->stage = 5;
        break;
    case 5:
        this->stage = (g_GameManager.unk_3dbac >> 11 & 3) != 0 ? 7 : 6;
        break;
    case 6:
        this->stage = 7;
        break;
    }
}

GameManager::GameManager()
{
    memset(this, 0, sizeof(GameManager));
    this->arena[0] = 32.0f;
    this->arena[1] = 16.0f;
    this->arena[2] = 384.0f;
    this->arena[3] = 448.0f;
    this->unk_3dbb4 = 3;
}
