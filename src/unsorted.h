// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <mmsystem.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3d8.h>
#include <d3dx8math.h>

#include "archive.h"
#include "common.h"
#include "dsutil.h"
#include "resultscreen.h"
#include "screeneffect.h"
#include "sprite.h"
#include "timer.h"

// The application: window, Direct3D, configuration and the main loop.
// Only the fields known so far. Its start matches th07's App: the same
// fields are used at the same offsets.
// The buttons each action is mapped to, as pad button numbers; negative
// when unmapped. The same as th07's.
struct ControllerMapping
{
    short shootButton;
    short bombButton;
    short focusButton;
    short menuButton;
    short upButton;
    short downButton;
    short leftButton;
    short rightButton;
    short skipButton;
};
ASSERT_SIZE(ControllerMapping, 0x12);

class AnmLoaded;
class MidiPlayer;
class MidiTimer;

// The config's option bits, in th07's order: the bits used so far test
// the same options as th07's.
struct GameConfigFlags
{
    unsigned int emulateColorAdd : 1;
    unsigned int noVertexBuffer : 1;
    unsigned int force16BitTextures : 1;
    unsigned int clearBackBuffer : 1;
    unsigned int noFrameItems : 1;
    unsigned int noGouraudShading : 1;
    unsigned int noDepthTest : 1;
    unsigned int force60Fps : 1;
    unsigned int noColorBlending : 1;
    unsigned int referenceRasterizer : 1;
    unsigned int noFog : 1;
    unsigned int noDirectInput : 1;
    unsigned int redrawHudEveryFrame : 1;
    unsigned int preloadBgm : 1;
    unsigned int noVsync : 1;
    // Text rendering's environment is not detected automatically. New
    // in th08.
    unsigned int noTextRenderDetect : 1;
    unsigned int unusedBits : 16;
};

class App
{
public:
    // Zeroes the whole App and sets bits 0x40 and 0x100 of stateFlags.
    // Emitted where g_App is defined (0x445b7a).
    App();
    // Resets the scene and adds OnUpdate at priority 0, with its added
    // and deleted callbacks, then OnDraw at 0x10, OnDrawLogo at 0 and
    // OnDrawLoading at 2 to the draw chain. Returns 0, -1 to quit, or
    // anything else to restart: what adding OnUpdate returned.
    static int RegisterChain();
    // The main update chain callback and its added and deleted
    // callbacks.
    static int OnUpdate(App *arg);
    // The update element's added callback: loads the score file's
    // chunks, notes which shot types have Extra open, opens th08.dat,
    // shows the logo and the loading sprites, measuring the refresh rate
    // unless presentImmediate is set, then runs LoadingThread in the sub
    // thread for the rest. Returns 0, -1 on failure, or -2 when the
    // refresh rate needs a restart.
    static int AddedCallback(App *arg);
    // The rest of th07's AddedCallback, run in the sub thread (with /Gr,
    // a fastcall function passed to CreateThread).
    static void LoadingThread(App *arg);
    static int DeletedCallback(App *arg);
    // The draw chain callback: draws the frame rate. Returns 1.
    static int OnDraw(App *arg);
    // The first draw chain callback, new in th08: from loadingState 2 on,
    // counts it up and blinks "Press Shot Button" from 5 to 64, then
    // shows the logo in surface 8 while loadingState is nonzero, or
    // releases it. Returns 1.
    static int OnDrawLogo(App *arg);
    // The third draw chain callback, new in th08: draws
    // g_LoadingSprites, each moved by its posOffset. Returns 1.
    static int OnDrawLoading(App *arg);
    // Whether holding shoot also focuses (shootFocuses). th07 read the
    // field directly; th08 calls this out-of-line copy.
    int GetShootFocuses();
    // Enters and leaves criticalSections[idx], counting the depth in
    // criticalSectionDepths. New in th08.
    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);
    // Closes the sub thread first, then runs `proc` with `arg` in a new
    // one, setting subThreadRunning. Returns 0, or -1 if it could not be
    // created. New in th08.
    int StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg);
    // Asks the sub thread to close (closeSubThread) and waits for it,
    // then closes its handle. New in th08.
    void CloseSubThread();
    // Unless the loading screen shows (loadingState), starts
    // g_LoadingSprites on scripts 0 to 2 of loadingAnm at `pos` and sets
    // loadingState to 1. New in th08.
    // Whether the sub thread is running.
    BOOL HasSubThread()
    {
        return this->subThread != NULL;
    }
    void StartLoadingSprites(D3DXVECTOR3 *pos);
    // If the loading screen is in state 1, sends g_LoadingSprites
    // interrupt 1 and clears loadingState. Then ends
    // g_LoadingScreenEffect if there is one.
    void StopLoadingSprites();
    // StartLoadingSprites, then has the back buffer copied to texture 8
    // after the frame unless a copy is pending.
    void StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos);
    // As StopLoadingSprites, with interrupt 2 and state 2: the screen
    // waits for the shot button.
    void FinishLoadingSprites();
    // Starts g_LoadingScreenEffect (type `type` + 5, 60 frames) unless
    // there is one.
    void StartLoadingScreenEffect(int type);
    // Clears th07's clock drift counters. th07 defined it in its title
    // menu file; here the copy (0x470dd5) follows InitTitleScene, its
    // caller.
    void ResetClockDriftCounters();
    // Initializes and deletes criticalSections.
    void InitCriticalSections();
    void DeleteCriticalSections();
    // Advances `frames` by one frame, or by a fraction of one into
    // `subFrame` while the game runs slow.
    // Reads the config file at `path` into the config, resetting it to
    // the defaults if the file is missing or invalid, checks the header
    // of thbgm.dat, logs the options in effect and writes the config
    // back. Returns 0, or -1 if thbgm.dat has the wrong version or
    // writing fails.
    int LoadConfig(char *path);
    // th06's LoadMidi: loads the MIDI file `path` into slot `slot` of
    // `midi` in MIDI mode (2), and returns 0. In wave mode (1) it queues
    // sound command 1 for the .wav of the same name instead. Returns 1
    // outside MIDI mode.
    int LoadMidi(int slot, char *path);
    // th06's PlayMidi: plays MIDI slot `slot` from the start in MIDI
    // mode. In wave mode it queues sound command 2 for the slot, after
    // command 4 with g_App.flags.preloadBgm. Either way it then marks
    // `bgm` heard in g_GameManager.plst, unless bit 3 or 1 of
    // g_GameManager.unk_3dbac is set. Returns 0.
    int PlayMidi(int slot, int bgm);
    // Plays the MIDI file at `path` from the start in MIDI mode, or
    // queues sound command 2 for the .wav of the same name in wave mode,
    // then marks `bgm` heard as PlayMidi. Returns -1 if the music mode is
    // neither, otherwise 0.
    int PlayMusic(char *path, int bgm);
    // th06's StopMusic: stops the MIDI player in MIDI mode; in wave mode
    // it queues sound command 3, or 4 with g_App.flags.preloadBgm.
    // Returns -1 if the music mode is neither, otherwise 0.
    int StopMusic();
    // Fades the music out over `seconds`: in wave mode by queueing sound
    // command 5 with the whole seconds, stretched by gameSpeed while the
    // game runs slow. Returns -1 if the music mode is neither wave nor
    // MIDI, otherwise 0.
    int FadeOutMusic(float seconds);
    // Whether g_GameManager.unk_4 is set and its scoreDisabled is not 0.
    // The result screen records no score then.
    int IsScoreDisabled();
    // Bit 3 of stateFlags: no score is recorded. Inline; the copy at
    // 0x453c40 is the replay object's.
    int GetNoScore()
    {
        return this->stateFlags >> 3 & 1;
    }
    // Flushes the sprites and turns fog on unless fogState is 1 already;
    // returns 0 when it was.
    int EnableFog();
    // Flushes the sprites and turns fog off if fogState is set; returns
    // 0 when it was not.
    int DisableFog();
    // Flushes the sprites and sets render state `state` to `value`.
    // Returns what SetRenderState returned.
    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    // Adds the time since the last call to the run time in
    // g_GameManager.plst. th08 restarts from now when the clock went
    // back, where th07 counted from 0.
    void AddPlayTime();
    // Adds the time since the last call to the time played in
    // g_GameManager.plst.
    void AddTotalPlayTime();
    // Checks the version file against `version` ("0100d") and the
    // executable's size and checksum: finds a line starting with the
    // version and reads "%d %d" after it. Returns 0 without a version
    // file, for a "debug" version or when both numbers match, otherwise
    // -1, which makes OnUpdate quit. Unlike th07's, a line whose numbers
    // differ does not end the search.
    int CheckVersionFile(char *version, DWORD size, DWORD checksum);
    void TickTimer(int *frames, float *subFrame);
    // Saves the back buffer as a 24-bit 640x480 bitmap at `path`, from an
    // X8R8G8B8 back buffer only (a 16-bit one is logged and skipped).
    // Returns 1 for any other format, otherwise 0.
    int SaveScreenshot(char *path);
    // Getters for config options that th07 read directly; th08 calls
    // these out-of-line copies (0x443a50..0x443ad0).
    int GetEmulateColorAdd();
    int GetNoVertexBuffer();
    int GetForce16BitTextures();
    int GetReferenceRasterizer();
    int GetWindowed();
    // More of them, out-of-line copies from other objects: noFrameItems
    // (0x438a29), noDepthTest (0x438a41), noColorBlending (0x438a59) and
    // redrawHudEveryFrame (0x438a71) from the Gui object.
    int GetNoFrameItems();
    int GetNoDepthTest();
    int GetNoColorBlending();
    int GetRedrawHudEveryFrame();
    // flags.noFog. Inline: the copy at 0x406580 comes from the /Od ascii
    // object, the first in link order to use it.
    int GetNoFog()
    {
        return this->flags.noFog;
    }
    // flags.preloadBgm. Inline: the copy at 0x449c79 comes from the /Os
    // music screen object.
    int GetPreloadBgm()
    {
        return this->flags.preloadBgm;
    }

    // Whether the back buffer is cleared every frame: forced, or because
    // the items around the game are not drawn. Inline; the operands are
    // evaluated right to left.
    int ClearsBackBuffer()
    {
        return this->flags.clearBackBuffer | this->flags.noFrameItems;
    }
    // As th07's: whether color blending is off or color add emulated.
    // AsciiText::AddString only keeps a string's highlight then. Inline;
    // the copy is at 0x406550.
    int HasFlag0Or8()
    {
        return this->flags.noColorBlending | this->flags.emulateColorAdd;
    }

    HINSTANCE hInstance;
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    IDirectInput8 *dinput;
    IDirectInputDevice8 *keyboard;
    IDirectInputDevice8 *joystick;
    DIDEVCAPS joystickCaps;
    HWND hwnd;
    // D3DMATRIX, not D3DXMATRIX: App::App would call D3DXMATRIX's
    // constructor (not inlined without /Ob1).
    D3DMATRIX viewMatrix;
    D3DMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;
    D3DPRESENT_PARAMETERS presentParams;
    MidiTimer *timer;
    ControllerMapping controllerMapping;
    DWORD version;
    // How far the DirectInput stick must lean to count as a direction.
    short padThresholdX;
    short padThresholdY;
    BYTE lifeCount;
    BYTE bombCount;
    BYTE colorMode16Bits;
    BYTE musicMode;
    BYTE playSounds;
    BYTE defaultDifficulty;
    BYTE windowed;
    BYTE frameSkip;
    // 0 to 2, 2 by default.
    BYTE effectsQuality;
    // The options screen's last entry: 0 or 1.
    BYTE optionsMenuCursor;
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames.
    BYTE shootFocuses;
    // The music and sound effect volumes in percent: 100 and 80 by
    // default.
    char bgmVolume;
    char seVolume;
    unsigned char pad_141[0xf];
    // th07's flags, 4 bytes further on.
    GameConfigFlags flags;
    // Counts OnUpdate's calls.
    int frames;
    // The scene set up; OnUpdate switches it to `scene` when they
    // differ.
    int curScene;
    // The scene the game is in or switches to.
    int scene;
    // The scene the game came from.
    int prevScene;
    // Cleared by GameManager::AddedCallback in scenes 3, 11 and 12, else
    // set; unk_168 the same by DeletedCallback.
    int loadsResources;
    int freesResources;
    // Set by OnUpdate when a game restarts from scene 11.
    int isRestarting;
    unsigned char pad_170[0x4];
    // How many more frames to draw in full, after the device was reset.
    int redrawFrames;
    // DrawFps shows the frame rate while it is 0; AddedCallback sets it
    // while the refresh rate is measured.
    int hideFps;
    // Frames are paced by the clock rather than by the presents.
    int presentImmediate;
    // Set while the device is created asking for 60 Hz.
    int wants60Hz;
    // Cleared by InitD3dDevice.
    int clearedOnD3dInit;
    // The game's speed: 1, or less while it runs slow. Timer::Add counts
    // in fractions of a frame below 0.99.
    float gameSpeed;
    // The MIDI player; its long messages come back to WindowProc.
    MidiPlayer *midi;
    // Their ratio is the game's speed, which the score entry records
    // as slowdown.
    float achievedFrames;
    float expectedFrames;
    // The frame rate rounded, as DrawFps shows it with bit 3 of
    // g_GameManager.unk_3dbac.
    short roundedFps;
    unsigned char pad_19a[0x2];
    // text.anm, loaded by LoadingThread.
    AnmLoaded *textAnm;
    // nowloading.anm, with g_LoadingSprites' scripts (AddedCallback).
    AnmLoaded *loadingAnm;
    // As th07's: bit 1: the device does T&L in hardware; 2: the back
    // buffer is lockable; 4: A8R8G8B8 textures work; 8: no score is
    // recorded; 0x10: the device was reset and no frame has been drawn
    // since; 0x20: Timer::Add steps by whole frames. New in th08: bit
    // 0x40 is set by CheckSingleInstance when the startup info has no
    // title, cleared when it has one; bit 0x80 makes OnUpdate quit once
    // subThread is clear.
    DWORD stateFlags;
    // timeGetTime() when AddPlayTime last added to the run time.
    DWORD lastPlayTimeTick;
    // timeGetTime() when AddTotalPlayTime last added to the play time.
    DWORD lastTotalPlayTimeTick;
    D3DCAPS8 caps;
    // The sub thread StartSubThread starts (AddedCallback runs the
    // loading in it), and its id.
    HANDLE subThread;
    DWORD subThreadId;
    // Set by CloseSubThread while it waits for the thread to end.
    int closeSubThread;
    // Set by StartSubThread. OnUpdate returns early while unk_294 is set
    // and this is.
    int subThreadRunning;
    // OnUpdate quits when it is 2.
    int loadingThreadState;
    // Locked by EnterCriticalSection: 2 around file access, 3 around the
    // log.
    CRITICAL_SECTION criticalSections[4];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[4];
    // The loading screen's state: 0 for none, 1 while g_LoadingSprites
    // show, then from 2 on a frame counter while OnDrawLogo blinks
    // "Press Shot Button". OnDrawLogo shows surface 8 while it is
    // nonzero.
    int loadingState;
    // QueryPerformanceFrequency's result, as AddedCallback read it. Only
    // 4-aligned, so not a LARGE_INTEGER.
    DWORD perfFrequency[2];
    // th07's clock drift measurement, which th08 no longer runs.
    DWORD startCounter[2];
    DWORD counter[2];
    SYSTEMTIME startTime;
    SYSTEMTIME time;
    int fastClockStreak;
    // DrawFps shows the rate in red when it is set.
    int fpsWarning;
    int maxFastClockStreak;
    int fastClockCycleCount;
    int fastClockCycles;
    int clockMeasurementPending;
    // Set to 0xff before each frame is drawn; DisableFog clears it.
    DWORD fogState;
    // The sum of the executable's dwords and its size, from
    // ChecksumExecutable.
    DWORD exeChecksum;
    DWORD exeSize;
    // The version file OpenGameArchive loads, and its size.
    int verFileSize;
    void *verFile;
};
ASSERT_SIZE(App, 0x364);

