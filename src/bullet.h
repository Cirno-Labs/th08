// The enemies' bullets and lasers, as th07's bullet.h.

#pragma once

#include <string.h>
#include <windows.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// A bullet type's sprites: the bullet's and those of its spawning
// effects, as th07's.
struct BulletTypeSprites
{
    Sprite sprite0;
    Sprite sprite1;
    Sprite sprite2;
    Sprite sprite3;
    Sprite sprite4;
    // The size of the bullet's hitbox.
    D3DXVECTOR3 hitboxSize;
    // Only copied from the shooter's type into each bullet.
    BYTE unusedByteD40;
    // The height of the bullet's sprite.
    BYTE height;
    // A size class: 5 up to 8 high, 4 or 3 up to 16, 2 or 1 up to 32,
    // 0 above; the draw list the bullet goes into.
    BYTE sizeClass;
    BYTE unusedByteD43;
};
ASSERT_SIZE(BulletTypeSprites, 0xd44);

// One step of a bullet's behavior, as th07's: `flag` is the
// Bullet::flags bit it runs under.
struct BulletCommand
{
    float arg0;
    float arg1;
    // Frames, for most flags.
    int frames;
    int maxTurns;
    DWORD flag;
    int concurrentFlag;
};
ASSERT_SIZE(BulletCommand, 0x18);

// The state of one of a bullet's running behaviors, as th07's.
struct BulletEx
{
    Timer timer;
    float turnSpeed;
    float turnAngle;
    // Added to the velocity (flag 0x10).
    D3DXVECTOR3 accel;
    // How long the behavior runs, in frames.
    int duration;
    int maxTurns;
    int turnCount;
};
ASSERT_SIZE(BulletEx, 0x2c);

// One enemy bullet (0x10b8), as th07's.
struct Bullet
{
    BulletTypeSprites sprites;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 accel;
    float speed;
    float accelSpeed;
    float turnSpeed;
    float angle;
    float angularVelocity;
    float turnAngle;
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_d98[0x10];
    // Set by a command with flag 0x2000.
    int boundsCheckDelay;
    // The behaviors running, by BulletCommand::flag.
    DWORD activeFlags;
    // BulletShooter::flags.
    DWORD flags;
    short color;
    WORD unusedWordDb6;
    // 0 while the slot is free; 6 marks the spare bullet after the last.
    WORD state;
    WORD outOfBoundsFrames;
    BYTE spawnedFlag;
    BYTE grazed;
    // Set while it spawns inside one of the player's hit areas: it turns
    // into an item as it finishes spawning. New in th08.
    BYTE spawnedInArea;
    BYTE unusedByteDbf;
    // The next bullet in the same BulletManager draw list.
    Bullet *next;
    // 0 when fired; free for the g_EnemyScriptFuncs that work on bullets,
    // which use it as a turned mark, a laser index or a countdown (the
    // zone functions: frames before they may turn it again).
    int scriptFuncState;
    // The sound the bullet's behaviors play as they start.
    int sound;
    // The command to start next.
    int commandIndex;
    BulletCommand commands[18];
    BulletEx ex[7];
    // Set as the toggling script functions switch it to its additive
    // sprite; it neither grazes nor hits while set.
    BYTE unk_10b4;
    unsigned char unk_10b5[0x3];
};
ASSERT_SIZE(Bullet, 0x10b8);

// What an enemy fires bullets with, as th07's (0x210).
struct BulletShooter
{
    // Inline; the copy is at 0x42a410.
    BulletShooter()
    {
        memset(this, 0, sizeof(BulletShooter));
        this->bulletSound = -1;
    }

    // The bullet type, and its color.
    short sprite;
    short color;
    D3DXVECTOR3 pos;
    float angle1;
    float angle2;
    float speed1;
    float speed2;
    // Copied into each bullet.
    BulletCommand commands[18];
    // Extra values for the lasers.
    float exFloats[4];
    int exInts[4];
    DWORD laserEndHitFrames;
    short count1;
    short count2;
    // Which of the fire instructions (96 on) set it.
    WORD aimMode;
    // Cleared by the fire instructions; nothing reads it.
    WORD unusedAfterFireC2;
    int flags;
    int sound;
    // Bullet::sound.
    int bulletSound;
    // Bullet::commandIndex, the command its bullets start from. New in
    // th08.
    int commandIndex;
    // FirePattern sets it to the bullet type's sprites.
    BulletTypeSprites *typeSprites;
};
ASSERT_SIZE(BulletShooter, 0x210);

