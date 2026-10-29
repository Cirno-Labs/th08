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
    char bgmHeard[0x20];
};
ASSERT_SIZE(Plst, 0x228);

// A high score chunk of the score file ("HSCR", version 4). Only the
// fields known so far, at th07's offsets; its size is not known yet.
struct Hscr
{
    DWORD magic;
    WORD size;
    // The size again.
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    DWORD score;
    unsigned char unk_10[0x4];
    // The shot and difficulty of its list.
    BYTE shot;
    BYTE difficulty;
    unsigned char unk_16[0x10];
    char continuesUsed;
};

// A node of the score file's lists. Each list starts with a head node
// whose `data` is NULL; the entries after it are sorted.
struct ListNode
{
    ListNode()
    {
        this->prev = NULL;
        this->next = NULL;
        this->data = NULL;
    }

    ListNode *prev;
    ListNode *next;
    struct Hscr *data;
};

// Frees every node after `head`.
void FreeListNodes(ListNode *head);
// Links `entry` in after `head`, before the first entry whose score is no
// higher. Returns its rank, from 0.
int InsertScore(ListNode *head, Hscr *entry);

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

// The score file's first chunk ("TH8K", th07's "TH7K").
struct Th8k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
};

// The chunk with the game's version ("VRSM"), as th07's: written from the
// executable's size and checksum.
struct Vrsm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    char gameVersion[0x8];
    DWORD exeSize;
    DWORD exeChecksum;
};
ASSERT_SIZE(Vrsm, 0x1c);

// "score.dat" as loaded, th07's: a 0x1c-byte header, then chunks. On disk
// the chunks are compressed, every byte from the third on is xored with a
// key that runs over the bytes before it, and the whole file is encrypted
// with DecryptData's block cipher (new in th08).
struct ScoreDat
{
    BYTE unusedByte00;
    // Where the key starts; random.
    BYTE key;
    // The sum of the bytes from the fifth on.
    WORD checksum;
    // 1 (th07 0xb).
    WORD version;
    BYTE unk_06[0x2];
    // Where the chunks start.
    int dataOffset;
    // The list of the score entries read from the chunks.
    ListNode *scores;
    // The size with the chunks uncompressed, header included.
    int fileSize;
    // The size of the chunks, uncompressed and compressed.
    int dataSize;
    int packedSize;
};
ASSERT_SIZE(ScoreDat, 0x1c);

// Reads "score.dat" at `path`, as th07's: decrypts and decodes it, checks
// the checksum, the header size and version 1, unpacks the chunks into a
// new block with room for 0xa0000 bytes, then checks every "VRSM" chunk
// against the executable, that no chunk has size 0 and that a version 1
// "TH8K" chunk is there. On any failure it makes a new, empty one. Either
// way it gives it an empty list.
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
