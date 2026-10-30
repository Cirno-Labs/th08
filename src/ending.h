// The ending and staff roll: a text script of lines shown one at a time,
// with commands after '@', as th07's.

#pragma once

#include <string.h>
#include <windows.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

class Ending
{
public:
    // Constructs the Sprites, then zeroes the whole ending and starts
    // its timers, as th07's. Inline; RegisterChain calls the copy at
    // 0x4297b0.
    Ending()
    {
        memset(this, 0, sizeof(Ending));
        this->lineDelay = 8;
        this->lineTimer = 0;
        this->timer = 0;
        this->scrollX = 0.0f;
        this->scrollY = 0.0f;
        this->scrollSpeed = 0.0f;
    }

    // Reads a number at the cursor and moves the cursor past it and the
    // NULs after it (0x428890).
    int ReadInt();
    // Steps the fade and draws it over the screen (0x428910).
    void DrawFade();
    // Runs the script until it waits: shows text lines, runs the '@'
    // commands. Returns 0, or -1 at its end ("@z") or on failure
    // (0x428b80).
    int RunScript();
    // Loads the script at `path` and starts it. Returns 0, or -1 after
    // logging an error (0x4295e0).
    int Load(char *path);
    // Creates the ending's chain elements: OnUpdate at priority 5 with
    // its added and deleted callbacks, OnDraw at 4. Returns 0, or -1 if
    // adding fails (0x4296a0).
    static int RegisterChain();
    // The update chain element's callback: runs the script and steps
    // the sprites, up to 9 times a frame while skipping (th07 5). Returns
    // 0 at the script's end, otherwise 1 (0x429860).
    static int OnUpdate(Ending *ending);
    // The draw chain element's callback: draws the background, the
    // sprites and the fade. Returns 1 (0x4298f0).
    static int OnDraw(Ending *ending);
    // The update chain element's added callback: loads staff01.anm (slot
    // 0x18), records the clear and loads the shot type's ending.
    // Returns 0, or -1 on failure (0x429980).
    static int AddedCallback(Ending *ending);
    // The update chain element's deleted callback: frees what the ending
    // loaded, removes the draw chain element and frees the ending
    // (0x429d40).
    static int DeletedCallback(Ending *ending);

    ChainElem *calcChain;
    ChainElem *drawChain;
    // Where the background image (surface 0) is copied from: "@v" sets
    // the y, which "@V" scrolls back to 0 by its speed per frame.
    float scrollX;
    float scrollY;
    float scrollSpeed;
    // The text lines and the "@a" pictures.
    Sprite sprites[0x10];
    // The script file.
    char *file;
    // Set once the ending has been seen: the skip button (0x100) then
    // runs it up to 9 times as fast.
    int canSkip;
    // What canSkip becomes when "@F" loads the next file. New in th08.
    int nextCanSkip;
    // staff01.anm, which "@a" runs scripts from. New in th08.
    AnmLoaded *anm;
    Timer timer;
    // The wait after a line ("@w" or the line delay), and "@r"'s wait,
    // which clears the text when it ends. A press of the shot button ends
    // them, once their minimum frames have passed.
    Timer lineTimer;
    Timer waitTimer;
    int waitMinFrames;
    int lineMinFrames;
    // The frames to wait after a line ("@s"), and while the shot button is
    // held.
    int lineDelay;
    int lineDelayFast;
    unsigned char pad_2a98[0x4];
    // The line the text goes to.
    int line;
    // "@c": the text color.
    D3DCOLOR textColor;
    // "@0" to "@3": fades in and out of black and white.
    D3DCOLOR fadeColor;
    int fadeTimer;
    int fadeDuration;
    int fadeType;
    // The script position.
    char *cursor;
};
ASSERT_SIZE(Ending, 0x2ab8);

// The endings' scripts by character: the bad endings (without bit 4 of
// g_GameManager.stateFlags), and the good endings after stage 6 and after
// stage 7. th07's were by shot type.
extern char *g_BadEndingFiles[];
extern char *g_EndingFiles[];
extern char *g_FinalBEndingFiles[];