// One enemy laser, as th07's.
struct Laser
{
    Sprite sprite0;
    Sprite sprite1;
    D3DXVECTOR3 pos;
    float angle;
    float startOffset;
    float endOffset;
    float maxLength;
    float width;
    float baseWidth;
    float speed;
    int startFrames;
    int hitStartFrame;
    int durationFrames;
    int endFrames;
    int endHitFrames;
    int active;
    Timer timer;
    WORD flags;
    short color;
    BYTE state;
    BYTE drawDelayFlag;
    unsigned char pad_59a[0x2];
};
ASSERT_SIZE(Laser, 0x59c);

// The bullets, at 0xf54e90.
class BulletManager
{
public:
    // Constructs the members and resets.
    BulletManager();

    // Zeroes everything, marks the spare bullet and clears the script
    // index of every bullet's sprites (0x42f360).
    void Reset();
    // Ends every bullet and every laser without flag 4 (all of them for
    // `mode` 4), as th07's: bullets in one of the player's hit areas turn
    // into items of Player::areaItemType; otherwise with `mode` 4 they
    // fade out, else they, and points every 32 along each laser, turn
    // into `mode` items of type clearItemType. Then sets clearAllFrames
    // to 10. A timer callback ending a spell card calls it with 4.
    void ClearAll(int mode);
    // Clears the bullets, as th07's ClearAll (0x415c60).
    void Func415c60();
    // Clears the bullets for a bonus of up to `maxBonus`, as th07's
    // (0x430aa0). Returns the total.
    int ClearAllWithBonus(int maxBonus, int toItems);
    // Spawns bullet (`col`, `row`) of `shooter`'s pattern, aimed at
    // `aimAngle` by the aiming modes that use it, as th07's. Returns 0,
    // or 1 if no bullet is free.
    int SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle);
    // Turns every bullet of g_BulletManager within `radius` of `pos` into
    // an item of type 6, as th07's.
    void ConvertBulletsToItems(D3DXVECTOR3 *pos, float radius);
    // Fires `shooter`'s pattern, as th07's (0x430e10).
    int FirePattern(BulletShooter *shooter);

    // Loads the bullet sprites and adds the bullets' chain elements, as
    // th07's (0x4311a0). Returns 0, or -1 on failure.
    static int RegisterChain(char *anmPath);
    // Removes the bullets' chain elements, as th07's (0x433850).
    static void RemoveChain();

    BulletTypeSprites typeSprites[0x20];
    // The last one is a spare whose state 6 sends searches back to the
    // first.
    Bullet bullets[0x601];
    Laser lasers[0x100];
    // The bullets alive.
    int bulletCount;
    int clearAllFrames;
    Timer updateTimer;
    int updateFrameCount;
    // RegisterChain's argument.
    char *anmPath;
    // Six lists of the bullets to draw, in order, linked by
    // Bullet::next.
    Bullet *drawLists[6];
    // Where SpawnBullet looks for a free bullet first.
    Bullet *nextBullet;
    // The item type cleared bullets turn into.
    int clearItemType;
    // A copy of g_EtamaAnm, which the items also run their scripts from.
    AnmLoaded *etamaAnm;
};
ASSERT_SIZE(BulletManager, 0x6ba578);
extern BulletManager g_BulletManager;

// Sprite offsets by color, for bullets up to 16 and up to 32 high, as
// th07's.
extern int g_BulletColorOffsets16[16];
extern int g_BulletColorOffsets32[8];

// Shows sprite `src` in `color` on `dst`, by the size of `bullet`'s
// sprite, unless `dst` shows it already, as th07's. Inline, defined in
// bullet.cpp.
void ShowSpawnSprite(Sprite *dst, Sprite *src, Bullet *bullet, int color);

// Starts the bullet's commands from commandIndex on, as th07's: skips
// those whose flag the bullet lacks, and stops at an empty one, at one
// whose concurrentFlag is 0 while a behavior is still running, or after
// starting one behavior in its BulletEx; some commands (sounds, sprite
// changes, bounds delays, firing) go on to the next.
void RunBulletCommands(Bullet *bullet);
// etama.anm (slot 6), which BulletManager's added callback loads.
extern AnmLoaded *g_EtamaAnm;
