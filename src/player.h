// The player and its shots. Only the fields known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

class Player;
struct Effect;

struct PlayerBullet;
struct ShotData;

// A shot's callbacks, as th07's. Under /Gr these are __fastcall.
typedef int (*ShotFireFunc)(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
typedef int (*ShotUpdateFunc)(Player *player, PlayerBullet *bullet);
typedef int (*ShotHitFunc)(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);

// One shot of a shot type, as th07's (0x34 there). A table of them ends
// with an interval below 0. Only the fields known so far.
struct ShotData
{
    // Fired on frames where frame % interval is phase, as th07's.
    short interval;
    short phase;
    unsigned char unk_4[0x24];
    // Fires it, and the callbacks copied into the bullet it fires.
    ShotFireFunc fire;
    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
};
ASSERT_SIZE(ShotData, 0x38);

// The shots of a shot type up to a power level, as th07's.
struct ShotTable
{
    ShotData *shots;
    int maxPower;
};

// The shot callbacks a shot file names by index, which LoadShotTypeFile
// turns into pointers, as th07's. Entry 0 is NULL.
extern ShotFireFunc g_ShotFireFuncs[];
extern ShotUpdateFunc g_ShotUpdateFuncs[];
extern ShotUpdateFunc g_ShotDrawFuncs[];
extern ShotHitFunc g_ShotHitFuncs[];

// A shot type's data, behind Player::shotType and shotTypeFocused, as
// th07's. Only the fields known so far.
struct ShotTypeData
{
    unsigned char pad_0[0x2];
    // How many entries tables has, as th07's.
    WORD tableCount;
    // The bombs the player respawns with.
    float bombs;
    // Player::deathbombFrames once the player has respawned, as th07's.
    int deathbombFrames;
    // The hitbox and the graze box, both square, as th07's.
    float hitboxSize;
    float grazeSize;
    // The speed of the items drawn to the player, as th07's.
    float itemPullSpeed;
    // The width and height of the box the player collects items in, as
    // th07's.
    float itemCatchSize;
    // Above this line the player draws the items in, as th07's (which had
    // deathScoreShare before it).
    float itemAutoCollectLine;
    unsigned char pad_20[0x4];
    // The movement speeds, straight and diagonal, unfocused and focused,
    // as th07's. Each shot type file has all four; HandleInput takes the
    // unfocused ones from shotType and the focused ones from
    // shotTypeFocused.
    float speed;
    float speedFocused;
    float speedDiagonal;
    float speedDiagonalFocused;
    // How fast the items move, as a factor of their velocity. New in th08.
    float itemSpeed;
    ShotTable tables[1];
};

// One of the player's hit areas (0x40 bytes), in two arrays of 0xc0:
// Player::bombHitboxes, the boxes and circles that damage enemies, and
// Player::hitAreas, th07's PlayerArea grown, which hit bullets.
// Placeholder name.
struct PlayerBombHitbox
{
    // Zeroes it, then sets unk_38. Inline; the copy is at 0x44e370.
    void Reset()
    {
        memset(this, 0, sizeof(PlayerBombHitbox));
        this->unk_38 = 1;
    }
    // Inline; the copy is at 0x44e350.
    void Deactivate()
    {
        this->active = 0;
    }

    float x;
    float y;
    // A circle's radius while width is 0, and what it grows by each frame.
    float radius;
    float radiusGrowth;
    float width;
    float height;
    // What width and height grow by each frame.
    float widthGrowth;
    float heightGrowth;
    // The box's rotation.
    float angle;
    // Counts down; the area ends when it reaches 0. Below 0 it never ends.
    int framesLeft;
    // Player::areaItemType when CheckAreaHit meets it (hitAreas).
    int itemType;
    // The damage it deals (bombHitboxes).
    int damage;
    // How often it hit.
    int hits;
    int unk_34;
    int unk_38;
    BYTE active;
    unsigned char unk_3d[0x3];
};

// One of the player's options (0x2f4). New in th08: some shot types
// have up to four, each with its own update and draw callbacks.
struct PlayerOption
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 unk_2b0;
    D3DXVECTOR3 unk_2bc;
    // Whether it is in use.
    int active;
    int unk_2cc;
    // Its index among the four.
    int index;
    unsigned char unk_2d4[0xc];
    Timer timer;
    // From g_OptionUpdateFuncs and g_OptionDrawFuncs by character.
    int (*update)(Player *player, PlayerOption *option);
    int (*draw)(Player *player, PlayerOption *option);
};
ASSERT_SIZE(PlayerOption, 0x2f4);

