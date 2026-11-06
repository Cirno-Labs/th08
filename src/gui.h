// The front: the interface drawn over the game and the dialogue, as th06's.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// A dialogue file. Only what is known so far.
struct MsgFile
{
    int count;
    // The scripts, by index.
    void *scripts[1];
};

// The arguments of a message instruction, as th07's: who it is for (a
// portrait 0 to 3; for opcode 3 the speaker, whose colours it takes) and a
// sprite, script or line number, then the text. Opcodes 16, 19 and 20
// have only the text.
struct MsgArgs
{
    short who;
    union
    {
        short index;
        BYTE value;
    };
    char text[1];
};

// The arguments of opcodes 15 and 17: the portrait that speaks, then a
// sprite for each portrait (negative to keep it). New in th08.
struct MsgPortraitArgs
{
    int who;
    int sprites[4];
};

// One instruction of a message script, as th07's.
struct MsgInstr
{
    // The frame it runs on.
    WORD time;
    BYTE opcode;
    // The size of the arguments, in bytes.
    BYTE argsSize;
    union
    {
        MsgArgs args;
        MsgPortraitArgs portraitArgs;
        int i;
        BYTE b;
    };
};

// The dialogue state, as th07's GuiMsg (0x1570 bytes now).
struct GuiMsg
{
    MsgFile *msgFile;
    // The instruction being run.
    MsgInstr *instr;
    // The script running, -1 for none.
    int scriptIndex;
    Timer timer;
    int waitFrames;
    // How long opcodes 4 and 21 wait before the shot button ends them: 30
    // once a wait ran out, 8 once the button ended one. New in th08.
    int minWaitFrames;
    Sprite portraits[4];
    Sprite text[2];
    Sprite nameplates[2];
    // The text and shadow colours of the two lines, by character now.
    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];
    int fontSize;
    // While it is above 0, the dialogue does not hold the timeline.
    DWORD timelineRelease;
    // Whether the dialogue can be skipped.
    BYTE skippable;
    // The portrait that speaks (opcodes 15 and 17): opcode 16 draws its
    // text in its colours.
    BYTE speaker;
    // Set when the speaker changes or a wait ends: opcode 16 then clears
    // the second line and starts again from the first.
    BYTE newPage;
    // The line opcode 16 writes next.
    BYTE nextLine;
    // The portrait shown in front (interrupt 3), 0xff for none.
    BYTE activePortrait;
    // Set by opcode 18; DrawMsg tests it.
    BYTE textBoxShown;
    // The Extra/clear level StartMsgScript picks for the ending dialogue
    // (index 10 of stage 6), kept in bits 11-12 of
    // GameManager::stateFlags. Opcode 21 lets the player pick it (up and
    // down) and opcode 22 keeps it and starts script 1 or 2.
    BYTE clearLevel;
    unsigned char pad_156f[0x1];
};
ASSERT_SIZE(GuiMsg, 0x1570);

// A value shown for a while at a position, as th07's GuiPopup.
struct GuiPopup
{
    D3DXVECTOR3 pos;
    int value;
    // Whether it shows, or what it shows.
    int shown;
    Timer timer;
};
ASSERT_SIZE(GuiPopup, 0x20);

// The Gui's state behind Gui::impl, as th07's GuiImpl (0x230b8 bytes,
// "FRScreenImplInf"). Only the fields known so far.
struct GuiImpl
{
    // Starts dialogue script `index`, as th07's: clears msg (keeping the
    // file), picks the ending dialogue by the clears for index 10 of
    // stage 6 and a shorter one past index 5 of stage 6B while
    // stats->clock is 12 or more, sets the dialogue's defaults and
    // clears the bullets, the enemies and the items (0x43396d).
    void StartMsgScript(int index);
    // Runs the dialogue script, as th07's (0x433db3).
    int RunMsg();
    // Draws the dialogue, as th07's (0x43542b).
    int DrawMsg();

    Sprite sprites[0x10];
    // The boss's bar (sprites[12]): 0 hidden, 1 appearing, 2 shown, 3
    // leaving, as th07's.
    BYTE bossBarState;
    unsigned char pad_2a41[0x3];
    Sprite trackNumberSprites[4];
    Sprite stageTitleTime;
    Sprite loadingIcon;
    Sprite loadingCaption;
    Sprite loadingBar;
    Sprite loadingBarTrail[8];
    Sprite livesBombsOverlay;
    Sprite backgroundTiles[0xa8];
    Sprite timePassedDigit;
    Sprite clock;
    int backgroundTileCount;
    GuiMsg msg;
    // Gui::OnDraw draws the stage clear screen (DrawStageClear) while it
    // is set, as th07's.
    int stageClearShown;
    // The stage clear screen's total, in tens, as th07's.
    int stageClearTotal;
    // The bonus, full power and spell card bonus popups, as th07's.
    GuiPopup bonusPopup;
    GuiPopup fullPowerPopup;
    GuiPopup spellBonusPopup;
    // The stage clear screen's clear bonus (in tens), point items,
    // grazes and time bonus count, as th07's.
    int stageClearBonus;
    int stageClearPower;
    int stageClearPoints;
    int stageClearGraze;
    int stageClearTime;
    int stageClearTimePassed;
    // Clock times in minutes (h * 60 + m): shown from stage 1 to 6 as two
    // times, and from stage 6B as the last time for the night bonus.
    int stageClearClock;
    int stageClearLastTime;
    int stageClearClock2;
    int stageClearClockTimer;
    Sprite difficultyLabel;
};
ASSERT_SIZE(GuiImpl, 0x230b8);