extern App g_App;

// Scene set-up and tear-down functions App::OnUpdate calls, not
// decompiled yet. The set-up ones return nonzero on failure.
// th07's GameManager::RegisterChain and RemoveChain.
int RegisterGameManagerChain();
void RemoveGameManagerChain();

// Counted down by App::OnUpdate every frame while nonzero.
extern int g_ScreenEffectHoldFrames;
// Cleared by GameManager::DeletedCallback.
extern int g_ScriptFunc0Int;
// Set with g_ScriptFunc0Int by SetScriptFunc0Vars.
extern float g_ScriptFunc0Float;
// GameManager::AddGauge only applies while it is clear, unless forced.
extern int g_PlayerBombRunning;
// The damage an enemy takes per time orb Player::CalcDamage spawns.
extern int g_DamagePerTimeOrb;
// Copies the stage's portrait into the dialogue texture by
// g_GameManager.stage and, in a spell card practice, the card (0x438046).
void CopyStageSprite();

// Measures the frame rate every half second, with the performance
// counter when there is one, and shows it when `draw` is set, as th07's.
void DrawFps(int draw);

// Opens th08.dat as g_Archive and loads its version file,
// "th08_0100d.ver", into g_App.verFile. Returns 0, or -1 after logging
// an error if either is missing.
int OpenGameArchive();

// Measures the refresh rate by presenting surface 8, up to 1800 times
// or until it has eight rates of 57 Hz or more, each over half a second
// or so. Unless g_App.flags.noVsync is set, it turns presentImmediate on
// and returns -2 when their average is over 160 Hz (or there are fewer
// than two), or 65 Hz or more. Otherwise returns 0.
int MeasureRefreshRate();

// Sets up DirectInput as the DirectX 8 SDK's Keyboard and Joystick
// samples: the keyboard, then the first joystick found, if any. Returns
// 0, or -1 if DirectInput is off or the keyboard fails.
int InitDirectInput(App *game);

// IDirectInput8::EnumDevices's callback for joysticks, from the DirectX
// 8 SDK's Joystick sample: creates a device for the first joystick that
// works, unless there already is one.
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);

// IDirectInputDevice8::EnumObjects's callback for the joystick's axes,
// from the same sample without its dialog updates: sets each axis's
// range to -1000 to 1000.
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);

// Hands finished MIDI long messages back to g_App.midi, keeps the
// cursor shown only in a window or while inactive, swallows
// WM_ERASEBKGND, and has WM_CLOSE end the main loop.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Presents the frame, resetting the device if that fails; does the
// pending texture copy; saves a screenshot when button 0x800 is pressed.
void PresentFrame();
// Creates g_App.d3d. Returns 0, or 1 on failure.
int CreateD3d();
// Creates the game window. Returns 0, or nonzero on failure.
int CreateGameWindow(HINSTANCE instance);
// Creates the Direct3D device. Returns 0, or nonzero on failure.
int InitD3dDevice();
// Clears the back buffer and the z buffer to `color` and presents, twice,
// resetting the device when presenting fails, as th07's.
void ClearScreen(D3DCOLOR color);
// Draws the batched quads, then clears the whole back buffer to `color`
// (0x45b0e0).
void ClearViewport(D3DCOLOR color);
// Draws `rect` with a color for each corner, blended across, as th07's
// (0x45b490).
void DrawGradientRect(float *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft, D3DCOLOR bottomRight);
// Sets the device's render and texture stage states, as the config
// flags pick them, and forgets SpriteEngine's cached states.
void SetDefaultRenderStates();
// Prints `name` and then 可 or 不可 (yes or no), by whether `caps` has
// any of `flag`, at `out`; returns the end of the text.
char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out);
// Writes a report of the device capabilities `caps` that matter to the
// game into `buffer`, in Japanese.
void FormatCapsReport(D3DCAPS8 *caps, char *buffer);

inline void Timer::Tick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->subFrame);
}

inline int Timer::NextTick()
{
    this->previous = this->current;
    g_App.TickTimer(&this->current, &this->subFrame);
    return this->current;
}

// The game window (at 0x17ce700), as in th07: the fields used so far are
// at the same offsets. Its 8-byte fields are only 4-aligned.
#pragma pack(push, 4)
struct GameWindow
{
    // Runs one frame of the main loop. Returns 0 to go on, else what
    // App::RegisterChain's result means. New in th08: paces the frames
    // by nextFrameTime, 1/60 s apart, and sleeps while it is ahead.
    int RunFrame();
    // The time in seconds by the performance counter, or else
    // timeGetTime() in milliseconds: RunFrame's pacing is off without a
    // performance counter.
    double GetTime();

    HWND hwnd;
    // Set when the window is being closed: the main loop ends.
    int isClosing;
    WPARAM isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char pad_11[0x3];
    // QueryPerformanceFrequency's result.
    DWORD perfFrequency[2];
    // Set when the game was started from a shortcut or another path;
    // paces every frame by the clock, even ones that are early.
    BYTE forceClockPacing;
    unsigned char pad_1d[0x3];
    // The screen saver and power settings, put back on exit.
    int screenSaveActive;
    int lowPowerActive;
    int powerOffActive;
    // New in th08: RunFrame paces the frames by these times.
    double now;
    double lastTime;
    double nextFrameTime;
};
#pragma pack(pop)
ASSERT_SIZE(GameWindow, 0x44);
extern GameWindow g_GameWindow;

// Creates g_AppMutex and returns -1 if another instance holds it, as
// th07's CheckSingleInstance. Also sets g_GameWindow.forceClockPacing
// when the game was started through a shortcut or a path other than its
// own, and sets bit 0x40 of g_App.stateFlags if the startup info has no
// title, clearing it otherwise.
int CheckSingleInstance(HINSTANCE instance);
// Brings `hwnd` to the foreground, lifting the foreground lock timeout
// for the time.
void ForceForegroundWindow(HWND hwnd);
// Sums the executable's dwords into g_App.exeChecksum and returns the
// sum, or -1 on failure.
int ChecksumExecutable();
// The mutex that keeps a second instance from starting.
extern HANDLE g_AppMutex;
// Resolves the shortcut at `path` into `out`, `size` bytes long.
int ResolveShortcut(char *path, char *out, int size);

// Debug output, compiled out of the release build: an empty function
// that the game still calls with messages.
void DebugPrint(const char *fmt, ...);

// Another empty debug output function, in an /Os object (0x437cdd), as
// th07's.
void DebugPrint2(const char *fmt, ...);

// Loads a whole file into a buffer from g_MemoryManager; NULL on
// failure. Unlike th07's, it stores the file's size in `size` if that is
// not NULL, where th07 kept it in g_FileSize; the last argument says
// whether the file is read from disk rather than from the archives.
void *LoadFileData(char *path, int *size, int isExternal);

// The object error messages go to, e.g. "error : MIDI File が読み込めない".
// The same layout as th07's: Flush uses the same offsets.
class Log
{
public:
    char buffer[0x2000];
    // The end of the text in `buffer`.
    char *cursor;
    // Set by LogError, the error logger.
    char hasError;

    // Starts empty. Not referenced: the out-of-line copy of an inline
    // constructor, at the end of the object.
    Log();
    // Empties the log. th07's WinMain did this in place.
    void Reset();
    // Writes the log to ./log.txt, if anything was logged, after a
    // closing line; shows it in a message box first if it holds an
    // error.
    void Flush();
};
extern Log g_Log;

// Logs a printf-style message to a log object.
int LogPrintf(Log *log, const char *fmt, ...);
// Like LogPrintf, and marks the log as holding an error.
int LogError(Log *log, const char *fmt, ...);

