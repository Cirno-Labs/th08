// Replays ("./replay/th8_%.2d.rpy"). Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"

// A frame of a replay in the format with seeds (ReplayHeader::withSeeds):
// the buttons held and the random seed at the start of the frame. New in
// th08.
struct ReplayFrameWithSeed
{
    WORD input;
    WORD events;
    WORD seed;
};

// A stage of a replay: the game's state at its start, then its frames
// (just the buttons held now, a WORD each). Only the fields known so far.
struct ReplayStage
{
    // The score the stage started with, divided by 10.
    int score;
    // GameManagerStats::pointItemsTotal.
    int pointItemsTotal;
    // GameManagerStats::grazeTotal.
    int grazeTotal;
    // GameManagerStats's unk_34, unk_38 and unk_24.
    int checksumKeyStage;
    int checksumKeyPoints;
    int maxPointValue;
    // GameManagerStats::unk_22.
    WORD gauge;
    // GameManager::rngSeed.
    WORD rngSeed;
    BYTE power;
    BYTE lives;
    BYTE bombs;
    // GameManager::rank.
    BYTE rank;
    // GameManager::unk_3dba8.
    BYTE powerItemCombo;
    // GameManagerStats::spellCardsCaptured.
    BYTE spellCardsCaptured;
    // GameManagerStats::unk_28.
    char clock;
    BYTE unusedByte23;
    WORD frames[1];
};

// The start of a replay file (0x134 bytes; th07 0xe8). Only the fields
// known so far.
struct ReplayHeader
{
    // "T8RP".
    char magic[4];
    // 6 in its low 12 bits.
    WORD version;
    // Set for the format with seeds: its frames are ReplayFrameWithSeed,
    // and playing it back records the seeds too. 0 when recording.
    BYTE withSeeds;
    BYTE isSaved;
    BYTE pad_08[0x4];
    // The size of the file up to the user section.
    int size;
    // 0x3f000318 plus the bytes from `key` on.
    int checksum;
    BYTE secondaryKey;
    // The bytes from `compressedSize` on are stored plus key, key + 7,
    // key + 14, ...
    BYTE key;
    BYTE pad_16[0x2];
    // In the file, the size of the compressed data after the first 0x68
    // bytes, and its size decompressed.
    int compressedSize;
    int dataSize;
    // Indexed by stage. In the file, the offsets of the stages' data.
    ReplayStage *stages[9];
    // The same for the frame rates kept every 30 frames.
    BYTE *extras[9];
    BYTE randomByte;
    // 100.
    BYTE versionByte;
    // The shot type.
    BYTE shotType;
    BYTE difficulty;
    // The date it was saved, "%m/%d".
    char date[6];
    // The player's name.
    char name[9];
    // Whether it is a practice.
    BYTE isPractice;
    // In spell card practice, the card's number, and its name; -1
    // otherwise.
    short spellCardNumber;
    char spellCardName[0x30];
    // 0x100.
    WORD subVersion;
    DWORD score;
    // The config the game was played with, a GameManagerUnk4.
    BYTE config[0x3c];
    unsigned char pad_f0[0x24];
    // How far the game ran below full speed, in percent, plus 1.12, and
    // as is.
    float slowdown2;
    float slowdown;
    // 2 if the shot type has cleared Extra or Phantasm, 1 if the final
    // stage, else 0.
    char clearLevel;
    BYTE pad_11d[0x3];
    // 30.
    int replayFormatVersion;
    // App::exeSize and exeChecksum, and "0100d".
    DWORD exeSize;
    DWORD exeChecksum;
    char exeVersion[6];
    BYTE pad_132[0x2];
};
ASSERT_SIZE(ReplayHeader, 0x134);

// The text section after a replay's data: "USER", its size counted from
// `magic`, and a type.
struct ReplayUserHeader
{
    DWORD magic;
    int size;
    BYTE type;
};

// Checks and decodes the replay file `data` of `size` bytes: its magic,
// version and checksum, then decompresses it into a new block, followed
// by the user section as it is, which it returns unless the config's
// scoreDisabled is set or App::CheckVersionFile rejects the executable it
// was recorded with. Frees `data`, and returns NULL if it is not a valid
// replay (0x451d90).
ReplayHeader *ValidateReplay(void *data, int size);
// Saves the replay being recorded to `path`, under `name`, unless it
// was played with another config than the current one (outside practice
// and Extra) or with the config's scoreDisabled set: the header's first
// 0x68 bytes as they are, the rest and the stages' frames and frame rates
// compressed with LzssCompress, all encoded as ValidateReplay decodes
// them, then a "USER" section with the game's details as text. Then frees
// the stages and removes the update element (0x4531f0).
void SaveReplay(char *path, char *name);

