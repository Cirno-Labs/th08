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
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
char *g_StageAnmPaths[9] = {
    "stg1bg.anm",
    "stg2bg.anm",
    "stg3bg.anm",
    "stg4abg.anm",
    "stg4abg.anm",
    "stg5bg.anm",
    "stg6bg.anm",
    "stg7bg.anm",
    "stg8bg.anm",
};
char *g_StageDataFiles[9] = {
    "stage1.std",
    "stage2.std",
    "stage3.std",
    "stage4a.std",
    "stage4b.std",
    "stage5.std",
    "stage6.std",
    "stage7.std",
    "stage8.std",
};
char *g_SpellPracticeStageDataFiles[9] = {
    "stage1_s.std",
    "stage2_s.std",
    "stage3_s.std",
    "stage4a_s.std",
    "stage4b_s.std",
    "stage5_s.std",
    "stage6_s.std",
    "stage7_s.std",
    "stage8_s.std",
};
char *g_EnemyAnmPaths[9] = {
    "stg1enm.anm",
    "stg2enm.anm",
    "stg3enm.anm",
    "stg4aenm.anm",
    "stg4benm.anm",
    "stg5enm.anm",
    "stg6enm.anm",
    "stg7enm.anm",
    "stg8enm.anm",
};
// The last words' (cards 0xcd to 0xdd): the code reads them as
// g_EnemyAnmPaths[card - 0xc4].
char *g_LastWordEnemyAnmPaths[17] = {
    "stg1enm.anm",
    "stg2enm.anm",
    "stg3enm.anm",
    "stg5enm.anm",
    "stg6enm.anm",
    "stg7enm.anm",
    "stg8enm.anm",
    "stg5enm.anm",
    "stg8enm.anm",
    "stg4aenm.anm",
    "stg4benm.anm",
    "stgenm_sk.anm",
    "stgenm_ym.anm",
    "stgenm_al.anm",
    "stgenm_rm.anm",
    "stgenm_yy.anm",
    "stgenm_yk.anm",
};
char *g_EclPaths[9] = {
    "ecldata1.ecl",
    "ecldata2.ecl",
    "ecldata3.ecl",
    "ecldata4a.ecl",
    "ecldata4b.ecl",
    "ecldata5.ecl",
    "ecldata6.ecl",
    "ecldata7.ecl",
    "ecldata8.ecl",
};
// The stages' scripts in stage practice, read as g_EclPaths[stage + 9],
// and the last words', read as g_EclPaths[card - 0xba].
char *g_StagePracticeEclPaths[10] = {
    "ecldata1sp.ecl",
    "ecldata2sp.ecl",
    "ecldata3sp.ecl",
    "ecldata4asp.ecl",
    "ecldata4bsp.ecl",
    "ecldata5sp.ecl",
    "ecldata6sp.ecl",
    "ecldata7sp.ecl",
    "ecldata8sp.ecl",
    NULL,
};
char *g_LastWordEclPaths[17] = {
    "ecldata1sp.ecl",
    "ecldata2sp.ecl",
    "ecldata3sp.ecl",
    "ecldata5sp.ecl",
    "ecldata6sp.ecl",
    "ecldata7sp.ecl",
    "ecldata8sp.ecl",
    "ecldata5sp.ecl",
    "ecldata8sp.ecl",
    "ecldata4asp.ecl",
    "ecldata4bsp.ecl",
    "ecldata_sk.ecl",
    "ecldata_ym.ecl",
    "ecldata_al.ecl",
    "ecldata_rm.ecl",
    "ecldata_yy.ecl",
    "ecldata_yk.ecl",
};
char *g_StageTextAnmPaths[9] = {
    "stg1txt.anm",
    "stg2txt.anm",
    "stg3txt.anm",
    "stg4atxt.anm",
    "stg4btxt.anm",
    "stg5txt.anm",
    "stg6txt.anm",
    "stg7txt.anm",
    "stg8txt.anm",
};
char *g_EffectAnmPaths[15] = {
    "eff01.anm",
    "eff02.anm",
    "eff03.anm",
    "eff04a.anm",
    "eff04b.anm",
    "eff05.anm",
    "eff06.anm",
    "eff07.anm",
    "eff08.anm",
    "eff09sk.anm",
    "eff09ym.anm",
    "eff09al.anm",
    "eff09rm.anm",
    "eff09yy.anm",
    "eff09yk.anm",
};
char *g_MsgPaths[9][12] = {
    {"msg1a.dat", "msg1b.dat", "msg1c.dat", "msg1d.dat", "msg1a.dat", "msg1a.dat", "msg1b.dat", "msg1b.dat", "msg1c.dat", "msg1c.dat", "msg1d.dat", "msg1d.dat"},
    {"msg2a.dat", "msg2b.dat", "msg2c.dat", "msg2d.dat", "msg2a.dat", "msg2a.dat", "msg2b.dat", "msg2b.dat", "msg2c.dat", "msg2c.dat", "msg2d.dat", "msg2d.dat"},
    {"msg3a.dat", "msg3b.dat", "msg3c.dat", "msg3d.dat", "msg3a.dat", "msg3a.dat", "msg3b.dat", "msg3b.dat", "msg3c.dat", "msg3c.dat", "msg3d.dat", "msg3d.dat"},
    {"msg4dm.dat", "msg4ab.dat", "msg4ac.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4ab.dat", "msg4ab.dat", "msg4ac.dat", "msg4ac.dat", "msg4dm.dat", "msg4dm.dat"},
    {"msg4ba.dat", "msg4dm.dat", "msg4dm.dat", "msg4bd.dat", "msg4ba.dat", "msg4ba.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4dm.dat", "msg4bd.dat", "msg4bd.dat"},
    {"msg5a.dat", "msg5b.dat", "msg5c.dat", "msg5d.dat", "msg5a.dat", "msg5a.dat", "msg5b.dat", "msg5b.dat", "msg5c.dat", "msg5c.dat", "msg5d.dat", "msg5d.dat"},
    {"msg6a.dat", "msg6b.dat", "msg6c.dat", "msg6d.dat", "msg6a.dat", "msg6a.dat", "msg6b.dat", "msg6b.dat", "msg6c.dat", "msg6c.dat", "msg6d.dat", "msg6d.dat"},
    {"msg7a.dat", "msg7b.dat", "msg7c.dat", "msg7d.dat", "msg7a.dat", "msg7a.dat", "msg7b.dat", "msg7b.dat", "msg7c.dat", "msg7c.dat", "msg7d.dat", "msg7d.dat"},
    {"msg8a.dat", "msg8b.dat", "msg8c.dat", "msg8d.dat", "msg8a.dat", "msg8a.dat", "msg8b.dat", "msg8b.dat", "msg8c.dat", "msg8c.dat", "msg8d.dat", "msg8d.dat"},
};
SpellCardBgm g_SpellCardBgms[19] = {
    {1, 1, "th08_00.mid", 0, 0},
    {12, 2, "th08_03.mid", 1, 0},
    {16, 3, "th08_04.mid", 0, 0},
    {31, 4, "th08_05.mid", 1, 0},
    {35, 5, "th08_06.mid", 0, 0},
    {53, 6, "th08_07.mid", 1, 0},
    {76, 8, "th08_09.mid", 1, 0},
    {99, 9, "th08_10.mid", 1, 0},
    {118, 11, "th08_12.mid", 1, 0},
    {122, 12, "th08_13.mid", 0, 0},
    {142, 13, "th08_14.mid", 1, 0},
    {146, 15, "th08_13b.mid", 2, 1},
    {150, 12, "th08_13.mid", 0, 0},
    {170, 14, "th08_15.mid", 1, 0},
    {190, 15, "th08_13b.mid", 2, 1},
    {193, 16, "th08_18.mid", 0, 0},
    {204, 17, "th08_19.mid", 1, 0},
    {222, 20, "th08_20.mid", 2, 0},
    {-1, 0, " ", 0, 0},
};
int g_TimeOrbsNeeded[9][4] = {
    {2000, 2500, 2700, 3000}, {6500, 7200, 7200, 7200}, {7500, 8500, 8800, 8800},
    {9999, 9999, 9999, 9999}, {7500, 8500, 8500, 8500}, {9999, 9999, 9999, 9999},
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
};
RankSettings g_Ranks[6] = {
    {10, 8, 16}, {10, 8, 16}, {8, 8, 12},
    {8, 8, 12}, {16, 15, 16}, {16, 15, 16},
};
GameManager g_GameManager;
ChainElem g_GameManagerCalcChainElem;
ChainElem g_GameManagerDrawProc;
#endif // TH08_EXE

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
    sum += ChecksumBytes((BYTE *)g_GameManager.savedConfig, 0x3c);
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

