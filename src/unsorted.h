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

#include "app.h"
struct GameWindow
{
    // Zeroes the window, as th07's; only g_GameWindow's static initializer
    // calls it (0x4418a0).
    GameWindow();

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

// Loads a whole file into a buffer from g_MemoryTracker; NULL on
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

    // Starts empty. Only g_Logger's static initializer calls it.
    Log();
    // Empty; the static initializer has atexit run it (0x43ef40, folded
    // with ~SpriteEngine and MmTimer::OnTimer).
    ~Log();
    // Empties the log. th07's WinMain did this in place.
    void Reset();
    // Writes the log to ./log.txt, if anything was logged, after a
    // closing line; shows it in a message box first if it holds an
    // error.
    void Flush();
};
extern Log g_Logger;

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
extern JOYCAPSA g_JoyDeviceCaps;

// Checks that joystick 1 is there and reads its capabilities into
// g_JoyDeviceCaps. Returns 1 when there is none.
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
extern WORD g_PreviousButtons;
// The buttons the game plays by this frame and the frame before: the
// held ones, or a replay's.
extern WORD g_PlayButtons;
extern WORD g_PreviousPlayButtons;
// 1 on every 8th frame once the buttons have been held unchanged for 30
// frames, for key repeat.
extern WORD g_RepeatButtons;
// How many frames the buttons have been held unchanged, cycling from 38
// back to 30.
extern WORD g_ButtonHeldFrames;

// Whether a button of `mask` went down this frame.
#define WAS_PRESSED(mask) ((g_Buttons & (mask)) && (g_Buttons & (mask)) != (g_PreviousButtons & (mask)))
// WAS_PRESSED, or held long enough to repeat.
#define WAS_PRESSED_OR_HELD(mask) (WAS_PRESSED(mask) || ((g_Buttons & (mask)) && g_RepeatButtons))

// Clears the down bit (0x80) of every key in the thread's keyboard
// state.
void ResetKeyboard();

// Decrypts `size` bytes of `data` into a new buffer from
// g_MemoryTracker and returns it (or `data` if the allocation fails).
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
    BYTE pad_3;
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
extern Rng g_Random;

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

#include "gdisurface.h"

#include "sound.h"

#include "gamemanager.h"

#include "gui.h"

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

#include "stage.h"

#include "spellcard.h"
