// The game's state across stages, as th06's GameManager.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// What th07's GameManager points to at +0x8 and +0x4. Only the fields
// known so far.
// th07's GameStats, laid out anew.
struct GameStats
{
    // The score shown.
    DWORD guiScore;
    // The grazes of the stage, as th07's graze.
    int graze;
    DWORD score;
    // The game's grazes.
    int grazeTotal;
    // How much GameManager::OnUpdate moves guiScore on per frame, as
    // th07's.
    DWORD scoreIncrement;
    // The high score to beat, from the score file, and what
    // LoadHighScores stores from the best entry, as th07's.
    DWORD highScore;
    BYTE bestContinuesUsed;
    unsigned char pad_19[0x3];
    int spellCardsCaptured;
    // A copy of gauge, which is kept in each replay stage; AddGauge
    // moves it.
    short gaugeCopy;
    short gauge;
    int maxPointValue;
    char clock;
    BYTE continuesUsed;
    unsigned char pad_2a[0x2];
    int stageClearPoints;
    int pointItemsTotal;
    // Kept in each replay stage.
    int checksumKeyStage;
    int checksumKeyPoints;
    // GetStageClearClockStep compares them.
    int timeOrbs;
    int timeOrbsNeeded;
    int timeOrbsCollected;
    // Set to 6543 plus a random number under 100000 by
    // RerollStatsChecksumKeys, as th07's.
    DWORD lifeChecksumKeys[7];
    // Floats, as th07's.
    float deaths;
    float stageDeaths;
    // Set to random floats from 6543 up by RerollStatsChecksumKeys, as
    // statsChecksumKeysB, lifeChecksumKeysF and statsChecksumKeysC.
    float statsChecksumKeysA[2];
    // The lives left, a float behind checksums, as the other counts.
    float lives;
    float statsChecksumKeysB[2];
    // The bombs left, the same way.
    float bombs;
    float bombsUsed;
    float stageBombsUsed;
    float lifeChecksumKeysF[3];
    // The power, a float now.
    float power;
    float statsChecksumKeysC[2];
    // Player::Graze subtracts it from GameManager::bonusPoints, as th07's.
    int grazeCount;
    int checksumKeysD[8];
    // ChecksumBytes adds checksumKeysE[2] to it for every byte it sums.
    int checksum;
    // The checksum, from ComputeChecksum.
    int computedChecksum;
    DWORD checksumKeysE[5];
};
struct GameConfig
{
    unsigned char pad_0[0x1c];
    // The lives a game starts with, as th07's.
    BYTE lives;
    unsigned char pad_1d[0x8];
    // App::IsScoreDisabled tests it.
    BYTE scoreDisabled;
    unsigned char pad_26[0x16];
};
ASSERT_SIZE(GameConfig, 0x3c);

