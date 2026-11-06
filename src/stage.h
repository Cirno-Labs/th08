// The stage background: its geometry, scripts and camera, as th06's.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// A run of quads in the stage data, ended by one with a negative type,
// as th07's.
struct StageQuad
{
    short type;
    // The offset of the next quad.
    short size;
    short script;
    // Set by Load: the quad's Sprite in Stage::sprites.
    short spriteIndex;
    D3DXVECTOR3 pos;
    float width;
    float height;
};

// A quad of type 1: a strip from `start` to `end`, `width` wide (or
// its sprite's width). New in th08.
struct StageLineQuad
{
    short type;
    short size;
    short script;
    short spriteIndex;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    float width;
};

// An instruction of the stage script: at `frame`, `opcode` acts on
// `args`. As th07's, with more opcodes.
struct StageInstr
{
    int frame;
    short opcode;
    short pad_06;
    D3DXVECTOR3 args;
};

// D3DRS_FOGSTART, D3DRS_FOGEND and D3DRS_FOGCOLOR, as th07's.
struct StageFog
{
    float nearPlane;
    float farPlane;
    D3DCOLOR color;
};

// An object of the stage data: its quads start at +0x1c. As th07's.
struct StageObject
{
    BYTE pad_00[0x2];
    // The background layer it is drawn on.
    char layer;
    // Bit 0: its sprites still run scripts. Bit 1: DrawLayer drew it.
    char flags;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    StageQuad quads[1];
};

// A placement of an object; a negative `id` ends the list.
struct StageInstance
{
    short id;
    short pad_02;
    D3DXVECTOR3 pos;
};

// The start of the stage data file, as th07's.
struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD pad_0c;
    char name[0x80];
    char songNames[4][0x80];
    // The stage's MIDI files; a path starting with ' ' is unused.
    char songPaths[4][0x80];
    // Then, at 0x490, the objects' offsets.
};

// A camera of the stage script: where it is, where it looks (relative to
// `eye`) and its field of view, as th07's, with `offset`.
struct StageCamera
{
    D3DXVECTOR3 eye;
    D3DXVECTOR3 at;
    D3DXVECTOR3 up;
    // `at`, normalized.
    D3DXVECTOR3 dir;
    // ProjectSprite projects a sprite's position plus this vector to
    // get the size it draws the sprite at.
    D3DXVECTOR3 cameraRight;
    // Set by opcode 32 and the camera shakes. New in th08.
    D3DXVECTOR3 offset;
    float fov;
};

class Stage;
// Moves `out` from `from` to `to` over stage->cameraFadeFrames[i] frames, on
// stage->cameraFadeTimer[i], eased by stage->cameraFadeMode[i]: 1 to 3 slow
// down, 4 to 6 speed up, 7 follows a Hermite curve with tangents m0 and m1.
// Clears the frame count once the timer reaches it. As th07's.
void UpdateEasedVector(Stage *stage, int i, D3DXVECTOR3 *out, D3DXVECTOR3 *from, D3DXVECTOR3 *to, D3DXVECTOR3 *m0, D3DXVECTOR3 *m1);
// The cubic Hermite curve from p0 to p1, with tangents m0 and m1, at `t`
// (0 to 1), as th07's.
float HermiteInterpolate(float p0, float p1, float m0, float m1, float t);

struct Effect;

// The object at 0x4e4030: th07's Stage, the background.
class Stage
{
public:
    // Zeroes the object, then puts the camera at (0, 0, 1000) looking
    // nowhere, with up along y and a field of view of pi / 6, and copies
    // it to cameraFrom and cameraTo, as th07's.
    Stage();

