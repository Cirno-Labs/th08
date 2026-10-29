// Full-screen effects such as fades, each with its own chain elements.

#pragma once

#include <windows.h>
#include <d3d8.h>

#include "chain.h"
#include "common.h"
#include "timer.h"

// One effect (0x34 bytes, 0x30 in th07): `type` picks its update and draw
// callbacks.
class ScreenEffect
{
public:
    // Creates an effect of `type` (0 fade in, 1 shake, 2 fade out, 3
    // flashes, 4 fade out over the whole screen, as th07's; new in th08:
    // 5 and 6 the loading screen's fade over the whole screen or the play
    // area, 7 a shake that rises, holds and falls) that lasts `duration`
    // frames, with `a`, `b` and `c` as its color, shake range, flashes
    // or phases, and adds its chain elements (update priority 3, draw
    // `drawPriority`). Returns it, or NULL if adding fails (0x45b8b0).
    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c, int drawPriority);
    // The update element's added callback: restarts `timer`. Returns 0.
    static int AddedCallback(ScreenEffect *effect);
    // The update element's deleted callback: removes the draw element
    // and deletes the effect. Returns 0.
    static int DeletedCallback(ScreenEffect *effect);
    // Type 0's update callback: fades `alpha` from 255 to 0 over
    // `duration` frames. Returns 0 when done, else 1.
    static int OnUpdateFadeIn(ScreenEffect *effect);
    // Type 1's update callback: shakes the screen, moving SpriteEngine's
    // shake offset at random by from shakeFrom to shakeTo pixels over
    // `duration` frames. Holds still while g_GameManager.isFrozen or its
    // flag bit 10 is set. Returns 0 when done, else 1.
    static int OnUpdateShake(ScreenEffect *effect);
    // Type 7's update callback: shakes the screen by up to `duration`
    // pixels, rising over riseFrames, holding for holdFrames and falling
    // over fallFrames. New in th08. Returns 0 when done, else 1.
    static int OnUpdateShakeRamp(ScreenEffect *effect);
    // Type 2's update callback: fades `alpha` from 0 to 255 over
    // `duration` frames, holding still while the game is paused or in
    // the continue menu. Returns 0 when done, else 1.
    static int OnUpdateFadeOut(ScreenEffect *effect);
    // Type 5 and 6's update callback: fades `alpha` from 0 to 128 over
    // `duration` frames, then back to 0 over 8 frames once Finish is
    // called. New in th08. Returns 0 when done, else 1.
    static int OnUpdateLoadingFade(ScreenEffect *effect);
    // Type 0's draw callback: covers the whole screen in `color` at
    // `alpha`, after resetting the viewport. Returns 1.
    static int OnDrawScreen(ScreenEffect *effect);
    // Type 5's draw callback: OnDrawScreen without the viewport. New in
    // th08. Returns 1.
    static int OnDrawWholeScreen(ScreenEffect *effect);
    // Type 2's draw callback: covers the play area in `color` at `alpha`.
    // Returns 1.
    static int OnDrawPlayArea(ScreenEffect *effect);
    // Type 3's update callback: fades `alpha` from flashColor's alpha to
    // 0 over `duration` frames, `flashes` times over. Returns 0 when
    // done, else 1.
    static int OnUpdateFlash(ScreenEffect *effect);
    // Type 3's draw callback: covers the play area in flashColor at
    // `alpha`. Returns 1.
    static int OnDrawFlash(ScreenEffect *effect);
    // Has the loading screen's fade end: sets finishing and restarts the
    // timer at 0 (0x45c160).
    void Finish();

    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    DWORD unusedDword0c;
    int alpha;
    // How many frames the effect lasts.
    int duration;
    union
    {
        // Fades: the color faded.
        D3DCOLOR color;
        // Shakes: how far the play area shakes at the start.
        int shakeFrom;
        // Type 3: how many more times the color flashes.
        int flashes;
        // Type 7: the frames the shake rises.
        int riseFrames;
    };
    union
    {
        // Shakes: how far the play area shakes at the end.
        int shakeTo;
        // Type 3: the color flashed, starting at its alpha.
        D3DCOLOR flashColor;
        // Type 7: the frames it holds.
        DWORD holdFrames;
    };
    union
    {
        int flashCount;
        // Type 7: the frames it falls.
        DWORD fallFrames;
    };
    // Set by Finish. New in th08.
    int finishing;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x34);

// The loading screen's effect, started by App::StartLoadingScreenEffect.
extern ScreenEffect *g_LoadingScreenEffect;