// Whether the file at `path` exists and can be opened for reading: 1 or
// 0.
int FileExists(char *path);

// Writes `size` bytes of `data` to the file at `path`. Returns 0, -1 if
// the file cannot be created, -2 if writing fails.
int WriteToFile(char *path, void *data, int size);

// The joystick's capabilities, from joyGetDevCaps.
extern JOYCAPSA g_JoyCaps;

// Checks that joystick 1 is there and reads its capabilities into
// g_JoyCaps. Returns 1 when there is none.
WORD InitJoystick();

// Ors `bit` into `buttons` while button `button` (a bit index into
// joyGetPosEx's dwButtons) is down, and returns what it ored in. A
// negative button is unmapped.
int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons);

// Like SetButtonFromJoystick, for a DirectInput state's rgbButtons:
// a button is down when its byte has bit 0x80 set.
int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons);

// How many frames shoot has been held, for App::shootFocuses; at most
// 20.
extern WORD g_ShotHeldFrames;

// Adds the joystick's buttons and directions to `buttons` and returns
// them: through DirectInput if there is a device, else joyGetPosEx.
// Unlike th07's, it does not map DirectInput button 7 to 0x2000.
WORD ReadJoystick(WORD buttons);

// The pad's buttons as GetPadButtons last read them: 0x80 when down.
extern BYTE g_PadButtons[0x80];
// Reads which pad buttons are down into g_PadButtons and returns it.
BYTE *GetPadButtons();

// Reads the keyboard and the joystick into button bits: through
// DirectInput if there is a keyboard device, else GetKeyboardState.
// th08 adds P as a second 0x800 key, and reacquires the keyboard on any
// DirectInput failure, not only a lost device.
WORD ReadButtons();

// The buttons held this frame and the frame before, as bits.
extern WORD g_Buttons;
extern WORD g_PrevButtons;
// The buttons the game plays by this frame and the frame before: the
// held ones, or a replay's.
extern WORD g_GameButtons;
extern WORD g_PrevGameButtons;
// 1 on every 8th frame once the buttons have been held unchanged for 30
// frames, for key repeat.
extern WORD g_ButtonRepeat;
// How many frames the buttons have been held unchanged, cycling from 38
// back to 30.
extern WORD g_ButtonFrames;

// Whether a button of `mask` went down this frame.
#define WAS_PRESSED(mask) ((g_Buttons & (mask)) && (g_Buttons & (mask)) != (g_PrevButtons & (mask)))
// WAS_PRESSED, or held long enough to repeat.
#define WAS_PRESSED_OR_HELD(mask) (WAS_PRESSED(mask) || ((g_Buttons & (mask)) && g_ButtonRepeat))

// Clears the down bit (0x80) of every key in the thread's keyboard
// state.
void ResetKeyboard();

// Decrypts `size` bytes of `data` into a new buffer from
// g_MemoryManager and returns it (or `data` if the allocation fails).
// The first `limit` bytes go in blocks of `blockSize`: each block's
// bytes are xored with a key that starts at `key` and grows by `step`,
// and land at every other byte from the block's end backwards, then
// in the gaps. A short tail (and the odd byte) is copied as is. New in
// th08.
BYTE *DecryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);

// The DecryptData parameters for one kind of data file.
struct CryptParams
{
    // The kind's letter ('M', 'T', 'A', 'J', 'E', 'W', '-', '*'), stored
    // plus 0x10 times (its index + 1).
    BYTE type;
    BYTE key;
    BYTE step;
    BYTE unusedByte3;
    int blockSize;
    int limit;
};
ASSERT_SIZE(CryptParams, 0xc);
extern CryptParams g_CryptParams[8];
// "edz" plus 0x20, 0x40 and 0x60: the first three bytes of an encrypted
// data file.
extern BYTE g_CryptMagic[4];

// If `data` (`fileSize` bytes) starts with the encrypted file header
// ("edz" and a type letter from g_CryptParams), decrypts the rest into a
// new buffer, frees `data` and returns the new one; else returns `data`.
// Leaves `size` alone, so a caller's size still counts the 4-byte
// header.
BYTE *DecryptFileData(BYTE *data, int *size, int fileSize);

// The inverse of DecryptData: reads each block from its end backwards,
// every other byte, then the gaps, xoring with the growing key.
BYTE *EncryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);

// The game's random number generator: a 16-bit state stepped with a xor,
// a subtraction and a rotation, as in th06.
class Rng
{
public:
    // Advances the generator and returns 16 random bits.
    WORD GetRandomU16();
    // `generated`, read through a temporary.
    DWORD GetGenerated()
    {
        return this->generated;
    }
    // Unlike th06's, leaves `generated` alone.
    void SetSeed(WORD seed)
    {
        this->seed = seed;
    }
    // Clears `generated`. New in th08 (th07 wrote it directly).
    void ResetGenerated()
    {
        this->generated = 0;
    }
    // `seed`. New in th08.
    WORD GetSeed()
    {
        return this->seed;
    }
    // 32 random bits, from two GetRandomU16 calls.
    DWORD GetRandomU32();
    WORD GetRandomU16InRange(WORD range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }
    DWORD GetRandomU32InRange(DWORD range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }
    // A random float from 0 up to 1.
    float GetRandomF32ZeroToOne();
    // A random float from -1 up to 1. New in th08.
    float GetRandomF32MinusOneToOne();
    // A random float from 0 up to `range`.
    float GetRandomF32InRange(float range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }
    // A random float from -`range` up to `range`. New in th08; the copy
    // at 0x4143c0 is in an /Od object.
    float GetRandomF32SignedInRange(float range)
    {
        return this->GetRandomF32MinusOneToOne() * range;
    }

    WORD seed;
    // Where GameManager::OnUpdate keeps the seed while it rerolls the
    // checksum keys on pausing. New in th08.
    WORD savedSeed;
    // How many times the generator was advanced.
    DWORD generated;
};
extern Rng g_Rng;

// A 2D vector, as th07's.
struct Vector2
{
    // Sets x and y to `radius` along `angle`, with one fsincos (0x4286e0).
    void SetPolar(float angle, float radius);
    // SetPolar with a separate x and y radius (0x428700). New in th08.
    void SetPolarEllipse(float angle, float radiusX, float radiusY);

    float x;
    float y;
};

// a + b, brought back into [-pi, pi] (giving up after 17 turns).
float AddNormalizeAngle(float a, float b);

// Rotates `in` by `angle` radians in the xy plane into `out` (z is left
// alone). The opposite direction to th07's.
void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle);
// Sets *sinOut and *cosOut to the sine and cosine of `angle`, with one
// fsincos, as th07's. The compiler keeps an out-of-line copy (0x433880).
inline void SinCos(float *sinOut, float *cosOut, float angle)
{
    __asm
    {
        fld angle
        fsincos
        mov eax, cosOut
        fstp [eax]
        mov eax, sinOut
        fstp [eax]
    }
}

// How a D3DFORMAT's pixels are laid out, for a DIB section.
struct FormatInfo
{
    // A D3DFORMAT; -1 ends the table.
    int format;
    int bitCount;
    DWORD alphaMask;
    DWORD redMask;
    DWORD greenMask;
    DWORD blueMask;
};

// A pixel in D3DFMT_A1R5G5B5.
struct A1R5G5B5
{
    WORD b : 5;
    WORD g : 5;
    WORD r : 5;
    WORD a : 1;
};

// A GDI DIB section text is drawn into before it is copied to a Direct3D
// surface; `format` is the D3DFORMAT its pixels follow. As in th07.
class GdiSurface
{
public:
    GdiSurface();
    ~GdiSurface();
    // Selects the old object back into `hdc`, then deletes the DC and the
    // bitmap. Returns whether there was a DC.
    bool Release();
    // Creates the DIB section in `format`, or failing that in a 32-bit
    // format with the same channels. Returns whether it was created.
    bool Create(int width, int height, int format);
    // Creates the DIB section and its DC, after releasing any old ones.
    bool CreateInFormat(int width, int height, int format);
    // The entry of g_FormatInfos for `format`, or the end of the table
    // when it is missing; NULL for -1.
    FormatInfo *FindFormat(int format);
    // Copies the pixels into `surface` when it has the same format.
    // Returns false when there is no bitmap or the surface can't be
    // locked.
    bool CopyToSurface(IDirect3DSurface8 *surface);
    // Flips the alpha of the pixels in rows `y` on (GDI leaves alpha at
    // 0), taking `width` and `height` in 16-bit pixels. In A1R5G5B5 it
    // also shades the colors darker towards the bottom, a quarter as
    // much when `lightShading` is set, and makes the uncovered pixels
    // black. Returns false for other formats.
    bool InvertAlpha(int x, int y, int width, int height, int lightShading);
    // th07's inline getters, called out of line in th08 (0x43f828 on).
    void *GetBits();
    int GetWidthBytes();
    int GetHeight();
    HDC GetHdc();
    int GetWidth();
    int GetFormat();
    bool IsCreated();

    int format;
    int width;
    int height;
    DWORD imageSize;
    int widthBytes;
    HDC hdc;
    HGDIOBJ oldObject;
    HBITMAP bitmap;
    void *bits;
};
ASSERT_SIZE(GdiSurface, 0x24);
// The formats GdiSurface supports: X8R8G8B8, A8R8G8B8, X1R5G5B5, R5G6B5,
// A1R5G5B5 and A4R4G4B4.
extern FormatInfo g_FormatInfos[];

// The 1024 by 64 surface text is drawn on with GDI before it is copied
// into a texture (640 by 64 in th06).
extern IDirect3DSurface8 *g_TextSurface;

// Creates g_TextSurface.
void CreateTextSurface();
// Releases g_TextSurface, if there is one.
void ReleaseTextSurface();

// Draws `text` at `x` in "ＭＳ ゴシック" `fontHeight` high (not bold, unlike
// th07's), with a black outline when `shadowColor` is -1 and a wider
// black shadow otherwise, on g_TextSurface at twice the size, then scales
// `width` by `fontHeight` of it into `texture`, `destHeight` rows high at row
// `y`.
void RenderTextToTexture(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);

// th07's RenderTextToTexture, but semibold rather than bold. The other
// font of SpriteEngine's text rendering (0x466340).
void RenderTextToTextureBold(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                             COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);

class CSoundManager;
class CStreamingSound;

// A command PlayQueuedSounds carries out over several frames, for the
// streaming wave music. The same as th07's.
struct SoundCommand
{
    // 0 ends the queue.
    int type;
    int arg;
    // How far the command has got.
    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);

// An entry of thbgm.fmt: where a track is in thbgm.dat and its format.
// The same as th07's.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // CWaveFile::ResetFile limits the track to `loopEnd` bytes and, when
    // looping, skips its first `loopStart`. SoundPlayer::PlayQueuedSounds
    // fills the buffer with Loops() as the repeat flag.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;

    BOOL Loops()
    {
        return this->loopEnd != 0;
    }
};
ASSERT_SIZE(BgmFormat, 0x34);

