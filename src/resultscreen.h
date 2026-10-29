// The result screen and the score file. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// A chunk of the score file ("PLST") with the play statistics, th07's
// grown to 0x228 bytes (0x160 there).
struct Plst
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    // How long the game has run, App::AddPlayTime keeps it up to date.
    DWORD runHours;
    DWORD runMinutes;
    DWORD runSeconds;
    DWORD runMilliseconds;
    // How long it has been played, App::AddTotalPlayTime keeps it up to date.
    DWORD playHours;
    DWORD playMinutes;
    DWORD playSeconds;
    DWORD playMilliseconds;
    unsigned char unk_2c[0x1dc];
    // Set for each piece of music played (App::PlayMidi's `bgm`), for
    // the music room. New in th08.
    BYTE bgmHeard[0x20];
};
ASSERT_SIZE(Plst, 0x228);
