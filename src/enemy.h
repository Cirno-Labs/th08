// Enemies. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "bullet.h"
#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// One argument of a script instruction, as th07's.
union EnemyScriptArg
{
    int i;
    float f;
};

// One instruction of an enemy script, as th07's.
struct EnemyScriptInstr
{
    // The frame it runs on.
    int time;
    short opcode;
    // The size of the instruction, arguments included.
    short size;
    BYTE pad_8;
    BYTE difficultyMask;
    // Bit n: argument n names a variable.
    WORD paramMask;
    EnemyScriptArg args[1];
};

// The arguments a subroutine call passes (th07's ints3 and floats3).
struct EnemyScriptCallArgs
{
    int ints[4];
    float floats[4];
};

// An enemy script's variables, as th07's, by operand number: ints
// 10000 to 10007, floats 10016 to 10023, ints2 10036 to 10039, floats2
// 10094 and 10095, then the call's arguments: ints 10053 to 10056 and
// floats 10057 to 10060.
struct EnemyScriptVars
{
    int ints[8];
    float floats[8];
    int ints2[4];
    float floats2[2];
    EnemyScriptCallArgs args;
};
ASSERT_SIZE(EnemyScriptVars, 0x78);

struct Enemy;
struct EnemyScriptInstr;

// A function of g_EnemyScriptFuncs, as th07's: runs with the instruction
// that started it, from opcode 136, or each frame from RunEnemy after
// opcode 137.
typedef void (*EnemyScriptFunc)(Enemy *enemy, EnemyScriptInstr *instr);
// The functions opcodes 136 and 137 run (0x4c6cb0).
extern EnemyScriptFunc g_EnemyScriptFuncs[32];

// What an interpolation loop runs each frame, as th07's: ScriptMoveLinear
// or ScriptMoveHermite.
typedef void (*EnemyScriptInterp)(Enemy *enemy, EnemyScriptInstr *instr, float t);
// The interpolations by loop type, as th07's (0x4c6c90).
extern EnemyScriptInterp g_EnemyScriptInterps[8];

// One of an enemy script's interpolation loops, as th07's, the
// EnemyScriptInterp it runs.
struct EnemyScriptLoop
{
    EnemyScriptInterp func;
    Timer timer;
    // Its length in frames, its kind (picks func) and easing.
    int duration;
    int type;
    int mode;
    // The start and end values, and the curve's tangents.
    float args[4];
    // The variable it sets.
    float target;
};
ASSERT_SIZE(EnemyScriptLoop, 0x30);

// Where an enemy script is, as th07's.
struct EnemyScriptState
{
    void *code;
    Timer timer;
    // Runs each frame with funcInstr, from opcode 137, as th07's.
    EnemyScriptFunc func;
    EnemyScriptInstr *funcInstr;
    EnemyScriptVars vars;
    Timer waitTimer;
    EnemyScriptLoop loops[8];
    int compareResult;
    int timedCallActive;
    short sub;
    short pad_226;
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

// One of an enemy's four extra script threads ("ECLInt"), which RunEnemy
// runs after its main context each frame: opcode 135 starts subroutine
// `sub` in one with the current variables. New in th08.
struct EnemyScriptThread
{
    int sub;
    short pad_4;
    // Enemy::stackDepth while the thread runs.
    short stackDepth;
    EnemyScriptState ctx;
    // Its call stack (Enemy::callStack while it runs).
    EnemyScriptState callStack[16];
};
ASSERT_SIZE(EnemyScriptThread, 0x24b0);

// The arguments of the spawn instructions (90 to 94), as th07's. Its
// implicit constructor is 0x41f400.
struct EnemySpawnArgs
{
    int sub;
    D3DXVECTOR3 pos;
    int life;
    int item;
    int score;
};

// An enemy. Only what is known so far; th07's fields where they
// correspond.
struct Enemy
{
    // Whether leader is clear and nextFollower is set. Inline; the copy at
    // 0x41f000 is in an /Od object.
    BOOL IsLeader()
    {
        return this->leader == 0 && this->nextFollower != NULL ? TRUE : FALSE;
    }
    // How many enemies follow it through nextFollower, when IsLeader. Inline;
    // the copy at 0x41fd40 is in an /Od object. DrawBanners shows it
    // for the first boss.
    int CountFollowers()
    {
        Enemy *enemy;
        int count;

        enemy = this;
        count = 0;
        if (this->IsLeader())
        {
            while (enemy->nextFollower != NULL)
            {
                enemy = enemy->nextFollower;
                count++;
            }
        }
        return count;
    }