// Plays the sound effects and music (at 0x18b8a68), as th07's. The queue
// of sound effects grew from 5 to 12 entries, each with up to 0x80
// arguments, and the music and sound effects got their own volumes.
class SoundPlayer
{
public:
    // Zeroes everything; no sound is loaded. An out-of-line copy at the
    // end of the sound object (0x45e320); th07's was a plain method.
    SoundPlayer()
    {
        int i;

        memset(this, 0, sizeof(*this));
        for (i = 0; i < 0x80; i++)
        {
            this->queuedSound[i] = -1;
        }
    }
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure.
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound.
    int Release();
    // Stops the wave music: ends the notification thread and releases
    // the stream.
    void StopBgm();
    // The index in bgmFormat of the track named as the file name of
    // `path`, or 0 if there is none.
    int FindTrackIndex(char *path);
    // Loads the wave file at `path` into soundBuffers[`index`]. Returns
    // 0, also when there is no sound, or -1 on failure.
    int LoadSound(int index, char *path);
    // Loads the track list at `path` (thbgm.fmt) into bgmFormat. Returns
    // 0 or -1.
    int LoadBgmFormat(char *path);
    // Starts streaming the wave music from the file at `path`
    // (thbgm.dat), as bgmFormat's first track, after stopping what
    // plays, and starts the thread that streams it. Returns 0 or -1.
    int LoadBgm(char *path);
    // Switches the wave music to the track named as the file name of
    // `path`. Returns 0, or -1 when no music is loaded.
    int SwitchBgmTrack(char *path);
    // Reads the track named as the file name of `path` out of bgmPath
    // into preload slot `slot`, unless it is there already. Does nothing
    // unless g_App.flags.preloadBgm is set. Returns 0 or -1.
    int PreloadBgm(int slot, char *path);
    // Starts the wave music from preload slot `slot`, as LoadBgm does,
    // or switches the stream to its track without g_App.flags.preloadBgm.
    // Returns 0 or -1.
    int PlayPreloadedBgm(int slot);
    // Loads the sound effects ("data/wav/se_plst00.wav", ...).
    int LoadSounds();
    // Carries out the commands' next stages and plays the queued sound
    // effects. Returns the first command's type: WinMain calls it until
    // it returns 0.
    int PlayQueuedSounds();
    // Fades the wave music out over `seconds`. An out-of-line copy at the
    // end of the sound object (0x45e3c0).
    void FadeOutBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeOut(seconds);
        }
    }
    // Fades the wave music in over `seconds`. New in th08; inline, the
    // copy is at 0x406ac0.
    void FadeInBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeIn(seconds);
        }
    }
    // The same for fades 4 (down) and 3 (up). New in th08; inline, the
    // copies are at 0x406b40 and 0x406bb0.
    void FadeDownBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeDown(seconds);
        }
    }
    void FadeUpBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeUp(seconds);
        }
    }
    // Queues the "Pause" command (6), as th07 did by hand. Inline; the
    // copy is at 0x406c30.
    void Pause()
    {
        this->QueueSoundCommand(6, 0, "Pause");
    }
    // Queues the "UnPause" command (7). Inline; the copy is at 0x406c50.
    void UnPause()
    {
        this->QueueSoundCommand(7, 0, "UnPause");
    }
    // Frees preload slot `slot`'s data, if any. An out-of-line copy at
    // the end of the sound object (0x45e370). Not in th07.
    void FreePreloadedBgm(int slot)
    {
        if (this->bgmPreloadData[slot] != NULL)
        {
            g_MemoryManager.Free(this->bgmPreloadData[slot]);
            this->bgmPreloadData[slot] = NULL;
        }
    }
    // Queues sound effect `idx` with `arg` to start on the next update:
    // a sound already queued collects up to 0x80 arguments, and up to 12
    // sounds are queued. th07's queued 5, without arguments.
    void QueueSound(int idx, int arg);
    // QueueSound with the pan for horizontal position `x`: -1000 at 0,
    // the left edge, to 1000 at 384. Not in th07.
    void QueueSoundPanned(int idx, float x);
    // Queues a command of `type` with `arg` and `path` in the first free
    // entry of `commands`.
    void QueueSoundCommand(int type, int arg, char *path);
    // Steps the BGM's fades, if there is a BGM: UpdateFade, then
    // UpdateFadeIn, UpdateFadeDown and UpdateFadeUp. In App's object
    // (0x447764); th07's App::OnUpdate called UpdateFade itself.
    void UpdateBgmFades();

    IDirectSound8 *dsound;
    DWORD unusedDword004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set when a sound is queued; -1 at first.
    int queuedSound[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD unusedDword61c;
    // The sound effects to play, or -1.
    int queue[0xc];
    // How many arguments each queued sound has, and the arguments. Not
    // in th07.
    int queueArgCounts[0xc];
    int queueArgs[0xc][0x80];
    // The thbgm.fmt entries of the preloaded tracks, their data, and
    // its size.
    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];
    // The preloaded track playing.
    int bgmPlaying;
    // The contents of thbgm.fmt.
    BgmFormat *bgmFormat;
    // The commands for the wave music, in order.
    SoundCommand commands[0x20];
    // The file names of the preloaded tracks.
    char bgmNames[0x10][0x100];
    // The file the wave music streams from.
    char bgmPath[0x100];
    // The wave music playing; NULL when stopped.
    CStreamingSound *bgm;
    // Signalled as the wave music plays.
    HANDLE notifyEvent;
    DWORD unusedDword5210;
    // Added to every BgmFormat offset by CWaveFile::ResetFile (th07's
    // unk_39c8).
    DWORD bgmFileOffset;
    // The music and sound effect volumes in percent, 100 at first, then
    // copied from the config (App+0x13f, App+0x140). Not in th07:
    // CSound::SetVolume scales by bgmVolume.
    int bgmVolume;
    int seVolume;
    DWORD bgmBufferVolume;
};
ASSERT_SIZE(SoundPlayer, 0x5224);
extern SoundPlayer g_SoundPlayer;

// What th07's GameManager points to at +0x8 and +0x4. Only the fields
// known so far.
// th07's GameStats, laid out anew.
struct GameManagerStats
{
    // The score shown.
    DWORD guiScore;
    // The grazes of the stage, as th07's graze.
    int graze;
    DWORD score;
    // The game's grazes.
    int grazeTotal;
    // How much GameManager::OnUpdate moves guiScore on per frame, as
    // th07's.
    DWORD scoreIncrement;
    // The high score to beat, from the score file, and what
    // LoadHighScores stores from the best entry, as th07's.
    DWORD highScore;
    BYTE bestContinuesUsed;
    unsigned char pad_19[0x3];
    int spellCardsCaptured;
    // A copy of unk_22, which is kept in each replay stage; AddGauge
    // moves it.
    short gaugeCopy;
    short gauge;
    int maxPointValue;
    char clock;
    BYTE continuesUsed;
    unsigned char pad_2a[0x2];
    int stageClearPoints;
    int pointItemsTotal;
    // Kept in each replay stage.
    int checksumKeyStage;
    int checksumKeyPoints;
    // GetStageClearClockStep compares them.
    int timeOrbs;
    int timeOrbsNeeded;
    int timeOrbsCollected;
    // Set to 6543 plus a random number under 100000 by
    // RerollStatsChecksumKeys, as th07's.
    DWORD lifeChecksumKeys[7];
    // Floats, as th07's.
    float deaths;
    float stageDeaths;
    // Set to random floats from 6543 up by RerollStatsChecksumKeys, as
    // statsChecksumKeysB, lifeChecksumKeysF and statsChecksumKeysC.
    float statsChecksumKeysA[2];
    // The lives left, a float behind checksums, as the other counts.
    float lives;
    float statsChecksumKeysB[2];
    // The bombs left, the same way.
    float bombs;
    float bombsUsed;
    float stageBombsUsed;
    float lifeChecksumKeysF[3];
    // The power, a float now.
    float power;
    float statsChecksumKeysC[2];
    // Player::Graze subtracts it from GameManager::bonusPoints, as th07's.
    int grazeCount;
    int checksumKeysD[8];
    // ChecksumBytes adds checksumKeysE[2] to it for every byte it sums.
    int checksum;
    // The checksum, from ComputeChecksum.
    int computedChecksum;
    DWORD checksumKeysE[5];
};
struct GameConfig
{
    unsigned char pad_0[0x1c];
    // The lives a game starts with, as th07's.
    BYTE lifeCount;
    unsigned char pad_1d[0x8];
    // App::IsScoreDisabled tests it.
    BYTE scoreDisabled;
    unsigned char pad_26[0x16];
};
ASSERT_SIZE(GameConfig, 0x3c);

