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
    unsigned int unk12 : 1;
    unsigned int preloadBgm : 1;
    unsigned int unk14 : 1;
    // Text rendering's environment is not detected automatically. New
    // in th08.
    unsigned int unk15 : 1;
    unsigned int unk16 : 16;
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
    // flags.preloadBgm, through an out-of-line copy in an /Os object
    // (0x449c79).
    int GetPreloadBgm();

    // Whether the back buffer is cleared every frame: forced, or because
    // the items around the game are not drawn. Inline; the operands are
    // evaluated right to left.
    int ClearsBackBuffer()
    {
        return this->flags.clearBackBuffer | this->flags.noFrameItems;
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
    unsigned char unk_141[0xf];
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
    unsigned char unk_164[0x8];
    // Set by OnUpdate when a game restarts from scene 11.
    int unk_16c;
    unsigned char unk_170[0x4];
    // How many more frames to draw in full, after the device was reset.
    int redrawFrames;
    // DrawFps shows the frame rate while it is 0; AddedCallback sets it
    // while the refresh rate is measured.
    int hideFps;
    // Frames are paced by the clock rather than by the presents.
    int presentImmediate;
    // Set while the device is created asking for 60 Hz.
    int wants60Hz;
    int unk_184;
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
    unsigned char unk_19a[0x2];
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
    int unk_294;
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
extern int g_Unknown18b8a2c;

// Measures the frame rate every half second, with the performance
// counter when there is one, and shows it when `draw` is set, as th07's.
void DrawFps(int draw);

// Opens th08.dat as g_Archive and loads its version file,
// "th08_0100d.ver", into g_App.verFile. Returns 0, or -1 after logging
// an error if either is missing.
int OpenGameArchive();

// Measures the refresh rate by presenting surface 8, up to 1800 times
// or until it has eight rates of 57 Hz or more, each over half a second
// or so. Unless g_App.flags.unk14 is set, it turns presentImmediate on
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

// The game window (at 0x17ce700), as in th07: the fields used so far are
// at the same offsets.
struct GameWindow
{
    HWND hwnd;
    // Set when the window is being closed: the main loop ends.
    int isClosing;
    WPARAM isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char unk_11[0x3];
    // QueryPerformanceFrequency's result.
    DWORD perfFrequency[2];
    // Set when the game was started from a shortcut or another path;
    // paces every frame by the clock, even ones that are early.
    BYTE unk_1c;
    unsigned char unk_1d[0x3];
    // The screen saver and power settings, put back on exit.
    int screenSaveActive;
    int lowPowerActive;
    int powerOffActive;
};
extern GameWindow g_GameWindow;

// Creates g_AppMutex and returns -1 if another instance holds it, as
// th07's CheckSingleInstance. Also sets g_GameWindow.unk_1c when the
// game was started through a shortcut or a path other than its own, and
// sets bit 0x40 of g_App.stateFlags if the startup info has no title,
// clearing it otherwise.
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
    BYTE unk_3;
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

    WORD seed;
    // How many times the generator was advanced.
    DWORD generated;
};
extern Rng g_Rng;

// a + b, brought back into [-pi, pi] (giving up after 17 turns).
float AddNormalizeAngle(float a, float b);

// Rotates `in` by `angle` radians in the xy plane into `out` (z is left
// alone). The opposite direction to th07's.
void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle);

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
// `width` by `fontHeight` of it into `texture`, `unk6` rows high at row
// `y`.
void RenderTextToTexture(int x, int y, int width, int unk4, int fontHeight, int unk6, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);

// th07's RenderTextToTexture, but semibold rather than bold. The other
// font of SpriteEngine's text rendering (0x466340).
void RenderTextToTextureBold(int x, int y, int width, int unk4, int fontHeight, int unk6, COLORREF textColor,
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
            this->unk_408[i] = -1;
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
    DWORD unk_004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set when a sound is queued; -1 at first.
    int unk_408[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD unk_61c;
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
    DWORD unk_5210;
    // Added to every BgmFormat offset by CWaveFile::ResetFile (th07's
    // unk_39c8).
    DWORD unk_5214;
    // The music and sound effect volumes in percent, 100 at first, then
    // copied from the config (App+0x13f, App+0x140). Not in th07:
    // CSound::SetVolume scales by bgmVolume.
    int bgmVolume;
    int seVolume;
    DWORD unk_5220;
};
ASSERT_SIZE(SoundPlayer, 0x5224);
extern SoundPlayer g_SoundPlayer;

// What th07's GameManager points to at +0x8 and +0x4. Only the fields
// known so far.
struct GameManagerStats;
struct GameManagerUnk4
{
    unsigned char unk_0[0x25];
    // App::IsScoreDisabled tests it.
    BYTE scoreDisabled;
};

// th07's GameManager (g_Unknown626270 there), at 0x160f508. Only the
// fields known so far.
class GameManager
{
public:
    // Whether shot type `shot` (0 to 3) cleared one of the four main
    // difficulties, as th07's (0x43c15f).
    int HasExtraOpen(int shot);
    // Whether a shot type has Extra open (0x43c1e9).
    int AnyShotHasExtraOpen();
    // Whether a shot type has 0x43c23b set (0x43c2c5).
    int Func43c2c5();
    // Whether every shot type has Extra open. In App's object.
    int AllShotsHaveExtraOpen();
    // Moves `stage` on to the next stage (0x43c4b3).
    void AdvanceStage();

    DWORD unk_0;
    GameManagerUnk4 *unk_4;
    GameManagerStats *stats;
    // The score file's chunks, loaded by App::AddedCallback.
    Flsp flsp;
    BYTE unk_2c;
    // As th07's unk_d: while it is set, the frame rate is not measured
    // and the buttons are only added to.
    char unk_2d;
    unsigned char unk_2e[0x2];
    int difficulty;
    unsigned char unk_34[0xc];
    Catk catks[0xde];
    // A copy of catks, as th07's savedCatks.
    Catk savedCatks[0xde];
    Clrd clrds[0xd];
    Pscr pscrs[0xc];
    // The play statistics of the score file.
    Plst plst;
    unsigned char unk_3da2c[0x180];
    union
    {
        // Bit 2: the speed is measured; 3: it is shown, as th07's
        // unk_93d8.
        DWORD unk_3dbac;
        struct
        {
            unsigned int unk_3dbac_0 : 15;
            // AnyShotHasExtraOpen, Func43c2c5 and AllShotsHaveExtraOpen
            // as App::AddedCallback found them.
            unsigned int anyShotHasExtraOpen : 1;
            unsigned int unk_3dbac_16 : 1;
            unsigned int allShotsHaveExtraOpen : 1;
            unsigned int unk_3dbac_18 : 14;
        };
    };
    unsigned char unk_3dbb0[0x214];
    // The stage being played, from 0.
    int stage;
};
extern GameManager g_GameManager;

// The thread LoadBgm starts, with the main window: streams the wave music
// each time notifyEvent is signalled, until WM_QUIT or the music is gone.
DWORD WINAPI BgmNotificationProc(LPVOID lpParameter);

// Each sound effect's sound buffer and volume, as in th07.
struct SoundInfo
{
    int buffer;
    short volume;
    short unk_6;
};
// 0x2e sound effects (th07 0x26).
extern SoundInfo g_SoundInfos[0x2e];
// The sound files LoadSounds loads, by index: 0x24 (th07 0x1e).
extern char *g_SoundPaths[];
