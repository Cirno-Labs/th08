// The application: configuration, window, Direct3D and the main loop.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

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
    // g_LoadingScreenSprites, each moved by its posOffset. Returns 1.
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
    // g_LoadingScreenSprites on scripts 0 to 2 of loadingAnm at `pos` and sets
    // loadingState to 1. New in th08.
    // Whether the sub thread is running.
    BOOL HasSubThread()
    {
        return this->subThread != NULL;
    }
    void StartLoadingSprites(D3DXVECTOR3 *pos);
    // If the loading screen is in state 1, sends g_LoadingScreenSprites
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
    // g_GameManager.stateFlags is set. Returns 0.
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
    // Whether g_GameManager.savedConfig is set and its scoreDisabled is not 0.
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
    BYTE lives;
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
    // set; freesResources the same by DeletedCallback.
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
    // g_GameManager.stateFlags.
    short roundedFps;
    unsigned char pad_19a[0x2];
    // text.anm, loaded by LoadingThread.
    AnmLoaded *textAnm;
    // nowloading.anm, with g_LoadingScreenSprites' scripts (AddedCallback).
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
    // Set by StartSubThread. OnUpdate returns early while
    // loadingThreadState is set and this is.
    int subThreadRunning;
    // OnUpdate quits when it is 2.
    int loadingThreadState;
    // Locked by EnterCriticalSection: 2 around file access, 3 around the
    // log.
    CRITICAL_SECTION criticalSections[4];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[4];
    // The loading screen's state: 0 for none, 1 while g_LoadingScreenSprites
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
