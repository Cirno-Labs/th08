// The music screen: lists the game's tunes with their comments and plays
// the one picked.

#pragma once

#include <string.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"

// A tune in the music screen's list.
struct MusicTune
{
    // Inline; AddedCallback's new[] calls the copy at 0x449bee.
    MusicTune()
    {
        memset(this, 0, sizeof(MusicTune));
    }

    // The MIDI file; App::PlayMusic finds the wave files from it.
    char path[0x40];
    char title[0x42];
    // The comment shown for the tune: 7 lines in th08 (th07 8), and the
    // first line again as the "Now Playing" line.
    char comment[8][0x42];
};
ASSERT_SIZE(MusicTune, 0x292);

// The music screen, the static at 0x17cf4c8. Runs on two chain
// elements: one updates it each frame, one draws it.
class MusicScreen
{
public:
    // Constructs the Sprites, then zeroes the whole screen. Inline:
    // RegisterChain calls the copy at 0x449331.
    MusicScreen()
    {
        memset(this, 0, sizeof(MusicScreen));
    }

    // Sets up the music screen, a static constructed on first use, and
    // adds it to the chains. Returns 0, or -1 if adding fails.
    static int RegisterChain();
    // The update chain element's added callback: loads the screen's
    // image, music00.anm and sprt/musiccmt.txt, the list of tunes, and
    // sets up the titles: "？？？？？？？？" for a tune not heard in a
    // game yet. Returns 0, or -1 on failure.
    static int AddedCallback(MusicScreen *screen);
    // The update chain element's deleted callback: frees what
    // AddedCallback loaded and removes the draw chain element.
    static int DeletedCallback(MusicScreen *screen);
    // The update chain element's callback: runs the current state's
    // handler and counts its frames, then steps the sprites. Returns 0
    // while a handler asks to stop, otherwise 1.
    static int OnUpdate(MusicScreen *screen);
    // The draw chain element's callback: draws the list, with an arrow
    // at the tune under the cursor, the comment and the tune playing
    // under "Now Playing". Returns 1.
    static int OnDraw(MusicScreen *screen);
    // State 0: on its first frame, sends interrupt 1 to the title under
    // the cursor and the comment lines and 2 to the other titles; after
    // 8 frames moves to state 1. Returns 0.
    int UpdateIntro();
    // State 1: the list of tunes. Up and down move the cursor, sending
    // interrupt 1 to the title under it and 2 to the others and the
    // comment, which then shows again a line every 2 frames; a tune not
    // heard yet shows g_MusicNotHeardComment unless it is playing. Shot
    // or enter plays the tune; 0x100 fades the music out, 0x4000
    // restarts the tune playing. Returns nonzero to stop.
    int UpdateList();

    ChainElem *calcChain;
    ChainElem *drawChain;
    // music00.anm, loaded by AddedCallback.
    AnmLoaded *anm;
    // Each tune's g_GameManager.plst.bgmHeard, as AddedCallback found
    // it.
    int bgmHeard[0x18];
    // Frames spent in `state`.
    int stateFrames;
    int state;
    // The tune under the cursor.
    int cursor;
    // The tune playing.
    int playing;
    // The first tune the list shows; it shows 10.
    int listTop;
    int tuneCount;
    MusicTune *tunes;
    // An array of one: it is constructed in a loop.
    Sprite bgSprite[1];
    // One per tune in the list.
    Sprite titles[0x1f];
    // One per line of the selected tune's comment; the last shows the
    // tune playing.
    Sprite comment[8];
};
ASSERT_SIZE(MusicScreen, 0x6a28);

// The comment shown for a tune not heard in a game yet (0x4c7aac).
extern char *g_MusicNotHeardComment[8];
