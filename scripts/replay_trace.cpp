// th08's replay tracer: plays one replay in an unmodified game and
// writes a trace of its state, one record per calculation
// (tools/replay/trace.h has the format and the runtime it uses).
//
// tools/replay/replay.py adds this DLL to a copy of an executable's
// import table, so it loads before the game starts, and tells it where
// the game's code and data are (TRACE_ADDR, from symbols.csv or the
// build's map file); it uses the project's headers only for the layouts
// of the game's structures. DllMain:
//
// - replaces the update callback SetUpTitleScene gives the main menu
//   with one that starts the replay as the replay menu does, once the
//   title screen has waited a second for a key;
// - redirects the call to Chain::RunCalcChain in GameWindow::RunFrame,
//   to hold each stage's loading to a fixed number of passes, to stop
//   the game after it returned to the title, and to give up after a
//   limit;
// - replaces the callbacks ReplayManager::RegisterChain installs while
//   playing (PlayOnUpdate, PlayOnUpdate2, PlayAddedCallback and
//   DeletedCallback) with wrappers that call them and record the state;
// - installs the runtime clock for timeGetTime and the performance
//   counter, the null renderer and the crash handler.
//
// Besides the runtime's settings, from the environment:
//
//   TRACE_REPLAY  the replay's path, from the game's directory
//   TRACE_START   stage,difficulty,shot,spell card as the replay menu
//                 sets them (the card -1 outside spell card practice)
//   TRACE_EXE_SIZE, TRACE_EXE_CHECKSUM
//                 original identity for the periodic version-file check
//   TRACE_TAIL    calculations to run after the game is back on the
//                 title (default 60)
//   TRACE_LIMIT   passes of the main loop into the update chain after
//                 which it gives up (dialogue going fast makes several
//                 calculations a pass)
//
// A calc record is taken after PlayOnUpdate, in the middle of a
// calculation (update chain priority 6): its game state is how the
// calculation before ended, with this one's input. `calcs` is
// App::frames, which counts every pass of the update chain, and
// `fastForwards` counts PlayOnUpdate2 starting the chain over during
// dialogue.
//
// th08 loads each stage in a second thread (GameManager::LoadingThread),
// which also starts the replay's stage (RegisterChain,
// PlayAddedCallback), while the main loop goes on with
// GameManager::OnUpdate returning 3 and counting loadingFrames. For a
// replay the thread waits for 0x50 of those passes, but how many more
// there are depends on the thread's timing (the main loop also steps
// the ANM loading it waits for). So `passes` leaves out the passes that
// start in the game while the GameManager is loading (or about to, from
// the end of a stage on), unless they reach PlayOnUpdate
// (as the thread finished meanwhile), and `calcs` App::frames' counts of
// them. Stage records, which the loading thread writes, repeat the
// counts of the calculation before.

// cflags: -/GS
// No CRT: everything below runs on kernel32.

#include <windows.h>

#include "trace.h"

#include "bullet.h"
#include "enemy.h"
#include "item.h"
#include "mainmenu.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

extern "C" int _fltused = 0;

#define FIELDS(X) \
    X(kind) \
    X(seq) \
    X(calcs) \
    X(passes) \
    X(fastForwards) \
    X(replayFrame) \
    X(entry) \
    X(recordedInput) \
    X(clearFrame) \
    X(stage) \
    X(stageFrames) \
    X(flags) \
    X(pause) \
    X(scene) \
    X(score) \
    X(guiScore) \
    X(lives) \
    X(bombs) \
    X(power) \
    X(graze) \
    X(grazeTotal) \
    X(pointItemsTotal) \
    X(maxPointValue) \
    X(stageClearPoints) \
    X(deaths) \
    X(bombsUsed) \
    X(spellCardsCaptured) \
    X(continuesUsed) \
    X(gauge) \
    X(clock) \
    X(timeOrbs) \
    X(timeOrbsCollected) \
    X(timeOrbsNeeded) \
    X(gaugeFrames) \
    X(humanFrames) \
    X(rank) \
    X(subRank) \
    X(powerItemCombo) \
    X(progressFrames) \
    X(rngSeed) \
    X(rngGenerated) \
    X(buttons) \
    X(playerX) \
    X(playerY) \
    X(playerState) \
    X(playerBomb) \
    X(bulletCount) \
    X(enemyCount) \
    X(itemCount) \
    X(spellCard) \
    X(spellCardNumber) \
    X(spellCardBonus) \
    X(timeOrbBonus) \
    X(bulletHash) \
    X(laserHash) \
    X(enemyHash) \
    X(itemHash) \
    X(shotHash)