    // Sets the view and projection for things drawn on the screen
    // rather than in the stage (0x40b5a0).
    void SetupFixedCamera();
    // Sets the stage's view and projection (0x40b6d0).
    void SetupCamera();
    // Removes the stage's chain elements, as th07's (0x409ca0).
    static void RemoveChain();
    // Zeroes the stage (keeping the data when restarting), remembers
    // `stage` and adds the update element (priority 8) and the draw
    // elements (priorities 6 and 7), as th07's (0x409b20). Returns 0, or
    // -1 if adding fails.
    static int RegisterChain(int stage);
    // Runs the stage script up to the current frame, as th07's, with
    // th08's opcodes: 32 sets camera.offset, 33 starts a camera shake
    // (shakeMode), 34 starts backgroundLayer2's script. In stage 7 it
    // first spawns stage7Effect and runs its script by waitLabel. Then fades
    // the cameras and the fog, shakes the camera, runs the sprites and
    // spawns effect 0x3e at the first 12 of effectSpawnPos every third frame
    // from frame 700 (or in spell practice). Returns 1, or 4 when
    // ChecksumMismatch finds the player's numbers tampered with
    // (checked every 500 frames).
    static int OnUpdate(Stage *stage);
    // Starts and ends the spell card background: spellCardState 1 from
    // frame 0, and 0. Inline; the copies at 0x415ce0 and 0x416ad0 are the
    // spell card object's.
    void StartSpellCard()
    {
        this->spellCardState = 1;
        this->spellCardFrames = 0;
    }
    void EndSpellCard()
    {
        this->spellCardState = 0;
    }
    // Releases the background ANM (slot 4) and frees the sprites, and
    // the data unless restarting (0x409c20). Returns 0.
    static int DeletedCallback(Stage *stage);
    // Sets tintColor to `color`, or to the average of the two while its
    // alpha byte is set, as th07's.
    void SetTintColor(D3DCOLOR color);
    // Clears effectSpawnPosCount and the first 0x10 of effectSpawnPos, sets
    // the play area viewport, resets the SpriteEngine's render state, clears
    // the play area to black once if clearScreenOnce is set, makes the
    // SpriteEngine modulate by tintColor if its alpha is set (and resets it
    // to gray), draws backgroundLayer0, backgroundLayer1 and stage7Effect,
    // clears to clearColor (through DrawColorRect unless it is opaque), sets
    // the fog (its color modulated as the SpriteEngine's) and draws layers 0
    // and 1 under the same conditions as OnDraw2. Returns 1. As th07's, with
    // th08's additions.
    static int OnDraw(Stage *stage);
    // Draws the objects on `layer` near enough the camera, as th07's
    // (0x40a1b0). Returns 0.
    // Draws layers 2 and 3 unless spellCardState is above 1 or the Gui's
    // loading sprite shows, with a rectangle fading in while
    // spellCardState is 1; then dialogueSprites's first
    // dialogueSpriteCount sprites and spellCardDrawCallback, the fixed camera
    // (SetupFixedCamera) and the fog at 1000 to 2000. Resets the
    // SpriteEngine's color unless keepColorModulation was set, and clears it
    // and recordEffectSpawnPos. Returns 1. As th07's.
    static int OnDraw2(Stage *stage);
    int DrawLayer(int layer);
    // Resets the script, loads the stage's background ANM into slot 4
    // (or takes the one already there when restarting) and its data
    // (the spell practice version in spell practice), and sets the fog
    // and the cameras up as the constructor, with draw distance
    // drawDistanceSq by stage. Returns 0, or -1 if loading fails. As th07's.
    static int AddedCallback(Stage *stage);
    // Loads the stage data from `dataPath` (keeping the old data when
    // restarting) and starts the background sprites' scripts, as
    // th07's, then starts colorScaleSprite (0x409ce0). Returns 0, or -1 on
    // failure.
    int Load(char *dataPath);
    // Sends colorScaleSprite interrupt 1 or 2 as g_Player.isYoukai is set or
    // cleared, runs it, then runs one frame of the scripts of each
    // animated object's sprites, as th07's; objects whose last sprite has
    // autoRotate 1 get colorScaleSprite's color as color2 (0x409f40). Returns 0.
    int StepObjects();

