// Screen effects: fades, shakes and flashes. Only what is known so far.

#pragma once

#include "common.h"

// One effect (0x34 bytes, 0x30 in th07).
class ScreenEffect
{
public:
    // Creates an effect of `type` that lasts `duration` frames, as
    // th07's, with one more parameter, and adds its chain elements.
    // Returns it, or NULL if adding fails (0x45b8b0).
    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c, int d);
    // Sets unk_24 and restarts the timer at 0 (0x45c160).
    void Finish();
};

// The loading screen's effect, started by App::StartLoadingScreenEffect.
extern ScreenEffect *g_LoadingScreenEffect;