// th07's GameManager (g_Unknown626270 there), at 0x160f508. Only the
// fields known so far.
class GameManager
{
public:
    // The update chain callback and its added and deleted callbacks,
    // as th07's GameManager's.
    static int OnUpdate(GameManager *mgr);
    // The update element's added callback, split as App's: clears or sets
    // App::unk_164 by the scene, sets unk_38, starts the loading sprites
    // (and, coming from the title, the loading screen effect), turns the
    // result screen's mode 2 and 3 into 1 and runs LoadingThread in the
    // sub thread for th07's AddedCallback proper. Returns 0.
    static int AddedCallback(GameManager *mgr);
    // The rest of th07's AddedCallback, run in the sub thread (with /Gr,
    // a fastcall function passed to CreateThread). Works on g_GameManager.
    static void LoadingThread(GameManager *arg);
    static int DeletedCallback(GameManager *mgr);
    // The draw chain callback: moves isPaused on from 1 to 2. Returns 1,
    // or 3 outside scene 2, in the result screen's mode (bits 5-6 of
    // unk_3dbac) 1 or while unk_38 is set.
    static int OnDraw(GameManager *mgr);
    // Zeroes the GameManager and sets arena to the play area (32, 16,
    // 384, 448) and demoIndex to 3. Emitted where g_GameManager is defined
    // (0x43c5e1).
    GameManager();
    // Whether the checksummed fields of `stats` no longer agree, as
    // th07's (0x40bb80).
    int ChecksumMismatch();
    // Rerolls the checksum keys lifeChecksumKeys[2] and checksumKeysD[3]
    // and recomputes the checksums, as th07's. Inline; the copy at
    // 0x406e50 is the ascii object's.
    void RerollChecksumKeys()
    {
        this->stats->lifeChecksumKeys[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->checksumKeysD[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->checksum = this->stats->lifeChecksumKeys[2];
        this->stats->computedChecksum = this->ComputeChecksum();
        this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
    }
    // Add `lives` to stats->lives and `bombs` to stats->bombs and
    // recompute the checksums; wipe g_App with 0xff first if the checksums
    // were already wrong, as th07's. Inline; the copies are at 0x43c641
    // (the GameManager object's) and 0x439883 (another object's).
    void AddLives(int lives)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->lives += lives;
        this->RerollChecksumKeys();
    }
    void AddBombs(int bombs)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->bombs += bombs;
        this->RerollChecksumKeys();
    }
    // The same for `power` and stats->power, as th07's. Inline; the copy
    // at 0x441850 is the item object's.
    void AddPower(int power)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->power += power;
        this->RerollChecksumKeys();
    }
    // Sets arena to the play area (32, 16, 384, 448) and arena2 to (8,
    // 16, 368, 416), as th07's. Inline; the copy at 0x43c686 is the
    // GameManager object's.
    void SetPlayArea()
    {
        this->arena[0] = 32.0f;
        this->arena[1] = 16.0f;
        this->arena[2] = 384.0f;
        this->arena[3] = 448.0f;
        this->arena2[0] = 8.0f;
        this->arena2[1] = 16.0f;
        this->arena2[2] = 368.0f;
        this->arena2[3] = 416.0f;
    }
    // The checksum: ChecksumBytes over stats from lifeChecksumKeys to
    // checksum, over stats->checksumKeysE, over the config unk_4 points
    // at, over g_App's config (0x3c bytes each) and, new in th08, over
    // hscr.
    int ComputeChecksum();
    // Whether the character plays alone (4 and up) and is the human of
    // its team (even) or the youkai (odd). Inline; the copies at 0x42f230
    // and 0x42f270 are in an /Od object.
    int IsSoloHuman()
    {
        return this->character >= 4 && !(this->character & 1) ? TRUE : FALSE;
    }
    int IsSoloYoukai()
    {
        return this->character >= 4 && (this->character & 1) ? TRUE : FALSE;
    }
    // Rerolls stats' lifeChecksumKeys and lifeChecksumKeysF, as th07's.
    // Inline; the copy at 0x44e160 is the player object's.
    void RerollLifeChecksumKeys()
    {
        this->stats->lifeChecksumKeys[0] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[1] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[2] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[3] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeys[4] = g_Rng.GetRandomU32InRange(100000) + 6543;
        this->stats->lifeChecksumKeysF[0] = g_Rng.GetRandomF32InRange(100000.0f) + 6543.0f;
        this->stats->lifeChecksumKeysF[1] = g_Rng.GetRandomF32InRange(100000.0f) + 6543.0f;
        this->stats->lifeChecksumKeysF[2] = g_Rng.GetRandomF32InRange(100000.0f) + 6543.0f;
    }
    // Adds `deaths` to stats->deaths and unk_68 and counts a miss in
    // hscr; adds `bombs` to stats->bombsUsed and unk_88. Both wipe g_App
    // first if the checksums were wrong, as th07's (which were out of
    // line). Inline; the copies at 0x44e260 and 0x44e2e0 are the player
    // object's.
    void AddDeaths(int deaths)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->deaths += deaths;
        this->stats->stageDeaths += deaths;
        this->hscr.misses++;
        this->RerollChecksumKeys();
    }
    void AddBombsUsed(int bombs)
    {
        if (this->ChecksumMismatch())
        {
            memset(&g_App, 0xff, sizeof(App));
        }
        this->stats->bombsUsed += bombs;
        this->stats->stageBombsUsed += bombs;
        this->RerollChecksumKeys();
    }
    // Sets stats->unk_22. Inline; the copy at 0x44e140 is the player
    // object's.
    void SetGauge(short value)
    {
        this->stats->gauge = value;
    }
    // Sets isPaused. Inline and expanded where it is used (SetUpTitleScene,
    // in the MainMenu object): that expansion is what leaves the 11 slots
    // MainMenu's too-large inline constructor reserves there.
    void SetIsPaused(BYTE value)
    {
        this->isPaused = value;
    }
    // Gives a life, or a bomb when the lives are full (8). Either one
    // plays sound 0x1c, adds 200 to the subrank and marks the Gui's
    // count changed, as th07's.
    void AddLifeOrBomb();
    // Whether a `width` by `height` box centred on (`x`, `y`) touches the
    // play area, as th07's.
    BOOL IsInPlayArea(float x, float y, float width, float height);
    // Sets the rank and its bounds from g_Ranks for the difficulty.
    void SetRankForDifficulty();
    // Adds `amount` hundredths to the rank, as th07's.
    void AddSubRank(int amount);
    // Takes `amount` hundredths from the rank, as th07's.
    void SubSubRank(int amount);
    // Unless g_PlayerBombRunning is set and `force` is not, adds `amount` to
    // stats->unk_22, within unk_3ddf8 and unk_3ddfa, and copies it to
    // stats->unk_20.
    void AddGauge(int amount, int force);
    // Whether shot type `shot` (0 to 3) cleared one of the four main
    // difficulties, as th07's (0x43c15f).
    int HasExtraOpen(int shot);
    // Whether a shot type has Extra open (0x43c1e9).
    int AnyShotHasExtraOpen();
    // Whether shot type `shot` (0 to 3) has bit 0x8000 set in
    // stagesCleared of one of the four main difficulties.
    int HasClearedFinalA(int shot);
    // Whether a shot type has 0x43c23b set (0x43c2c5).
    int AnyShotHasClearedFinalA();
    // Whether every shot type has Extra open. In App's object.
    int AllShotsHaveExtraOpen();
    // Whether spell card `card` was attempted, in a game or in practice,
    // or, for a last spell, seen (flsp). The copy at 0x46fd5f is the
    // title menu object's.
    int IsSpellCardSeen(int card);
    // th07's IsPhantasmOpen, which returns 0 in th08 (0x43c317).
    int IsPhantasmOpen();
    // Whether a replay (bit 3 of unk_3dbac) of a practice is playing.
    int IsPracticeReplay();
    // By stage: 0 for 6 and 7, 4 past them, else 1, or 2 while stats'
    // unk_3c is below unk_40.
    int GetStageClearClockStep();
    // Moves `stage` on to the next stage: from stage 3 (index 2) to 4A or
    // 4B by the shot type, and from 6 to 6B if bits 11-12 of unk_3dbac
    // are set, else 6A (0x43c4b3).
    void AdvanceStage();
    // stats->unk_22, and whether it reached each of gaugeLow1, gaugeLow2,
    // gaugeHigh1 and gaugeHigh2. Inline; the copies at 0x406cf0 and
    // 0x406d10 to 0x406da0 are in an /Od object.
    int GetGauge()
    {
        return this->stats->gauge;
    }
    BOOL IsGaugeAtLow1()
    {
        return this->stats->gauge <= this->gaugeLow1;
    }
    BOOL IsGaugeAtLow2()
    {
        return this->stats->gauge <= this->gaugeLow2;
    }
    BOOL IsGaugeAtHigh1()
    {
        return this->stats->gauge >= this->gaugeHigh1;
    }
    BOOL IsGaugeAtHigh2()
    {
        return this->stats->gauge >= this->gaugeHigh2;
    }
    // stats->unk_3c and unk_40, which DrawHud shows as "%d/%d" on its last
    // row, in 0xfffff0c0 once unk_3c reaches unk_40. Inline; the copies at
    // 0x421b60 and 0x421b80 are in an /Od object.
    int GetTimeOrbs()
    {
        return this->stats->timeOrbs;
    }
    // Adds `amount` time orbs (stats->unk_3c; a negative amount larger
    // than what there is clears them instead) to unk_3c, stats->unk_44
    // and hscr.timeOrbs; a gain also raises stats->unk_24 by 10 per two,
    // rounding with unk_44's parity. Inline; the copy at 0x418220 is in
    // an /Od object.
    void AddTimeOrbs(int amount)
    {
        if (amount >= 0 || this->stats->timeOrbs >= -amount)
        {
            this->stats->timeOrbs += amount;
            this->stats->timeOrbsCollected += amount;
            this->hscr.timeOrbs += amount;
            this->RerollChecksumKeys();
            if (amount > 0)
            {
                amount += this->stats->timeOrbsCollected & 1;
                this->stats->maxPointValue += amount / 2 * 10;
            }
        }
        else
        {
            this->stats->timeOrbs = 0;
        }
    }
    int GetTimeOrbsNeeded()
    {
        return this->stats->timeOrbsNeeded;
    }
    // Whether a demo plays: unk_3dbac's bit 1, as th07's stateFlags.
    // Inline; the copy at 0x406cb0 is in an /Od object.
    int IsDemo()
    {
        return this->stateFlags >> 1 & 1;
    }
    // Whether the game is a practice: unk_3dbac's bit 0, which th07 read
    // directly. Inline; the copy at 0x406cd0 is in an /Od object.
    int IsInPractice()
    {
        return this->stateFlags & 1;
    }
    // The deaths and bombs used, as ints. New in th08; inline, the copies
    // at 0x453c80 and 0x453ca0 are the replay object's.
    int GetDeaths()
    {
        return this->stats->deaths;
    }
    int GetBombsUsed()
    {
        return this->stats->bombsUsed;
    }
    // Adds `points` to stats->score, which counts in tens, as th07's.
    // Inline; the copy at 0x4181f0 is in an /Od object.
    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }
    // The power, lives and bombs as ints, and setters for them (as ints;
    // they also update what depends on them). Copies in other objects:
    // GetPower 0x422480, GetLives 0x42f2b0, GetBombs 0x4398ce, SetLives
    // 0x406e20, SetBombs 0x406f30, SetPower 0x406fa0.
    int GetPower()
    {
        return this->stats->power;
    }
    // From `lo` to `hi` by the rank (0 to 32), as th07's free RankLerp
    // and RankLerpF. Inline; the copies at 0x421ba0 and 0x422b80 are an
    // /Od object's.
    int RankLerp(int lo, int hi)
    {
        return this->rank * (hi - lo) / 32 + lo;
    }
    float RankLerpF(float lo, float hi)
    {
        return this->rank * (hi - lo) / 32.0f + lo;
    }
    int GetLives()
    {
        return this->stats->lives;
    }
    int GetBombs()
    {
        return this->stats->bombs;
    }
    void SetLives(int lives)
    {
        this->stats->lives = lives;
        this->RerollChecksumKeys();
    }
    // SetBombs recomputes the checksums without new keys. Inline; the
    // copy at 0x406f30 is the ascii object's.
    void SetBombs(int bombs)
    {
        this->stats->bombs = bombs;
        this->stats->checksum = this->stats->lifeChecksumKeys[2];
        this->stats->computedChecksum = this->ComputeChecksum();
        this->checksumValue = this->stats->computedChecksum + this->stats->checksumKeysD[3];
    }
    // Sets stats->power (a float) and rerolls the checksum keys. Inline;
    // the copy at 0x406fa0 is the ascii object's.
    void SetPower(int power)
    {
        this->stats->power = power;
        this->RerollChecksumKeys();
    }
    // stats->unk_28 and a setter for it, inline: the copies at 0x406dd0
    // and 0x453c60 are another object's and the replay object's.
    char GetClock()
    {
        return this->stats->clock;
    }
    void SetClock(int value)
    {
        this->stats->clock = value;
    }
    // Adds `amount` to stats->unk_28. Inline; the copy at 0x406df0 is in
    // an /Od object.
    void AddClock(char amount)
    {
        this->stats->clock += amount;
    }
    // Whether a spell card is practised (bit 14 of unk_3dbac). Inline;
    // the copy at 0x406c70 is an /Od object's.
    int IsInSpellCardPractice()
    {
        return this->stateFlags >> 14 & 1;
    }
    // Whether the spell card practised (bit 14 of unk_3dbac) is `card`,
    // or one from `first` to `last`. Inline.
    int IsPracticingSpellCard(int card)
    {
        return this->stateFlags >> 14 & 1 ? this->practiceSpellCard == card : FALSE;
    }
    int IsPracticingSpellCardIn(int first, int last)
    {
        return this->stateFlags >> 14 & 1 ? (this->practiceSpellCard >= first && this->practiceSpellCard <= last ? TRUE : FALSE)
                                         : FALSE;
    }
    // Whether shot type `shot` cleared stage bit `stage` of `difficulty`
    // without continues, or at all. Inline; the copies at 0x439829 and
    // 0x439856 are the Gui object's.
    int HasClearedStage(int stage, int shot, int difficulty)
    {
        return this->clrds[shot].stagesClearedNoContinue[difficulty] & (1 << stage);
    }
    int HasClearedStageWithContinues(int stage, int shot, int difficulty)
    {
        return this->clrds[shot].stagesCleared[difficulty] & (1 << stage);
    }
    // Whether a replay plays (bit 3 of unk_3dbac). Inline; the copy at
    // 0x406c90 is an /Od object's.
    void SetIsReplay(int isReplay)
    {
        this->isReplay = isReplay;
    }
    int IsReplay()
    {
        return this->stateFlags >> 3 & 1;
    }

    // A block of random size LoadingThread allocates and frees at once,
    // as th07's tempBuffer.
    void *tempBuffer;
    GameConfig *savedConfig;
    GameManagerStats *stats;
    // The score file's chunks, loaded by App::AddedCallback.
    Flsp flsp;
    // Set while the game is paused: the screen shakes hold still. As
    // th07's isFrozen.
    char isFrozen;
    // As th07's unk_d: while it is set, the frame rate is not measured
    // and the buttons are only added to.
    char skipFpsMeasure;
    unsigned char pad_2e[0x2];
    int difficulty;
    // 1 << difficulty, or 0xf from Extra on, as th07's.
    int difficultyMask;
    // OnDraw returns 3 while it is set; OnUpdate returns 4 while it is 2
    // and counts unk_3c up while it is otherwise set.
    int loadingThreadState;
    int loadingFrames;
    Catk catks[0xde];
    // A copy of catks, as th07's savedCatks.
    Catk savedCatks[0xde];
    Clrd clrds[0xd];
    Pscr pscrs[0xc];
    // The play statistics of the score file.
    Plst plst;
    // The score entry of the game being played: HandleNameEntry fills
    // in the rest and enters a copy. New in th08.
    Hscr hscr;
    DWORD playFrames;
    // Set when the game is paused; the replay records it (as th07's).
    int pauseRequested;
    // The scene OnUpdate switches to when the result screen's mode ends,
    // or -1.
    int nextScene;
    int humanFrames;
    int humanRateFrames;
    BYTE powerItemCombo;
    // As th07's: the shot type is character + shotType.
    BYTE character;
    BYTE shotType;
    // character + shotType, as th07's shotIndex.
    BYTE shotIndex;
    union
    {
        // Bit 2: the speed is measured; 3: it is shown, as th07's
        // unk_93d8.
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits3dbac_0 : 2;
            // Set while neither paused nor in the continue menu, as
            // th07's isPlaying.
            unsigned int isPlaying : 1;
            // IsReplay, as th07's isInReplay.
            unsigned int isReplay : 1;
            unsigned int isGameCleared : 1;
            // The result screen's mode: OnDraw returns 3 in mode 1, and
            // AddedCallback turns 2 and 3 into 1.
            unsigned int stageEndState : 2;
            // Gui::DrawHud draws GuiImpl::unk_5484 with the lives and
            // bombs while it is 1 and g_SpellCard.IsActive().
            unsigned int lastSpellState : 2;
            unsigned int stageClearStarted : 1;
            // The screen shakes hold still while it is set.
            unsigned int deathbombFreeze : 1;
            // The clear level GuiImpl::StartMsgScript picked for the
            // ending dialogue: AdvanceStage goes to 6B unless it is 0.
            unsigned int clearLevel : 2;
            unsigned int bossHitAnywhere : 1;
            // Set while a spell card is practised (IsInSpellCardPractice).
            unsigned int isInSpellCardPractice : 1;
            // AnyShotHasExtraOpen, AnyShotHasClearedFinalA and AllShotsHaveExtraOpen
            // as App::AddedCallback found them.
            unsigned int anyShotHasExtraOpen : 1;
            unsigned int anyShotHasClearedFinalA : 1;
            unsigned int allShotsHaveExtraOpen : 1;
            unsigned int unusedBits3dbac_18 : 14;
        };
    };
    // The spell card being practised, kept in the replay when bit 14 of
    // unk_3dbac is set.
    short practiceSpellCard;
    // While it is set, PresentFrame keeps redrawFrames and fade outs
    // hold still. As th07's isPaused.
    BYTE isPaused;
    // Set while the continue menu shows, as th07's: fade outs hold
    // still.
    BYTE isInContinueMenu;
    // The demo being played (0 to 3), as th07's.
    BYTE demoIndex;
    // 2 while a replay is fast-forwarded, as th07's.
    BYTE replayMenuChoice;
    unsigned char pad_3dbb6[0x2];
    // The frames a demo has played, as th07's.
    int demoFrames;
    // The replay file to play, as th07's.
    char replayFile[0x200];
    // The random seed each replay stage starts with, as th07's.
    WORD rngSeed;
    unsigned char pad_3ddbe[0x2];
    // The screen shakes end at once while this is 1 or less, as th07's.
    int stageFrames;
    // The stage being played, from 0.
    int stage;
    // The replay stage being recorded.
    int replayStage;
    unsigned char pad_3ddcc[0x4];
    // ORed into the shot type's stage clear flags when a game ends.
    WORD stageBit;
    unsigned char pad_3ddd2[0x2];
    // The play area: x, y, width and height, and a second rectangle set
    // with it by SetPlayArea, as th07's.
    float arena[4];
    float arena2[4];
    // As th07's checksumValue: SetBombs sets it.
    float checksumValue;
    // The bounds of stats->unk_22.
    short gaugeMin;
    short gaugeMax;
    // Where AsciiText::DrawPopups starts tinting the stats->unk_22
    // gauge: blue at or below gaugeLow1 (light blue at gaugeLow2), red at
    // or above gaugeHigh1 (light red at gaugeHigh2).
    short gaugeLow1;
    short gaugeHigh1;
    short gaugeLow2;
    short gaugeHigh2;
    // The frames played, as th07's.
    int progressFrames;
    // Counts the frames that slowdown may skip, as th07's.
    DWORD slowdownCounter;
    unsigned char pad_3de0c[0x4];
    // Cleared by LoadingThread.
    int gaugeFramesTotal;
    // Frames counted for the stage clear screen's "over 80%" lines:
    // all of them, and the two kinds counted.
    int gaugeFrames;
    int highGaugeFramesTotal;
    int lowGaugeFramesTotal;
    int highGaugeFrames;
    int lowGaugeFrames;
    // Set when the stage music is to start: OnUpdate stops the loading
    // screen and, while it is 1, plays the stage's music.
    int pendingMusicStart;
    // A replay stage keeps it. The rank, as th07's: SetRankForDifficulty
    // sets it and its bounds from g_Ranks.
    int rank;
    int maxRank;
    int minRank;
    // Hundredths of a rank, moved by AddSubRank and SubSubRank.
    int subRank;
};
ASSERT_SIZE(GameManager, 0x3de3c);
extern GameManager g_GameManager;

