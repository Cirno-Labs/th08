// Enemies. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "bullet.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// An enemy script's variables. Only its size is known so far (th07's
// was 0x68).
struct EnemyScriptVars
{
    unsigned char unk_0[0x78];
};

// One of an enemy script's interpolation loops, as th07's.
struct EnemyScriptLoop
{
    void *func;
    Timer timer;
    unsigned char unk_10[0x20];
};
ASSERT_SIZE(EnemyScriptLoop, 0x30);

// Where an enemy script is, as th07's.
struct EnemyScriptState
{
    void *code;
    Timer timer;
    void *func;
    void *funcInstr;
    EnemyScriptVars vars;
    Timer waitTimer;
    EnemyScriptLoop loops[8];
    int compareResult;
    int timedCallActive;
    short sub;
    short unk_226;
};
ASSERT_SIZE(EnemyScriptState, 0x228);

// A box along an enemy's trail, as th07's.
struct EnemyBox
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    float angle;
};
ASSERT_SIZE(EnemyBox, 0x1c);

// An enemy. Only what is known so far; th07's fields where they
// correspond.
struct Enemy
{
    // Whether unk_2da4 is clear and unk_8 is set. Inline; the copy at
    // 0x41f000 is in an /Od object.
    BOOL Func41f000()
    {
        return this->unk_2da4 == 0 && this->unk_8 != NULL ? TRUE : FALSE;
    }
    // How many enemies follow it through unk_8, when Func41f000. Inline;
    // the copy at 0x41fd40 is in an /Od object. DrawBanners shows it
    // for the first boss.
    int Func41fd40()
    {
        Enemy *enemy;
        int count;

        enemy = this;
        count = 0;
        if (this->Func41f000())
        {
            while (enemy->unk_8 != NULL)
            {
                enemy = enemy->unk_8;
                count++;
            }
        }
        return count;
    }

    // Called on the bosses with 0 as a spell card bomb ends (0x42adb0).
    void Func42adb0(int arg);

