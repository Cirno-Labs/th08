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

// The score file's other chunks, by their magic. Only their sizes are
// known so far.
// "FLSP", new in th08 (version 1).
struct Flsp
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char unk_9[0x17];
};
ASSERT_SIZE(Flsp, 0x20);

// "CATK": a spell card's record (version 3), by card number (222).
struct Catk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char unk_9[0x223];
};
ASSERT_SIZE(Catk, 0x22c);

// "CLRD": a shot type's clears (version 4), 13 of them.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char unk_9[0x1b];
};
ASSERT_SIZE(Clrd, 0x24);

// "PSCR": practice scores (version 2), 12 of them.
struct Pscr
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char unk_9[0x16f];
};
ASSERT_SIZE(Pscr, 0x178);

// The score file as loaded, with its chunks unpacked. Its layout is not
// known yet.
struct ScoreDat;

// Reads "score.dat" at `path`, as th07's, or makes a new one
// (0x45a5e0).
ScoreDat *LoadScoreDat(char *path);
// Copy the score file's chunks of each kind over the given ones, as
// th07's: LoadClears and LoadPracticeScores first fill theirs with fresh
// chunks. Return 0, or -1 if the destination is NULL; LoadFlspChunk
// returns 1 if it found its chunk, else 0.
int LoadClears(ScoreDat *scoreDat, Clrd *clrds);
int LoadSpellCards(ScoreDat *scoreDat, Catk *catks);
int LoadFlspChunk(ScoreDat *scoreDat, Flsp *flsp);
int LoadPracticeScores(ScoreDat *scoreDat, Pscr *pscrs);
// Copies the score file's last "PLST" chunk to `plst`. Returns 0.
int LoadPlstChunk(ScoreDat *scoreDat, Plst *plst);
// Frees `dat` (0x45afc0).
void ReleaseScoreDat(ScoreDat *dat);

class ResultScreen
{
public:
    // Adds the result screen's chain elements, as th07's (0x4582a0).
    // Returns 0, or nonzero on failure.
    static int RegisterChain(int unk);
};
