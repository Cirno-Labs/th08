// The title menu ("bgm/th08_01.mid"). Only the fields known so far.

#pragma once

#include <string.h>
#include <windows.h>

#include "chain.h"
#include "common.h"
#include "replay.h"
#include "sprite.h"
#include "unsorted.h"

// The 0x3c bytes of App's config, from controllerMapping, as one block
// to copy (0x38 in th07).
struct ConfigCopy
{
    unsigned char pad_0[0x1e];
    BYTE colorMode16Bits;
    unsigned char pad_1f[0x3];
    BYTE windowed;
    unsigned char pad_23[0x19];
};
ASSERT_SIZE(ConfigCopy, 0x3c);

class MainMenu
{
public:
    // Constructs the sprites, then zeroes the menu. Inline, but too large
    // to expand: SetUpTitleScene calls the copy at 0x471586.
    MainMenu()
    {
        memset(this, 0, sizeof(MainMenu));
    }

    // The update element's callback: does nothing while the title is
    // loading (returns 4 if the loading failed, else 1), otherwise runs
    // the state's function, then steps the sprites.
    static int OnUpdate(MainMenu *menu);
    // The draw element's callback.
    static int OnDraw(MainMenu *menu);
    // The update element's added callback: runs InitTitleScene.
    static int AddedCallback(MainMenu *menu);
    // The update element's deleted callback: releases the title's ANM
    // slots 0x14 and 0x16 and surface 0, removes the draw element, frees
    // the menu's buffers and the menu. Returns 0.
    static int DeletedCallback(MainMenu *menu);
    // The body of the added callback: resets the game's numbers, shows
    // the info screens of newly opened stages, reloads the score file,
    // puts the cursor on the entry the previous scene came from, starts
    // the loading sprites and LoadingThread. Returns 0.
    int InitTitleScene();
    // Loads the title's ANMs and surface in the sub thread, then the
    // music; sets loadingState to 0, or 2 on failure. New in th08.
    static void LoadingThread(MainMenu *arg);
    // Shows the picture at `path` for up to 100 seconds, fading in and
    // out; shot or bomb skips it after 3 seconds. New in th08.
    static void ShowInfoScreen(char *path);
    // Opens the last spells (FLSP) whose conditions the score file
    // meets: Extra clears, practice captures, captures and attempts of
    // given cards (0x46cbbb). New in th08.
    void UnlockLastSpells();
    // Writes the card at the cursor's details into cardDetailSprites in state
    // 17: number and name, comment and difficulty, the counts for the
    // character and all, and the two comment lines or, for a last
    // spell not yet open, how to open it. Outside subState 0, only what
    // cardDetailsTimer (counting down from 0x15 as the cursor moves) reaches
    // this frame (0x46d7f9).
    void UpdateCardDetails();

    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters. Inline: the copy is at
    // 0x4694c3.
    void SetState(int state)
    {
        this->prevState = this->state;
        this->state = state;
        this->stateFrames = 0;
        this->inputLockout = 0;
        this->subState = 0;
        this->subMenuIdleFrames = 0;
    }
    // The state functions OnUpdate runs.
    int StartTitle();
    int UpdateOptions();
    int UpdateKeyConfig();
    int UpdateDifficultyMenu();
    int UpdateCharacterMenu();
    int UpdateGameStartMenu();
    int UpdateReplayMenu();
    // States 16 and 17, new in th08.
    int UpdateState16();
    int UpdateState17();
    int DrawReplayMenu();
    // Draws over the character menu (state 5).
    int DrawCharacterMenu();
    // Draw states 16 and 17's text.
    int DrawState16();
    int DrawState17();
    int DrawPracticeScores();
    // Frees loadedReplay and the sprites. Returns 0.
    int ReleaseTitleScene();
    // Gives every button of controllerMapping assigned `button` the
    // value `replacement`. The third argument is unused.
    void ReassignButton(short button, short replacement, int unused);
    // Shows `value` as two digits on `vm` and the sprite after it, or
    // hides both when it is negative. Returns 0.
    int ShowButtonDigits(Sprite *vm, short value);
    // Moves the cursor up or down among `count` entries, wrapping, when
    // up or down is pressed or repeats. Returns -1 for up, 1 for down,
    // else 0.
    int MoveMenuCursor(int count);
    // MoveMenuCursor with left and right.
    int MoveMenuCursorLR(int count);

