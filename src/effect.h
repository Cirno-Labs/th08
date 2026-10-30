// Short-lived sprite effects, as th07's.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// One effect, as th07's. Only the fields known so far.
struct Effect
{
    // Constructs the sprite, the vectors and the timer (0x4287e0).
    Effect();

    Sprite sprite;
    D3DXVECTOR3 pos;
    // Zero, or the velocity EffectManager::SpawnEffectWithVelocity is
    // given.
    D3DXVECTOR3 velocity;
    // Added to the position each frame by the moving types, and changed by
    // acceleration.
    D3DXVECTOR3 step;
    D3DXVECTOR3 acceleration;
    D3DXVECTOR3 truePos;
    // The point an effect circles or comes in to, and its axis or
    // direction.
    D3DXVECTOR3 origin;
    D3DXVECTOR3 axis;
    // Set to (0, -1, 0) by EffectInitRing, as axis to (0, 0, 1).
    D3DXVECTOR3 unusedVector2f8;
    D3DXQUATERNION rotation;
    float radius;
    float angle;
    unsigned char pad_31c[0x4];
    // How wide a ring effect is.
    float ringWidth;
    // How many segments a ring effect has.
    int segmentCount;
    // The reserved slot it was started in, by SpawnReservedEffect.
    int reservedSlot;
    // A ring's y radius (an ellipse) when waveCount is 0, else how far its
    // radius waves.
    float radiusY;
    // A ring ellipse's rotation, or where its wave starts.
    float ellipseAngle;
    // How many waves go round a ring.
    float waveCount;
    Timer timer;
    unsigned char pad_344[0x4];
    // From g_EffectInfos. Called every frame while set; the effect ends
    // when it returns anything but 1.
    int (*update)(Effect *effect);
    // Draws the effect instead of EffectManager's draw callbacks, when
    // set.
    int (*draw)(Effect *effect);
    // Whether it runs.
    char active;
    // The type it was started as.
    char type;
    char fadeOut;
    char fadeFrames;
    // Which of EffectManager's draw lists it goes in: 0 the first (the
    // fourth with useDrawList3, the fifth with sprite blend mode 1), 2 the
    // third, others the second.
    char drawList;
    char useDrawList3;
    // Whether a ring's vertices are to be worked out again.
    char verticesDirty;
    // Runs while bit 10 of GameManager::stateFlags stops the others.
    char alwaysUpdates;
    // Memory the effect allocated, freed when its slot is reused.
    VertexTex1DiffuseXyzrwh *vertices;
    // The next effect in its draw list.
    Effect *next;
};
ASSERT_SIZE(Effect, 0x360);

// What each type of effect starts with, as th07's.
struct EffectInfo
{
    int script;
    int (*update)(Effect *effect);
    // Called once the effect is started, if set; the effect ends at once
    // when it returns nonzero.
    int (*init)(Effect *effect);
};
extern EffectInfo g_EffectInfos[];

// An effect init, as th07's: a random step that slows to a stop over 19
// frames, both scaled by App::gameSpeed. Returns 0.
int EffectInitDrift(Effect *effect);

// An effect update: moves by step, which changes by acceleration, as
// th07's. Returns 1.
int EffectUpdateMove(Effect *effect);

// Like EffectInitDrift, faster, slowing to a stop over 20 frames, as
// th07's.
int EffectInitDriftFast(Effect *effect);

// An effect init: puts it in the third draw list, with no axis and no
// radius. Returns 0.
int EffectInitForeground(Effect *effect);

// An effect update, as th07's: circles origin at `radius` about `axis`,
// at `angle` (flattened, at z 0); once fadeOut is set, fades out and
// grows over 16 frames, then ends.
int EffectUpdateOrbit(Effect *effect);

// An effect init for petals that drift to the boss, like th07's
// EffectInitPetal20: starts truePos at g_Stage.camera.eye plus half of
// g_Stage.camera.at, scattered at random, with a random step on top of
// velocity and a little acceleration, both scaled by App::gameSpeed; puts
// it in the second draw list. Sets posOffset.x to -9999 (no boss yet)
// and clears the sprite fields EffectDrawBossPetal uses. Returns 0. New
// in th08; placeholder name.
int EffectInitBossPetal(Effect *effect);

// An effect update for the petals of EffectInitBossPetal, as
// EffectUpdatePetal; while the first boss has bit 3 of flags,
// posOffset follows it (a tenth of the way each frame). Its color2 is its
// color scaled by g_Stage.colorScaleSprite.color. New in th08; placeholder name.
int EffectUpdateBossPetal(Effect *effect);

// EffectInitBossPetal scattered twice as far up and down, with a y step
// of 0.4 instead of velocity.y.
int EffectInitBossPetal2(Effect *effect);

// EffectUpdateBossPetal without the color2, checking the first boss
// without EnemyManager::AnyBossActive.
int EffectUpdateBossPetal2(Effect *effect);

// An effect init, as th07's: sets origin to where it starts (at z 0) and
// a random direction. Returns 0.
int EffectInitConverge(Effect *effect);

