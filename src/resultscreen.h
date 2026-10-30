// The result screen and the score file. Only what is known so far.

#pragma once

#include <string.h>
#include <windows.h>

#include "chain.h"
#include "common.h"
#include "replay.h"
#include "sprite.h"

// One difficulty's line of the play statistics, th07's with 12 shot
// types.
struct PlstCounts
{
    // The games played: [0] with every shot type, then by shot type.
    int plays[0xd];
    int retries;
    int clears;
    int continues;
    int practices;
};
ASSERT_SIZE(PlstCounts, 0x44);

// A chunk of the score file ("PLST") with the play statistics, th07's
// grown to 0x228 bytes (0x160 there).
struct Plst
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
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
    // By difficulty, then the totals.
    PlstCounts counts[7];
    // Set for each piece of music played (App::PlayMidi's `bgm`), for
    // the music room. New in th08.
    char bgmHeard[0x20];
};
ASSERT_SIZE(Plst, 0x228);

// A high score chunk of the score file ("HSCR", version 4; th07 0x28
// bytes). Only the fields known so far, at th07's offsets.
struct Hscr
{
    DWORD magic;
    WORD size;
    // The size again.
    WORD size2;
    BYTE version;
    // Set on the entry just added, which is the one being named.
    BYTE isNew;
    BYTE pad_0a[0x2];
    DWORD score;
    // How much the game slowed down, in percent.
    float slowdown;
    // The shot and difficulty of its list.
    BYTE shot;
    BYTE difficulty;
    // The stage reached, 99 for a clear.
    BYTE stage;
    // Eight characters and a NUL.
    char name[9];
    // "%m/%d" and a NUL.
    char date[6];
    BYTE continuesUsed;
    unsigned char unusedByte27;
    union
    {
        // The config it was played with, in GameConfig's layout:
        // GameManager's copy takes App's (LoadScoreFile).
        BYTE config[0x3c];
        struct
        {
            unsigned char pad_28[0x1c];
            // The lives the game started with, less one: the config's
            // lifeCount.
            BYTE initialLives;
            unsigned char pad_45[0x1f];
        };
    };
    // The frames played: HandleNameEntry copies GameManager::unk_3db94.
    int playFrames;
    // The point items collected.
    int pointItems;
    int unusedInt6c;
    // The misses, bombs, last spells and pauses.
    int misses;
    int bombs;
    int lastSpells;
    int pauses;
    // The time orbs (刻符) collected.
    int timeOrbs;
    // How human the player was, in hundredths of a percent:
    // GameManager::unk_3dba0 / unk_3dba4 times 10000 (HandleNameEntry).
    int humanRate;
    // Set for each spell card captured, by card number.
    BYTE spellCards[0xde];
    // 1 in the entries the lists start with.
    BYTE isDefault;
    BYTE unusedByte167;
};
ASSERT_SIZE(Hscr, 0x168);

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
    unsigned char pad_9[0x3];
    // The last spells (cards 0xcd on) seen: entry i holds 0xcd + i once
    // it was.
    BYTE lastSpellsSeen[0x14];

    // Opens last spell `card` (0xcd on). Inline; MainMenu::UnlockLastSpells
    // expands it.
    void Open(int card)
    {
        this->lastSpellsSeen[card - 0xcd] = card;
    }
};
ASSERT_SIZE(Flsp, 0x20);

// The chunk with the last name entered ("LSNM"), as th07's.
struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
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
    BYTE pad_09[0x3];
    // The card's number.
    WORD index;
    // A checksum: the low byte of the name's characters plus all the
    // counters below (SpellCard::Start).
    BYTE checksum;
    // The card's difficulty, 0 to 3, or 4 for none.
    BYTE difficulty;
    // The card's name.
    char name[0x30];
    // The card's comment, set with the name as the card starts.
    char comment[0x30];
    // SpellCard::unk_74 and unk_b4 (xor 0xdd and 0xee), set as a
    // practised card is captured.
    char note[0x40];
    char note2[0x40];
    // By shot type, then with every shot type, as th07's (DWORDs now):
    // the best bonus, the attempts and the captures.
    DWORD maxBonus[0xd];
    DWORD attempts[0xd];
    DWORD captures[0xd];
    DWORD practiceMaxBonus[0xd];
    // The same in spell card practice.
    DWORD practiceAttempts[0xd];
    DWORD practiceCaptures[0xd];
    unsigned char pad_228[0x4];

    // Whether the card was met with shot type `shot`, in a game or in
    // practice. Inline; WriteScoreText calls the copy at 0x45a4a0.
    int WasAttempted(int shot)
    {
        return this->attempts[shot] > 0 || this->practiceAttempts[shot] != 0;
    }
    // Whether the card was captured with shot type `shot`, in a game or
    // in practice, and in practice. Inline; MainMenu::UnlockLastSpells
    // expands them.
    int WasCaptured(int shot)
    {
        return this->captures[shot] > 0 || this->practiceCaptures[shot] > 0;
    }
    int WasPracticeCaptured(int shot)
    {
        return this->practiceCaptures[shot] > 0;
    }
};
ASSERT_SIZE(Catk, 0x22c);