// One of PlayerBomb's 0x80 parts, as th07's.
struct PlayerBombPart
{
    int state;
    int burstTimer;
    float unk_8;
    float speed;
    float angle;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 trail[0x20];
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 acceleration;
    Sprite sprites[8];
    Effect *effect;
    Timer timer;
    unsigned char unk_16e8[0x8];
};
ASSERT_SIZE(PlayerBombPart, 0x16f0);

// A shot type's bomb callbacks: one for each kind of bomb (0 and 1 the
// two characters', 2 and 3 their last words, 4 during a spell card),
// by Player::bomb.type.
struct BombFuncs
{
    void (*funcs[5])(Player *player);
};

// The bomb update and draw callbacks, by character * 2 and character *
// 2 + 1.
extern BombFuncs g_BombFuncs[];

// The option callbacks, by character; NULL where a character has fewer
// than four options.
typedef int (*OptionFunc)(Player *player, PlayerOption *option);
extern OptionFunc g_OptionUpdateFuncs[][4];
extern OptionFunc g_OptionDrawFuncs[][4];

// The shot type files of each character's two shot types, and its ANM,
// by character.
extern char *g_ShotFiles[];
extern char *g_ShotFilesFocused[];
extern char *g_PlayerAnmPaths[];

// Loads the shot type file at `path` into `*out`, as th07's (0x44dd70).
// Returns 0, or -1 if the file does not load.
int LoadShotTypeFile(ShotTypeData **out, char *path);

// The player's bomb, as th07's.
struct PlayerBomb
{
    // Whether a bomb runs.
    int running;
    // Which of the callbacks run: see BombFuncs.
    int type;
    // How long it runs; 999 when it starts.
    int duration;
    int unk_c;
    // The bombs it used (last words take up to two).
    int bombsUsed;
    int unk_14;
    Timer timer;
    BombFuncs update;
    BombFuncs draw;
    PlayerBombPart parts[0x80];
    D3DXVECTOR3 startPos;
};
ASSERT_SIZE(PlayerBomb, 0xb7858);

// One of the player's bullets (0x484).
struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 trail[0x20];
    D3DXVECTOR3 size;
    unsigned char unk_43c[0x18];
    Timer timer;
    unsigned char unk_460[0x2];
    // 0 unused, 1 flying, 2 hit, as th07's.
    short state;
    unsigned char unk_464[0x20];
};
ASSERT_SIZE(PlayerBullet, 0x484);

// One of the player's lasers, as th07's: its clock and its bullet.
struct PlayerLaser
{
    Timer timer;
    PlayerBullet *bullet;
};
ASSERT_SIZE(PlayerLaser, 0x10);

// OnUpdate's steps, as th07's (0x44c5b0, 0x44c650, 0x44cbf0,
// 0x44d180, 0x44d2c0, 0x44aec0, 0x44d420): UpdateDyingState returns
// nonzero when the player respawns.
void UpdateHitboxAreas(Player *player);
void UpdateBombState(Player *player);
int UpdateDyingState(Player *player);
void UpdateRespawningState(Player *player);
void UpdateInvulnerability(Player *player);
int HandleInput(Player *player);
void ResetHomingAndText(Player *player);
// The bullets' update, firing and drawing, as th07's, in the shot
// object (0x451150, 0x451500, 0x4512f0, 0x451400).
void UpdateBullets(Player *player);
int UpdateFiring(Player *player);
void DrawBullets(Player *player);
void DrawHitBullets(Player *player);

// The player, the global at 0x17d5ef8.
class Player
{
public:
    // Empty, and defined out of line: constructs the members (0x449ca0).
    Player();

