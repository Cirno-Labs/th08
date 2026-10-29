// The result screen and the score file. Only what is known so far.

#pragma once

#include <string.h>
#include <windows.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"

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
    BYTE continuesUsed;
};

// A node of the score file's lists. Each list starts with a head node
// whose `data` is NULL; the entries after it are sorted.
struct ListNode
{
    // Inline; the ResultScreen constructor's vector constructor calls the
    // copy at 0x45a4dc.
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
// Inserts the score file's version 4 high score chunks for `shot` and
// `difficulty` into `list`, or into the file's own list if `list` is
// NULL, as th07's. Stores the best entry's continues in `bestContinues`
// if it is not NULL. Returns the best score, at least 100000.
DWORD LoadHighScores(void *scoreDat, ListNode *list, int shot, int difficulty, BYTE *bestContinues);

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

// The chunk with the last name entered ("LSNM"), as th07's.
struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    char name[0xc];
};
ASSERT_SIZE(Lsnm, 0x18);

// "CATK": a spell card's record (version 3), by card number (222).
struct Catk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    // The card's number.
    WORD index;
    unsigned char unk_0e[0x21e];
};
ASSERT_SIZE(Catk, 0x22c);

// "CLRD": a shot type's clears (version 4), 13 of them.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    // By difficulty, as th07's (WORDs now, 5 difficulties): the stages
    // cleared, without continues and with.
    WORD stagesClearedNoContinue[5];
    WORD stagesCleared[5];
    BYTE unk_20;
    // The shot type.
    BYTE shot;
    BYTE unk_22[0x2];
};
ASSERT_SIZE(Clrd, 0x24);

// "PSCR": practice scores (version 2), 12 of them.
struct Pscr
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_09[0x3];
    unsigned char unk_0c[0x168];
    // The shot type.
    BYTE shot;
    BYTE unk_175;
    BYTE unk_176[0x2];
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
// Fills `clrds` (by shot type, 13) with fresh version 4 chunks, then
// copies the score file's "CLRD" chunks over them, stopping at one out of
// range. Returns 0, or -1 if `clrds` is NULL.
int LoadClears(void *scoreDat, Clrd *clrds);
// Copies the score file's version 3 "CATK" chunks to `catks`, by card
// number, stopping at one numbered 0xde or more. Returns 0, or -1 if
// `catks` is NULL.
int LoadSpellCards(void *scoreDat, Catk *catks);
// Copies the score file's version 1 "FLSP" chunk to `flsp`. Returns 1, or
// 0 if there is none. New in th08.
int LoadFlspChunk(void *scoreDat, Flsp *flsp);
// Copies the score file's version 1 "LSNM" chunk to `lsnm`. Returns 1,
// or 0 if there is none.
int LoadLsnmChunk(void *scoreDat, Lsnm *lsnm);
// Fills `pscrs` (by shot type, 12) with fresh version 2 chunks, then
// copies the score file's "PSCR" chunks over them, stopping at one out of
// range. Returns 0, or -1 if `pscrs` is NULL.
int LoadPracticeScores(void *scoreDat, Pscr *pscrs);
// Copies the score file's last version 2 "PLST" chunk to `plst`.
// Returns 0.
int LoadPlstChunk(void *scoreDat, Plst *plst);
// Frees `dat` and its list.
void ReleaseScoreDat(ScoreDat *dat);

// The result screen's scene object (0x477b0 bytes; th07 0xce6c). Only
// the fields known so far.
class ResultScreen
{
public:
    // Constructs the Sprites and lists, then zeroes the whole screen.
    // Inline: RegisterChain calls the copy at 0x45842b.
    ResultScreen()
    {
        memset(this, 0, sizeof(ResultScreen));
    }
    // Frees the score file. Inline; DeletedCallback's delete calls the
    // copy at 0x45a0dc.
    ~ResultScreen()
    {
        g_MemoryManager.Free(this->scoreDat);
    }

    // Allocates the screen and adds it to the chains (update priority
    // 0x10, draw 0x12). With `mode` 1 it starts in state 9 after a game,
    // in a practice 0x11, or 0x16 in a spell card practice; with 2 (on
    // exit) in state 0x12, running only AddedCallback. Returns 0, or -1
    // if adding fails.
    static int RegisterChain(int mode);
    static int OnUpdate(ResultScreen *screen);
    static int OnDraw(ResultScreen *screen);
    static int AddedCallback(ResultScreen *screen);
    // Saves and frees the score file, empties the lists, releases ANMs
    // 0x15 and 0x16, moves surface 0 to slot 8, removes the draw element
    // and deletes the screen. Returns 0.
    static int DeletedCallback(ResultScreen *screen);

    // Frees the entries of the high score list for `difficulty` and
    // `shot` (0x454c87).
    void ReleaseScores(int difficulty, int shot);

    // The score file, as loaded.
    ScoreDat *scoreDat;
    // Counts the frames in the current state.
    int frame;
    int state;
    unsigned char unk_0c[0x10];
    // The menu item selected.
    int cursor;
    unsigned char unk_20[0x10];
    // A second cursor, moved by LoadScreenPlstChunk, as th07's.
    int spellCursor;
    unsigned char unk_34[0x16c];
    Sprite sprites[0x48];
    // Text drawn with RenderText, as th07's.
    Sprite textSprites[0x1e];
    Sprite unk_10ef8;
    Sprite unk_1119c;
    unsigned char unk_11440[0xc];
    // The high score lists.
    ListNode scores[0x3c];
    unsigned char unk_1171c[0x34d28];
    // The "TH8K" chunk SaveScoreDat writes.
    Th8k th8k;
    Lsnm lsnm;
    ChainElem *calcChain;
    ChainElem *drawChain;
    unsigned char unk_46470[0x1340];
};
ASSERT_SIZE(ResultScreen, 0x477b0);

// Writes "score.dat" from `screen`'s file header, the "TH8K" chunk, the
// lists and g_GameManager's chunks, as th07's (0x453d0d).
void SaveScoreDat(ResultScreen *screen);

// The stages' names ("Stage 1", ..., "Stage 4-uncanny", "Stage
// 4-powerful", ..., "Extra Stage"), by stage. New in th08.
extern char *g_StageNames[9];
// The shot types' names ("霊夢＆紫", ..., "西行寺　幽々子"), then
// "全主人公合計", as th07's.
extern char *g_ShotNames[13];

// Returns g_StageNames[stage], or "Clear" for 9 and over. New in th08.
char *GetStageName(int stage);
// Returns g_ShotNames[shot]. New in th08.
char *GetShotName(int shot);

// Moves `screen`'s cursor up or down through `count` items, wrapping.
// Returns -1 for up, 1 for down, 0 if it did not move.
int MoveCursorVertical(ResultScreen *screen, int count);
// The same for `screen`'s spellCursor.
int LoadScreenPlstChunk(ResultScreen *screen, int count);
// The same with left and right.
int MoveCursorHorizontal(ResultScreen *screen, int count);
