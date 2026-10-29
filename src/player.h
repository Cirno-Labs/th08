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
    unsigned char pad_20[0x14];
    // How fast the items move, as a factor of their velocity. New in th08.
    float itemSpeed;
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
    // From g_OptionUpdateFuncs and g_OptionDrawFuncs by shot type.
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

// The bomb update and draw callbacks of a shot type, from
// g_BombData by character.
struct BombData
{
    BombFuncs update;
    BombFuncs draw;
};
extern BombData g_BombData[];

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

// The player, the global at 0x17d5ef8.
class Player
{
public:
    // Empty, and defined out of line: constructs the members (0x449ca0).
    Player();

    // Adds the player's chain elements, as th07's (0x44c230). Returns 0,
    // or -1 on failure.
    static int RegisterChain(int arg);
    // Removes the player's chain elements, as th07's (0x44dd10).
    static void RemoveChain();
    // Whether a `size` box at `pos` meets one of hitAreas: a circle
    // while its radius is set, else a box, turned by its angle when that
    // is set. Keeps the area's itemType in areaItemType, counts its hits
    // and returns 2 if so, else returns 0. As th07's, which tested the
    // whole box and had no turned ones; `size` is unused now.
    int CheckAreaHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // The angle from `pos` to the player, as th07's (0x44c1b0).
    float AngleFrom(D3DXVECTOR3 *pos);
    // Whether the box of `size` at `pos` touches the player's graze box,
    // as th07's (0x44a5a0).
    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Starts the first free hit area of the second array at `pos` (x
    // and y), `width` by `height`, with unk_2c `a` and unk_24 `b`, and
    // returns it (0x44de60).
    PlayerBombHitbox *Func44de60(D3DXVECTOR3 *pos, float width, float height, int a, int b);
    // The same in the first array (0x44dfa0).
    PlayerBombHitbox *Func44dfa0(D3DXVECTOR3 *pos, float width, float height, int a, int b);
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
    // Picks unk_e2a78 over shotType for the items' pull speed, and the
    // direction a time orb moves stats->unk_22. Probably the focus.
    BYTE unk_3;
    BYTE unk_4;
    // Stage::StepObjects sends interrupt 1 to g_Stage.unk_844 as it is
    // set, 2 as it is cleared.
    BYTE unk_5;
    BYTE unk_6;
    unsigned char unk_7[0x5];
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
    ShotTypeData *unk_e2a78;
    // 1 on the frames a bomb starts, else 0, as th07's. A time orb counts
    // 100 points while it is set.
    int bombJustStarted;
    unsigned char unk_e2a80[0x10];
    // The itemType of the area CheckAreaHit met last, as th07's.
    int areaItemType;
    int unk_e2a94;
    int unk_e2a98;
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
    Timer unk_e2af4;
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