int GameManager::OnUpdate(GameManager *mgr)
{
    int check;
    DWORD i;
    DWORD step;
    int next;
    int stage;
    int j;

    g_GameManager.playFrames++;
    if ((mgr->stateFlags >> 5 & 3) != 0)
    {
        if ((mgr->stateFlags >> 5 & 3) == 2)
        {
            mgr->stateFlags |= 0x60;
            g_GameManager.loadingThreadState = 1;
            g_GameManager.nextScene = -1;
            if (!(g_GameManager.stateFlags >> 3 & 1))
            {
                if (!g_GameManager.stats->continuesUsed)
                {
                    g_GameManager.clrds[g_GameManager.character].stagesClearedNoContinue[g_GameManager.difficulty] |=
                        g_GameManager.stageBit;
                    g_GameManager.clrds[12].stagesClearedNoContinue[g_GameManager.difficulty] |= g_GameManager.stageBit;
                }
                g_GameManager.clrds[g_GameManager.character].stagesCleared[g_GameManager.difficulty] |=
                    g_GameManager.stageBit;
                g_GameManager.clrds[12].stagesCleared[g_GameManager.difficulty] |= g_GameManager.stageBit;
            }
            mgr->stats->guiScore = mgr->stats->score;
            if (mgr->stateFlags & 1)
            {
                g_GameManager.stats->guiScore = g_GameManager.stats->score;
                g_GameManager.nextScene = 6;
                return 3;
            }
            if (g_GameManager.stage != 6 && g_GameManager.stage != 7 && g_GameManager.stage != 8)
            {
                if (g_GameManager.stateFlags >> 3 & 1)
                {
                    next = 0;
                    for (stage = g_GameManager.stage + 1; stage < 9; stage++)
                    {
                        if ((BOOL)(g_ReplayManager->data->stages[stage] != NULL))
                        {
                            next = stage;
                            break;
                        }
                    }
                    if (next == 0)
                    {
                        g_App.scene = 7;
                    }
                    else
                    {
                        g_GameManager.stage = next;
                        g_App.scene = 3;
                    }
                }
                else
                {
                    if (g_GameManager.GetClock() >= 12)
                    {
                        g_GameManager.stateFlags &= ~0x10;
                        g_GameManager.nextScene = 9;
                        return 3;
                    }
                    g_GameManager.AdvanceStage();
                    g_App.scene = 3;
                }
            }
            else if (g_GameManager.stateFlags >> 3 & 1)
            {
                g_GameManager.nextScene = 7;
            }
            else
            {
                if (g_GameManager.difficulty >= 4)
                {
                    if (g_GameManager.difficulty == 4)
                    {
                        g_GameManager.clrds[g_GameManager.character].stagesClearedNoContinue[g_GameManager.difficulty] |=
                            0x8000;
                        g_GameManager.clrds[12].stagesCleared[g_GameManager.difficulty] |= 0x8000;
                    }
                    g_GameManager.plst.counts[g_GameManager.difficulty].clears++;
                    g_GameManager.stateFlags |= 0x10;
                    g_GameManager.stats->guiScore = g_GameManager.stats->score;
                    g_GameManager.nextScene = 6;
                    return 3;
                }
                g_GameManager.stateFlags |= 0x10;
                g_GameManager.nextScene = 9;
                return 3;
            }
            if (g_GameManager.nextScene < 0)
            {
                g_Gui.StartLoadingBar();
            }
        }
        if (WAS_PRESSED(0x1001) || g_GameManager.stateFlags >> 3 & 1 || g_GameManager.stage == 6 ||
            g_GameManager.stage == 7 || g_GameManager.stage == 8)
        {
            mgr->stateFlags &= ~0x60;
            if (g_GameManager.nextScene >= 0)
            {
                g_App.scene = g_GameManager.nextScene;
            }
        }
    }
    if (mgr->loadingThreadState != 0)
    {
        if (mgr->loadingThreadState == 2)
        {
            return 4;
        }
        mgr->loadingFrames++;
        return 3;
    }
    if (mgr->pendingMusicStart != 0)
    {
        CopyStageSprite();
        g_SpriteEngine->ReleaseSurface(8);
        g_App.loadingState = 0;
        if (mgr->pendingMusicStart == 1)
        {
            if (!(g_GameManager.stateFlags >> 14 & 1))
            {
                g_App.PlayMidi(0, g_StageBgms[g_GameManager.stage].stageBgm);
            }
            else
            {
                j = 0;
                while (g_SpellCardBgms[j].lastCard >= 0)
                {
                    if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[j].lastCard)
                    {
                        g_App.PlayMidi(0, g_SpellCardBgms[j].bgm);
                        break;
                    }
                    j++;
                }
            }
        }
        mgr->pendingMusicStart = 0;
    }
    if (!mgr->isInContinueMenu && !mgr->isPaused && !(mgr->stateFlags >> 1 & 1) && !mgr->skipFpsMeasure && WAS_PRESSED(8))
    {
        mgr->isPaused = 1;
        g_GameManager.arena[0] = 32.0f;
        g_GameManager.arena[1] = 16.0f;
        g_GameManager.arena[2] = 384.0f;
        g_GameManager.arena[3] = 448.0f;
        mgr->pauseRequested = 1;
        g_SoundPlayer.QueueSoundCommand(6, 0, "Pause");
        g_SoundPlayer.QueueSound(0x22, 0);
        g_App.AddTotalPlayTime();
        g_Rng.savedSeed = g_Rng.seed;
        mgr->hscr.pauses++;
        g_GameManager.RerollChecksumKeys();
        g_Rng.seed = g_Rng.savedSeed;
    }
    g_App.viewport.X = mgr->arena[0];
    g_App.viewport.Y = mgr->arena[1];
    g_App.viewport.Width = mgr->arena[2];
    g_App.viewport.Height = mgr->arena[3];
    g_App.viewport.MinZ = 0.0f;
    g_App.viewport.MaxZ = 1.0f;
    g_SpriteEngine->ClearLastCameraSetupFlag();
    if (g_GameManager.stateFlags >> 3 & 1 && g_GameManager.replayMenuChoice == 1 && !g_Gui.IsMsgRunning())
    {
        mgr->slowdownCounter++;
        if ((g_App.roundedFps < 20 && mgr->slowdownCounter % 3 != 0) ||
            (g_App.roundedFps >= 20 && g_App.roundedFps < 30 && mgr->slowdownCounter % 2 != 0) ||
            (g_App.roundedFps >= 30 && g_App.roundedFps < 40 && mgr->slowdownCounter % 3 == 0) ||
            (g_App.roundedFps >= 40 && g_App.roundedFps < 50 && mgr->slowdownCounter % 6 == 0))
        {
            return 3;
        }
    }
    if (mgr->stateFlags >> 1 & 1)
    {
        if (WAS_PRESSED(0xffff))
        {
            g_App.scene = 1;
        }
        mgr->demoFrames++;
        if ((mgr->demoIndex == 0 && mgr->demoFrames == 6000) || (mgr->demoIndex == 1 && mgr->demoFrames == 4800) ||
            (mgr->demoIndex == 2 && mgr->demoFrames == 4920) || (mgr->demoIndex == 3 && mgr->demoFrames == 6900))
        {
            ScreenEffect::StartEffect(2, 120, 0, 0, 0, 0x15);
            g_App.FadeOutMusic(3.0f);
        }
        if ((mgr->demoIndex == 0 && mgr->demoFrames >= 6120) || (mgr->demoIndex == 1 && mgr->demoFrames >= 4920) ||
            (mgr->demoIndex == 2 && mgr->demoFrames >= 5040) || (mgr->demoIndex == 3 && mgr->demoFrames == 7020))
        {
            g_App.scene = 1;
            return 3;
        }
    }
    g_GameManager.stats->checksum = g_GameManager.stats->lifeChecksumKeys[2];
    check = mgr->ComputeChecksum();
    g_GameManager.checksumValue = (float)check + g_GameManager.stats->checksumKeysD[3];
    for (i = 0; i < 7; i++)
    {
        if ((int)mgr->stats->lifeChecksumKeys[i] < 6543 || (int)mgr->stats->lifeChecksumKeys[i] > 106543)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->statsChecksumKeysB[i] < 6543.0f || mgr->stats->statsChecksumKeysB[i] > 106543.0f)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    mgr->isPlaying = !mgr->isInContinueMenu && !mgr->isPaused ? 1 : 0;
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->statsChecksumKeysA[i] < 6543.0f || mgr->stats->statsChecksumKeysA[i] > 106543.0f)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 8; i++)
    {
        if (mgr->stats->checksumKeysD[i] < 6543 || mgr->stats->checksumKeysD[i] > 106543)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_Stage.fog.color, 1.0f, 0);
    if (mgr->isPaused == 1 || mgr->isPaused == 2 || mgr->isInContinueMenu)
    {
        return 3;
    }
    if (mgr->stats->score >= 1000000000)
    {
        mgr->stats->score = 999999999;
    }
    if (mgr->stats->guiScore != mgr->stats->score)
    {
        if (mgr->stats->score < mgr->stats->guiScore)
        {
            mgr->stats->score = mgr->stats->guiScore;
        }
        step = (mgr->stats->score - mgr->stats->guiScore) >> 5;
        if (step >= 0x8d55e)
        {
            step = 0x8d55e;
        }
        else if (step == 0)
        {
            step = 1;
        }
        if (mgr->stats->scoreIncrement < step)
        {
            mgr->stats->scoreIncrement = step;
        }
        if (mgr->stats->guiScore + mgr->stats->scoreIncrement > mgr->stats->score)
        {
            mgr->stats->scoreIncrement = mgr->stats->score - mgr->stats->guiScore;
        }
        mgr->stats->guiScore += mgr->stats->scoreIncrement;
        if (mgr->stats->guiScore >= mgr->stats->score)
        {
            mgr->stats->scoreIncrement = 0;
            mgr->stats->guiScore = mgr->stats->score;
        }
        if (mgr->stats->highScore < mgr->stats->guiScore)
        {
            mgr->stats->highScore = mgr->stats->guiScore;
            mgr->stats->bestContinuesUsed = mgr->stats->continuesUsed;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (mgr->stats->lifeChecksumKeysF[i] < 6543.0f || mgr->stats->lifeChecksumKeysF[i] > 106543.0f)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (mgr->stats->statsChecksumKeysC[i] < 6543.0f || mgr->stats->statsChecksumKeysC[i] > 106543.0f)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    for (i = 0; i < 5; i++)
    {
        if ((int)mgr->stats->checksumKeysE[i] < 6543 || (int)mgr->stats->checksumKeysE[i] > 106543)
        {
            g_GameManager.checksumValue = -9999.0f;
        }
    }
    if (g_GameManager.savedConfig->scoreDisabled)
    {
        g_GameManager.skipFpsMeasure = 0;
        mgr->slowdownCounter++;
        if ((g_BulletManager.bulletCount >= 320 && mgr->slowdownCounter % 3 == 0) ||
            (g_BulletManager.bulletCount < 320 && g_BulletManager.bulletCount >= 224 && mgr->slowdownCounter % 4 == 0) ||
            (g_BulletManager.bulletCount < 224 && g_BulletManager.bulletCount >= 128 && mgr->slowdownCounter % 5 == 0))
        {
            g_GameManager.skipFpsMeasure = 1;
            return 3;
        }
        if (g_BulletManager.bulletCount < 128)
        {
            mgr->slowdownCounter = 0;
        }
    }
    g_GameManager.ChecksumMismatch();
    mgr->stageFrames++;
    return 1;
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
    if ((mgr->stateFlags >> 5 & 3) == 1)
    {
        return 3;
    }
    if (mgr->loadingThreadState != 0)
    {
        return 3;
    }
    return 1;
}

int RegisterGameManagerChain()
{
    GameManager *mgr = &g_GameManager;

    g_GameManagerCalcChainElem.SetCallback((ChainCallback)GameManager::OnUpdate);
    g_GameManagerCalcChainElem.addedCallback = (ChainCallback)GameManager::AddedCallback;
    g_GameManagerCalcChainElem.deletedCallback = (ChainCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChainElem.arg = mgr;
    mgr->stageFrames = 0;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChainElem, 2) != 0)
    {
        return -1;
    }
    g_GameManagerDrawProc.SetCallback((ChainCallback)GameManager::OnDraw);
    g_GameManagerDrawProc.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawProc, 5);
    return 0;
}

