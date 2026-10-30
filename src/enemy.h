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
    BYTE unusedByte8;
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
    short unusedShort226;
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
    short unusedShort4;
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
    // Whether unk_2da4 is clear and unk_8 is set. Inline; the copy at
    // 0x41f000 is in an /Od object.
    BOOL Func41f000()
    {
        return this->leader == 0 && this->nextFollower != NULL ? TRUE : FALSE;
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
            while (enemy->nextFollower != NULL)
            {
                enemy = enemy->nextFollower;
                count++;
            }
        }
        return count;
    }

    // The last enemy following it through unk_8, when Func41f000, else
    // itself. Inline; the copy at 0x41efc0 is in an /Od object.
    Enemy *Func41efc0()
    {
        Enemy *enemy;

        enemy = this;
        if (this->Func41f000())
        {
            while (enemy->nextFollower != NULL)
            {
                enemy = enemy->nextFollower;
            }
        }
        return enemy;
    }

    // Whether unk_2da4 is set. Inline; the copy at 0x41fd20 is in an
    // /Od object.
    BOOL Func41fd20()
    {
        return this->leader != 0;
    }

    // Breaks up the list of enemies following it through unk_8 (and
    // takes it out of its own leader's): with `collect`, each counts
    // as a kill shown by AsciiText::Func403460, bursts time orbs (more
    // for solo characters) and a hit area, drops items unless a boss
    // fight runs outside a spell card, and plays sound 2 or 3; then
    // it shows unk_3380 and bursts twice as many orbs. A follower with
    // `collect` also lowers stats->unk_22 by a twelfth and drops an orb.
    // Called with 0 on the bosses as a spell card bomb ends (0x42adb0).
    void Func42adb0(int collect);
    // Ends it, as th07's: Func42adb0(0) first; unless deathMode is set,
    // deactivates it; a boss leaves its slot, hides its nearby label and
    // ends the boss fight; drops its effects, sends unk_53c8 interrupt
    // 3, clears its callbacks and frees threads (0x42bcf0).
    void Despawn();
    // Takes it out of the list through unk_4 and unk_8 while Func41fd20,
    // clearing unk_2da4; else just clears both links (0x42b2f0).
    void Func42b2f0();
    // Follows Player::unk_5 (the human/youkai side?): as it changes,
    // bursts effect 0x1f or 0x1e, sends unk_53c8 interrupt 2 or 1 and
    // plays sound 0x28 or 0x27 (0x42c420).
    void Func42c420();
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
    // Runs the first life callback whose life the enemy dropped below,
    // as th07's: restarts its script there, ends its timer callback,
    // frees threads, Func42adb0(1), kills the other non-boss enemies and
    // makes the player invulnerable for 70 frames after a boss's.
    // Returns 1 then, else 0 after setting unk_3328_4 by how close the
    // life is to the next callback (or to 0) (0x42b490).
    int CheckLifeCallback();
    // Runs the timer callback once callbackTimer reaches
    // timerCallbackTime, as th07's: shows boss 0's time left; then drops
    // the life to the highest life callback, restarts the script at
    // timerCallbackSub, ends the spell card (unless keepSpellCard),
    // makes the player invulnerable for 70 frames after a boss's,
    // Func42adb0(0), kills the other non-boss enemies and frees threads.
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
    // Passes half of `damage` on to unk_2da4 while Func41fd20 (a ninth
    // while unk_2da4, a boss, is invulnerable; none for others then),
    // keeping its life above its life callbacks (0x42b370).
    void Func42b370(int damage);
    // Drops its items, as th07's OnUpdate did: its item (or, at -1,
    // every third enemy one of g_EnemyItemDrops) with effect
    // deathEffectIndex + 4, then unk_330c power items (big ones at full
    // power) and unk_3308 point items around unk_2d88; `byBomb` makes
    // them fly to the player (0x42bea0).
    void DropItems(int byBomb);
    // Frees and clears threads (0x42bc90).
    void Func42bc90();

    // The next enemy in its draw list (EnemyManager::drawListHeads), as
    // th07's.
    Enemy *nextInDrawList;
    // The enemies before and after it in a list (Func42b2f0).
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
    short unusedShort2cec;
    short deathSub;
    // The subroutines of the interrupts opcode 125 (or another enemy's
    // 89) starts, set by opcode 126, as th07's.
    short interrupts[0x20];
    short pendingInterrupt;
    short unusedShort2d32;
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
    // The enemy it shares its damage with (Func42b370).
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
    // g_Unknown18b8a24 in EnemyManager's template; Player::AddedCallback
    // sets the template's from Player::unk_e2b2c.
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
    char deathEffect;
    BYTE deathEffectIndex;
    BYTE deathEffectVariant;
    // Its index in EnemyManager::bosses while isBoss is set.
    BYTE bossIndex;
    BYTE hitFlashTimer;
    unsigned char pad_3315[0x3];
    Timer unusedTimer3318;
    // Bit 3: the petal effects drift towards it while it is the first
    // boss. Bit 30 is cleared with unk_2dfc.
    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD isBoss : 1;
            DWORD shootable : 1;
            DWORD contactDamage : 1;
            DWORD hideFollowEffects : 1;
            DWORD hiddenByBomb : 1;
            // Cleared for the bosses as a Dissolve Spell starts.
            DWORD hitByShots : 1;
            DWORD grazeable : 1;
            DWORD followsYoukaiState : 1;
            // Func42adb0 moves unk_2d40 to its leader's pos while set.
            DWORD posRelativeToLeader : 1;
            // Set by Func42adb0 as it leaves its leader's list.
            DWORD releasedFromLeader : 1;
            // Player::unk_5 as Func42c420 last saw it.
            DWORD isYoukai : 1;
            DWORD moveMode : 2;
            DWORD moveEasing : 3;
            DWORD holdFire : 1;
            DWORD mirrored : 1;
            DWORD clampPos : 1;
            DWORD deathMode : 3;
            DWORD unusedFlag3324_23 : 1;
            DWORD onScreen : 1;
            DWORD rotateSprite : 1;
            // Calls save no context while set, as th07's.
            DWORD disableCallStack : 1;
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
        DWORD unusedDword3328;
        struct
        {
            DWORD unusedFlag3328_0 : 1;
            // Func42c420 bursts effect 0x26 every other frame while set.
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
    // Func42c420 as Player::unk_5 changes.
    BYTE drawListIndex;
    // 0x40 while Player::unk_5 is set, else 0x20 (Func42c420).
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
    // Shown by DrawBanners next to Func41fd40.
    int followerCount;
    // Freed by Func42bc90.
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

// The head of an enemy script file, as th07's after a version (0x800):
// the subroutine count, the timeline count, the offsets of sixteen
// timelines, then those of the subroutines.
struct EnemyScriptHeader
{
    int version;
    short subCount;
    short timelineCount;
    void *timelines[16];
};

// The enemy scripts (ECL), at 0x4eccb8, as th07's. Only what is known
// so far.
class EnemyScripts
{
public:
    // Starts subroutine `sub` in `ctx`, as th07's (0x418450). Returns 0.
    int StartSub(EnemyScriptState *ctx, short sub);
    // Runs `enemy`'s script for a frame, as th07's (0x4184b0): its main
    // context, then each of its threads in turn, each followed by the
    // current context's function and interpolation loops while alive;
    // then the movement and the periodic shots. Returns -1 once the
    // script ends, else 0.
    int RunEnemy(Enemy *enemy);
    // Frees the script file, as th07's (0x418420).
    void Release();

    // Loads the script file at `path` and turns its offsets into
    // pointers, as th07's (0x418330). Returns 0, or -1 if loading fails.
    int Load(char *path);

    // How many timelines the file has. Inline; the copy is at 0x42dfb0.
    int GetTimelineCount()
    {
        return this->file->timelineCount;
    }
    // Timeline `i` of the file, as th07's. Inline; the copy is at
    // 0x42dfd0.
    EnemyTimelineEntry *GetTimeline(int i)
    {
        return (EnemyTimelineEntry *)this->file->timelines[i];
    }

    EnemyScriptHeader *file;
    void **subs;
    unsigned char unused8[0x160];
    // The arguments of the next subroutine call, as th07's shared
    // variables: ints 10061 to 10064 and floats 10065 to 10068.
    // ScriptCallSub copies them into the called context.
    EnemyScriptCallArgs callArgs;
};
ASSERT_SIZE(EnemyScripts, 0x188);
extern EnemyScripts g_EnemyScripts;
// The value of script operand `id` for `enemy`, as th07's: a variable
// (floats truncated) or another value the scripts read by number from
// 10000 to 10100; any other number is its own value (0x41f420).
int GetIntOperand(Enemy *enemy, int id);
// The int variable script operand `operand` names, as th07's, or
// `operand` itself if it names none or if `bit` is not negative and
// clear in `mask` (0x41fe10).
int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit);
// GetIntOperand for float operands, as th07's, with a random angle for
// 10082 (0x420120).
float GetFloatOperand(Enemy *enemy, float id);
// ResolveOperand for float variables, as th07's; 10042 to 10044 name
// pos, which GetFloatOperand reads from unk_2d88 (0x420950).
float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);
// Starts a timed move at an angle (argument 2, normalized), as th07's:
// moveDelta is argument 3 times argument 0, from unk_2d88 (0x420d10).
void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr);
// Starts a timed move from unk_2d88 to (argument 2, argument 3), as
// th07's, with moveStart at pos (0x420f40).
void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr);
// The interpolations of an instruction's loop, as th07's: writes to
// the variable argument 8 names, `t` of the way from argument 4 to
// argument 5, linearly (0x421120) or along a Hermite curve with the
// tangents of arguments 6 and 7 (0x421180).
void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);
void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);
// Start and end a spell card, as th07's, now through SpellCard::Start
// (card number in the high half of argument 0, the name from argument
// 2, three more texts) and SpellCard::End (0x421280, 0x4212e0).
void StartSpellCard(Enemy *enemy, EnemyScriptInstr *instr);
void EndSpellCard(Enemy *enemy, EnemyScriptInstr *instr);
// Sets the float variable argument 0 names to argument 3 of the way
// from argument 2 to argument 1. New in th08 (0x421300).
void ScriptLerp(Enemy *enemy, EnemyScriptInstr *instr);
// Starts an interpolation loop on the variable argument 0 names, in
// the first free loop of the current context or the one already on it,
// as th07's instruction 27 (0x4213f0).
void ScriptStartInterp(Enemy *enemy, EnemyScriptInstr *instr);
// Instructions 40 to 51 (placeholder name): compares arguments 0 and 1,
// as ints or floats, by ==, !=, <, <=, > or >=; if that holds, sets the
// current context's time to argument 2 and returns the instruction
// argument 3 bytes on, else NULL (0x4215f0).
EnemyScriptInstr *ScriptJumpIf(Enemy *enemy, EnemyScriptInstr *instr);
// Calls subroutine `sub` from `instr`, as th07's call instruction:
// saves the current context on callStack (unless disableCallStack),
// starts `sub` in it with callArgs (0x421bd0).
void ScriptCallSub(Enemy *enemy, EnemyScriptInstr *instr, int sub);
// Returns from a subroutine call, as th07's return instruction: takes
// the caller's context back from callStack and returns 0; past the
// bottom of the stack, frees thread timedCallActive - 1,
// goes back to ctx, savedCtx and savedStackDepth and returns 1. Prints
// an error under disableCallStack (0x421cb0).
int ScriptReturn(Enemy *enemy, EnemyScriptInstr *instr);
// Sets the turning sprite scripts, as th07's instruction 96, and
// clears spriteDirection (0x421de0).
void ScriptSetTurnSprites(Enemy *enemy, EnemyScriptInstr *instr, int still, int left, int right,
                          int stillFromLeft, int stillFromRight, int unk);