// The game's HUD and dialogue, at 0x160f428 (0x9c bytes), as th07's.
// Only the fields known so far.
class Gui
{
public:
    // The update chain callback, as th07's: unless g_GameManager.isFrozen
    // is set, updates the interface and the dialogue, keeps
    // g_App.redrawFrames at 8 or more while button 0x100 is held and
    // counts frameCounter up. Returns 1.
    static int OnUpdate(Gui *gui);
    // The draw chain callback: DrawStageClear while impl->stageClearShown
    // is set, then the dialogue, the banners, the HUD and the popups.
    // Returns 1.
    static int OnDraw(Gui *gui);
    // Whether the dialogue holds the timeline: impl is set,
    // msg.timelineRelease is 0 and a script runs, as th07's.
    BOOL MsgHoldsTimeline();
    // Whether impl->loadingIcon shows a sprite whose script waits for an
    // interrupt, as th07's IsLoadingShown (0x437d87).
    BOOL IsLoadingShown();
    // Show `bonus` in bonusPopup at (416, 48), `points` in fullPowerPopup
    // at (416, 168) shown as `kind`, and `value` in spellBonusPopup at
    // (224, 16); each sets g_App.redrawFrames to 2. As th07's (0x437ddd,
    // 0x437e5d, 0x437edc).
    void ShowBonusPopup(int bonus);
    void ShowFullPowerPopup(int points, int kind);
    void ShowSpellCardBonus(int value);
    // Whether a dialogue runs, and whether it can be skipped, as th07's
    // (0x4358bb, 0x437dc7).
    int IsMsgRunning();
    int IsMsgSkippable();
    // Updates the HUD's sprites and values, as th07's (0x435900).
    void UpdateInterface();
    // Draws the HUD and the banners, as th07's (0x43625d, 0x43741d).
    void DrawHud();
    void DrawBanners();
    // Draws the stage clear screen, as th07's OnDraw did (0x43826b).
    void DrawStageClear();
    // Draws the bonus, full power and spell card bonus popups, as th07's
    // OnDraw did (0x438a89).
    void DrawPopups();
    // Called by GameManager::OnUpdate when the result screen's mode ends
    // without a scene to go to (0x438f58).
    void StartLoadingBar();
    // Restart the clock sprite (impl->clock) of times.anm on the
    // hour (GameManager::GetClock), show the hour with interrupt 1 or 2,
    // or hide it (0x439007, 0x439050, 0x439093, 0x4390d6). Return 0.
    int StartClock();
    int AdvanceClock();
    int AdvanceClockToEnd();
    int HideClock();
    // Loads the interface's sprites and the stage's portraits and
    // messages, as th07's (0x4390ee). Returns 0, or -1 when a file fails
    // to load.
    int LoadResources();
    // Hides loadingIcon, loadingCaption and loadingBar (spriteIndex -1)
    // and clears impl->backgroundTileCount, as th07's (0x4396b8).
    void HideInterface();
    // Loads the dialogue file `path` and makes its script offsets
    // pointers, as th07's (0x439710). Returns 0, or -1 if it fails to
    // load.
    int LoadMsg(char *path);
    // Frees the dialogue file, as th07's (0x4397d5).
    void FreeMsgFile();
    // Starts dialogue script `index`, as th07's (0x439810).
    void StartMsg(int index);
    // Sets bossHealth, as th07's. Inline; the copy at 0x4230c0 is an /Od
    // object's.
    void SetBossHealth(float health)
    {
        this->bossHealth = health;
    }
    // Sets bossTimer, as th07's. Inline; the copy at 0x42f340 is an /Od
    // object's.
    void SetBossTimer(int seconds)
    {
        this->bossTimer = seconds;
    }
    // Sets section `idx` of the boss bar, as th07's. Inline; the copy at
    // 0x4230e0 is an /Od object's.
    void SetBossSection(int idx, float end, float start)
    {
        this->sectionEnds[idx] = end;
        this->sectionStarts[idx] = start;
    }
    // Sets the color of boss bar section `idx`, as th07's. Inline; the
    // copy at 0x423110 is an /Od object's.
    void SetBossSectionColor(int idx, int color)
    {
        this->sectionColors[idx] = color;
    }
    // Sets bossLives, as th07's. Inline; the copy at 0x423130 is an /Od
    // object's.
    void SetBossLives(int lives)
    {
        this->bossLives = lives;
    }
    // Sets bossPresent and returns `present`. New in th08. Inline; the
    // copy at 0x422c20 is an /Od object's.
    BYTE SetBossPresent(BYTE present)
    {
        this->bossPresent = present;
        return present;
    }
    // Whether a boss fight runs, as th07's. Inline; the copy at
    // 0x42f320 is an /Od object's.
    BYTE IsBossPresent()
    {
        return this->bossPresent;
    }
    // The update element's added callback: runs LoadResources.
    static int AddedCallback(Gui *gui);
    // Releases ANM slot 0xd unless GuiIsRestarting, frees the dialogue file
    // and, if GuiIsFreeingResources, releases ANM slots 0xa to 0xe and
    // frees impl. Returns 0.
    static int DeletedCallback(Gui *gui);
    // Adds the Gui's chain elements, as th07's (0x437ad0): if
    // GuiIsLoadingResources, zeroes the Gui and allocates impl
    // ("FRScreenImplInf"); then adds the update element (priority 15)
    // and the draw element (priority 17). Returns 0, or -1 if adding
    // fails.
    static int RegisterChain();
    // Removes the Gui's chain elements, as th07's (0x437d64).
    static void RemoveChain();