// th07's GameManager (g_GameManager there), at 0x160f508. Only the
// fields known so far.
class GameManager
{
public:
    // The update chain callback and its added and deleted callbacks,
    // as th07's GameManager's.
    static int OnUpdate(GameManager *mgr);
    // The update element's added callback, split as App's: clears or sets
    // App::loadsResources by the scene, sets loadingThreadState, starts the
    // loading sprites (and, coming from the title, the loading screen
    // effect), turns the result screen's mode 2 and 3 into 1 and runs
    // LoadingThread in the sub thread for th07's AddedCallback proper.
    // Returns 0.
    static int AddedCallback(GameManager *mgr);
    // The rest of th07's AddedCallback, run in the sub thread (with /Gr,
    // a fastcall function passed to CreateThread). Works on g_GameManager.
    static void LoadingThread(GameManager *arg);
    static int DeletedCallback(GameManager *mgr);
    // The draw chain callback: moves isPaused on from 1 to 2. Returns 1,
    // or 3 outside scene 2, in the result screen's mode (bits 5-6 of
    // stateFlags) 1 or while loadingThreadState is set.
    static int OnDraw(GameManager *mgr);
    // Zeroes the GameManager and sets arena to the play area (32, 16,
    // 384, 448) and demoIndex to 3. Emitted where g_GameManager is defined
    // (0x43c5e1).
    GameManager();
    // Whether the checksummed fields of `stats` no longer agree, as
    // th07's (0x40bb80).
    int ChecksumMismatch();
    // Rerolls the checksum keys lifeChecksumKeys[2] and checksumKeysD[3]
    // and recomputes the checksums, as th07's. Inline; the copy at
    // 0x406e50 is the ascii object's.
    void RerollChecksumKeys()
    {
        this->stats->lifeChecksumKeys[2] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->checksumKeysD[3] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->checksum = this->stats->lifeChecksumKeys[2];
        this->stats->computedChecksum = this->ComputeChecksum();
        this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
    }
    // Add `lives` to stats->lives and `bombs` to stats->bombs and
    // recompute the checksums; wipe g_App with 0xff first if the checksums
    // were already wrong, as th07's. Inline; the copies are at 0x43c641
    // (the GameManager object's) and 0x439883 (another object's).
    void AddLives(int lives)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->lives += lives;
        this->RerollChecksumKeys();
    }
    void AddBombs(int bombs)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->bombs += bombs;
        this->RerollChecksumKeys();
    }
    // The same for `power` and stats->power, as th07's. Inline; the copy
    // at 0x441850 is the item object's.
    void AddPower(int power)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->power += power;
        this->RerollChecksumKeys();
    }
    // Sets arena to the play area (32, 16, 384, 448) and playerArea to (8,
    // 16, 368, 416), as th07's. Inline; the copy at 0x43c686 is the
    // GameManager object's.
    void SetPlayArea()
    {
        this->arena[0] = 32.0f;
        this->arena[1] = 16.0f;
        this->arena[2] = 384.0f;
        this->arena[3] = 448.0f;
        this->playerArea[0] = 8.0f;
        this->playerArea[1] = 16.0f;
        this->playerArea[2] = 368.0f;
        this->playerArea[3] = 416.0f;
    }
    // The checksum: ChecksumBytes over stats from lifeChecksumKeys to
    // checksum, over stats->checksumKeysE, over the config savedConfig points
    // at, over g_App's config (0x3c bytes each) and, new in th08, over
    // hscr.
    int ComputeChecksum();
    // Whether the character plays alone (4 and up) and is the human of
    // its team (even) or the youkai (odd). Inline; the copies at 0x42f230
    // and 0x42f270 are in an /Od object.
    int IsSoloHuman()
    {
        return this->character >= 4 && !(this->character & 1) ? TRUE : FALSE;
    }
    int IsSoloYoukai()
    {
        return this->character >= 4 && (this->character & 1) ? TRUE : FALSE;
    }
    // Rerolls stats' lifeChecksumKeys and lifeChecksumKeysF, as th07's.
    // Inline; the copy at 0x44e160 is the player object's.
    void RerollLifeChecksumKeys()
    {
        this->stats->lifeChecksumKeys[0] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[1] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[2] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[3] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[4] = g_Random.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeysF[0] = g_Random.GetRandomF32InRange(100000.0f) + 6543.0f;
        this->stats->lifeChecksumKeysF[1] = g_Random.GetRandomF32InRange(100000.0f) + 6543.0f;
        this->stats->lifeChecksumKeysF[2] = g_Random.GetRandomF32InRange(100000.0f) + 6543.0f;
    }
    // Adds `deaths` to stats->deaths and stageDeaths and counts a miss in
    // hscr; adds `bombs` to stats->bombsUsed and stageBombsUsed. Both wipe g_App
    // first if the checksums were wrong, as th07's (which were out of
    // line). Inline; the copies at 0x44e260 and 0x44e2e0 are the player
    // object's.
    void AddDeaths(int deaths)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->deaths += deaths;
        this->stats->stageDeaths += deaths;
        this->hscr.misses++;
        this->RerollChecksumKeys();
    }
    void AddBombsUsed(int bombs)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->bombsUsed += bombs;
        this->stats->stageBombsUsed += bombs;
        this->RerollChecksumKeys();
    }
    // Sets stats->gauge. Inline; the copy at 0x44e140 is the player
    // object's.
    void SetGauge(short value)
    {
        this->stats->gauge = value;
    }
    // Sets isPaused. Inline and expanded where it is used (SetUpTitleScene,
    // in the MainMenu object): that expansion is what leaves the 11 slots
    // MainMenu's too-large inline constructor reserves there.
    void SetIsPaused(BYTE value)
    {
        this->isPaused = value;
    }
    // Gives a life, or a bomb when the lives are full (8). Either one
    // plays sound 0x1c, adds 200 to the subrank and marks the Gui's
    // count changed, as th07's.
    void AddLifeOrBomb();
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th07's.
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Sets the rank and its bounds from g_Ranks for the difficulty.
    void SetRankForDifficulty();
    // Adds `amount` hundredths to the rank, as th07's.
    void AddSubRank(int amount);
    // Takes `amount` hundredths from the rank, as th07's.
    void SubSubRank(int amount);
    // Unless g_Player.bomb.running is set and `force` is not, adds `amount` to
    // stats->gauge, within gaugeMin and gaugeMax, and copies it to
    // stats->gaugeCopy.
    void AddGauge(int amount, int force);
    // Whether shot type `shot` (0 to 3) cleared one of the four main
    // difficulties, as th07's (0x43c15f).
    int HasExtraOpen(int shot);
    // Whether a shot type has Extra open (0x43c1e9).
    int AnyShotHasExtraOpen();
    // Whether shot type `shot` (0 to 3) has bit 0x8000 set in
    // stagesCleared of one of the four main difficulties.
    int HasClearedFinalA(int shot);
    // Whether a shot type has 0x43c23b set (0x43c2c5).
    int AnyShotHasClearedFinalA();
    // Whether every shot type has Extra open. In App's object.
    int AllShotsHaveExtraOpen();
    // Whether spell card `card` was attempted, in a game or in practice,
    // or, for a last spell, seen (flsp). The copy at 0x46fd5f is the
    // title menu object's.
    int IsSpellCardSeen(int card);
    // th07's IsPhantasmOpen, which returns 0 in th08 (0x43c317).
    int IsPhantasmOpen();
    // Whether a replay (bit 3 of stateFlags) of a practice is playing.
    int IsPracticeReplay();
    // By stage: 0 for 6 and 7, 4 past them, else 1, or 2 while stats'
    // timeOrbs is below timeOrbsNeeded.
    int GetStageClearClockStep();
    // Moves `stage` on to the next stage: from stage 3 (index 2) to 4A or
    // 4B by the shot type, and from 6 to 6B if bits 11-12 of stateFlags
    // are set, else 6A (0x43c4b3).
    void AdvanceStage();
    // stats->gauge, and whether it reached each of gaugeLow1, gaugeLow2,
    // gaugeHigh1 and gaugeHigh2. Inline; the copies at 0x406cf0 and
    // 0x406d10 to 0x406da0 are in an /Od object.
    int GetGauge()
    {
        return this->stats->gauge;
    }
    BOOL IsGaugeAtLow1()
    {
        return this->stats->gauge <= this->gaugeLow1;
    }
    BOOL IsGaugeAtLow2()
    {
        return this->stats->gauge <= this->gaugeLow2;
    }
    BOOL IsGaugeAtHigh1()
    {
        return this->stats->gauge >= this->gaugeHigh1;
    }
    BOOL IsGaugeAtHigh2()
    {
        return this->stats->gauge >= this->gaugeHigh2;
    }
    // stats->timeOrbs and timeOrbsNeeded, which DrawHud shows as "%d/%d" on
    // its last row, in 0xfffff0c0 once timeOrbs reaches timeOrbsNeeded.
    // Inline; the copies at 0x421b60 and 0x421b80 are in an /Od object.
    int GetTimeOrbs()
    {
        return this->stats->timeOrbs;
    }
    // Adds `amount` time orbs (stats->timeOrbs; a negative amount larger than
    // what there is clears them instead) to timeOrbs, stats->timeOrbsCollected
    // and hscr.timeOrbs; a gain also raises stats->maxPointValue by 10 per two,
    // rounding with timeOrbsCollected's parity. Inline; the copy at 0x418220 is
    // in an /Od object.
    void AddTimeOrbs(int amount)
    {
        if (amount >= 0 || this->stats->timeOrbs >= -amount)
        {
            this->stats->timeOrbs += amount;
            this->stats->timeOrbsCollected += amount;
            this->hscr.timeOrbs += amount;
            this->RerollChecksumKeys();
            if (amount > 0)
            {
                amount += this->stats->timeOrbsCollected & 1;
                this->stats->maxPointValue += amount / 2 * 10;
            }
        }
        else
        {
            this->stats->timeOrbs = 0;
        }
    }
    int GetTimeOrbsNeeded()
    {
        return this->stats->timeOrbsNeeded;
    }
    // Whether a demo plays: stateFlags's bit 1, as th07's stateFlags.
    // Inline; the copy at 0x406cb0 is in an /Od object.
    int IsDemo()
    {
        return this->stateFlags >> 1 & 1;
    }
    // Whether the game is a practice: stateFlags's bit 0, which th07 read
    // directly. Inline; the copy at 0x406cd0 is in an /Od object.
    int IsInPractice()
    {
        return this->stateFlags & 1;
    }
    // The deaths and bombs used, as ints. New in th08; inline, the copies
    // at 0x453c80 and 0x453ca0 are the replay object's.
    int GetDeaths()
    {
        return this->stats->deaths;
    }
    int GetBombsUsed()
    {
        return this->stats->bombsUsed;
    }
    // Adds `points` to stats->score, which counts in tens, as th07's.
    // Inline; the copy at 0x4181f0 is in an /Od object.
    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }
    // The power, lives and bombs as ints, and setters for them (as ints;
    // they also update what depends on them). Copies in other objects:
    // GetPower 0x422480, GetLives 0x42f2b0, GetBombs 0x4398ce, SetLives
    // 0x406e20, SetBombs 0x406f30, SetPower 0x406fa0.
    int GetPower()
    {
        return this->stats->power;
    }
    // From `lo` to `hi` by the rank (0 to 32), as th07's free RankLerp
    // and RankLerpF. Inline; the copies at 0x421ba0 and 0x422b80 are an
    // /Od object's.
    int RankLerp(int lo, int hi)
    {
        return this->rank * (hi - lo) / 32 + lo;
    }
    float RankLerpF(float lo, float hi)
    {
        return this->rank * (hi - lo) / 32.0f + lo;
    }
    int GetLives()
    {
        return this->stats->lives;
    }
    int GetBombs()
    {
        return this->stats->bombs;
    }
    void SetLives(int lives)
    {
        this->stats->lives = lives;
        this->RerollChecksumKeys();
    }
    // SetBombs recomputes the checksums without new keys. Inline; the
    // copy at 0x406f30 is the ascii object's.
    void SetBombs(int bombs)
    {
        this->stats->bombs = bombs;
        this->stats->checksum = this->stats->lifeChecksumKeys[2];
        this->stats->computedChecksum = this->ComputeChecksum();
        this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
    }
    // Sets stats->power (a float) and rerolls the checksum keys. Inline;
    // the copy at 0x406fa0 is the ascii object's.
    void SetPower(int power)
    {
        this->stats->power = power;
        this->RerollChecksumKeys();
    }
    // stats->clock and a setter for it, inline: the copies at 0x406dd0
    // and 0x453c60 are another object's and the replay object's.
    char GetClock()
    {
        return this->stats->clock;
    }
    void SetClock(int value)
    {
        this->stats->clock = value;
    }
    // Adds `amount` to stats->clock. Inline; the copy at 0x406df0 is in
    // an /Od object.
    void AddClock(char amount)
    {
        this->stats->clock += amount;
    }
    // Whether a spell card is practised (bit 14 of stateFlags). Inline;
    // the copy at 0x406c70 is an /Od object's.
    int IsInSpellCardPractice()
    {
        return this->stateFlags >> 14 & 1;
    }
    // Whether the spell card practised (bit 14 of stateFlags) is `card`,
    // or one from `first` to `last`. Inline.
    int IsPracticingSpellCard(int card)
    {
        return this->stateFlags >> 14 & 1 ? this->practiceSpellCard == card : FALSE;
    }
    int IsPracticingSpellCardIn(int first, int last)
    {
        return this->stateFlags >> 14 & 1 ? (this->practiceSpellCard >= first && this->practiceSpellCard <= last ? TRUE : FALSE)
                                         : FALSE;
    }
    // Whether shot type `shot` cleared stage bit `stage` of `difficulty`
    // without continues, or at all. Inline; the copies at 0x439829 and
    // 0x439856 are the Gui object's.
    int HasClearedStage(int stage, int shot, int difficulty)
    {
        return this->clrds[shot].stagesClearedNoContinue[difficulty] & (1 << stage);
    }
    int HasClearedStageWithContinues(int stage, int shot, int difficulty)
    {
        return this->clrds[shot].stagesCleared[difficulty] & (1 << stage);
    }
    // Whether a replay plays (bit 3 of stateFlags). Inline; the copy at
    // 0x406c90 is an /Od object's.
    void SetIsInReplay(int isInReplay)
    {
        this->isInReplay = isInReplay;
    }
    int IsReplay()
    {
        return this->stateFlags >> 3 & 1;
    }

    // A block of random size LoadingThread allocates and frees at once,
    // as th07's tempBuffer.
    void *tempBuffer;
    GameConfig *savedConfig;
    GameStats *stats;
    // The score file's chunks, loaded by App::AddedCallback.
    Flsp flsp;
    // Set while the game is paused: the screen shakes hold still. As
    // th07's isFrozen.
    char isFrozen;
    // As th07's skipFpsMeasure: while it is set, the frame rate is not measured
    // and the buttons are only added to.
    char skipFpsMeasure;
    unsigned char pad_2e[0x2];
    int difficulty;
    // 1 << difficulty, or 0xf from Extra on, as th07's.
    int difficultyMask;
    // OnDraw returns 3 while it is set; OnUpdate returns 4 while it is 2
    // and counts loadingFrames up while it is otherwise set.
    int loadingThreadState;
    int loadingFrames;
    Catk catks[0xde];
    // A copy of catks, as th07's savedCatks.
    Catk savedCatks[0xde];
    Clrd clrds[0xd];
    Pscr pscrs[0xc];
    // The play statistics of the score file.
    Plst plst;
    // The score entry of the game being played: HandleNameEntry fills
    // in the rest and enters a copy. New in th08.
    Hscr hscr;
    DWORD playFrames;
    // Set when the game is paused; the replay records it (as th07's).
    int pauseRequested;
    // The scene OnUpdate switches to when the result screen's mode ends,
    // or -1.
    int nextScene;
    int humanFrames;
    int humanRateFrames;
    BYTE powerItemCombo;
    // As th07's: the shot type is character + shotType.
    BYTE character;
    BYTE shotType;
    // character + shotType, as th07's shotIndex.
    BYTE shotIndex;
    union
    {
        // Bit 2: the speed is measured; 3: it is shown, as th07's
        // stateFlags.
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits3dbac_0 : 2;
            // Set while neither paused nor in the continue menu, as
            // th07's isPlaying.
            unsigned int isPlaying : 1;
            // IsReplay, as th07's isInReplay.
            unsigned int isInReplay : 1;
            unsigned int isGameCleared : 1;
            // The result screen's mode: OnDraw returns 3 in mode 1, and
            // AddedCallback turns 2 and 3 into 1.
            unsigned int stageEndState : 2;
            // Gui::DrawHud draws GuiImpl::livesBombsOverlay with the lives and
            // bombs while it is 1 and g_SpellCard.IsActive().
            unsigned int lastSpellState : 2;
            unsigned int stageClearStarted : 1;
            // The screen shakes hold still while it is set.
            unsigned int deathbombFreeze : 1;
            // The clear level GuiImpl::StartMsgScript picked for the
            // ending dialogue: AdvanceStage goes to 6B unless it is 0.
            unsigned int clearLevel : 2;
            unsigned int bossHitAnywhere : 1;
            // Set while a spell card is practised (IsInSpellCardPractice).
            unsigned int isInSpellCardPractice : 1;
            // AnyShotHasExtraOpen, AnyShotHasClearedFinalA and
            // AllShotsHaveExtraOpen as App::AddedCallback found them.
            unsigned int anyShotHasExtraOpen : 1;
            unsigned int anyShotHasClearedFinalA : 1;
            unsigned int allShotsHaveExtraOpen : 1;
            unsigned int unusedBits3dbac_18 : 14;
        };
    };
    // The spell card being practised, kept in the replay when bit 14 of
    // stateFlags is set.
    short practiceSpellCard;
    // While it is set, PresentFrame keeps redrawFrames and fade outs
    // hold still. As th07's isPaused.
    BYTE isPaused;
    // Set while the continue menu shows, as th07's: fade outs hold
    // still.
    BYTE isInContinueMenu;
    // The demo being played (0 to 3), as th07's.
    BYTE demoIndex;
    // 2 while a replay is fast-forwarded, as th07's.
    BYTE replayMenuChoice;
    unsigned char pad_3dbb6[0x2];
    // The frames a demo has played, as th07's.
    int demoFrames;
    // The replay file to play, as th07's.
    char replayFile[0x200];
    // The random seed each replay stage starts with, as th07's.
    WORD rngSeed;
    unsigned char pad_3ddbe[0x2];
    // The screen shakes end at once while this is 1 or less, as th07's.
    int stageFrames;
    // The stage being played, from 0.
    int stage;
    // The replay stage being recorded.
    int replayStage;
    unsigned char pad_3ddcc[0x4];
    // ORed into the shot type's stage clear flags when a game ends.
    WORD stageBit;
    unsigned char pad_3ddd2[0x2];
    // The play area: x, y, width and height, and a second rectangle set
    // with it by SetPlayArea, as th07's.
    float arena[4];
    float playerArea[4];
    // As th07's checksumValue: SetBombs sets it.
    float checksumValue;
    // The bounds of stats->gauge.
    short gaugeMin;
    short gaugeMax;
    // Where AsciiText::DrawPopups starts tinting the stats->gauge
    // gauge: blue at or below gaugeLow1 (light blue at gaugeLow2), red at
    // or above gaugeHigh1 (light red at gaugeHigh2).
    short gaugeLow1;
    short gaugeHigh1;
    short gaugeLow2;
    short gaugeHigh2;
    // The frames played, as th07's.
    int progressFrames;
    // Counts the frames that slowdown may skip, as th07's.
    DWORD slowdownCounter;
    unsigned char pad_3de0c[0x4];
    // Cleared by LoadingThread.
    int gaugeFramesTotal;
    // Frames counted for the stage clear screen's "over 80%" lines:
    // all of them, and the two kinds counted.
    int gaugeFrames;
    int highGaugeFramesTotal;
    int lowGaugeFramesTotal;
    int highGaugeFrames;
    int lowGaugeFrames;
    // Set when the stage music is to start: OnUpdate stops the loading
    // screen and, while it is 1, plays the stage's music.
    int pendingMusicStart;
    // A replay stage keeps it. The rank, as th07's: SetRankForDifficulty
    // sets it and its bounds from g_Ranks.
    int rank;
    int maxRank;
    int minRank;
    // Hundredths of a rank, moved by AddSubRank and SubSubRank.
    int subRank;
};
ASSERT_SIZE(GameManager, 0x3de3c);
extern GameManager g_GameManager;

