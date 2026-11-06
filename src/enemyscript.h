// The enemy scripts (ECL) file and its interpreter, as th06's.
// Included by enemy.h where these declarations were; it relies on what
// enemy.h declares before it.

#pragma once

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

// th06's ScriptStarPattern keeps its star here; in th08 nothing uses
// it, but it ends with eight vectors, whose (empty) constructors the
// constructor runs (0x418300).
struct EnemyScriptsStar
{
    EnemyScriptsStar();

    unsigned char pad_0[0x100];
    D3DXVECTOR3 ends[8];
};

// The enemy scripts (ECL), at 0x4eccb8, as th07's. Only what is known
// so far.
class EnemyScripts
{
public:
    // Constructs star (0x4182e0).
    EnemyScripts();

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
    EnemyScriptsStar star;
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
// pos, which GetFloatOperand reads from absPos (0x420950).
float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);
// Starts a timed move at an angle (argument 2, normalized), as th07's:
// moveDelta is argument 3 times argument 0, from absPos (0x420d10).
void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr);
// Starts a timed move from absPos to (argument 2, argument 3), as
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
// saves the current context on callStack (unless noCallStack),
// starts `sub` in it with callArgs (0x421bd0).
void ScriptCallSub(Enemy *enemy, EnemyScriptInstr *instr, int sub);
// Returns from a subroutine call, as th07's return instruction: takes
// the caller's context back from callStack and returns 0; past the
// bottom of the stack, frees thread timedCallActive - 1,
// goes back to ctx, savedCtx and savedStackDepth and returns 1. Prints
// an error under noCallStack (0x421cb0).
int ScriptReturn(Enemy *enemy, EnemyScriptInstr *instr);
// Sets the turning sprite scripts, as th07's instruction 96, and
// clears spriteDirection (0x421de0).
void ScriptSetTurnSprites(Enemy *enemy, EnemyScriptInstr *instr, int still, int left, int right,
                          int stillFromLeft, int stillFromRight, int extra);
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

// Script function 0: sets g_AsciiText.ringAlpha and g_AsciiText.ringRadius from
// the first int and float variables (0x423390).
void SetScriptFunc0Vars(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 1: a white flash of 60 frames, as th07's FlashScreen
// (0x4233d0).
void FlashScreen60(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 2: bounces the enemy off the sides and the top,
// falling with gravity floats[6] down to speed floats[7]; staysOffscreen
// ends below y 480 (0x423400).
void BounceOffSides(Enemy *enemy, EnemyScriptInstr *instr);

// Draws reserved effects 9 and 10 as the stage's spellCardDrawCallback
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

// Script function 30: sets g_ScreenEffectHoldFrames to argument 1 (0x424a00).
void SetScreenEffectHoldFrames(Enemy *enemy, EnemyScriptInstr *instr);

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
// spell card (GameManager::practiceSpellCard) (0x424fc0).
void GetPracticeCard(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 22: starts 「リザレクション」 through
// SpellCard::StartNameOnly (0x424ff0).
void StartResurrection(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 23: SpellCard::EndSpellCardName (0x425020).
void ScriptEndSpellCardName(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 24: sets the first int variable to the spell cards
// captured this game (0x425040).
void GetSpellCardsCaptured(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 26: sets GameManager::isFrozen to argument 1 and sends
// the stage's first two dialogue sprites interrupt 2 (frozen) or 1
// (0x425070).
void SetFrozen(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 31: drops item 3 at the enemy while a bomb runs,
// else item 5 (0x425390).
void DropBombOrLifeItem(Enemy *enemy, EnemyScriptInstr *instr);

// Spawns subroutine argument 0 at (argument 1, argument 2) with life, item and
// score arguments 3 to 5 and the current context's variables, as th07's spawn
// instructions; unless the enemy is dead or releasedFromLeader is set, which
// sets spawnFailed. Returns the enemy, or the spare one (0x41f110).
Enemy *ScriptSpawnEnemy(Enemy *enemy, EnemyScriptInstr *instr);

// ScriptSpawnEnemy relative to the enemy (absPos) (0x41f280).
Enemy *ScriptSpawnEnemyRel(Enemy *enemy, EnemyScriptInstr *instr);

// The fire instructions (96 on), as th07's 64 to 72: sets the shooter
// up from the arguments, with the rank outside spell cards, and fires;
// unless flag 0x8000 or 0x10000 of argument 7 rules out the enemy's side
// (isYoukai) or the player is within sqrt(minShootDistSq) (0x422720).
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

// Script function 9: a beam 590 long and 160 wide from absPos minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 128-wide box grazes (0x424730).
void BeamHitsPlayer160(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 11: a beam 590 long and 240 wide from pos minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 192-wide box grazes (0x424820).
void BeamHitsPlayer240(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 25: a beam 590 long and 288 wide from absPos minus
// floats[0] and floats[1], turned by the sprite's rotation, hits the
// player; every 12 frames a 224-wide box grazes (0x424910).
void BeamHitsPlayer288(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 12: toggles the bullets with any activeFlags bit of
// ints[0]: autoRotate 1 ones turn additive (sprite + 0x10) and move at
// floats[0] and floats[1], the others back; then halts the followers
// (halted) unless ints[1], with the stage's dialogue sprites
// interrupt 2, else releases them with 1 (0x424a20).
void ToggleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 14: like ToggleMarkedBullets' bullet loop in three
// steps: autoRotate 1 turn additive at their own angle and floats[1],
// 0 fade in over 15 frames as 2, the others go back (0x424c40).
void CycleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr);

// Script function 8: numbers the followers of leader with the same
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

// The enemy ANMs by stage. The code reads on into the last spells'
// table after it (card - 0xc4).
extern char *g_EnemyAnmPaths[9];
// The enemy scripts by stage. The code reads on into the tables after it:
// by stage in spell practice (stage + 9), then by last spell (card -
// 0xba).
extern char *g_EclPaths[9];