// An effect update, as th07's: comes in from 256 along axis to origin
// over 60 frames, at z 0. Returns 1.
int EffectUpdateConverge(Effect *effect);

// An effect update, as th07's: follows the player while its script runs.
// Returns 1, or 0 once the script has ended.
int EffectUpdateFollowPlayer(Effect *effect);

// Like EffectUpdateConverge, over 240 frames and keeping origin's z, as
// th07's.
int EffectUpdateConvergeSlow(Effect *effect);

// Moves truePos of every effect of type 51 (EffectInitBossPetal's) among
// the first 0x200 by `offset`, as th07's.
void OffsetPetalEffects(D3DXVECTOR3 *offset);

// An effect update for falling petals, as th07's: moves truePos by step,
// which changes by acceleration, and draws the effect there, turning it
// by its x rotation. Ends it (returns 0) once it is out of the view along
// g_Stage.camera.dir from g_Stage.camera.eye, or at z 0 and above;
// otherwise returns 1.
int EffectUpdatePetal(Effect *effect);

// An effect init for falling petals, as th07's EffectInitPetal20 without
// its bonus sprite: starts truePos at g_Stage.camera.eye plus half of
// g_Stage.camera.at, scattered at random, with a random step on top of
// velocity and a little acceleration, both scaled by App::gameSpeed, and
// a random spin; puts it in the second draw list. Returns 0.
int EffectInitPetal(Effect *effect);

// An effect init like EffectInitConverge, as th07's: the direction is
// velocity.x as an angle, or random when that is -990 or less, and axis
// is 1 to 2.5 long. Returns 0.
int EffectInitDiverge(Effect *effect);

// An effect update, as th07's: goes out from origin along axis, easing
// out to 128 axis lengths at frame 90, at z 0. Returns 1.
int EffectUpdateDivergeEased(Effect *effect);

// Returns 1, so the effect only runs its script.
int EffectUpdateNone(Effect *effect);

// An effect init: moves the effect by 256 along velocity.x as an angle,
// turned to face that way. Returns 0. New in th08.
int EffectInitMoveOut(Effect *effect);

// An effect init for a ring around pos: allocates vertices for up to 0x80
// segments (-1 on failure), takes its angle, radius and width from
// velocity and draws it with EffectDrawRing, in 0x18 segments. Returns 0.
// New in th08; placeholder name.
int EffectInitRing(Effect *effect);

// An effect draw callback: works out the ring's vertices when they are
// dirty, a circle, an ellipse (radiusY, turned by ellipseAngle) or a wavy
// circle (waveCount waves of radiusY), and draws them as a strip. Returns
// 1. New in th08; placeholder name.
int EffectDrawRing(Effect *effect);

// EffectInitRing for the fourth draw list.
int EffectInitRing2(Effect *effect);

// Ring effect updates: take the ring's shape from the sprite its script
// runs: the width from scaleY, the radius from pos.x, the y radius or
// wave from pos.y, the angle from rotation.z, the ellipse rotation or
// wave start from rotation.y, the segment count from scriptVar0 and the
// waves from scriptVar1. Each returns 1 while the effect lasts. New in
// th08; placeholder names.
// Width and radius.
int EffectUpdateRing(Effect *effect);
// All of it.
int EffectUpdateRingFull(Effect *effect);
// 0x20 segments, width, radius and y radius; ends at frame 120.
int EffectUpdateRingTimed(Effect *effect);
// Width, radius, y radius and angle; ends once the sprite's alpha is 0.
int EffectUpdateRingFade(Effect *effect);
// All of it, the radius from scriptVarF1, centred on the sprite.
int EffectUpdateRingAt(Effect *effect);

// Ring effect updates of the bombs (bomb.cpp), placeholder names by
// effect type: verticesDirty every frame. 42: radius 256 easing out over
// 40 frames, 0x40 segments, 5 waves of growing height after frame 40, and
// growing width.
int EffectUpdateBombRing42(Effect *effect);
// 43: radius 256 easing out over 40 frames, 0x30 segments, an ellipse
// half as high turned by pi/4, growing width after frame 40.
int EffectUpdateBombRing43(Effect *effect);
// 44: 43 turned the other way (and keeping waveCount).
int EffectUpdateBombRing44(Effect *effect);
// 45: radius 192 easing out over 40 frames, 8 segments, 8 wide.
int EffectUpdateBombRing45(Effect *effect);
// 41: 0x30 segments, 32 wide, radius 64, 72 on odd frames.
int EffectUpdateBombRing41(Effect *effect);
// 46: grows by 8 a frame, 0xc segments, 32 wide.
int EffectUpdateBombRing46(Effect *effect);
// 36 and 37: inits that start effect 0x23 at the same position with the
// same velocity in the same reserved slot, then make this one a ring
// shrinking onto it (EffectUpdateBombRing36, 0x2c segments, 4 wide; or
// EffectUpdateBombRing37, 0x36 segments, 6 wide) that ends in a burst of
// player bullets. Return 0.
int EffectInitBombRing36(Effect *effect);
int EffectUpdateBombRing36(Effect *effect);
int EffectUpdateBombRing37(Effect *effect);
int EffectInitBombRing37(Effect *effect);
// 48: a flat ellipse of radius 192 turned by pi/2, 0x30 segments, 3
// wide, for 30 frames; then opening up and widening.
int EffectUpdateBombRing48(Effect *effect);