int GameManager::AddedCallback(GameManager *mgr)
{
    if (g_App.scene != 3 && g_App.scene != 11 && g_App.scene != 12)
    {
        g_App.loadsResources = 1;
    }
    else
    {
        g_App.loadsResources = 0;
    }
    g_GameManager.loadingThreadState = 1;
    if (g_App.prevScene == 1)
    {
        D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else
    {
        D3DXVECTOR3 pos(280.0f, 430.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
    }
    if (mgr->stageEndState >= 2)
    {
        mgr->stageEndState = 1;
    }
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)GameManager::LoadingThread, NULL);
    return 0;
}

void GameManager::LoadingThread(GameManager *arg)
{
    GameManager *mgr = &g_GameManager;
    int size;
    WORD seed;
    int j;

    mgr->loadingFrames = 0;
    g_App.lastTotalPlayTimeTick = timeGetTime();
    mgr->stageBit = 1 << mgr->stage;
    mgr->replayStage = mgr->stage;
    if (mgr->difficulty < 4)
    {
        mgr->difficultyMask = 1 << mgr->difficulty;
    }
    else
    {
        mgr->difficultyMask = 0xf;
    }
    mgr->shotIndex = mgr->character + mgr->shotType;
    g_App.gameSpeed = 1.0f;
    mgr->stateFlags &= ~0x400;
    if (g_App.loadsResources != 0 || mgr->stateFlags >> 14 & 1 || g_GameManager.stateFlags & 1 || g_GameManager.difficulty >= 4)
    {
        if (mgr->savedConfig != NULL)
        {
            delete mgr->savedConfig;
            mgr->savedConfig = NULL;
        }
        if (mgr->stats != NULL)
        {
            delete mgr->stats;
            mgr->stats = NULL;
        }
        size = g_Rng.GetRandomU32InRange(0xffff) + 0x10;
        mgr->tempBuffer = malloc(size);
        mgr->savedConfig = new GameConfig;
        mgr->stats = new GameManagerStats;
        RerollStatsChecksumKeys();
        *mgr->savedConfig = *(GameConfig *)&g_App.controllerMapping;
        MemFree(mgr->tempBuffer);
        mgr->powerItemCombo = 0;
        mgr->stats->gauge = 0;
        mgr->SetClock(g_GameManager.stage == 8 ? 6 : 0);
        if (g_GameManager.difficulty >= 4)
        {
            mgr->savedConfig->lifeCount = 2;
        }
        if (g_GameManager.stateFlags & 1)
        {
            mgr->savedConfig->lifeCount = 8;
        }
        if (Player::RegisterChain(0) != 0)
        {
            if (g_App.closeSubThread)
            {
                return;
            }
            LogPrintf(&g_Log, "error : プレイヤーの初期化に失敗しました\r\n");
            goto fail;
        }
        if (!(g_GameManager.stateFlags >> 3 & 1))
        {
            g_GameManager.SetLives(mgr->savedConfig->lifeCount);
            g_GameManager.SetBombs(g_Player.shotType->bombs);
        }
        mgr->SetPlayArea();
        mgr->stats->power = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->progressFrames = 0;
        g_GameManager.humanRateFrames = 0;
        g_GameManager.humanFrames = 0;
        mgr->stats->guiScore = 0;
        mgr->stats->score = 0;
        mgr->stats->scoreIncrement = 0;
        mgr->stats->highScore = 100000;
        mgr->stats->continuesUsed = 0;
        mgr->stats->grazeTotal = 0;
        mgr->stats->pointItemsTotal = 0;
        if (mgr->difficulty >= 4 || mgr->stateFlags & 1 || mgr->stateFlags >> 14 & 1)
        {
            mgr->savedConfig->scoreDisabled = 0;
        }
        switch (g_GameManager.difficulty)
        {
        case 0:
            mgr->stats->maxPointValue = 60000;
            break;
        case 1:
            mgr->stats->maxPointValue = 100000;
            break;
        case 2:
            mgr->stats->maxPointValue = 200000;
            break;
        case 3:
            mgr->stats->maxPointValue = 300000;
            break;
        case 4:
            mgr->stats->maxPointValue = 300000;
            break;
        }
        mgr->stats->checksumKeyStage = 0;
        SetChecksumKeyPoints();
        if (LoadScoreFile() != 0)
        {
            goto fail;
        }
        mgr->SetRankForDifficulty();
        mgr->stats->deaths = 0.0f;
        mgr->stats->stageDeaths = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->stats->bombsUsed = 0.0f;
        mgr->stats->stageBombsUsed = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->stats->spellCardsCaptured = 0;
        mgr->gaugeFramesTotal = 0;
        mgr->highGaugeFramesTotal = 0;
        mgr->lowGaugeFramesTotal = 0;
        if (!(g_GameManager.stateFlags >> 3 & 1) && !(g_GameManager.stateFlags >> 14 & 1))
        {
            if (!mgr->savedConfig->scoreDisabled)
            {
                IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[0], 999999);
                IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[6].plays[0], 999999);
                IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[mgr->character + 1],
                                     999999);
                IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[6].plays[mgr->character + 1], 999999);
                if (g_App.scene == 10)
                {
                    IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].retries, 999999);
                    IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[6].retries, 999999);
                }
                if (g_GameManager.stateFlags & 1 && !(g_GameManager.stateFlags >> 14 & 1))
                {
                    IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].practices, 999999);
                    IncrementStatCounter((DWORD *)&g_GameManager.plst.counts[6].practices, 999999);
                }
            }
        }
        else
        {
            mgr->savedConfig->scoreDisabled = 0;
        }
    }
    else
    {
        mgr->stats->guiScore = mgr->stats->score;
        mgr->stats->scoreIncrement = 0;
        mgr->stats->stageDeaths = 0.0f;
        mgr->RerollChecksumKeys();
        mgr->stats->stageBombsUsed = 0.0f;
        mgr->RerollChecksumKeys();
        if (Player::RegisterChain(0) != 0)
        {
            if (g_App.closeSubThread)
            {
                return;
            }
            LogPrintf(&g_Log, "error : プレイヤーの初期化に失敗しました\r\n");
            goto fail;
        }
    }
    mgr->subRank = 0;
    mgr->stats->stageClearPoints = 0;
    mgr->stats->graze = 0;
    mgr->isPaused = 0;
    mgr->stateFlags &= ~0x180;
    mgr->stateFlags &= ~0x2000;
    mgr->gaugeFrames = 0;
    mgr->highGaugeFrames = 0;
    mgr->lowGaugeFrames = 0;
    mgr->stats->gaugeCopy = mgr->stats->gauge;
    mgr->stats->timeOrbs = 0;
    mgr->stats->timeOrbsCollected = 0;
    if (!(g_GameManager.stateFlags >> 14 & 1))
    {
        mgr->stats->timeOrbsNeeded = g_TimeOrbsNeeded[mgr->stage][g_GameManager.difficulty];
    }
    else
    {
        mgr->stats->timeOrbsNeeded = 0;
    }
    if (mgr->stateFlags & 1)
    {
        if (!(mgr->stateFlags >> 14 & 1))
        {
            switch (mgr->stage)
            {
            case 0:
                mgr->stats->power = 0.0f;
                mgr->RerollChecksumKeys();
                break;
            case 1:
                mgr->stats->power = 112.0f;
                mgr->RerollChecksumKeys();
                break;
            default:
                mgr->stats->power = 128.0f;
                mgr->RerollChecksumKeys();
                break;
            }
        }
        else if (mgr->practiceSpellCard <= 1)
        {
            mgr->stats->power = 30.0f;
            mgr->RerollChecksumKeys();
        }
        else if (mgr->practiceSpellCard <= 12)
        {
            mgr->stats->power = 80.0f;
            mgr->RerollChecksumKeys();
        }
        else
        {
            mgr->stats->power = 128.0f;
            mgr->RerollChecksumKeys();
        }
    }
    if (g_GameManager.stateFlags >> 3 & 1)
    {
        mgr->SetRankForDifficulty();
        ReplayManager::RegisterChain(1, g_GameManager.replayFile);
        seed = g_Rng.seed;
        mgr->RerollChecksumKeys();
        g_Rng.seed = seed;
    }
    mgr->rngSeed = g_Rng.seed;
    if (Stage::RegisterChain(mgr->stage) != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : 背景データの初期化に失敗しました\r\n");
        goto fail;
    }
    if (BulletManager::RegisterChain("etama.anm") != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : 敵弾の初期化に失敗しました\r\n");
        goto fail;
    }
    if (EnemyManager::RegisterChain() != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : 敵の初期化に失敗しました\r\n");
        goto fail;
    }
    if (EffectManager::RegisterChain() != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : エフェクトの初期化に失敗しました\r\n");
        goto fail;
    }
    if (Gui::RegisterChain() != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : 2D表示の初期化に失敗しました\r\n");
        goto fail;
    }
    if (SpellCard::RegisterChain() != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Log, "error : スペルカードの初期化に失敗しました\r\n");
        goto fail;
    }
    if (!(g_GameManager.stateFlags >> 3 & 1))
    {
        ReplayManager::RegisterChain(0, "replay/th8_00.rpy");
    }
    if (g_GameManager.stateFlags >> 14 & 1)
    {
        switch (g_GameManager.stage)
        {
        case 5:
            // Tested and dropped, as the original does.
            g_GameManager.IsPracticingSpellCard(0xd4);
            break;
        case 6:
            if (!g_GameManager.IsPracticingSpellCardIn(0x77, 0x7a))
            {
                g_Stage.dialogueScriptOffset = 2;
            }
            break;
        case 7:
            if (!g_GameManager.IsPracticingSpellCardIn(0x93, 0x96))
            {
                g_Stage.dialogueScriptOffset = 2;
            }
            break;
        case 8:
            if (!g_GameManager.IsPracticingSpellCardIn(0xbf, 0xc1) && !g_GameManager.IsPracticingSpellCard(0xd5))
            {
                g_Stage.dialogueScriptOffset = 2;
            }
            break;
        }
    }
    if (g_App.isRestarting == 0)
    {
        if (g_GameManager.stateFlags >> 14 & 1)
        {
            j = 0;
            while (g_SpellCardBgms[j].lastCard >= 0)
            {
                if (g_GameManager.practiceSpellCard <= g_SpellCardBgms[j].lastCard)
                {
                    g_App.LoadMidi(0, g_SpellCardBgms[j].midiPath);
                    break;
                }
                j++;
            }
        }
        else
        {
            g_App.LoadMidi(0, g_Stage.data->songPaths[0]);
            if (g_Stage.data->songPaths[1][0] != ' ')
            {
                g_App.LoadMidi(1, g_Stage.data->songPaths[1]);
            }
            if (g_Stage.data->songPaths[2][0] != ' ')
            {
                g_App.LoadMidi(2, g_Stage.data->songPaths[2]);
            }
        }
    }
    mgr->isInContinueMenu = 0;
    mgr->stateFlags |= 4;
    if (g_App.isRestarting != 0 && g_GameManager.stateFlags >> 14 & 1 && !GetPracticeBgmRestarts(g_GameManager.practiceSpellCard))
    {
        mgr->pendingMusicStart = 2;
    }
    else
    {
        mgr->pendingMusicStart = 1;
    }
    if (g_App.scene != 3)
    {
        g_App.achievedFrames = 0.0f;
        g_App.expectedFrames = 0.0f;
    }
    mgr->isFrozen = 0;
    mgr->stats->score = 0;
    mgr->stateFlags &= ~0x10;
    g_AsciiText.Initialize();
    g_AsciiText.StartScripts();
    g_GameManager.skipFpsMeasure = 0;
    g_AsciiText.ringAlpha = 0;
    DrawFps(0);
    if (g_GameManager.stateFlags >> 3 & 1)
    {
        while (mgr->loadingFrames < 0x50)
        {
            Sleep(0x11);
        }
    }
    else
    {
        while (mgr->loadingFrames < 0x1e)
        {
            Sleep(0x11);
        }
    }
    g_App.FinishLoadingSprites();
    while ((mgr->stateFlags >> 5 & 3) != 0)
    {
        Sleep(0x11);
    }
    g_GameManager.loadingThreadState = 0;
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
    g_App.redrawFrames = 60;
    mgr->stateFlags &= ~0x200;
    g_App.isRestarting = 0;
    g_ScreenEffectHoldFrames = 2;
    return;