#define FIELD_MEMBER(name) DWORD name;
#define FIELD_NAME(name) #name "\n"

struct Record
{
    FIELDS(FIELD_MEMBER)
};

static const char s_fieldNames[] = FIELDS(FIELD_NAME);

// What replay.py passes in TRACE_ADDR, by name.
#define ADDRS(X) \
    X(RunFrame) \
    X(RunFrameSize) \
    X(RunCalcChain) \
    X(SetUpTitleScene) \
    X(SetUpTitleSceneSize) \
    X(MenuOnUpdate) \
    X(RegisterChain) \
    X(RegisterChainSize) \
    X(PlayOnUpdate) \
    X(PlayOnUpdate2) \
    X(PlayAddedCallback) \
    X(DeletedCallback) \
    X(GameManager) \
    X(App) \
    X(Random) \
    X(Buttons) \
    X(PlayButtons) \
    X(Player) \
    X(BulletManager) \
    X(EnemyManager) \
    X(ItemManager) \
    X(SpellCard)

#define ADDR_ENUM(name) A_##name,
#define ADDR_NAME(name) #name,

enum
{
    ADDRS(ADDR_ENUM) NUM_ADDRS
};

static const char *s_addrNames[] = {ADDRS(ADDR_NAME)};
static DWORD s_addrs[NUM_ADDRS];

#define GM ((GameManager *)s_addrs[A_GameManager])
#define APP ((App *)s_addrs[A_App])
#define RNG ((Rng *)s_addrs[A_Random])
#define BUTTONS (*(WORD *)s_addrs[A_Buttons])
#define PLAY_BUTTONS (*(WORD *)s_addrs[A_PlayButtons])
#define PLAYER ((Player *)s_addrs[A_Player])
#define BULLETS ((BulletManager *)s_addrs[A_BulletManager])
#define ENEMIES ((EnemyManager *)s_addrs[A_EnemyManager])
#define ITEMS ((ItemManager *)s_addrs[A_ItemManager])
#define SPELLCARD ((SpellCard *)s_addrs[A_SpellCard])

typedef int(__fastcall *ReplayCallback)(ReplayManager *replay);
typedef int(__fastcall *MenuCallback)(MainMenu *menu);

// Passes of the main loop into the update chain: all of them, for the
// limit; since the replay started; and of those, the ones that did not
// only wait for a stage to load. Whether the current pass is counted
// (it did not start while the GameManager was loading). App::frames
// as the replay started. Then the settings.
static DWORD s_allPasses;
static DWORD s_passes;
static DWORD s_gamePasses;
static int s_passCounted;
static DWORD s_startFrames;
static DWORD s_seq;
static DWORD s_tail;
static DWORD s_limit;
static char s_rpy[MAX_PATH];
static int s_start[4];

// 0 waiting for the title, 1 playing, 2 back on the title after the
// replay; the calculations the title was up for.
static int s_phase;
static DWORD s_titlePasses;
static DWORD s_fastForwards;
// The counts of the last calculation record, for the stage records.
static DWORD s_lastCalcs;
static DWORD s_lastPasses;
// A crash's code and address.
static DWORD s_crashCode;
static DWORD s_crashAddress;

// The stage PlayAddedCallback last started, whose entries `entry`
// counts, and the replay frame its stage clear started at (stateFlags
// bit 9 as PlayOnUpdate saw it), or -1.
static int s_stage = -1;
static DWORD s_clearFrame = 0xffffffff;

static DWORD Bits(float f)
{
    return *(DWORD *)&f;
}

// FNV-1a over the bytes of `v`.
static DWORD Mix(DWORD h, DWORD v)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        h = (h ^ (v & 0xff)) * 16777619;
        v >>= 8;
    }
    return h;
}

static DWORD MixVector(DWORD h, D3DXVECTOR3 *v)
{
    return Mix(Mix(Mix(h, Bits(v->x)), Bits(v->y)), Bits(v->z));
}