    // One Sprite per object sprite in the data.
    Sprite *sprites;
    Sprite backgroundLayer0;
    Sprite backgroundLayer1;
    // Started by opcode 34; its color becomes clearColor while it runs.
    // New in th08.
    Sprite backgroundLayer2;
    // The stage's background ANM (slot 4). New in th08.
    AnmLoaded *anm;
    StageData *data;
    int quadCount;
    int objectCount;
    StageObject **objects;
    StageInstance *instances;
    StageInstr *script;
    Timer scriptTime;
    int scriptIndex;
    int checksumCheckFrames;
    // RegisterChain's argument.
    int stage;
    D3DXVECTOR3 cameraPos;
    D3DCOLOR clearColor;
    // Follows g_Player.isYoukai: set, colorScaleSprite gets interrupt 1,
    // cleared, interrupt 2. New in th08.
    BYTE colorScaleOn;
    unsigned char pad_835[0x3];
    // Restarted whenever colorScaleOn changes. New in th08.
    Timer colorScaleTimer;
    // Script 0x21 of g_App.textAnm. Its color scales the color of the
    // petals and of some of the stage's sprites. New in th08.
    Sprite colorScaleSprite;
    // Stage 7's effect. New in th08.
    Effect *stage7Effect;
    StageFog fog;
    StageFog fogStart;
    StageFog fogTarget;
    int fogFadeFrames;
    Timer fogFadeTime;
    // Set by SetDefaultRenderStates, as th07's.
    BYTE fogColorChanged;
    unsigned char pad_b21[0x3];
    int spellCardState;
    int spellCardFrames;
    int clearScreenOnce;
    // Set to 2 by EffectManager::AddedCallback, which clears
    // dialogueScriptOffset.
    int dialogueSpriteCount;
    // Set to 2 by GameManager::LoadingThread unless a stage 6 or Extra
    // spell card is practised, and by the Gui.
    int dialogueScriptOffset;
    Sprite dialogueSprites[0x20];
    Sprite pad_5fb8;
    // Called by OnDraw2 after the dialogue sprites. New in th08.
    void (*spellCardDrawCallback)();
    int waitLabel;
    StageCamera cameraFrom;
    StageCamera cameraTo;
    StageCamera cameraFadeTarget;
    StageCamera cameraFadeStart;
    StageCamera camera;
    int cameraFadeFrames[5];
    Timer cameraFadeTimer[5];
    int cameraFadeMode[5];
    D3DXVECTOR3 nextKeyPos;
    int nextKeyFrame;
    D3DXVECTOR3 keyPos;
    int keyFrame;
    BYTE cameraOffsetChanged;
    unsigned char pad_6465[0x3];
    ColorBytes tintColor;
    int keepColorModulation;
    // Probably the squared distance DrawLayer draws objects within:
    // 1150 squared, 1350 squared for stage 5, 1800 squared for stages 6
    // and 7. New in th08.
    float drawDistanceSq;
    // The camera shake opcode 33 starts: 1 to 3. New in th08.
    BYTE shakeMode;
    unsigned char pad_6475[0x3];
    int effectSpawnPosCount;
    int recordEffectSpawnPos;
    D3DXVECTOR3 effectSpawnPos[0x20];
};
ASSERT_SIZE(Stage, 0x6600);
extern Stage g_StageManager;
// Stage's statically allocated chain elements.
extern ChainElem g_StageBgCalcElem;
extern ChainElem g_StageDrawElem;
extern ChainElem g_StageDrawElem2;
// The background ANM of each stage (4A and 4B share one).
extern char *g_StageAnmPaths[9];
// The stage data file of each stage.
extern char *g_StageDataFiles[9];
// The same in spell practice ("stage1_s.std", ...). New in th08.
extern char *g_SpellPracticeStageDataFiles[9];
