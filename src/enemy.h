// Enemies. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "bullet.h"
#include "chain.h"
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
    // Frees and clears unk_3384 (0x42bc90).
    void Func42bc90();

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
    // Its life, as th07's. Cleared for the bosses as a spell card bomb
    // (Player::bomb.type 4) ends.
    int life;
    int maxLife;
    int unk_2e04;
    int score;
    // Its index in EnemyManager::enemies.
    int index;
    // g_Unknown18b8a24 in EnemyManager's template; Player::AddedCallback
    // sets the template's from Player::unk_e2b2c.
    int unk_2e10;
    Timer callbackTimer;
    D3DCOLOR color;
    BulletShooter shooter;
    unsigned char unk_3034[0x2c];
    int shootInterval;
    Timer shootTimer;
    BulletShooter laserShooter;
    struct Laser *lasers[0x20];
    int laserIndex;
    int item;
    // Set by timeline instructions 11 and 12.
    int unk_3308;
    int unk_330c;
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
    // Freed by Func42bc90.
    void *unk_3384[4];
    EnemyBox trailBoxes[0x60];
    VertexTex1DiffuseXyzrwh spriteStripVertices[0xc2];
    unsigned char unk_534c[0x8];
    Timer invulnTimer;
    // Effects that follow it, as th07's.
    struct Effect *effects[0x18];
    int effectCount;
    // How far the effects circle it, as th07's.
    float effectRadius;
    unsigned char unk_53c8[0x8];
};
ASSERT_SIZE(Enemy, 0x53d0);

// The arguments of the timeline's spawn instructions (0, 1 and 15 at a
// position, 2 and 4 at a random x from xMin to xMax, 3 and 5 at a random
// x in the play area, 11 and 12 with two more values for the enemy).
struct EnemyTimelineSpawn
{
    int sub;
    float x;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnRandomX
{
    int sub;
    float xMin;
    float xMax;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnAnyX
{
    int sub;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnWithArgs
{
    int sub;
    float x;
    float y;
    int life;
    int unk_10;
    int unk_14;
    int score;
};

// One instruction of a timeline: from `time` on, `opcode` runs on the
// difficulties in the bits of `difficulties`. New layout in th08.
struct EnemyTimelineEntry
{
    int time;
    short opcode;
    BYTE size;
    BYTE difficulties;
    int args[1];
};

// One of EnemyManager's 16 timelines, as th07's. Its constructor is
// folded with PlayerLaser's (0x449f50).
struct EnemyTimeline
{
    // Runs the instructions due this frame, then ticks timer, as th07's
    // (0x42a8a0).
    void RunTimeline();

    Timer timer;
    EnemyTimelineEntry *pos;
};
ASSERT_SIZE(EnemyTimeline, 0x10);

// Keeps `enemy`'s effects on it, circling at effectRadius, as th07's
// (0x42e010).
void MoveEnemyEffects(Enemy *enemy);
// Fades out and drops `enemy`'s effects, as th07's (0x42a820).
void ReleaseEnemyEffects(Enemy *enemy);

// The angle `t` of the way from `a` to `b`, the short way round, as
// th07's (0x42eb10).
float LerpAngle(float a, float b, float t);

// The enemy scripts (ECL), at 0x4eccb8, as th07's. Only what is known
// so far.
class EnemyScripts
{
public:
    // Starts subroutine `sub` in `ctx`, as th07's (0x418450). Returns 0.
    int StartSub(EnemyScriptState *ctx, short sub);
    // Runs `enemy`'s script for a frame, as th07's (0x4184b0). Returns
    // -1 once the script ends, else 0.
    int RunEnemy(Enemy *enemy);
    // Frees the script file, as th07's (0x418420).
    void Release();

    void *file;
    void **subs;
};
extern EnemyScripts g_EnemyScripts;

// The enemies, at 0x577f20. Only what is known so far.
class EnemyManager
{
public:
    // Constructs the members, then Initialize (0x42a210).
    EnemyManager();
    // Zeroes everything and sets enemyTemplate's defaults, as th07's
    // (0x429e00).
    void Initialize();
    // Starts subroutine `sub` on a free enemy at `pos`, as th07's, and
    // keeps its index; marks bit 12 of g_ReplayManager->frameEvents and
    // sets spawnFailed (0x42a4e0).
    Enemy *Spawn(int sub, D3DXVECTOR3 *pos, int life, int item, int score, int mirrored);
    // Spawn with the script variables `vars`, as th07's (0x42a680).
    Enemy *SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, int item, int score, EnemyScriptVars *vars);
    // Draw the enemies of draw lists `from` to `to`, as th07's
    // (0x42e140).
    static int DrawEnemies(EnemyManager *mgr, int from, int to);
    // The draw chain callbacks, as th07's: lists 0 to 2, then 2 to 4 in
    // red while bit 10 of g_GameManager.unk_3dbac is set (0x42e120,
    // 0x42eb90).
    static int OnDraw(EnemyManager *mgr);
    static int OnDraw2(EnemyManager *mgr);
    // Whether any of the bosses is set, as th07's (0x42f1f0).
    int AnyBossActive();
    // Removes the enemies' chain elements, as th07's (0x42ef70).
    static void RemoveChain();
    // Initializes the enemies and adds their chain elements, as th07's
    // (0x42c590): update priority 11, draw 8 and 11. Returns 0, or -1 on
    // failure.
    static int RegisterChain();
    // The chain callbacks (0x42c660, 0x42ebf0, 0x42ee80).
    static int OnUpdate(EnemyManager *mgr);
    static int AddedCallback(EnemyManager *mgr);
    static int DeletedCallback(EnemyManager *mgr);
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
    unsigned char unk_9dcedc[0x1c];
    // Whether the last Spawn found no free enemy.
    int spawnFailed;
    // -1 after Initialize; timeline instructions 13 and 14 use them.
    int unk_9dcefc[4];
    // While set, the timelines spawn nothing.
    int unk_9dcf0c;
};
ASSERT_SIZE(EnemyManager, 0x9dcf10);
extern EnemyManager g_EnemyManager;
extern ChainElem g_EnemyManagerCalcChain;
extern ChainElem g_EnemyManagerDrawChain;
extern ChainElem g_EnemyManagerDrawChain2;