// The update and draw chain elements of g_GameManager, after it.
extern ChainElem g_GameManagerCalcChain;
extern ChainElem g_GameManagerDrawChain;

// The rank a game starts at and its bounds, as th07's.
struct RankSettings
{
    int rank;
    int minRank;
    int maxRank;
};
// By difficulty.
extern RankSettings g_Ranks[6];

// The sum of the `size` bytes at `data`. Adds
// g_GameManager.stats->checksumKeysE[2] to its checksum for every byte, as
// th07's.
int ChecksumBytes(BYTE *data, int size);
// Fills the checksum keys in g_GameManager.stats with random numbers
// from 6543 up, then stores the checksum, as th07's.
void RerollStatsChecksumKeys();
// The score loading of th07's LoadScoreFile: saves the score file
// (ResultScreen::RegisterChain(2)), resets the spell cards, then loads the
// high score, the spell cards, the clears and the practice scores from
// "score.dat" (a practice takes its high score from the practice scores,
// counts the play and marks the shot type's practice scores to be saved)
// and keeps a copy of the spell cards in savedCatks. New in th08: then
// starts hscr with the shot type, difficulty and config. Returns 0, or -1
// if the file cannot be read.
int LoadScoreFile();
// Adds one to `*count` while it is under 999999. Its callers pass
// 999999 as `max`, which it ignores.
void IncrementStatCounter(DWORD *count, DWORD max);

// A dialogue file. Only what is known so far.
struct MsgFile
{
    int count;
    // The scripts, by index.
    void *scripts[1];
};

// The arguments of a message instruction, as th07's: who it is for (a
// portrait 0 to 3; for opcode 3 the speaker, whose colours it takes) and a
// sprite, script or line number, then the text. Opcodes 16, 19 and 20
// have only the text.
struct MsgArgs
{
    short who;
    union
    {
        short index;
        BYTE value;
    };
    char text[1];
};

// The arguments of opcodes 15 and 17: the portrait that speaks, then a
// sprite for each portrait (negative to keep it). New in th08.
struct MsgPortraitArgs
{
    int who;
    int sprites[4];
};

// One instruction of a message script, as th07's.
struct MsgInstr
{
    // The frame it runs on.
    WORD time;
    BYTE opcode;
    // The size of the arguments, in bytes.
    BYTE argsSize;
    union
    {
        MsgArgs args;
        MsgPortraitArgs portraitArgs;
        int i;
        BYTE b;
    };
};

// The dialogue state, as th07's GuiMsg (0x1570 bytes now).
struct GuiMsg
{
    MsgFile *msgFile;
    // The instruction being run.
    MsgInstr *instr;
    // The script running, -1 for none.
    int scriptIndex;
    Timer timer;
    int waitFrames;
    // How long opcodes 4 and 21 wait before the shot button ends them: 30
    // once a wait ran out, 8 once the button ended one. New in th08.
    int minWaitFrames;
    Sprite portraits[4];
    Sprite text[2];
    Sprite nameplates[2];
    // The text and shadow colours of the two lines, by character now.
    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];
    int fontSize;
    // While it is above 0, the dialogue does not hold the timeline.
    DWORD timelineRelease;
    // Whether the dialogue can be skipped.
    BYTE skippable;
    // The portrait that speaks (opcodes 15 and 17): opcode 16 draws its
    // text in its colours.
    BYTE speaker;
    // Set when the speaker changes or a wait ends: opcode 16 then clears
    // the second line and starts again from the first.
    BYTE newPage;
    // The line opcode 16 writes next.
    BYTE nextLine;
    // The portrait shown in front (interrupt 3), 0xff for none.
    BYTE activePortrait;
    // Set by opcode 18; DrawMsg tests it.
    BYTE textBoxShown;
    // The Extra/clear level StartMsgScript picks for the ending dialogue
    // (index 10 of stage 6), kept in bits 11-12 of
    // GameManager::unk_3dbac. Opcode 21 lets the player pick it (up and
    // down) and opcode 22 keeps it and starts script 1 or 2.
    BYTE clearLevel;
    unsigned char pad_156f[0x1];
};
ASSERT_SIZE(GuiMsg, 0x1570);

// A value shown for a while at a position, as th07's GuiPopup.
struct GuiPopup
{
    D3DXVECTOR3 pos;
    int value;
    // Whether it shows, or what it shows.
    int shown;
    Timer timer;
};
ASSERT_SIZE(GuiPopup, 0x20);

// The Gui's state behind Gui::impl, as th07's GuiImpl (0x230b8 bytes,
// "FRScreenImplInf"). Only the fields known so far.
struct GuiImpl
{
    // Starts dialogue script `index`, as th07's: clears msg (keeping the
    // file), picks the ending dialogue by the clears for index 10 of
    // stage 6 and a shorter one past index 5 of stage 6B while
    // stats->unk_28 is 12 or more, sets the dialogue's defaults and
    // clears the bullets, the enemies and the items (0x43396d).
    void StartMsgScript(int index);
    // Runs the dialogue script, as th07's (0x433db3).
    int RunMsg();
    // Draws the dialogue, as th07's (0x43542b).
    int DrawMsg();

    Sprite sprites[0x10];
    // The boss's bar (sprites[12]): 0 hidden, 1 appearing, 2 shown, 3
    // leaving, as th07's.
    BYTE bossBarState;
    unsigned char pad_2a41[0x3];
    Sprite trackNumberSprites[4];
    Sprite stageTitleTime;
    Sprite loadingIcon;
    Sprite loadingCaption;
    Sprite loadingBar;
    Sprite loadingBarTrail[8];
    Sprite livesBombsOverlay;
    Sprite backgroundTiles[0xa8];
    Sprite timePassedDigit;
    Sprite clock;
    int backgroundTileCount;
    GuiMsg msg;
    // Gui::OnDraw draws the stage clear screen (DrawStageClear) while it
    // is set, as th07's.
    int stageClearShown;
    // The stage clear screen's total, in tens, as th07's.
    int stageClearTotal;
    // The bonus, full power and spell card bonus popups, as th07's.
    GuiPopup bonusPopup;
    GuiPopup fullPowerPopup;
    GuiPopup spellBonusPopup;
    // The stage clear screen's clear bonus (in tens), point items,
    // grazes and time bonus count, as th07's.
    int stageClearBonus;
    int stageClearPower;
    int stageClearPoints;
    int stageClearGraze;
    int stageClearTime;
    int stageClearTimePassed;
    // Clock times in minutes (h * 60 + m): shown from stage 1 to 6 as two
    // times, and from stage 6B as the last time for the night bonus.
    int stageClearClock;
    int stageClearLastTime;
    int stageClearClock2;
    int stageClearClockTimer;
    Sprite difficultyLabel;
};
ASSERT_SIZE(GuiImpl, 0x230b8);