// "CLRD": a shot type's clears (version 4), 13 of them.
struct Clrd
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
    // By difficulty, as th07's (WORDs now, 5 difficulties): the stages
    // cleared, without continues and with.
    WORD stagesClearedNoContinue[5];
    WORD stagesCleared[5];
    BYTE canSkipEnding;
    // The shot type.
    BYTE shot;
    BYTE pad_22[0x2];
};
ASSERT_SIZE(Clrd, 0x24);

// "PSCR": practice scores (version 2), 12 of them.
struct Pscr
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
    // How often each stage was practised, by stage and difficulty
    // (LoadScoreFile counts them).
    DWORD playCount[9][5];
    // The best score, by stage and difficulty.
    DWORD scores[9][5];
    // The shot type.
    BYTE shot;
    // Set once the shot type has a practice score: SaveScoreDat skips
    // the chunk while it is 0.
    BYTE practised;
    BYTE pad_176[0x2];
};
ASSERT_SIZE(Pscr, 0x178);

// The score file's first chunk ("TH8K", th07's "TH7K").
struct Th8k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE pad_09[0x3];
};

// The chunk with the game's version ("VRSM"), as th07's: written from the
// executable's size and checksum.
struct Vrsm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE clearedOnSave;
    BYTE pad_0a[0x2];
    // "0100d".
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
    BYTE secondaryKey;
    BYTE unusedByte07;
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
    // Runs the current state's handler: the menu (0, 1), choosing a
    // difficulty and shot type and showing their high scores (3 to 5),
    // the same for the spell cards (6 to 8), after a game the name entry
    // (9), replay saving (10 to 14), waiting after the name entry (15,
    // 16) and the statistics (0x13 to 0x15). Steps the sprites. Returns
    // 1, or 0 once it sets the scene back to the title (states 2, 0x11,
    // 0x16) or in state 0x12.
    static int OnUpdate(ResultScreen *screen);
    // Draws the sprites, the score lists or the spell cards, the name
    // entry's alphabet, the replay slots, then DrawStatistics, as
    // th07's. Returns 1.
    static int OnDraw(ResultScreen *screen);
    // Fills the lists with "--------" entries, loads the background,
    // the ANMs and the sprites (unless in state 0x12), the score file
    // and its lists, the last name and, outside a game, the spell cards,
    // clears and practice scores, as th07's. After a practice (0x11) it
    // keeps the practice score; after a practice or a spell card
    // practice (0x16) it goes on to state 10. Counts the captured spell
    // cards and adds up the clears and continues. In state 0x12 it
    // deletes the screen and returns -1, else returns 0, or -1 if a file
    // fails to load.
    static int AddedCallback(ResultScreen *screen);
    // Saves and frees the score file, empties the lists, releases ANMs
    // 0x15 and 0x16, moves surface 0 to slot 8, removes the draw element
    // and deletes the screen. Returns 0.
    static int DeletedCallback(ResultScreen *screen);

    // Dumps the records as text to "score.txt": the high scores, spell
    // cards and play statistics (0x454298). New in th08; the menu runs it
    // on button 0x2000.
    void WriteScoreText();
    // The state handlers OnUpdate runs. New in th08, which split th07's
    // OnUpdate into them; what each one shows is a guess from where it
    // goes next and how many items its cursor has.
    int UpdateMenu();
    int UpdateScoreDifficultySelect();
    int UpdateScoreShotSelect();
    int UpdateScoreList();
    int UpdateSpellDifficultySelect();
    int UpdateSpellShotSelect();
    int UpdateSpellList();
    // Lets the player enter a name for the score just made, as th07's.
    int HandleNameEntry();
    // Offers to save the replay and lists the slots, as th07's.
    int HandleReplaySaving();
    // After a name is entered, waits for the shot button, as th07's.
    int WaitAfterNameEntry();
    // Runs states 0x13 to 0x15, the statistics, as th07's. Returns
    // nonzero to go back to the menu.
    int RunStatisticsMenu();
    // In states 0xf and 0x10, shows the game's statistics next to sprite
    // 0x47: score, continues (up to 9), difficulty, progress, continues,
    // deaths, bombs, spell cards and slowdown. It also works out a rating
    // from them that it never shows. Returns 0.
    int DrawStatistics();
    // Switches to `state`, keeping the old one in prevState, and clears
    // the frame count and subState. New in th08: th07 set `state` alone.
    void SetState(int state);
    // Inserts `entry` into the high score list for `difficulty` and
    // `shot`.
    int AddScore(Hscr *entry, int difficulty, int shot);
    // Frees the entries of the high score list for `difficulty` and
    // `shot` (0x454c87).
    void ReleaseScores(int difficulty, int shot);

    // The score file, as loaded.
    ScoreDat *scoreDat;
    // Counts the frames in the current state.
    int frame;
    int state;
    // The state SetState set last.
    int lastSetState;
    // A step within the state; SetState clears it.
    int subState;
    // The state before.
    int prevState;
    int subStateFrames;
    // The menu item selected.
    int cursor;
    // The menu cursor, kept while away, as th07's.
    int savedCursor;
    int unusedInt24;
    // The replay slot chosen, 0 to 14, as th07's.
    int replaySlot;
    // The character of g_NameAlphabet selected, as th07's.
    int charIndex;
    // A second cursor, moved by LoadScreenPlstChunk, as th07's.
    int spellCursor;
    // The spellCursor whose spell cards textSprites show, as th07's.
    int shownShotType;
    // Set while the spell card rows slide in, as th07's.
    int spellInfoSliding;
    // The shot type whose list is shown, as th07's: the score's shot
    // type in the name entry. -1 to redraw.
    int character;
    // The spell card page (ten cards each) textSprites show; -1 to
    // redraw.
    int spellPage;
    // The difficulty whose list is shown, as th07's.
    int difficulty;
    // The spell card list shown (g_SpellCardLists' index), kept while
    // away.
    int spellList;
    // How far the button sequence that marks every stage cleared and
    // every spell card met has got, as th07's: three of 0x80, two of
    // 0x40, two of 0x2000, three of 0x200 while 4 or 0x100 is held.
    int cheatProgress;
    // When set, the name entry starts on the end mark, as th07's.
    int nameEntryAtEnd;
    // Set to leave the spell card list once its sprites are out.
    int leavingSpellList;
    // The name entered last.
    char name[0xc];
    // By spell card list and shot type (and all of them): the spell cards
    // captured.
    int capturedSpellCounts[6][0xd];
    // The runSeconds RunStatisticsMenu shows, as th07's.
    BYTE shownRunSeconds;
    unsigned char pad_19d[0x3];
    Sprite sprites[0x48];
    // Text drawn with RenderText, as th07's.
    Sprite textSprites[0x1e];
    // Never drawn; AddedCallback clears its spriteIndex, as th07's.
    Sprite unusedSprite10ef8;
    Sprite spellCardRowDivider;
    // "result00.anm" and "resulttext.anm".
    AnmLoaded *resultAnm;
    AnmLoaded *resultTextAnm;
    unsigned char pad_11448[0x4];
    // The high score lists, by difficulty and shot.
    ListNode scores[5][0xc];
    // The entries the lists start with, by difficulty, shot and rank.
    Hscr defaultScores[5][0xc][10];
    // The score being entered.
    Hscr entry;
    // The "TH8K" chunk SaveScoreDat writes.
    Th8k th8k;
    Lsnm lsnm;
    ChainElem *calcChain;
    ChainElem *drawChain;
    // The headers of "./replay/th8_01.rpy" to "th8_15.rpy", where they
    // are valid.
    ReplayHeader replays[0xf];
    // The date and score of the replay being saved.
    ReplayHeader replay;
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