// Starts script argument 1 on sprites[argument 0] (from stageAnm with
// useStageAnm, else anm), or stops it for a negative script (0x421e50).
void ScriptSetSubSprite(Enemy *enemy, EnemyScriptInstr *instr);
// Moves in a random direction away from the player's side, turned back
// near the bounds, as th07's instruction 52: at speed argument 2, or
// over argument 0 frames through ScriptMoveAngleTimed (0x422020).
void ScriptMoveRandom(Enemy *enemy, EnemyScriptInstr *instr);
// ScriptMoveAngle at `angle` with speed argument 2, without mirroring
// (0x4222b0).
void ScriptMoveAngleTimed(Enemy *enemy, EnemyScriptInstr *instr, float angle);
// Like ScriptMoveRandom, but three times in four towards the side with
// more room within 384 pixels of the player, else in any direction;
// turned back only near the top and bottom bounds (0x4224a0).
void ScriptMoveRandom2(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 0: sets g_Unknown4e3d28 and g_Unknown4e3d24 from
// the first int and float variables (0x423390).
void ScriptSetUnknown4e3d28(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 1: a white flash of 60 frames, as th07's FlashScreen (0x4233d0).
void FlashScreen60(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 2: bounces the enemy off the sides and the top,
// falling with gravity floats[6] down to speed floats[7]; staysOffscreen
// ends below y 480 (0x423400).
void BounceOffSides(Enemy *enemy, EnemyScriptInstr *instr);

// Draws reserved effects 9 and 10 as the stage's unk_625c callback
// (0x4235a0).
void DrawTwinEffects();

// Script function 3: starts effect 0x38 in reserved slots 9 and 10
// at the enemy, slot 10's sprite on etama script 0x61, and has the
// stage draw them through DrawTwinEffects (0x423530).
void SpawnTwinEffects38(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 20: starts effect 0x41 in reserved slots 9 and 10
// at the enemy, slot 10's sprite on etama script 0x63, and has the
// stage draw them through DrawTwinEffects (0x423db0).
void SpawnTwinEffects41(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 5: ends reserved effects 9 and 10 and sets the
// stage's dialogueSpriteCount to 2 (0x424130).
void EndTwinEffects(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 6: starts effect 0x3a in reserved slots 9 and 10
// at the enemy, slot 10's sprite on etama script 0x65, and has the
// stage draw them through DrawTwinEffects (0x424170).
void SpawnTwinEffects3A(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 10: a dim white flash of 30 frames and a shake that
// rises over 120 frames, holds for 190 and falls over 60 (0x4246e0).
void FlashAndShakeScreen(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 30: sets g_Unknown18b8a2c to argument 1 (0x424a00).
void SetUnknown18b8a2c(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 13: tints the stage 0xffc03030 (0x424e00).
void TintStageRed(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 15: a shake of strength 16 that rises, holds and
// falls for 20 frames each (0x424e20).
void ShakeScreenRamp(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 17: a white flash of 180 frames (0x424f60).
void FlashScreen180(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 18: runs the game at 1 / argument 1 speed, like
// StartSlowdown without the sprites and bullets (0x424f90).
void SetGameSpeed(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 19: sets the first int variable to the practised
// spell card (GameManager::unk_3dbb0) (0x424fc0).
void GetPracticeCard(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 22: starts 「リザレクション」 through
// SpellCard::Func415d10 (0x424ff0).
void StartResurrection(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 23: SpellCard::Func416160 (0x425020).
void SpellCardFunc416160(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 24: sets the first int variable to the spell cards
// captured this game (0x425040).
void GetSpellCardsCaptured(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 26: sets GameManager::isFrozen to argument 1 and
// sends the stage's first two dialogue sprites interrupt 2 (frozen) or 1 (0x425070).
void SetFrozen(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 31: drops item 3 at the enemy while a bomb runs,
// else item 5 (0x425390).
void DropBombOrLifeItem(Enemy *enemy, EnemyScriptInstr *instr);

// Spawns subroutine argument 0 at (argument 1, argument 2) with life,
// item and score arguments 3 to 5 and the current context's variables,
// as th07's spawn instructions; unless the enemy is dead or unk_3324_10
// is set, which sets spawnFailed. Returns the enemy, or the spare one (0x41f110).
Enemy *ScriptSpawnEnemy(Enemy *enemy, EnemyScriptInstr *instr);

// ScriptSpawnEnemy relative to the enemy (unk_2d88) (0x41f280).
Enemy *ScriptSpawnEnemyRel(Enemy *enemy, EnemyScriptInstr *instr);

// The fire instructions (96 on), as th07's 64 to 72: sets the shooter
// up from the arguments, with the rank outside spell cards, and fires;
// unless flag 0x8000 or 0x10000 of argument 7 rules out the enemy's side
// (unk_3324_11) or the player is within sqrt(unk_3350) (0x422720).
void ScriptFireBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Moves the enemy by its move mode, as th07's RunEnemy did after the
// instructions: 1 by angle and speed, 2 a timed eased move by moveDelta,
// 3 around moveStart at orbitRadius (0x422c40).
void ScriptUpdateMovement(Enemy *enemy);

// The periodic shots and the turning sprite scripts, as th07's
// RunEnemy did after the movement: repeats shootInstr every
// shootInterval frames and turns the sprite by velocity.x (0x423150).
void ScriptUpdateShooting(Enemy *enemy);

// Script function 28: slows the game to 1 / argument 1 speed, as th07's:
// sends the stage's first two dialogue sprites interrupt 2, scales the
// bullets' velocities and shows sprites 0x60 to 0x6f as 0x6f (0x4251b0).
void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 29: ends StartSlowdown, as th07's; sets App
// stateFlags bit 5 if argument 1 is above 1 (0x425290).
void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 16: for each bullet with activeFlags bit 20, the
// enemy's followers within 64 pixels of it with ints2[2] at 0 get 60
// there and the enemy's ints[7] (0x424e50).
void MarkFollowersNearBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 27: spawns subroutine ints2[2] (life 800, item -2,
// score 10, floats[0] at its angle) at each bullet with activeFlags bit
// 20, which it clears (0x4250d0).
void SpawnEnemiesAtMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 9: a beam 590 long and 160 wide from unk_2d88 minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 128-wide box grazes (0x424730).
void BeamHitsPlayer160(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 11: a beam 590 long and 240 wide from pos minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 192-wide box grazes (0x424820).
void BeamHitsPlayer240(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 25: a beam 590 long and 288 wide from unk_2d88 minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 224-wide box grazes (0x424910).
void BeamHitsPlayer288(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 12: toggles the bullets with any activeFlags bit of
// ints[0]: autoRotate 1 ones turn additive (sprite + 0x10) and move at
// floats[0] and floats[1], the others back; then halts the followers
// (unk_3328_7) unless ints[1], with the stage's dialogue sprites
// interrupt 2, else releases them with 1 (0x424a20).
void ToggleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 14: like ToggleMarkedBullets' bullet loop in three
// steps: autoRotate 1 turn additive at their own angle and floats[1],
// 0 fade in over 15 frames as 2, the others go back (0x424c40).
void CycleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 8: numbers the followers of unk_2da4 with the same
// ints2[2] (into their ints2[1]), counts them into ints[6] (ints[5] set
// when the count changed) and, for all but the leader, eases orbitAngle
// towards an even share of the circle from the leader's (0x4244f0).
void KeepFormation(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 4: bounces the bullets back where they cross
// between three zones (an inner and an outer box), scaling them about
// (192.0, 208.0) across the gap (0x423a60).
void BounceBulletsAtZones(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 21: bounces the bullets back where they cross
// between three zones (an inner and an outer box), scaling them about
// (192.0, 208.0) across the gap (0x423e20).
void BounceBulletsAtZones2(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 7: bounces the bullets back where they cross
// between three zones (an inner and an outer box), scaling them about
// (192.0, 224.0) across the gap (0x4241e0).
void BounceBulletsAtZones3(Enemy *enemy, EnemyScriptInstr *instr);

// The enemy ANMs by stage, then by last spell (card - 0xc4).
extern char *g_EnemyAnmPaths[];
// The enemy scripts by stage, by stage in spell practice (stage + 9),
// then by last spell (card - 0xba).
extern char *g_EclPaths[];

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
// Cleared when this enemy despawns.
extern Enemy *g_Unknown18b89b4;
// Where the player's homing shots go, as th07's Player::orbHomingTarget:
// the first boss, else the lowest enemy shot at; g_HomingTargetSet once
// a boss is shot at.
extern D3DXVECTOR3 g_OrbHomingTarget;
extern int g_HomingTargetSet;
extern ChainElem g_EnemyManagerCalcChain;
extern ChainElem g_EnemyManagerDrawChain;
extern ChainElem g_EnemyManagerDrawChain2;