static DWORD BulletHash()
{
    DWORD h = 2166136261u;
    int i;
    Bullet *b;

    // The last one is the spare that ends searches.
    for (i = 0; i < (int)ARRAY_SIZE(BULLETS->bullets) - 1; i++)
    {
        b = &BULLETS->bullets[i];
        if (b->state == 0)
        {
            continue;
        }
        h = Mix(h, i);
        h = Mix(h, b->state | b->color << 16);
        h = Mix(h, b->activeFlags);
        h = Mix(h, b->flags);
        h = MixVector(h, &b->pos);
        h = MixVector(h, &b->velocity);
        h = Mix(h, Bits(b->speed));
        h = Mix(h, Bits(b->angle));
        h = Mix(h, b->timer.current);
        h = Mix(h, b->grazed | b->spawnedInArea << 8 | b->harmless << 16);
        h = Mix(h, b->commandIndex);
    }
    return h;
}

static DWORD LaserHash()
{
    DWORD h = 2166136261u;
    int i;
    Laser *l;

    for (i = 0; i < (int)ARRAY_SIZE(BULLETS->lasers); i++)
    {
        l = &BULLETS->lasers[i];
        if (!l->active)
        {
            continue;
        }
        h = Mix(h, i);
        h = Mix(h, l->state | l->flags << 16);
        h = MixVector(h, &l->pos);
        h = Mix(h, Bits(l->angle));
        h = Mix(h, Bits(l->startOffset));
        h = Mix(h, Bits(l->endOffset));
        h = Mix(h, Bits(l->width));
        h = Mix(h, l->timer.current);
    }
    return h;
}

static DWORD EnemyHash()
{
    DWORD h = 2166136261u;
    int i;
    Enemy *e;

    for (i = 0; i < (int)ARRAY_SIZE(ENEMIES->enemies); i++)
    {
        e = &ENEMIES->enemies[i];
        if (!e->active)
        {
            continue;
        }
        h = Mix(h, i);
        h = Mix(h, e->flags);
        h = MixVector(h, &e->pos);
        h = Mix(h, e->life);
        h = Mix(h, e->maxLife);
    }
    return h;
}

static DWORD ItemHash()
{
    DWORD h = 2166136261u;
    int i;
    Item *it;

    // The last one is the spare SpawnItem returns when all are taken.
    for (i = 0; i < (int)ARRAY_SIZE(ITEMS->items) - 1; i++)
    {
        it = &ITEMS->items[i];
        if (!it->active)
        {
            continue;
        }
        h = Mix(h, i);
        h = Mix(h, (BYTE)it->type | (BYTE)it->state << 8 | (BYTE)it->countsFullValue << 16);
        h = MixVector(h, &it->pos);
        h = MixVector(h, &it->velocity);
    }
    return h;
}

static DWORD ShotHash()
{
    DWORD h = 2166136261u;
    int i;
    PlayerBullet *b;

    for (i = 0; i < (int)ARRAY_SIZE(PLAYER->bullets); i++)
    {
        b = &PLAYER->bullets[i];
        if (b->state == 0)
        {
            continue;
        }
        h = Mix(h, i);
        h = Mix(h, (WORD)b->state | b->damage << 16);
        h = MixVector(h, &b->pos);
        h = Mix(h, Bits(b->angle));
    }
    return h;
}