    // The frames the Gui has run, as th07's.
    int frameCounter;
    // Set to 2 when the lives or bombs change, as th07's.
    unsigned int livesChanged : 2;
    unsigned int bombsChanged : 2;
    unsigned int powerChanged : 2;
    unsigned int grazeChanged : 2;
    unsigned int pointsChanged : 2;
    unsigned int timeOrbsChanged : 2;
    GuiImpl *impl;
    // front.anm (ANM slot 10), as th07's ANM slot.
    AnmLoaded *anm;
    // The stage's text ANM (slot 0xd), as th07's stdNtxt.anm.
    AnmLoaded *textAnm;
    // times.anm (ANM slot 0xe), with the clock, and the character's
    // loading ANM (slot 0xc).
    AnmLoaded *timesAnm;
    AnmLoaded *characterAnm;
    // The boss bar's alpha, as th07's.
    DWORD bossBarAlpha;
    // How many boss lives DrawBanners marks beside the bar.
    int bossLives;
    // The seconds left on the boss's timer.
    int bossTimer;
    // bossTimer as DrawBanners last drew it: it plays sound 0x1d when
    // the timer changes under 10 (0x26 under 3, new in th08).
    int lastDrawnBossTimer;
    // Set while a boss fight runs.
    BYTE bossPresent;
    unsigned char pad_2d[0x3];
    // The boss's life as a fraction of its maximum, and what the bar
    // shows, following it.
    float bossHealth;
    float bossHealthShown;
    unsigned char pad_38[0x4];
    // Eight sections DrawBanners draws over the boss bar: where each
    // ends and starts, as fractions of the bar (unused while the end
    // is 0), and its color.
    float sectionStarts[8];
    float sectionEnds[8];
    int sectionColors[8];
};
ASSERT_SIZE(Gui, 0x9c);
extern Gui g_Gui;

// The update and draw chain elements of g_Gui.
extern ChainElem g_GuiCalcElem;
extern ChainElem g_GuiDrawElem;

// Whether the game restarts (App::isRestarting), so the stage keeps its
// loaded ANMs. Inline; the copy at 0x40b900 is an /Od object's.
inline int IsRestarting()
{
    return g_App.isRestarting;
}

// App::loadsResources and freesResources: whether the scene's objects load their
// files as they are added, and free them as they are deleted. Inline;
// the copies at 0x4338b0 and 0x4338c0 are an /Od object's.
inline int IsLoadingResources()
{
    return g_App.loadsResources;
}
inline int IsFreeingResources()
{
    return g_App.freesResources;
}

// Whether the scene's objects keep the files they loaded
// (!IsLoadingResources()). Inline; the copy at 0x44e0e0 is the player
// object's.
inline int KeepsResources()
{
    return !IsLoadingResources();
}

// App::loadsResources, freesResources and isRestarting (0x438fe9, 0x438ff3,
// 0x438ffd).
int GuiIsLoadingResources();
int GuiIsFreeingResources();
int GuiIsRestarting();

// Decodes a dialogue string (each byte xor 0x77) from `src` into `dst`,
// up to and including the terminating 0 (0x4353ec).
void DecodeMsgString(char *dst, char *src);

// Has the back buffer's play area copied to texture 10 at sprite
// `sprite` of ANM slot 10 (0x437f5c).
void CopySpriteToTexture(int sprite);
// The character's loading ANM, the stages' text ANMs (the last for
// Extra's last spell cards) and the dialogue files by stage and
// character.
extern char *g_LoadingAnmPaths[];
extern char *g_StageTextAnmPaths[];
extern char *g_MsgPaths[][12];
// "AM" and "PM", picked by whether the hour is before 12 (backwards).
extern char *g_AmPmNames[2];
// The colors of the boss timer: 20 seconds and more, 10, 5 and under 5,
// as th07's.
extern D3DCOLOR g_BossTimerColors[4];
// The stage clear bonus, by stage.
extern int g_StageClearBonuses[];
// The dialogue's text colours, by character.
extern D3DCOLOR g_MsgColors[][4];

// The thread LoadBgm starts, with the main window: streams the wave music
// each time notifyEvent is signalled, until WM_QUIT or the music is gone.
DWORD WINAPI BgmNotificationProc(LPVOID lpParameter);
