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
    unsigned int unk15 : 17;
};

class App
{
public:
    // Whether holding shoot also focuses (shootFocuses). th07 read the
    // field directly; th08 calls this out-of-line copy.
    int GetShootFocuses();
    // Enters and leaves criticalSections[idx], counting the depth in
    // criticalSectionDepths. New in th08.
    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);
    // Asks the sub thread (App+0x284, new in th08) to close and waits for
    // it (0x44871b).
    void CloseSubThread();
    // Getters for config options that th07 read directly; th08 calls
    // these out-of-line copies (0x443a50..0x443ad0).
    int GetEmulateColorAdd();
    int GetNoVertexBuffer();
    int GetForce16BitTextures();
    int GetReferenceRasterizer();
    int GetWindowed();

    HINSTANCE hInstance;
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    IDirectInput8 *dinput;
    IDirectInputDevice8 *keyboard;
    IDirectInputDevice8 *joystick;
    DIDEVCAPS joystickCaps;
    HWND hwnd;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
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
    BYTE unk_13c;
    BYTE unk_13d;
    // Whether holding shoot also focuses: ReadJoystick reports focus
    // once shoot has been held for 10 frames.
    BYTE shootFocuses;
    unsigned char unk_13f[0x11];
    // th07's flags, 4 bytes further on.
    GameConfigFlags flags;
    unsigned char unk_154[0x50];
    // Bit 0x40 is set by CheckSingleInstance when the startup info has
    // no title, cleared when it has one.
    DWORD unk_1a4;
    unsigned char unk_1a8[0xf0];
    // Locked by EnterCriticalSection: 2 around file access, 3 around the
    // log.
    CRITICAL_SECTION criticalSections[4];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[4];
    unsigned char unk_2fc[0x58];
    // The sum of the executable's dwords and its size, from
    // ChecksumExecutable.
    DWORD exeChecksum;
    DWORD exeSize;
};

extern App g_App;

// Prints `name` and then 可 or 不可 (yes or no), by whether `caps` has
// any of `flag`, at `out`; returns the end of the text.
char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out);
// Writes a report of the device capabilities `caps` that matter to the
// game into `buffer`, in Japanese.
void FormatCapsReport(D3DCAPS8 *caps, char *buffer);

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
// sets bit 0x40 of g_App.unk_1a4 if the startup info has no title,
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

// An entry of thbgm.fmt: where a track is in thbgm.dat and its format.
// The same as th07's.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // CWaveFile::ResetFile limits the track to `loopEnd` bytes and, when
    // looping, skips its first `loopStart`.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;

    BOOL Loops()
    {
        return this->loopEnd != 0;
    }
};
ASSERT_SIZE(BgmFormat, 0x34);

// Plays the sound effects and music, as th07's (0x39cc bytes there).
// Only the fields known so far.
class SoundPlayer
{
public:
    unsigned char unk_0[0x5214];
    // Added to every BgmFormat offset by CWaveFile::ResetFile (th07's
    // unk_39c8).
    DWORD unk_5214;
    // The music and sound effect volumes in percent, copied from the
    // config (App+0x13f, App+0x140). New in th08: CSound::SetVolume
    // scales by bgmVolume.
    int bgmVolume;
    int seVolume;
    unsigned char unk_5220[4];
};
ASSERT_SIZE(SoundPlayer, 0x5224);
extern SoundPlayer g_SoundPlayer;