    // The selected entry.
    int cursor;
    int spellPage;
    // The cursor the menus last highlighted.
    int prevCursor;
    int subState;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char pad_14[0x54];
    // The state before the current one.
    int prevState;
    int returningFromPractice;
    // The replay menu's entries: their files' paths, the numbers shown
    // for them, and the replays' headers.
    char replayPaths[60][0x200];
    char replayLabels[60][8];
    ReplayHeader replays[60];
    // The replay picked in the replay menu.
    ReplayHeader *loadedReplay;
    int clearedInReplayMenu;
    // The number of replays listed, the one at the cursor and the
    // stage picked from it.
    int replayCount;
    int replayCursor;
    int replayStage;
    int subMenuIdleFrames;
    int spellCardCount;
    int cardDetailsTimer;
    float shownRates[4];
    // title01.anm (slot 0x14) and resulttext.anm (slot 0x16), loaded by
    // the loading thread.
    AnmLoaded *titleAnm;
    AnmLoaded *resultTextAnm;
    // The menu's sprites, spriteCount of them.
    Sprite *sprites;
    Sprite *helpSprite;
    Sprite helpSprites[0xe];
    Sprite textSprites[0x15];
    Sprite cardDetailSprites[0xe];
    int spriteCount;
    // The menu's state.
    int state;
    int inputLockout;
    // The frames the title menu has waited without input.
    int idleFrames;
    // 1 while the loading thread loads the title, 2 if it failed, else
    // 0. New in th08.
    int loadingState;
    // The update element (priority 4, with the added and deleted
    // callbacks) and the draw element (priority 3), created by
    // SetUpTitleScene.
    ChainElem *calcChain;
    ChainElem *drawChain;
    // The button mapping the options screen edits.
    ControllerMapping controllerMapping;
    unsigned char pad_14452[0x2];
    // A copy of g_App's config.
    ConfigCopy savedConfig;
};
ASSERT_SIZE(MainMenu, 0x14490);

// Sets up the title scene (0x47146d). Returns 0, or -1 if the update
// element cannot be added. The argument is unused.
int SetUpTitleScene(int unused);

// Writes `value` as `digits` full-width digits into g_FullWidthString
// and returns it. New in th08.
char *FormatFullWidthNumber(int value, int digits);
// Draws a fan of 63 triangles around `center`, `diameter` wide, from
// straight up, each `arc` * 2 pi / 62 wider than the last. New in th08.
void DrawFan(D3DXVECTOR3 *center, D3DCOLOR color, float arc, float diameter);
// A line of the hint the spell card list shows for a last spell not yet
// open: a format and up to five card numbers (shown plus 1).
struct LastSpellCondition
{
    char *format;
    int cards[5];
};
// Two lines for each card from 0xcc to 0xdd.
extern LastSpellCondition g_LastSpellConditions[18][2];
// The difficulty names the spell card list shows, by
// GetSpellCardDifficulty.
extern char *g_SpellPracticeDifficultyNames[6];
// The stage names the spell card practice's score page shows, with
// "Extra" and "Last Word".
extern char *g_SpellPracticeStageNames[10];
// FormatFullWidthNumber's result.
extern char g_FullWidthString[0x40];
// "０" to "９".
extern char *g_FullWidthDigits[10];

// The help line the title menu shows for each entry.
extern char *g_TitleHelpTexts[9];
// The demo replays the title menu plays in turn.
extern char *g_DemoReplayPaths[4];
// The character menu's sprites by entry: three shown with interrupt 9,
// then one with interrupt 0x17.
extern const int g_CharacterMenuSprites[12][4];
// The names the replay menu shows for shot types and difficulties.
extern char *g_MenuShotTypeNames[12];
extern char *g_MenuDifficultyNames[5];
// The spell cards of each stage, for the spell card practice menu.
extern int *g_StageSpellCards[];
// How many cards each list of g_StageSpellCards has.
extern int g_StageSpellCardCounts[];
// The stage names of the practice menu, which has no Extra, and of the
// replay menu.
extern char *g_PracticeStageNames[9];
// The help line the options screen shows for each entry.
extern char *g_OptionsHelpTexts[10];
// The help line the key config shows for each entry.
extern char *g_KeyConfigHelpTexts[12];
// The button the key config saw pressed last frame, so that a held
// button is assigned once.
extern short g_LastPressedKey;

// The title menu SetUpTitleScene created. New in th08: the loading
// thread reaches the menu through it.
extern MainMenu *g_MainMenu;

// The button mapping the key config's reset entry restores.
extern ControllerMapping g_DefaultControllerMapping;