    // Starts the fire timer from 0 unless it already runs, as th07's
    // (0x451640, in the shot object).
    static void StartFiring(Player *player);
    // Clears the player, keeping the shot types while KeepsResources(),
    // and adds it to the chains (update priority 9, draw 9 and 10),
    // keeping `arg` in registerChainArg, as th07's. Returns 0, or -1 if
    // adding fails.
    static int RegisterChain(int arg);
    // The chain callbacks (0x44c390, 0x44d530, 0x44d630, 0x44d650,
    // 0x44dc60).
    static int OnUpdate(Player *player);
    static int OnDraw(Player *player);
    static int OnDraw2(Player *player);
    static int AddedCallback(Player *player);
    static int DeletedCallback(Player *player);
    // Removes the player's chain elements, as th07's (0x44dd10).
    static void RemoveChain();
    // Whether a `size` box at `pos` meets one of hitAreas: a circle
    // while its radius is set, else a box, turned by its angle when that
    // is set. Keeps the area's itemType in areaItemType, counts its hits
    // and returns 2 if so, else returns 0. As th07's, which tested the
    // whole box and had no turned ones; `size` is unused now.
    int CheckAreaHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a `size` box at `pos` hits the player, as th07's: 2 if it
    // meets one of hitAreas (CheckAreaHit; areaItemType 6 otherwise),
    // else 1 if it meets the hitbox, else 0. A hit marks bit 1 of
    // g_ReplayManager->frameEvents and kills the player while
    // playerState is 0.
    int CheckHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // CheckHit without the hit areas: 1 if the box meets the hitbox,
    // else 0. New in th08; an enemy's body uses it (0x42c290).
    int CheckHitIgnoringAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a `size` box at `pos`, grown by 20 on each side, grazes the
    // player, as th07's: 2 if it meets one of hitAreas, else 0 while
    // respawning or dying or off the graze box, else grazes `pos` and
    // returns 1.
    int CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a laser hits or grazes the player, as th07's: `pos` and
    // `size` are its box in its own frame, into which the hitbox is
    // turned by `angle` around `origin`. On a hit, marks bit 1 of
    // g_ReplayManager->frameEvents and kills the player while
    // playerState is 0, returning 1, else 0. Otherwise, with `canGraze`,
    // grazes the player within 48 of the box unless it is respawning or
    // dying, returning 2; else returns 0.
    int CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze);
    // Scores a graze of the bullet at `pos` (0x44a930); `isLaser` is
    // set for lasers.
    void Graze(D3DXVECTOR3 *pos, int isLaser);
    // Starts dying, as th07's: unfreezes the game, rerolls the
    // checksums, bursts effect 6, sets playerState to 2 with stateTimer at
    // 0, plays sound 4 and marks bit 9 of g_ReplayManager->frameEvents.
    // With GameManager::unk_3dbac_7 set, just leaves 2 deathbombFrames
    // and sets unk_6. Otherwise clears stats->unk_22 and, with bombs
    // left, gives 6 deathbombFrames a bomb (7 more once the time orbs
    // reach their target), at most 15, doubled up to 30 in a spell card
    // and times 9/5 for characters 0, 4 and 5; tints the player and
    // starts the last-words ring effect 0x3b (unk_e2b28), setting
    // unk_3dbac_10 in a spell card. Without bombs, 2 deathbombFrames.
    // Then recycles the collected items.
    void Die();
    // The angle from `pos` to the player, as th07's (0x44c1b0).
    float AngleFrom(D3DXVECTOR3 *pos);
    // Whether the box of `size` at `pos` touches the player's graze box,
    // as th07's (0x44a5a0).
    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Takes the first free one of hitAreas's first 0xbf entries, or the
    // last, and starts it as a `width` by `height` box at `pos` (x and
    // y) for `framesLeft` frames with `itemType`, as th07's
    // AddHitboxArea. Returns it (0x44de60).
    PlayerBombHitbox *AddHitArea(D3DXVECTOR3 *pos, float width, float height, int itemType, int framesLeft);
    // The same with a circle of `radius`, growing by `radiusGrowth`
    // (0x44df00).
    PlayerBombHitbox *AddHitAreaCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int framesLeft,
                                       int itemType);
    // The same in bombHitboxes, dealing `damage` (0x44dfa0, 0x44e040).
    PlayerBombHitbox *AddBombHitbox(D3DXVECTOR3 *pos, float width, float height, int damage, int framesLeft);
    PlayerBombHitbox *AddBombHitboxCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int damage,
                                          int framesLeft);
    // Whether unk_5 is set. Inline; the copy is at 0x40bc40.
    int IsUnk5Set()
    {
        return this->unk_5;
    }
    // Whether unk_5 is clear. Inline; the copy is at 0x40bc20.
    int IsUnk5Clear()
    {
        return !this->unk_5;
    }

    // 0 normally, 1 respawning, 2 dying, 3 invulnerable, as th07's.
    char playerState;
    // RegisterChain's argument, as th07's.
    BYTE registerChainArg;
    char unk_2;
    // Whether the player is focused (HandleInput; 2 until the first
    // frame), as th07's. Picks shotTypeFocused over shotType and the
    // direction a time orb moves stats->unk_22.
    BYTE focused;
    // Set while the last words run (UpdateDyingState): a bomb then
    // starts as the other character's last word.
    BYTE unk_4;
    // Stage::StepObjects sends interrupt 1 to g_Stage.unk_844 as it is
    // set, 2 as it is cleared.
    BYTE unk_5;
    BYTE unk_6;
    unsigned char unk_7[0x1];
    // The frames since the player last switched focus.
    int focusFrames;
    // The character's ANM, in slot 5.
    AnmLoaded *anm;
    Sprite sprite;
    // The player's position.
    D3DXVECTOR3 pos;
    D3DXVECTOR3 unk_2c0;
    // The player's last positions.
    D3DXVECTOR3 posHistory[0x10];
    // The corners of the hitbox and of the graze box, as th07's.
    D3DXVECTOR3 hitboxMin;
    D3DXVECTOR3 hitboxMax;
    D3DXVECTOR3 grazeMin;
    D3DXVECTOR3 grazeMax;
    // The corners of the box TouchesGrazeBox tests, as th07's.
    D3DXVECTOR3 grazeTestMin;
    D3DXVECTOR3 grazeTestMax;
    // The boxes' half sizes, as th07's.
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 grazeSize;
    D3DXVECTOR3 grazeTestHalfSize;
    D3DXVECTOR3 unk_3f8;
    // Scale the player's movement, as th07's.
    float moveScaleX;
    float moveScaleY;
    PlayerOption options[4];
    PlayerBomb bomb;
    PlayerBombHitbox bombHitboxes[0xc0];
    PlayerBombHitbox hitAreas[0xc0];
    Effect *unk_be834;
    PlayerBullet bullets[0x80];
    PlayerLaser lasers[3];
    // As th07's: while the player dies, the frames left to bomb.
    int deathbombFrames;
    // Frames left before the next bomb, as th07's.
    int bombCooldown;
    // Frames left of clearing all bullets after a respawn, as th07's.
    int respawnClearFrames;
    ShotTypeData *shotType;
    ShotTypeData *shotTypeFocused;
    // 1 on the frames a bomb starts, else 0, as th07's. A time orb counts
    // 100 points while it is set.
    int bombJustStarted;
    unsigned char unk_e2a80[0x10];
    // The itemType of the area CheckAreaHit met last, as th07's.
    int areaItemType;
    int unk_e2a94;
    // 0 still, 1 up, 2 down, 3 left, 4 right, 5 to 8 diagonal, as
    // th07's.
    int direction;
    // The last frame's movement, as th07's.
    float velocityX;
    float velocityY;
    // What the homing shots aim at, as th07's; x is -100 or less when
    // there is nothing.
    D3DXVECTOR3 orbHomingTarget;
    D3DXVECTOR3 homingTarget;
    int unk_e2abc;
    int homingTargetSet;
    // While it is below 0, items flying up (state 3) are drawn in.
    Timer unk_e2ac4;
    Timer unk_e2ad0;
    // Collecting a time orb moves stats->unk_22 while it is 0.
    Timer unk_e2adc;
    Timer unk_e2ae8;
    // Runs through the player states, as th07's.
    Timer stateTimer;
    Timer unk_e2b00;
    // The angle the aimed shots fly at, as th07's.
    float aimedShotAngle;
    // The update element and the two draw elements, as th07's.
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *drawChain2;
    // Effects that follow the player, as th07's.
    Effect *invulnerabilityEffect;
    Effect *bombInvulnerabilityEffect;
    Effect *unk_e2b24;
    // The effect of the player's last words, started as it dies.
    Effect *unk_e2b28;
    int unk_e2b2c;
};
ASSERT_SIZE(Player, 0xe2b30);
extern Player g_Player;