// The game's HUD and dialogue, at 0x160f428 (0x9c bytes), as th07's.
// Only the fields known so far.
class Gui
{
public:
    // The update chain callback, as th07's: unless g_GameManager.isFrozen
    // is set, updates the interface and the dialogue, keeps
    // g_App.redrawFrames at 8 or more while button 0x100 is held and
    // counts frameCounter up. Returns 1.
    static int OnUpdate(Gui *gui);
    // The draw chain callback: DrawStageClear while impl->stageClearShown is set,
    // then the dialogue, the banners, the HUD and the popups. Returns 1.
    static int OnDraw(Gui *gui);
    // Whether the dialogue holds the timeline: impl is set,
    // msg.timelineRelease is 0 and a script runs, as th07's.
    BOOL MsgHoldsTimeline();
    // Whether impl->loadingIcon shows a sprite whose script waits for an
    // interrupt, as th07's IsLoadingShown (0x437d87).
    BOOL IsLoadingShown();
    // Show `bonus` in bonusPopup at (416, 48), `points` in fullPowerPopup
    // at (416, 168) shown as `kind`, and `value` in spellBonusPopup at
    // (224, 16); each sets g_App.redrawFrames to 2. As th07's (0x437ddd,
    // 0x437e5d, 0x437edc).
    void ShowBonusPopup(int bonus);
    void ShowFullPowerPopup(int points, int kind);
    void ShowSpellCardBonus(int value);
    // Whether a dialogue runs, and whether it can be skipped, as th07's
    // (0x4358bb, 0x437dc7).
    int IsMsgRunning();
    int IsMsgSkippable();
    // Updates the HUD's sprites and values, as th07's (0x435900).
    void UpdateInterface();
    // Draws the HUD and the banners, as th07's (0x43625d, 0x43741d).
    void DrawHud();
    void DrawBanners();
    // Draws the stage clear screen, as th07's OnDraw did (0x43826b).
    void DrawStageClear();
    // Draws the bonus, full power and spell card bonus popups, as th07's
    // OnDraw did (0x438a89).
    void DrawPopups();
    // Called by GameManager::OnUpdate when the result screen's mode ends
    // without a scene to go to (0x438f58).
    void StartLoadingBar();
    // Restart the clock sprite (impl->unk_2156c) of times.anm on the
    // hour (GameManager::GetClock), show the hour with interrupt 1 or 2,
    // or hide it (0x439007, 0x439050, 0x439093, 0x4390d6). Return 0.
    int StartClock();
    int AdvanceClock();
    int AdvanceClockToEnd();
    int HideClock();
    // Loads the interface's sprites and the stage's portraits and
    // messages, as th07's (0x4390ee). Returns 0, or -1 when a file fails
    // to load.
    int LoadResources();
    // Hides loadingIcon, loadingCaption and loadingBar (spriteIndex -1)
    // and clears impl->backgroundTileCount, as th07's (0x4396b8).
    void HideInterface();
    // Loads the dialogue file `path` and makes its script offsets
    // pointers, as th07's (0x439710). Returns 0, or -1 if it fails to
    // load.
    int LoadMsg(char *path);
    // Frees the dialogue file, as th07's (0x4397d5).
    void FreeMsgFile();
    // Starts dialogue script `index`, as th07's (0x439810).
    void StartMsg(int index);
    // Sets bossHealth, as th07's. Inline; the copy at 0x4230c0 is an /Od
    // object's.
    void SetBossHealth(float health)
    {
        this->bossHealth = health;
    }
    // Sets bossTimer, as th07's. Inline; the copy at 0x42f340 is an /Od
    // object's.
    void SetBossTimer(int seconds)
    {
        this->bossTimer = seconds;
    }
    // Sets section `idx` of the boss bar, as th07's. Inline; the copy at
    // 0x4230e0 is an /Od object's.
    void SetBossSection(int idx, float end, float start)
    {
        this->sectionEnds[idx] = end;
        this->sectionStarts[idx] = start;
    }
    // Sets the color of boss bar section `idx`, as th07's. Inline; the
    // copy at 0x423110 is an /Od object's.
    void SetBossSectionColor(int idx, int color)
    {
        this->sectionColors[idx] = color;
    }
    // Sets bossLives, as th07's. Inline; the copy at 0x423130 is an /Od
    // object's.
    void SetBossLives(int lives)
    {
        this->bossLives = lives;
    }
    // Sets bossPresent and returns `present`. New in th08. Inline; the
    // copy at 0x422c20 is an /Od object's.
    BYTE SetBossPresent(BYTE present)
    {
        this->bossPresent = present;
        return present;
    }
    // Whether a boss fight runs, as th07's. Inline; the copy at
    // 0x42f320 is an /Od object's.
    BYTE IsBossPresent()
    {
        return this->bossPresent;
    }
    // The update element's added callback: runs LoadResources.
    static int AddedCallback(Gui *gui);
    // Releases ANM slot 0xd unless GuiIsRestarting, frees the dialogue file
    // and, if GuiIsFreeingResources, releases ANM slots 0xa to 0xe and frees impl.
    // Returns 0.
    static int DeletedCallback(Gui *gui);
    // Adds the Gui's chain elements, as th07's (0x437ad0): if
    // GuiIsLoadingResources, zeroes the Gui and allocates impl
    // ("FRScreenImplInf"); then adds the update element (priority 15)
    // and the draw element (priority 17). Returns 0, or -1 if adding
    // fails.
    static int RegisterChain();
    // Removes the Gui's chain elements, as th07's (0x437d64).
    static void RemoveChain();

    // The frames the Gui has run, as th07's.
    int frameCounter;
    // Set to 2 when the lives or bombs change, as th07's.
    unsigned int livesChanged : 2;
    unsigned int bombsChanged : 2;
    unsigned int powerChanged : 2;
    unsigned int grazeChanged : 2;
    unsigned int frameItemsHideTimer : 2;
    unsigned int timeOrbsChanged : 2;
    GuiImpl *impl;
    // front.anm (ANM slot 10), as th07's ANM slot.
    AnmLoaded *anm;
    // The stage's text ANM (slot 0xd), as th07's stdNtxt.anm.
    AnmLoaded *textAnm;
    // times.anm (ANM slot 0xe), with the clock, and the character's
    // loading ANM (slot 0xc).
    AnmLoaded *timesAnm;
    AnmLoaded *characterAnm;
    // The boss bar's alpha, as th07's.
    DWORD bossBarAlpha;
    // How many boss lives DrawBanners marks beside the bar.
    int bossLives;
    // The seconds left on the boss's timer.
    int bossTimer;
    // bossTimer as DrawBanners last drew it: it plays sound 0x1d when
    // the timer changes under 10 (0x26 under 3, new in th08).
    int lastDrawnBossTimer;
    // Set while a boss fight runs.
    BYTE bossPresent;
    unsigned char pad_2d[0x3];
    // The boss's life as a fraction of its maximum, and what the bar
    // shows, following it.
    float bossHealth;
    float bossHealthShown;
    unsigned char pad_38[0x4];
    // Eight sections DrawBanners draws over the boss bar: where each
    // ends and starts, as fractions of the bar (unused while the end
    // is 0), and its color.
    float sectionStarts[8];
    float sectionEnds[8];
    int sectionColors[8];
};
ASSERT_SIZE(Gui, 0x9c);
extern Gui g_Gui;

// The update and draw chain elements of g_Gui.
extern ChainElem g_GuiCalcChain;
extern ChainElem g_GuiDrawChain;

// Whether the game restarts (App::unk_16c), so the stage keeps its
// loaded ANMs. Inline; the copy at 0x40b900 is an /Od object's.
inline int IsRestarting()
{
    return g_App.isRestarting;
}

// App::unk_164 and unk_168: whether the scene's objects load their
// files as they are added, and free them as they are deleted. Inline;
// the copies at 0x4338b0 and 0x4338c0 are an /Od object's.
inline int IsLoadingResources()
{
    return g_App.loadsResources;
}
inline int IsFreeingResources()
{
    return g_App.freesResources;
}

// Whether the scene's objects keep the files they loaded
// (!IsLoadingResources()). Inline; the copy at 0x44e0e0 is the player
// object's.
inline int KeepsResources()
{
    return !IsLoadingResources();
}

// App::unk_164, unk_168 and unk_16c (0x438fe9, 0x438ff3, 0x438ffd).
int GuiIsLoadingResources();
int GuiIsFreeingResources();
int GuiIsRestarting();

// Decodes a dialogue string (each byte xor 0x77) from `src` into `dst`,
// up to and including the terminating 0 (0x4353ec).
void DecodeMsgString(char *dst, char *src);

// Has the back buffer's play area copied to texture 10 at sprite
// `sprite` of ANM slot 10 (0x437f5c).
void CopySpriteToTexture(int sprite);
// The character's loading ANM, the stages' text ANMs (the last for
// Extra's last spell cards) and the dialogue files by stage and
// character.
extern char *g_LoadingAnmPaths[];
extern char *g_StageTextAnmPaths[];
extern char *g_MsgPaths[][12];
// "AM" and "PM", picked by whether the hour is before 12 (backwards).
extern char *g_AmPmNames[2];
// The colors of the boss timer: 20 seconds and more, 10, 5 and under 5,
// as th07's.
extern D3DCOLOR g_BossTimerColors[4];
// The stage clear bonus, by stage.
extern int g_StageClearBonuses[];
// The dialogue's text colours, by character.
extern D3DCOLOR g_MsgColors[][4];

// The thread LoadBgm starts, with the main window: streams the wave music
// each time notifyEvent is signalled, until WM_QUIT or the music is gone.
DWORD WINAPI BgmNotificationProc(LPVOID lpParameter);

// Each sound effect's sound buffer and volume, as in th07.
struct SoundInfo
{
    int buffer;
    short volume;
    short priority;
};
// 0x2e sound effects (th07 0x26).
extern SoundInfo g_SoundInfos[0x2e];
// The sound files LoadSounds loads, by index: 0x24 (th07 0x1e).
extern char *g_SoundPaths[];

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
    short unusedShort06;
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
    short unusedShort02;
    D3DXVECTOR3 pos;
};

// The start of the stage data file, as th07's.
struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD unusedDword0c;
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
// stage->cameraFadeTimer[i], eased by stage->cameraFadeMode[i]: 1 to 3 slow down, 4
// to 6 speed up, 7 follows a Hermite curve with tangents m0 and m1.
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
    // first spawns unk_ae8 and runs its script by waitLabel. Then fades
    // the cameras and the fog, shakes the camera, runs the sprites and
    // spawns effect 0x3e at the first 12 of unk_6480 every third frame
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
    // Clears unk_6478 and the first 0x10 of unk_6480, sets the play area
    // viewport, resets the SpriteEngine's render state, clears the play
    // area to black once if clearScreenOnce is set, makes the
    // SpriteEngine modulate by tintColor if its alpha is set (and resets
    // it to gray), draws backgroundLayer0, backgroundLayer1 and
    // unk_ae8, clears to clearColor (through DrawColorRect unless it is
    // opaque), sets the fog (its color modulated as the SpriteEngine's)
    // and draws layers 0 and 1 under the same conditions as OnDraw2.
    // Returns 1. As th07's, with th08's additions.
    static int OnDraw(Stage *stage);
    // Draws the objects on `layer` near enough the camera, as th07's
    // (0x40a1b0). Returns 0.
    // Draws layers 2 and 3 unless spellCardState is above 1 or the Gui's
    // loading sprite shows, with a rectangle fading in while
    // spellCardState is 1; then dialogueSprites's first
    // dialogueSpriteCount sprites and unk_625c, the fixed camera
    // (SetupFixedCamera) and the fog at 1000 to 2000. Resets the
    // SpriteEngine's color unless keepTintColor was set, and clears it
    // and unk_647c. Returns 1. As th07's.
    static int OnDraw2(Stage *stage);
    int DrawLayer(int layer);
    // Resets the script, loads the stage's background ANM into slot 4
    // (or takes the one already there when restarting) and its data
    // (the spell practice version in spell practice), and sets the fog
    // and the cameras up as the constructor, with draw distance
    // unk_6470 by stage. Returns 0, or -1 if loading fails. As th07's.
    static int AddedCallback(Stage *stage);
    // Loads the stage data from `dataPath` (keeping the old data when
    // restarting) and starts the background sprites' scripts, as
    // th07's, then starts unk_844 (0x409ce0). Returns 0, or -1 on
    // failure.
    int Load(char *dataPath);
    // Sends unk_844 interrupt 1 or 2 as g_Player.unk_5 is set or
    // cleared, runs it, then runs one frame of the scripts of each
    // animated object's sprites, as th07's; objects whose last sprite has
    // autoRotate 1 get unk_844's color as color2 (0x409f40). Returns 0.
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
    // Follows g_Player.unk_5: set, unk_844 gets interrupt 1, cleared,
    // interrupt 2. New in th08.
    BYTE colorScaleOn;
    unsigned char pad_835[0x3];
    // Restarted whenever unk_834 changes. New in th08.
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
    Sprite unusedSprite5fb8;
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
    int keepTintColor;
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
extern Stage g_Stage;
// Stage's statically allocated chain elements.
extern ChainElem g_StageCalcChain;
extern ChainElem g_StageDrawChain;
extern ChainElem g_StageDrawChain2;
// The background ANM of each stage (4A and 4B share one).
extern char *g_StageAnmPaths[9];
// The stage data file of each stage.
extern char *g_StageDataFiles[9];
// The same in spell practice ("stage1_s.std", ...). New in th08.
extern char *g_SpellPracticeStageDataFiles[9];