static void Emit(int kind, ReplayManager *replay, int consumed)
{
    Record r;
    ReplayStage *stage;
    // InitTitleScene deletes the gameplay stats and allocates an
    // uninitialized GameManagerStats. Its heap contents are not title
    // state.
    GameManagerStats *stats = kind == TRACE_KIND_TITLE ? NULL : GM->stats;

    ZeroMemory(&r, sizeof(r));
    r.kind = kind;
    r.seq = s_seq++;
    if (kind == TRACE_KIND_STAGE_START || kind == TRACE_KIND_STAGE_END)
    {
        // In the loading thread, while the main loop runs on.
        r.calcs = s_lastCalcs;
        r.passes = s_lastPasses;
    }
    else
    {
        r.calcs = APP->frames - s_startFrames - (s_passes - s_gamePasses);
        r.passes = s_gamePasses;
    }
    if (kind == TRACE_KIND_CALC)
    {
        s_lastCalcs = r.calcs;
        s_lastPasses = r.passes;
    }
    r.fastForwards = s_fastForwards;
    if (kind == TRACE_KIND_CRASH)
    {
        r.passes = s_crashCode;
        r.calcs = s_crashAddress;
    }
    if (replay != NULL)
    {
        r.replayFrame = replay->frame;
        stage = replay->data != NULL && s_stage >= 0 && s_stage < 9 ? replay->data->stages[s_stage] : NULL;
        if (stage != NULL && replay->cursor != NULL)
        {
            r.entry = replay->cursor - stage->frames;
            if (kind == TRACE_KIND_CALC && consumed && r.entry > 0)
            {
                r.recordedInput = replay->cursor[-1];
            }
        }
    }
    r.clearFrame = s_clearFrame;
    r.stage = GM->stage;
    r.stageFrames = GM->stageFrames;
    // Bit 0: this calculation played a recorded input. Then
    // GameManager::stateFlags.
    r.flags = (consumed ? 1 : 0) | GM->stateFlags << 1;
    r.pause = GM->isPaused | GM->isInContinueMenu << 8 | (BYTE)GM->isFrozen << 16 | GM->replayMenuChoice << 24;
    // App::OnUpdate moves the scene on from 3 to 2 while the loading
    // thread writes the stage records.
    if (kind != TRACE_KIND_STAGE_START && kind != TRACE_KIND_STAGE_END)
    {
        r.scene = (WORD)APP->scene | APP->curScene << 16;
    }
    if (stats != NULL)
    {
        r.score = stats->score;
        r.guiScore = stats->guiScore;
        r.lives = Bits(stats->lives);
        r.bombs = Bits(stats->bombs);
        r.power = Bits(stats->power);
        r.graze = stats->graze;
        r.grazeTotal = stats->grazeTotal;
        r.pointItemsTotal = stats->pointItemsTotal;
        r.maxPointValue = stats->maxPointValue;
        r.stageClearPoints = stats->stageClearPoints;
        r.deaths = Bits(stats->deaths);
        r.bombsUsed = Bits(stats->bombsUsed);
        r.spellCardsCaptured = stats->spellCardsCaptured;
        r.continuesUsed = stats->continuesUsed;
        r.gauge = (WORD)stats->gauge | (WORD)stats->gaugeCopy << 16;
        r.clock = (BYTE)stats->clock;
        r.timeOrbs = stats->timeOrbs;
        r.timeOrbsCollected = stats->timeOrbsCollected;
        r.timeOrbsNeeded = stats->timeOrbsNeeded;
    }
    r.gaugeFrames = GM->gaugeFrames;
    r.humanFrames = GM->humanFrames;
    r.rank = GM->rank;
    r.subRank = GM->subRank;
    r.powerItemCombo = GM->powerItemCombo;
    r.progressFrames = GM->progressFrames;
    r.rngSeed = RNG->seed;
    r.rngGenerated = RNG->generated;
    r.buttons = PLAY_BUTTONS | BUTTONS << 16;
    r.playerX = Bits(PLAYER->pos.x);
    r.playerY = Bits(PLAYER->pos.y);
    r.playerState = (BYTE)PLAYER->playerState | PLAYER->focused << 8 | PLAYER->isYoukai << 16 |
                    PLAYER->lastWordReady << 24;
    r.playerBomb = (PLAYER->bomb.running ? 1 : 0) | (BYTE)PLAYER->bomb.type << 8 | PLAYER->bomb.timer.current << 16;
    r.bulletCount = BULLETS->bulletCount;
    r.enemyCount = ENEMIES->enemyCount;
    r.itemCount = ITEMS->activeCount;
    r.spellCard = SPELLCARD->flags;
    r.spellCardNumber = SPELLCARD->card;
    r.spellCardBonus = SPELLCARD->bonus;
    r.timeOrbBonus = SPELLCARD->timeOrbBonus;
    r.bulletHash = BulletHash();
    r.laserHash = LaserHash();
    r.enemyHash = EnemyHash();
    r.itemHash = ItemHash();
    r.shotHash = ShotHash();
    TraceWrite(&r, sizeof(r));
    if (kind != TRACE_KIND_CALC)
    {
        TraceFlush();
    }
}

static void Finish(int kind, UINT code)
{
    Emit(kind, NULL, 0);
    TraceExit(code);
}

static int __fastcall PlayOnUpdate(ReplayManager *replay)
{
    int frame = replay->frame;
    int res;

    // The loading thread finished after the pass started.
    if (!s_passCounted)
    {
        s_gamePasses++;
        s_passCounted = 1;
    }
    if (GM->stageClearStarted && s_clearFrame == 0xffffffff)
    {
        s_clearFrame = replay->frame;
    }
    res = ((ReplayCallback)s_addrs[A_PlayOnUpdate])(replay);

    Emit(TRACE_KIND_CALC, replay, replay->frame != frame);
    return res;
}