fail:
    g_GameManager.loadingThreadState = 2;
    g_App.FinishLoadingSprites();
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
    g_App.isRestarting = 0;
    g_ScreenEffectHoldFrames = 2;
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
    if (g_GameManager.stateFlags & 1)
    {
        g_GameManager.stats->highScore =
            g_GameManager.pscrs[g_GameManager.character].scores[g_GameManager.stage][g_GameManager.difficulty];
        g_GameManager.pscrs[g_GameManager.character].playCount[g_GameManager.stage][g_GameManager.difficulty]++;
        g_GameManager.pscrs[g_GameManager.character].practised = 1;
    }
    ReleaseScoreDat(data);
    memcpy(g_GameManager.savedCatks, g_GameManager.catks, sizeof(g_GameManager.catks));
    memset(&g_GameManager.hscr, 0, sizeof(g_GameManager.hscr));
    g_GameManager.hscr.shot = g_GameManager.character;
    g_GameManager.hscr.difficulty = g_GameManager.difficulty;
    memcpy(g_GameManager.hscr.config, &g_App.controllerMapping, sizeof(g_GameManager.hscr.config));
    g_GameManager.playFrames = 0;
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
    g_ScreenEffectHoldFrames = 1;
    g_AsciiText.ringAlpha = 0;
    if (g_App.scene != 3 && g_App.scene != 11 && g_App.scene != 12)
    {
        g_App.freesResources = 1;
    }
    else
    {
        g_App.freesResources = 0;
    }
    if (!(g_GameManager.stateFlags >> 14 & 1) || g_App.freesResources != 0)
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
    SpellCard::RemoveChain();
    Stage::RemoveChain();
    BulletManager::RemoveChain();
    Player::RemoveChain();
    EnemyManager::RemoveChain();
    EffectManager::RemoveChain();
    Gui::RemoveChain();
    if (!(g_GameManager.stateFlags >> 3 & 1))
    {
        ReplayManager::EndStage();
    }
    if (!(g_GameManager.stateFlags >> 3 & 1))
    {
        g_App.AddTotalPlayTime();
    }
    g_App.lastTotalPlayTimeTick = 0;
    g_App.AddPlayTime();
    mgr->stateFlags &= ~4;
    g_AsciiText.Initialize();
    g_GameManager.skipFpsMeasure = 0;
    g_GameManager.stageFrames = 0;
    return 0;
}