// The spell card lists of the result screen, by list (difficulty, the
// last one every card), and their lengths (42, 49, 50, 50, 14, 222). New
// in th08.
extern int *g_SpellCardLists[6];
extern int g_SpellCardListSizes[6];
// The Normal list of g_SpellCardLists.
extern int g_NormalSpellCards[];

// The characters a name can be made of, 16 to a row; the last two
// stand for a space and the end. As th07's.
extern char *g_NameAlphabet;
// The shot types' short names ("Rm & Yk", ...), by shot type, as
// th07's.
extern char *g_ShotTypeNames[0xc];
// The stages' numbers for the score lists, by stage. New in th08.
extern int g_StageNumbers[9];
// The difficulties' names ("Easy", ...), for score.txt. New in th08.
extern char *g_PlainDifficultyNames[5];
// The difficulties' names ("     Easy", ...), right-aligned, as th07's.
extern char *g_DifficultyNames[5];
// By difficulty: what the statistics' rating starts from, and what
// each spell card captured adds to it, as th07's.
extern float g_DifficultyRatings[5];
extern float g_SpellCardRatings[5];

// Returns g_StageNames[stage], or "Clear" for 9 and over. New in th08.
char *GetStageName(int stage);
// Returns g_ShotNames[shot]. New in th08.
char *GetShotName(int shot);

// Formats into `buf` like sprintf and returns the end of what it wrote,
// to format the next part there. New in th08.
char *FormatAndAdvance(char *buf, const char *fmt, ...);

// Writes today's date as "%m/%d" to `date`, as th07's.
void FormatTodayDate(char *date);

// Moves `screen`'s cursor up or down through `count` items, wrapping.
// Returns -1 for up, 1 for down, 0 if it did not move.
int MoveCursorVertical(ResultScreen *screen, int count);
// The same for `screen`'s spellCursor.
int LoadScreenPlstChunk(ResultScreen *screen, int count);
// The same with left and right.
int MoveCursorHorizontal(ResultScreen *screen, int count);