static int __fastcall PlayOnUpdate2(ReplayManager *replay)
{
    int res = ((ReplayCallback)s_addrs[A_PlayOnUpdate2])(replay);

    // 6 restarts the update chain: dialogue fast-forward.
    if (res == 6)
    {
        s_fastForwards++;
    }
    return res;
}

// Runs in GameManager::LoadingThread, while the main loop waits.
static int __fastcall PlayAddedCallback(ReplayManager *replay)
{
    int res;

    if (replay->mode == 1 && replay->data != NULL)
    {
        Emit(TRACE_KIND_STAGE_END, replay, 0);
    }
    res = ((ReplayCallback)s_addrs[A_PlayAddedCallback])(replay);
    s_stage = GM->stage;
    s_clearFrame = 0xffffffff;
    Emit(TRACE_KIND_STAGE_START, replay, 0);
    return res;
}

static int __fastcall DeletedCallback(ReplayManager *replay)
{
    if (replay->mode == 1 && s_phase == 1)
    {
        Emit(TRACE_KIND_REPLAY_END, replay, 0);
        s_phase = 2;
        s_titlePasses = 0;
    }
    return ((ReplayCallback)s_addrs[A_DeletedCallback])(replay);
}

// Starts the replay with the settings the replay menu takes from it
// (MainMenu::UpdateReplayMenu, after StartTitle cleared practice). The
// menu's update callback then returns 0, which removes the menu.
static void StartReplay()
{
    int i;

    GM->stateFlags &= ~1;
    GM->stateFlags &= ~0x4000;
    GM->isReplay = 1;
    for (i = 0; s_rpy[i] != '\0' && i < sizeof(GM->replayFile) - 1; i++)
    {
        GM->replayFile[i] = s_rpy[i];
    }
    GM->replayFile[i] = '\0';
    GM->difficulty = s_start[1];
    GM->character = s_start[2];
    GM->isInSpellCardPractice = s_start[3] >= 0;
    GM->practiceSpellCard = s_start[3];
    GM->stage = s_start[0];
    APP->scene = 2;
    GM->replayMenuChoice = 0;
    s_startFrames = APP->frames;
    s_passes = 0;
    s_gamePasses = 0;
}

// Replaces MainMenu::OnUpdate. After 60 updates in the title menu,
// starts playback with the fields the replay menu would set.

static int __fastcall MenuOnUpdate(MainMenu *menu)
{
    if (s_phase == 0 && menu->loadingState == 0 && menu->state == 0 && menu->subState == 1 && ++s_titlePasses >= 60)
    {
        StartReplay();
        s_phase = 1;
        return 0;
    }
    return ((MenuCallback)s_addrs[A_MenuOnUpdate])(menu);
}

static void __cdecl BeforeCalc()
{
    if (s_allPasses == 0)
    {
        APP->exeSize = TraceEnvNumber("TRACE_EXE_SIZE", APP->exeSize);
        APP->exeChecksum = TraceEnvNumber("TRACE_EXE_CHECKSUM", APP->exeChecksum);
    }
    s_allPasses++;
    s_passes++;
    // GameManager::OnUpdate also sets loadingThreadState as a stage
    // ends, and leaves it set after the game.
    s_passCounted = GM->loadingThreadState == 0 || APP->curScene != 2;
    if (s_passCounted)
    {
        s_gamePasses++;
    }
    if (s_allPasses >= s_limit)
    {
        Finish(TRACE_KIND_LIMIT, 3);
    }
    if (s_phase == 2 && APP->curScene == 1 && APP->scene == 1 && ++s_titlePasses >= s_tail)
    {
        Finish(TRACE_KIND_TITLE, 0);
    }
}

static void Crashed(DWORD code, DWORD address)
{
    s_crashCode = code;
    s_crashAddress = address;
    Finish(TRACE_KIND_CRASH, 4);
}

static DWORD s_runCalcChain;

// Replaces `call Chain::RunCalcChain` (thiscall, `this` in ecx).
static __declspec(naked) void RunCalcChainHook()
{
    __asm
    {
        push ecx
        call BeforeCalc
        mov ecx, [esp]
        call dword ptr [s_runCalcChain]
        pop ecx
        ret
    }
}