// Records and plays back replays, behind the pointer at 0x18b8a28. Only
// the fields known so far.
class ReplayManager
{
public:
    // Creates the ReplayManager, recording (`mode` 0) or playing (1) the
    // replay at `path`, and adds its chain elements; if it exists, runs
    // the mode's added callback again. Returns 0, or -1 if adding the
    // update element fails.
    static int RegisterChain(int mode, char *path);
    // The second update callback while recording (and while playing a
    // replay with seeds), run before the game's own: clears frameEvents
    // (marking 0x100 when GameManager::pauseRequested is set, which it
    // clears), keeps the random seed in frameSeed and clears the count of
    // random numbers. Returns 1.
    static int RecordOnUpdate2(ReplayManager *replay);
    // The update callback while recording: while bit 2 of
    // GameManager::unk_3dbac is set, plays by g_Buttons and, unless the
    // config's scoreDisabled or App::GetNoScore is set, records them (on
    // up to three frames while bit 9 is set) and every 30 frames the frame
    // rate (0x80 set while App::fastClockStreak is), then counts the frame.
    // Returns 1.
    static int RecordOnUpdate(ReplayManager *replay);
    // The second update callback while playing: unless bit 9 of
    // GameManager::unk_3dbac is set, while bit 2 is, returns 6 on two
    // frames of every three while g_Gui's IsMsgRunning and IsMsgSkippable
    // hold, and on four of every five while replayMenuChoice is 2 and
    // g_EnemyManager.AnyBossActive() does not; otherwise 1.
    static int PlayOnUpdate2(ReplayManager *replay);
    // The update callback while playing: while bit 2 of
    // GameManager::unk_3dbac is set and neither the config's
    // scoreDisabled nor bit 9 is, plays the recorded buttons for the
    // frame, with key repeat as in App::OnUpdate, and every 30 frames
    // puts the recorded frame rate back into App::roundedFps and
    // fpsWarning, then counts the frame. Returns 1.
    static int PlayOnUpdate(ReplayManager *replay);
    // The same for a replay with seeds, through seedCursor. New in th08.
    static int PlayOnUpdateWithSeeds(ReplayManager *replay);
    // The update element's added callback when recording: starts the
    // replay data the first time ("T8RP", the shot type, difficulty,
    // config and executable), otherwise keeps the score in the last stage
    // before; then (re)allocates the stage's frames and frame rates and
    // keeps the game's state in the stage. Returns 0.
    static int RecordAddedCallback(ReplayManager *replay);
    // The update element's added callback when playing: loads and checks
    // the replay at `path` the first time, turning its offsets into
    // pointers; then puts the stage's state back into the game. Returns
    // 0, or -1 if the replay or the stage is missing.
    static int PlayAddedCallback(ReplayManager *replay);
    // Removes the second update elements, frees the replay data and
    // frameRateBuffer and deletes the ReplayManager. Returns 0.
    static int DeletedCallback(ReplayManager *replay);
    // Ends the current stage's record with an empty frame and keeps
    // where it ends.
    static void EndStage();

    // 0 when recording, 1 when playing. Inline; the copy at 0x453cc0 is
    // the replay object's.
    int GetMode()
    {
        return this->mode;
    }

    // The current frame.
    int frame;
    // While bit 9 of GameManager::unk_3dbac is set, how many frames were
    // recorded.
    int framesAfterStageClear;
    // The replay data.
    ReplayHeader *data;
    // Freed by DeletedCallback; never allocated.
    void *frameRateBuffer;
    // RegisterChain's arguments.
    int mode;
    char *path;
    // Constructed and zeroed, never used.
    D3DXVECTOR3 unusedVector18;
    D3DXVECTOR3 unusedVector24;
    D3DXVECTOR3 unusedVector30;
    D3DXVECTOR3 unusedVector3c;
    unsigned char pad_48[0x6];
    WORD clearedAtRecordStart;
    // The last frame recorded or played; [stage] is where that stage's
    // frames end.
    WORD *cursor;
    WORD *stageEnds[9];
    // The same for the frames of a replay with seeds.
    ReplayFrameWithSeed *seedCursor;
    ReplayFrameWithSeed *seedStageEnds[9];
    // The same for the frame rates kept every 30 frames.
    BYTE *extraCursor;
    BYTE *extraEnds[9];
    // The update element, an unused one, the second update element when
    // playing and when recording.
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *calcChain2;
    ChainElem *calcChain3;
    // The random seed at the start of the frame.
    WORD frameSeed;
    // Bits set during a frame: 0x100 when the game is paused.
    WORD frameEvents;
};
ASSERT_SIZE(ReplayManager, 0xdc);
extern ReplayManager *g_ReplayManager;

// The difficulties' names for a replay's user section ("Easy" to
// "LastWord").
extern char *g_ReplayDifficultyNames[6];