// The effects, at 0x4ece60: 0x200 started in turn, 0x80 started in
// order from 0x200, 0xd reserved slots and a spare.
class EffectManager
{
public:
    // Resets and sets colorScale to 1, as th07's (0x428740).
    EffectManager();
    // Zeroes everything, as th07's (0x425410).
    void Reset();
    // Starts `count` effects of `type` at `pos` in `color`, as th07's
    // (0x425430). Returns the last effect started, or the spare one if
    // there were not enough free slots.
    Effect *SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // SpawnEffect with a starting `velocity`, without SpawnEffect's
    // sprite flag 0x2000, as th07's (0x425650).
    Effect *SpawnEffectWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count, D3DCOLOR color);
    // Starts an effect of `type` at `pos` in `color` in reserved slot
    // `slot` (from 0x280 on), as th07's (0x425870); its script only if it
    // has one. `count` is unused. Returns it.
    Effect *SpawnReservedEffect(int type, D3DXVECTOR3 *pos, int slot, int count, D3DCOLOR color);
    // SpawnReservedEffect with a starting `velocity` (0x4259e0). New in
    // th08.
    Effect *SpawnReservedWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int slot, int count,
                                      D3DCOLOR color);
    // Starts `count` effects of `type` at `pos` in `color` in the first
    // free slots from 0x200 to 0x27f, as th07's SpawnEffect did, without
    // clearing the rest of the effect (0x425b70). Returns the last effect
    // started, or the spare one. New in th08; placeholder name.
    Effect *SpawnEffect2(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // The update chain element's callback, as th07's: runs the active
    // effects (all but the spare; while bit 10 of
    // GameManager::stateFlags is set only those with alwaysUpdates), frees the
    // vertices of inactive ones and sorts the rest into the draw lists,
    // all but type 0x40; every 300 frames checks the player's numbers.
    // Returns 4 if they were tampered with, otherwise 1.
    static int OnUpdate(EffectManager *mgr);
    // The draw chain element's callback, as th07's: draws the first, third
    // and fifth draw lists, the first and fifth moved into the play area
    // (or through their draw callbacks). Returns 1.
    static int OnDraw(EffectManager *mgr);
    // Draws the fourth draw list like OnDraw's first, a little nearer.
    // Returns 1. New in th08; placeholder name.
    int OnDraw3();
    // Draws the second draw list, unless App::effectsQuality is 0; with 1
    // it returns at the first effect, as th07's. Returns 1.
    int OnDraw2();
    // Resets, takes the etama ANM and loads the stage's effect ANM into
    // slot 9 (the card's for the last spell cards in spell practice),
    // unless the game is restarting, which keeps the loaded one. Returns
    // 0, or -1 on failure.
    static int AddedCallback(EffectManager *mgr);
    // Frees the effects' vertices and, unless the game is restarting, the
    // stage's effect ANM. Returns 0.
    static int DeletedCallback(EffectManager *mgr);
    // Resets and adds the update element (priority 13) and the draw
    // element (priority 12), as th07's (0x428620). Returns 0, or -1 if
    // adding fails.
    static int RegisterChain();
    // Removes the two chain elements, as th07's (0x4286b0).
    static void RemoveChain();
    // Reserved effect `slot` (0x280 + slot). Inline; the copy is at
    // 0x4253e0.
    Effect *GetReservedEffect(int slot)
    {
        return &this->effects[slot + 0x280];
    }

    // Where SpawnEffect looks for a free effect next.
    int next;
    unsigned char pad_4[0x4];
    // How many effects were active this frame.
    int activeCount;
    // Set to 1 by the constructor and never read, as th07's.
    float colorScale[4];
    Effect effects[0x28e];
    // The heads of the draw lists, and their last effects.
    Effect drawList0;
    Effect drawList1;
    Effect drawList2;
    Effect drawList3;
    Effect drawList4;
    Effect *drawListTails[5];
    // Counts OnUpdate's calls.
    int frames;
    // The ANM in slot 6 (etama.anm), which the effects' scripts are in.
    AnmLoaded *anm;
    // The stage's effect ANM, in slot 9.
    AnmLoaded *stageAnm;
};
ASSERT_SIZE(EffectManager, 0x8b05c);
extern EffectManager g_EffectManager;

// EffectManager's statically allocated chain elements.
extern ChainElem g_EffectCalcChain;
extern ChainElem g_EffectDrawElement;

// The stages' effect ANMs, then those of the last spell cards (from card
// 0xd8).
extern char *g_EffectAnmPaths[];

// The projection callback of the boss petals (types 51 and 63) for
// SpriteEngine::DrawProjectedWithCallback: drifts their drawn position
// (by moveEnd) towards posOffset, the boss, faster and faster
// (moveStart.x), and away from the player, unless the game is paused.
// New in th08; placeholder name.
void EffectDrawBossPetal(Sprite *vm, D3DXVECTOR3 *pos);