    // The last enemy following it through nextFollower, when IsLeader, else
    // itself. Inline; the copy at 0x41efc0 is in an /Od object.
    Enemy *GetLastFollower()
    {
        Enemy *enemy;

        enemy = this;
        if (this->IsLeader())
        {
            while (enemy->nextFollower != NULL)
            {
                enemy = enemy->nextFollower;
            }
        }
        return enemy;
    }

    // Whether leader is set. Inline; the copy at 0x41fd20 is in an
    // /Od object.
    BOOL IsFollower()
    {
        return this->leader != 0;
    }

    // Breaks up the list of enemies following it through nextFollower (and
    // takes it out of its own leader's): with `collect`, each counts
    // as a kill shown by AsciiText::AddNumberPopup, bursts time orbs (more
    // for solo characters) and a hit area, drops items unless a boss
    // fight runs outside a spell card, and plays sound 2 or 3; then
    // it shows followerCount and bursts twice as many orbs. A follower with
    // `collect` also lowers stats->gauge by a twelfth and drops an orb.
    // Called with 0 on the bosses as a spell card bomb ends (0x42adb0).
    void ReleaseFollowers(int collect);
    // Ends it, as th07's: ReleaseFollowers(0) first; unless deathMode is set,
    // deactivates it; a boss leaves its slot, hides its boss marker and
    // ends the boss fight; drops its effects, sends attachedEffect interrupt
    // 3, clears its callbacks and frees threads (0x42bcf0).
    void Despawn();
    // Takes it out of the list through prevFollower and nextFollower while
    // IsFollower, clearing leader; else just clears both links (0x42b2f0).
    void LeaveFollowerList();
    // Follows Player::isYoukai (the human/youkai side?): as it changes,
    // bursts effect 0x1f or 0x1e, sends attachedEffect interrupt 2 or 1 and
    // plays sound 0x28 or 0x27 (0x42c420).
    void UpdateYoukaiState();
    // Moves it by velocity (x mirrored while mirrored is set), keeping
    // prevPos and posDelta, as th07's inline (0x42deb0).
    void Move();
    // Resets the rank speeds and counts after a life or timer callback,
    // as th07's did in place. Inline; the copy at 0x415c80 is an /Od
    // object's.
    void ResetRankSpeeds()
    {
        this->rankSpeedLow = -0.5f;
        this->rankSpeedHigh = 0.5f;
        this->rankCount1Low = 0;
        this->rankCount1High = 0;
        this->rankCount2Low = 0;
        this->rankCount2High = 0;
    }
    // Runs the first life callback whose life the enemy dropped below, as
    // th07's: restarts its script there, ends its timer callback, frees
    // threads, ReleaseFollowers(1), kills the other non-boss enemies and makes
    // the player invulnerable for 70 frames after a boss's. Returns 1 then,
    // else 0 after setting lifeCallbackProximity by how close the life is to
    // the next callback (or to 0) (0x42b490).
    int CheckLifeCallback();
    // Runs the timer callback once callbackTimer reaches
    // timerCallbackTime, as th07's: shows boss 0's time left; then drops
    // the life to the highest life callback, restarts the script at
    // timerCallbackSub, ends the spell card (unless keepSpellCard),
    // makes the player invulnerable for 70 frames after a boss's,
    // ReleaseFollowers(0), kills the other non-boss enemies and frees threads.
    // Returns 1 then, else 0 (0x42b930).
    int CheckTimerCallback();
    // Keeps pos within lowerBound and upperBound while clampPos is set,
    // as th07's (0x42c180).
    void ClampPos();
    // Grazes the player every sixth frame while grazeable, and takes 10
    // damage when its body (a box of size / 1.5) hits the player, unless
    // it is a boss or grazeable or a damage-sharing part of character 0
    // or 4. As th07's (0x42c290).
    void CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Passes half of `damage` on to leader while IsFollower (a ninth
    // while leader, a boss, is invulnerable; none for others then),
    // keeping its life above its life callbacks (0x42b370).
    void ShareDamageWithLeader(int damage);
    // Drops its items, as th07's OnUpdate did: its item (or, at -1,
    // every third enemy one of g_EnemyItemDrops) with effect
    // deathEffect2 + 4, then powerItems power items (big ones at full
    // power) and pointItems point items around absPos; `byBomb` makes
    // them fly to the player (0x42bea0).
    void DropItems(int byBomb);
    // Frees and clears threads (0x42bc90).
    void FreeThreads();

    // The next enemy in its draw list (EnemyManager::drawListHeads), as
    // th07's.
    Enemy *nextInDrawList;
    // The enemies before and after it in a list (LeaveFollowerList).
    Enemy *prevFollower;
    Enemy *nextFollower;
    Sprite sprite;
    Sprite sprites[2];
    EnemyScriptState ctx;
    EnemyScriptState savedCtx[16];
    // The context the script runs in: ctx, or a thread's.
    EnemyScriptState *currentCtx;
    // Where calls save their caller's context: savedCtx, or a thread's
    // callStack while it runs.
    EnemyScriptState *callStack;
    // Variables of the enemy itself, which all its subroutines share:
    // ints 10008 to 10015 and floats 10024 to 10031. New in th08.
    int enemyInts[8];
    float enemyFloats[8];
    // stackDepth to go back to as ScriptReturn leaves a context set.
    short savedStackDepth;
    short stackDepth;
    short pad_2cec;
    short deathSub;
    // The subroutines of the interrupts opcode 125 (or another enemy's
    // 89) starts, set by opcode 126, as th07's.
    short interrupts[0x20];
    short pendingInterrupt;
    short pad_2d32;
    // Where it is.
    D3DXVECTOR3 pos;
    D3DXVECTOR3 posOrigin;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 prevPos;
    D3DXVECTOR3 posDelta;
    D3DXVECTOR3 hitboxSize;
    // A second box the player's shots hurt it through, as th07's.
    D3DXVECTOR3 extraHitboxSize;
    // Where the player's homing options go to (MoveOptionToEnemy).
    D3DXVECTOR3 absPos;
    float angle;
    float angularVelocity;
    float orbitAngle;
    float orbitAngularVelocity;
    // The enemy it shares its damage with (ShareDamageWithLeader).
    Enemy *leader;
    float speed;
    float acceleration;
    float orbitRadius;
    float orbitRadiusVelocity;
    D3DXVECTOR3 shootOffset;
    // A timed move (moveMode 2), as th07's: by moveDelta from moveStart
    // over moveDuration frames, eased by moveEasing.
    D3DXVECTOR3 moveDelta;
    D3DXVECTOR3 moveStart;
    Timer moveTimer;
    int moveDuration;
    float rankSpeedLow;
    float rankSpeedHigh;
    short rankCount1Low;
    short rankCount1High;
    short rankCount2Low;
    short rankCount2High;
    // Its life, as th07's. Cleared for the bosses as a spell card bomb
    // (Player::bomb.type 4) ends.
    int life;
    int maxLife;
    int phaseStartLife;
    int score;
    // Its index in EnemyManager::enemies.
    int index;
    // Player::damagePerTimeOrb in EnemyManager's template, which
    // Player::AddedCallback sets.
    int timeOrbDamage;
    Timer callbackTimer;
    D3DCOLOR color;
    BulletShooter shooter;
    // The fire instruction (header and eight arguments) the periodic
    // shots repeat every shootInterval frames.
    unsigned char shootInstr[0x2c];
    int shootInterval;
    Timer shootTimer;
    BulletShooter laserShooter;
    struct Laser *lasers[0x20];
    int laserIndex;
    int item;
    // Set by timeline instructions 11 and 12: how many point and power
    // items DropItems drops.
    int pointItems;
    int powerItems;
    // The effects the enemy's death spawns, set together by opcode 138
    // (th06's 100): deathEffect, and deathEffect2 + 4, which an item drop
    // also spawns. Nothing reads deathEffect3.
    char deathEffect;
    BYTE deathEffect2;
    BYTE deathEffect3;
    // Its index in EnemyManager::bosses while isBoss is set.
    BYTE bossIndex;
    BYTE hitFlashTimer;
    unsigned char pad_3315[0x3];
    Timer pad_3318;
    // Bit 3: the petal effects drift towards it while it is the first
    // boss. Bit 30 is cleared with life.
    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD isBoss : 1;
            DWORD shootable : 1;
            DWORD contactDamage : 1;
            // Hides the enemy, as th06's: it isn't drawn, touched or shot,
            // and the effects following it are hidden too.
            DWORD hidden : 1;
            DWORD hiddenByBomb : 1;
            // Cleared for the bosses as a Dissolve Spell starts.
            DWORD hitByShots : 1;
            DWORD grazeable : 1;
            DWORD followsYoukaiState : 1;
            // ReleaseFollowers moves posOrigin to its leader's pos while set.
            DWORD posRelativeToLeader : 1;
            // Set by ReleaseFollowers as it leaves its leader's list.
            DWORD releasedFromLeader : 1;
            // Player::isYoukai as UpdateYoukaiState last saw it.
            DWORD isYoukai : 1;
            DWORD moveMode : 2;
            DWORD moveEasing : 3;
            DWORD holdFire : 1;
            DWORD mirrored : 1;
            DWORD clampPos : 1;
            DWORD deathMode : 3;
            DWORD unk_3324_23 : 1;
            DWORD onScreen : 1;
            DWORD rotateSprite : 1;
            // Calls save no context while set, as th07's.
            DWORD noCallStack : 1;
            // A timer callback leaves the spell card running while set,
            // as th07's.
            DWORD keepSpellCard : 1;
            // OnUpdate keeps it after it leaves the play area while set,
            // as th07's.
            DWORD staysOffscreen : 1;
            // OnUpdate neither moves nor clamps it while set, as th07's.
            DWORD isWarpingToBoss : 1;
            // Its script and callbacks wait while a bomb runs or the
            // player is not in state 0, as th07's.
            DWORD haltedByBomb : 1;
            // The player's shots miss it while a bomb runs.
            DWORD intangibleDuringBomb : 1;
        };
    };
    union
    {
        DWORD flags2;
        struct
        {
            DWORD pad_3328_0 : 1;
            // UpdateYoukaiState bursts effect 0x26 every other frame while set.
            DWORD emitsEffect26 : 1;
            // Its sprites run their scripts from the stage's enemy ANM
            // (EnemyManager::stageAnm) while set, else from enemy.anm.
            DWORD useStageAnm : 1;
            // Set once OnUpdate has run its death; cleared while its life
            // is above 0.
            DWORD hasDied : 1;
            // How close it is to its next life callback, 0 to 3
            // (CheckLifeCallback).
            DWORD lifeCallbackProximity : 2;
            // ClearEnemiesForBonus spares it while set.
            DWORD undying : 1;
            // Like haltedByBomb, but always.
            DWORD halted : 1;
            // DrawEnemies puts sprites[] at sprite's posOffset instead of
            // their own while set.
            DWORD useMainSpriteOffset : 1;
            DWORD unusedBits3328_9 : 23;
        };
    };
    unsigned char pad_332c[0x2];
    // The turning sprite script last started, as th07's: 0 still, 1
    // left, 2 right, 0xff none yet.
    BYTE spriteDirection;
    // The draw list OnUpdate puts it in, as th07's: 0 or 2 by
    // UpdateYoukaiState as Player::isYoukai changes.
    BYTE drawListIndex;
    // 0x40 while Player::isYoukai is set, else 0x20 (UpdateYoukaiState).
    BYTE difficultyMaskExtra;
    unsigned char pad_3331[0x1];
    // The sprite scripts for turning, as th07's, and a sixth one.
    short turnStill;
    short turnStillFromLeft;
    short turnStillFromRight;
    short turnLeft;
    short turnRight;
    short turnExtra;
    unsigned char pad_333e[0x2];
    // Where ClampPos keeps it, as th07's.
    float lowerBound[2];
    float upperBound[2];
    float minShootDistSq;
    // The damage it took this frame, as th07's.
    int lastDamage;
    int lifeCallbackLife[4];
    int lifeCallbackSub[4];
    int timerCallbackTime;
    int timerCallbackSub;
    // Shown by DrawBanners next to CountFollowers.
    int followerCount;
    // Freed by FreeThreads.
    EnemyScriptThread *threads[4];
    EnemyBox trailBoxes[0x60];
    VertexTex1DiffuseXyzrwh spriteStripVertices[0xc2];
    // Whether it has a trail, and how long it is, as th07's.
    BYTE trailFlags;
    unsigned char pad_534d[0x1];
    short trailSegmentCount;
    // How many trail boxes touch the player, every sixth, as th07's.
    short trailContactSegments;
    // DrawEnemies draws every trailSegmentStep-th trail box, as th07's.
    short trailSegmentStep;
    Timer invulnTimer;
    // Effects that follow it, as th07's.
    struct Effect *effects[0x18];
    int effectCount;
    // How far the effects circle it, as th07's.
    float effectRadius;
    // Sent interrupt 3 as it despawns.
    struct Effect *attachedEffect;
    // The seconds its timer callback had left when CheckLifeCallback
    // last ran a life callback.
    int phaseEndSecondsLeft;
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
    int pointItems;
    int powerItems;
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
float LerpAngleShort(float a, float b, float t);

#include "enemyscript.h"

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
    // red while bit 10 of g_GameManager.stateFlags is set (0x42e120,
    // 0x42eb90).
    static int OnDraw(EnemyManager *mgr);
    static int OnDraw2(EnemyManager *mgr);
    // Outside dialogue, raises the rank by 1 every 2400 frames, 240
    // fewer per life left (0x42c3b0).
    void AddRankOverTime();
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
    // Count and pick the items the enemies drop, as th07's.
    WORD dropCounter;
    WORD dropIndex;
    // The active enemies, as th07's.
    int enemyCount;
    unsigned char pad_9dcdc8[0x4];
    // Set by opcode 163, as th07's scriptArg147.
    int scriptArg163;
    EnemyTimeline timelines[16];
    // Counts the frames, as th07's.
    Timer frameTimer;
    // The enemies OnUpdate puts in each draw list, as th07's.
    Enemy *drawListHeads[4];
    // enemy.anm (slot 7) and the stage's enemy ANM (slot 8).
    AnmLoaded *anm;
    AnmLoaded *stageAnm;
    int unusedInt9dcef4;
    // Whether the last Spawn found no free enemy.
    int spawnFailed;
    // -1 after Initialize; timeline instructions 13 and 14 use them.
    int timelineSignals[4];
    // While set, the timelines spawn nothing.
    int holdTimelineSpawns;
};
ASSERT_SIZE(EnemyManager, 0x9dcf10);
extern EnemyManager g_EnemyManager;
// What the enemies with item -1 drop, in turn, as th07's.
extern BYTE g_EnemyItemDrops[0x20];
extern ChainElem g_EnemyManagerUpdate;
extern ChainElem g_EnemyDrawElem;
extern ChainElem g_EnemyDrawElem2;