// The update and draw chain elements of g_GameManager, after it.
extern ChainElem g_GameManagerCalcChainElem;
extern ChainElem g_GameManagerDrawChainElem;

// The rank a game starts at and its bounds, as th07's.
struct RankSettings
{
    int rank;
    int minRank;
    int maxRank;
};
// By difficulty.
extern RankSettings g_Ranks[6];

// The sum of the `size` bytes at `data`. Adds
// g_GameManager.stats->checksumKeysE[2] to its checksum for every byte, as
// th07's.
int ChecksumBytes(BYTE *data, int size);
// Fills the checksum keys in g_GameManager.stats with random numbers
// from 6543 up, then stores the checksum, as th07's.
void RerollStatsChecksumKeys();
// The score loading of th07's LoadScoreFile: saves the score file
// (ResultScreen::RegisterChain(2)), resets the spell cards, then loads the
// high score, the spell cards, the clears and the practice scores from
// "score.dat" (a practice takes its high score from the practice scores,
// counts the play and marks the shot type's practice scores to be saved)
// and keeps a copy of the spell cards in savedCatks. New in th08: then
// starts hscr with the shot type, difficulty and config. Returns 0, or -1
// if the file cannot be read.
int LoadScoreFile();
// Adds one to `*count` while it is under 999999. Its callers pass
// 999999 as `max`, which it ignores.
void IncrementStatCounter(DWORD *count, DWORD max);