    unsigned char unk_0[0x8];
    Enemy *unk_8;
    Sprite sprite;
    Sprite sprites[2];
    EnemyScriptState ctx;
    EnemyScriptState savedCtx[16];
    unsigned char unk_2ca0[0x4a];
    short stackDepth;
    short unk_2cec;
    short deathSub;
    unsigned char unk_2cf0[0x40];
    short pendingInterrupt;
    short unk_2d32;
    // Where it is.
    D3DXVECTOR3 pos;
    D3DXVECTOR3 unk_2d40;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 unk_2d58;
    D3DXVECTOR3 unk_2d64;
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 unk_2d7c;
    // Where the player's homing options go to (MoveOptionToEnemy).
    D3DXVECTOR3 unk_2d88;
    float angle;
    float angularVelocity;
    float unk_2d9c;
    float unk_2da0;
    int unk_2da4;
    float speed;
    float acceleration;
    float unk_2db0;
    float unk_2db4;
    D3DXVECTOR3 shootOffset;
    D3DXVECTOR3 unk_2dc4;
    D3DXVECTOR3 unk_2dd0;
    Timer moveTimer;
    int unk_2de8;
    float rankSpeedLow;
    float rankSpeedHigh;
    unsigned char unk_2df4[0x8];
    // 1 in EnemyManager's template. Cleared for the bosses as a spell
    // card bomb (Player::bomb.type 4) ends.
    int unk_2dfc;
    unsigned char unk_2e00[0x8];
    int score;
    int unk_2e0c;
    // g_Unknown18b8a24 in EnemyManager's template; Player::AddedCallback
    // sets the template's from Player::unk_e2b2c.
    int unk_2e10;
    Timer callbackTimer;
    int unk_2e20;
    BulletShooter shooter;
    unsigned char unk_3034[0x2c];
    int shootInterval;
    Timer shootTimer;
    BulletShooter laserShooter;
    struct Laser *lasers[0x20];
    int laserIndex;
    unsigned char unk_3304[0xc];
    char deathEffect;
    BYTE deathEffectIndex;
    BYTE deathEffectVariant;
    BYTE unk_3313;
    BYTE hitFlashTimer;
    unsigned char unk_3315[0x3];
    Timer unk_3318;
    // Bit 3: the petal effects drift towards it while it is the first
    // boss. Bit 30 is cleared with unk_2dfc.
    union
    {
        DWORD unk_3324;
        struct
        {
            DWORD active : 1;
            DWORD isBoss : 1;
            DWORD shootable : 1;
            DWORD contactDamage : 1;
            DWORD hideFollowEffects : 1;
            DWORD unk_3324_5 : 1;
            // Cleared for the bosses as a Dissolve Spell starts.
            DWORD unk_3324_6 : 1;
            DWORD grazeable : 1;
            DWORD unk_3324_8 : 4;
            DWORD moveMode : 2;
            DWORD unk_3324_14 : 3;
            DWORD holdFire : 1;
            DWORD mirrored : 1;
            DWORD clampPos : 1;
            DWORD deathMode : 3;
            DWORD unk_3324_23 : 1;
            DWORD onScreen : 1;
            DWORD rotateSprite : 1;
            DWORD unk_3324_26 : 4;
            DWORD unk_3324_30 : 1;
        };
    };
    unsigned char unk_3328[0xa];
    short turnStill;
    unsigned char unk_3334[0x4];
    short turnLeft;
    short turnRight;
    unsigned char unk_333c[0x14];
    float unk_3350;
    unsigned char unk_3354[0x4];
    int lifeCallbackLife[4];
    int lifeCallbackSub[4];
    int timerCallbackTime;
    int timerCallbackSub;
    // Shown by DrawBanners next to Func41fd40.
    int unk_3380;
    unsigned char unk_3384[0x10];
    EnemyBox trailBoxes[0x60];
    VertexTex1DiffuseXyzrwh spriteStripVertices[0xc2];
    unsigned char unk_534c[0x8];
    Timer invulnTimer;
    unsigned char unk_5360[0x60];
    int unk_53c0;
    unsigned char unk_53c4[0xc];
};
ASSERT_SIZE(Enemy, 0x53d0);

// One of EnemyManager's 16 timelines, as th07's. Its constructor is
// folded with PlayerLaser's (0x449f50).
struct EnemyTimeline
{
    Timer timer;
    void *pos;
};
ASSERT_SIZE(EnemyTimeline, 0x10);

// The enemies, at 0x577f20. Only what is known so far.
class EnemyManager
{
public:
    // Constructs the members, then Initialize (0x42a210).
    EnemyManager();
    // Zeroes everything and sets enemyTemplate's defaults, as th07's
    // (0x429e00).
    void Initialize();
    // Whether any of the bosses is set, as th07's (0x42f1f0).
    int AnyBossActive();
    // Removes the enemies' chain elements, as th07's (0x42ef70).
    static void RemoveChain();
    // Adds the enemies' chain elements, as th07's (0x42c590). Returns 0,
    // or -1 on failure.
    static int RegisterChain();
    // Clears the enemies for a bonus, as th07's (0x42efb0).
    int ClearEnemiesForBonus(int maxBonus, int start);

    // What a spawned enemy starts as.
    Enemy enemyTemplate;
    Enemy enemies[0x1e1];
    // The bosses, as th07's.
    Enemy *bosses[8];
    unsigned char unk_9dcdc0[0x10];
    EnemyTimeline timelines[16];
    Timer unk_9dced0;
    unsigned char unk_9dcedc[0x20];
    // -1 after Initialize.
    int unk_9dcefc[4];
    unsigned char unk_9dcf0c[0x4];
};
ASSERT_SIZE(EnemyManager, 0x9dcf10);
extern EnemyManager g_EnemyManager;