void RemoveGameManagerChain()
{
    g_Chain.Remove(&g_GameManagerCalcChainElem);
    g_Chain.Remove(&g_GameManagerDrawProc);
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

void GameManager::AddGauge(int amount, int force)
{
    if (g_Player.bomb.running != 0 && force == 0)
    {
        return;
    }
    this->stats->gauge += amount;
    if (this->stats->gauge < this->gaugeMin)
    {
        this->stats->gauge = this->gaugeMin;
    }
    else if (this->stats->gauge > this->gaugeMax)
    {
        this->stats->gauge = this->gaugeMax;
    }
    this->stats->gaugeCopy = this->stats->gauge;
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

int GameManager::HasClearedFinalA(int shot)
{
    return shot > 3 || this->clrds[shot].stagesCleared[0] & 0x8000 || this->clrds[shot].stagesCleared[1] & 0x8000 ||
                   this->clrds[shot].stagesCleared[2] & 0x8000 || this->clrds[shot].stagesCleared[3] & 0x8000
               ? TRUE
               : FALSE;
}

int GameManager::AnyShotHasClearedFinalA()
{
    return this->HasClearedFinalA(0) || this->HasClearedFinalA(1) || this->HasClearedFinalA(2) || this->HasClearedFinalA(3) ? TRUE : FALSE;
}

int GameManager::IsPhantasmOpen()
{
    return 0;
}

int GameManager::IsPracticeReplay()
{
    return (this->stateFlags >> 3 & 1) && g_ReplayManager->data->isPractice ? TRUE : FALSE;
}

int GameManager::GetStageClearClockStep()
{
    switch (g_GameManager.stage)
    {
    case 0:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 1:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 2:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 3:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 4:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
        if (value >= limit)
        {
            return 1;
        }
        return 2;
    }
    case 5:
    {
        int value = g_GameManager.stats->timeOrbs;
        int limit = g_GameManager.stats->timeOrbsNeeded;
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
        this->stage = (g_GameManager.stateFlags >> 11 & 3) != 0 ? 7 : 6;
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
    this->demoIndex = 3;
}