// Each stage's music: its BGM numbers for App::PlayMidi, the first for
// the stage.
struct StageBgm
{
    int stageBgm;
    int bossBgm;
    int bossBgm2;
};
extern StageBgm g_StageBgms[];
// The music a practised spell card plays, by card number: entries up to
// and including lastCard play bgm. Ends with lastCard -1.
struct SpellCardBgm
{
    int lastCard;
    int bgm;
    char *midiPath;
    int titleIndex;
    int restartsMusic;
};
extern SpellCardBgm g_SpellCardBgms[];
// By stage and difficulty: what LoadingThread sets stats->unk_40 to.
extern int g_TimeOrbsNeeded[][4];
// The difficulty (0 to 4) whose g_SpellCardLists list has spell card
// `card`, else 5.
int GetSpellCardDifficulty(int card);
// Whether spell card `card` is one of g_ListedSpellCards.
int IsListedSpellCard(int card);
// Read by SpellCard::Start to start the stage's dialogue sprites;
// nothing sets it.
extern AnmLoaded *g_DialogueSpritesAnm;
// 43 spell cards (the 3 or 4 difficulties of several bosses' cards, and
// 171 to 190), and how many.
extern int g_ListedSpellCards[];
extern int g_ListedSpellCardCount;
// The spell card manager, at 0x4ea670 (SpellCard::RegisterChain
// registers it). Only what is known so far.
struct SpellCard
{
    // Bit 0 of unk_0. Inline; the copy at 0x4178a0 is in an /Od object.
    int IsActive()
    {
        return this->flags & 1;
    }
    // Initializes g_SpellCard and adds its update element (priority 0xc)
    // and draw element (priority 0xf), both allocated by
    // Chain::CreateElem. Returns 0, or -1 if Initialize fails (0x417f60).
    static int RegisterChain();
    // The chain callbacks: run Update and Draw (0x418010, 0x418030).
    static int OnUpdate(SpellCard *sc);
    static int OnDraw(SpellCard *sc);
    // Releases the boss faces (slots 0x12 and 0x13) unless restarting,
    // and the background and team faces (0xf to 0x11) while the scene
    // frees its files, then clears the update element's deleted
    // callback and removes the draw element. Returns 0 (0x418050).
    static int DeletedCallback(SpellCard *sc);
    // Removes the update element, which deletes the draw element too;
    // GameManager::DeletedCallback calls it before the stage's
    // (0x4180f0).
    static void RemoveChain();
    // The card's frame: bonus, timers, banners (0x416b90).
    int Update();
    // Draws the card's portraits, name and bonus (0x4178c0).
    int Draw();
    // Zeroes everything and loads (or, while the scene keeps its files,
    // takes) the dialogue background (slot 0xf), the two faces of the
    // player's team (slots 0x10 and 0x11, not while a card is practised)
    // and the boss faces of the stage or the practised card (slots 0x12
    // and 0x13, kept while restarting); then starts the sprites' scripts
    // and hides them. Returns 0, or -1 if loading fails (0x414590).
    int Initialize();
    // Starts spell card `card` named `name` for `enemy`, from an enemy
    // script instruction (StartSpellCard; th07's was the script's own
    // code) (0x4152a0).
    void Start(int card, char *name, int face, DWORD bonus, struct Enemy *enemy, char *text1, char *text2,
               char *text3);
    // Shows the card's name `name` with sprite `sprite`, and face `face`
    // unless it is negative (0x415f00).
    void ShowName(int face, char *name, int sprite);
    // Ends the spell card, as th07's EndSpellCard did (0x4161b0).
    void End();
    // Starts the card named `name` (「リザレクション」) for `enemy` without a
    // capture: sets bit 10, clears bits 0 and 4 (0x415d10).
    void StartNameOnly(char *name, struct Enemy *enemy);
    // Ends the name banners ShowName started: sets unk_139c's pending
    // interrupt directly and sends unk_1e2c (and unk_2374 unless
    // unk_0_10) interrupt 2 (0x416160).
    void EndSpellCardName();
    // Hides the boss's card: clears unk_0_0 and ends the name banners
    // (0x416af0).
    void Abort();
    // Called with 8000 when a time orb is collected: unless unk_0_11 is
    // set, adds `amount` to the bonus, up to the bonus the card started
    // with, and below that `amount` / 120 to what it loses a second
    // (0x416b10).
    void AddBonus(int amount);
    // Shows the name of a bomb's spell card as the bomb starts
    // (StartBomb): the portrait for `face` (0 or 1, else none), `name`
    // and sprite `sprite` (0x415d60).
    void StartBombCard(int face, char *name, int sprite);
    // Bit 7 of unk_0: whether the player bombed during the spell card,
    // as th07's spellCardBombed? Inline; the copy is at 0x42dff0.
    int WasBombed()
    {
        return this->flags >> 7 & 1;
    }
    // Bit 9 of unk_0. Inline; the copy is at 0x405260.
    int IsCaptured()
    {
        return this->flags >> 9 & 1;
    }

    // Ends the spell card's bonus: sets unk_10f8's pending interrupt
    // directly and sends unk_1b88 interrupt 2. Inline; the copy at
    // 0x416130 is an /Od object's.
    void EndBombName()
    {
        this->bombName.pendingInterrupt = 1;
        this->bombNameBar.SetPendingInterrupt(2);
    }
    // Ends the spell card as a timer callback runs: clears unk_0_2 and
    // unk_fc, sets unk_0_3. Inline; the copy at 0x42bc50 is an /Od
    // object's.
    void TimeOut()
    {
        this->capture = 0;
        this->timedOut = 1;
        this->bonus = 0;
    }
    // Clear unk_0_2 and unk_fc; the first also copies bit 0 to bit 7.
    // Called as a bomb starts and as the player dies. Inline; the copies
    // at 0x44cba0 and 0x44d150 are the player object's.
    void LoseCaptureToBomb()
    {
        this->capture = 0;
        this->bonus = 0;
        this->bombed = this->active;
    }
    void LoseCapture()
    {
        this->capture = 0;
        this->bonus = 0;
    }

    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD unusedFlag0_1 : 1;
            DWORD capture : 1;
            DWORD timedOut : 1;
            DWORD unusedFlag0_4 : 1;
            DWORD lastSpell : 1;
            DWORD effectFixed : 1;
            DWORD bombed : 1;
            // Set while a bonus is pending from the last card.
            DWORD bonusPending : 1;
            DWORD captured : 1;
            DWORD nameOnly : 1;
            DWORD bonusFrozen : 1;
        };
    };
    // unk_10. Inline; its copy is folded with ReplayManager::GetMode
    // (0x453cc0).
    int GetTimeOrbBonus()
    {
        return this->timeOrbBonus;
    }
    // Whether unk_0_0 and unk_0_5 are set. Inline; the copy is at
    // 0x417860.
    int IsLastSpellActive()
    {
        return this->IsActive() && this->lastSpell ? TRUE : FALSE;
    }
    // Whether unk_0_0 and unk_0_2 are set. Inline; the copy is at
    // 0x41fd90.
    int IsCapturable()
    {
        return this->IsActive() && this->capture ? TRUE : FALSE;
    }
    // Set unk_0_6 and unk_0_11 (opcodes 164 and 184). Inline; the copies
    // at 0x41f0b0 and 0x41f0e0 are an /Od object's.
    void SetEffectFixed(int value)
    {
        this->effectFixed = value;
    }
    void SetBonusFrozen(int value)
    {
        this->bonusFrozen = value;
    }
    // Puts the card's effect at (x, y, z). Inline; the copy at 0x41f040
    // is an /Od object's.
    void SetEffectPos(float x, float y, float z);
    // unk_108 in frames. Inline; the copy is at 0x41fdd0.
    int GetTimeLeft()
    {
        return this->timeLeft.AsFrames();
    }

    // The boss whose card it is.
    struct Enemy *enemy;
    // The card's number.
    int card;
    // enemy->index.
    int enemyIndex;
    // Time orbs added as the card ends.
    int timeOrbBonus;
    // The card's name (decoded with 0xaa).
    char name[0x30];
    unsigned char pad_44[0x30];
    // Copied from Start's last two arguments while a card is practised.
    char text2[0x40];
    char text3[0x40];
    struct Effect *effect;
    // The ring effect after a capture, which End hands over from effect.
    struct Effect *captureEffect;
    // The bonus left, and what it loses each second.
    DWORD bonus;
    DWORD bonusStep;
    int capturedBonus;
    Timer timeLeft;
    Timer timeLimit;
    Sprite bombPortrait;
    Sprite spellPortrait;
    Sprite bombPortraitGlow;
    Sprite spellPortraitGlow;
    Sprite bombPortraitFrame;
    Sprite spellPortraitFrame;
    Sprite bombName;
    Sprite spellName;
    Sprite lastWordName1;
    Sprite lastWordName2;
    Sprite bombNameBar;
    Sprite spellNameBar;
    Sprite spellScoreDigit;
    Sprite spellScoreBar;
    float bombNameWidth;
    float spellNameWidth;
    D3DCOLOR nameColor;
    // The face ANMs (slots 0x10 to 0x13) of the dialogue's four portraits.
    // GuiImpl::StartMsgScript swaps the last two for stages 6B and Extra.
    AnmLoaded *faceAnms[4];
    // face_cdbg.anm (slot 0xf).
    AnmLoaded *backgroundAnm;
    // The bonus the card starts with.
    DWORD startBonus;
    ChainElem *calcChain;
    ChainElem *drawChain;
};
ASSERT_SIZE(SpellCard, 0x2644);
extern SpellCard g_SpellCard;
// Sets stats->unk_38 from stats->unk_34 by difficulty (0x440470).
void SetChecksumKeyPoints();
// SetChecksumKeyPoints's tables: stats->unk_38 by stats->unk_34 for the four main
// difficulties (then 500 more each), and for Extra and Phantasm (then
// 99999).
extern int g_ExtendPointItems[6];
extern int g_ExtendPointItemsExtra[3];
// g_SpellCardBgms' unk_10 and unk_c for the spell card practised; both
// ignore `card` (0x439916, 0x439961).
int GetPracticeBgmRestarts(int card);
int GetPracticeBgmTitleIndex(int card);