// Patches the code and checks each patch found what it expected.
static BOOL Patch()
{
    DWORD reg = s_addrs[A_RegisterChain];
    DWORD regSize = s_addrs[A_RegisterChainSize];
    int n;
    int calls;

    s_runCalcChain = s_addrs[A_RunCalcChain];
    n = TracePatchCalls(s_addrs[A_RunFrame], s_addrs[A_RunFrameSize], s_runCalcChain, (DWORD)RunCalcChainHook);
    TraceInfo("RunCalcChain=%d", n);
    if (n != 1)
    {
        return FALSE;
    }
    n = TracePatchImmediates(s_addrs[A_SetUpTitleScene], s_addrs[A_SetUpTitleSceneSize], s_addrs[A_MenuOnUpdate],
                             (DWORD)MenuOnUpdate);
    TraceInfo(" MenuOnUpdate=%d", n);
    if (n != 1)
    {
        return FALSE;
    }
    n = TracePatchImmediates(reg, regSize, s_addrs[A_PlayOnUpdate], (DWORD)PlayOnUpdate);
    TraceInfo(" PlayOnUpdate=%d", n);
    if (n != 1)
    {
        return FALSE;
    }
    n = TracePatchImmediates(reg, regSize, s_addrs[A_PlayOnUpdate2], (DWORD)PlayOnUpdate2);
    TraceInfo(" PlayOnUpdate2=%d", n);
    if (n != 1)
    {
        return FALSE;
    }
    n = TracePatchImmediates(reg, regSize, s_addrs[A_PlayAddedCallback], (DWORD)PlayAddedCallback);
    calls = TracePatchCalls(reg, regSize, s_addrs[A_PlayAddedCallback], (DWORD)PlayAddedCallback);
    TraceInfo(" PlayAddedCallback=%d+%d", n, calls);
    if (n != 1 || calls != 1)
    {
        return FALSE;
    }
    // Recording's and playing's.
    n = TracePatchImmediates(reg, regSize, s_addrs[A_DeletedCallback], (DWORD)DeletedCallback);
    TraceInfo(" DeletedCallback=%d", n);
    return n == 2;
}

static DWORD (WINAPI *s_clock)();

static BOOL WINAPI Counter(LARGE_INTEGER *value)
{
    value->QuadPart = s_clock();
    return TRUE;
}

static BOOL WINAPI Frequency(LARGE_INTEGER *value)
{
    value->QuadPart = 1000;
    return TRUE;
}

static BOOL Install()
{
    char path[MAX_PATH];
    char start[64];
    const char *p;
    int i;

    if (!TraceReadAddrs(s_addrNames, s_addrs, NUM_ADDRS) || !TraceEnvString("TRACE_REPLAY", s_rpy, sizeof(s_rpy)) ||
        !TraceEnvString("TRACE_START", start, sizeof(start)) || !TraceEnvString("TRACE_OUT", path, sizeof(path)))
    {
        OutputDebugStringA("th08trace: settings missing\n");
        return FALSE;
    }
    for (p = start, i = 0; i < 4; i++)
    {
        // The spell card is -1 outside spell card practice.
        if (*p == '-')
        {
            p++;
            s_start[i] = -(int)TraceNextNumber(&p);
        }
        else
        {
            s_start[i] = TraceNextNumber(&p);
        }
    }
    s_tail = TraceEnvNumber("TRACE_TAIL", 60);
    s_limit = TraceEnvNumber("TRACE_LIMIT", 2000000);
    if (!TraceOpen(path))
    {
        OutputDebugStringA("th08trace: cannot create the trace\n");
        return FALSE;
    }
    if (!Patch() || !TraceInstallClock() || !TraceInstallRenderer())
    {
        TraceInfo(" failed");
        return FALSE;
    }
    // Retrieve the runtime clock through the executable's IAT. DllMain
    // runs before any game code, so no caller sees the temporary value.
    s_clock = (DWORD (WINAPI *)())TracePatchImport("winmm.dll", "timeGetTime", (void *)Counter);
    TracePatchImport("winmm.dll", "timeGetTime", (void *)s_clock);
    if (!s_clock || !TracePatchImport("kernel32.dll", "QueryPerformanceCounter", (void *)Counter) ||
        !TracePatchImport("kernel32.dll", "QueryPerformanceFrequency", (void *)Frequency))
    {
        TraceInfo(" performance clock failed");
        return FALSE;
    }
    TraceOnCrash(Crashed);
    return TRUE;
}

extern "C" __declspec(dllexport) void Init()
{
}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    BOOL ok;

    if (reason == DLL_PROCESS_DETACH)
    {
        TraceFlush();
        return TRUE;
    }
    if (reason != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }
    ok = Install();
    TraceWriteHeader(s_fieldNames, sizeof(Record) / 4);
    return ok;
}
