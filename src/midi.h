// MIDI playback through WinMM.

#pragma once

#include <windows.h>
#include <mmsystem.h>

#include "common.h"

// A short MIDI message as midiOutShortMsg takes it: status and two data
// bytes, low byte first.
union MidiShortMsg
{
    DWORD dword;
    BYTE bytes[4];
};

// Swaps the bytes of a big-endian 16-bit value from a MIDI file.
WORD SwapBytes16(WORD value);

// Reads a MIDI variable-length quantity (7 bits per byte, high bit set on
// all but the last) and advances the cursor past it.
DWORD ReadVarLen(BYTE **cursor);

// Wraps a MIDI output device (midiOutOpen and friends).
class MidiDevice
{
public:
    MidiDevice();
    ~MidiDevice();

    // Returns -1 if no device is open, 0 once it is closed.
    int Close();

    // Opens the given device, closing another open one first. Returns
    // nonzero on failure, 0 on success or if the device is already open.
    int Open(UINT deviceId);

    // Prepares and sends a system-exclusive message. Returns 0 if no
    // device is open, nonzero if either step fails.
    int SendLongMsg(LPMIDIHDR header);

    // Sends a channel message. Returns 0 if no device is open, nonzero on
    // failure.
    int SendShortMsg(BYTE status, BYTE data1, BYTE data2);

    HMIDIOUT handle;
    UINT deviceId;
};

// A periodic WinMM multimedia timer. By default the timer calls OnTimer.
class MmTimer
{
public:
    MmTimer();
    ~MmTimer();

    // Starts a periodic timer event every `delay` ms at the finest
    // resolution, calling `callback` with `user`, or OnTimer if there is no
    // callback. Returns the timer ID, 0 on failure.
    UINT Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user);

    // Kills the timer event, if any, and ends the timer period. Returns 1.
    int Stop();

    // Called on every tick when the timer was started without a callback
    // of its own. Empty here; the linker folded it with another empty
    // function, which Func441e10 calls directly.
    virtual void OnTimer();

    // The timeSetEvent callback used when Start gets none: calls OnTimer
    // on the timer passed as user data.
    static void CALLBACK DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2);

    UINT timerId;
    TIMECAPS caps;
};
ASSERT_SIZE(MmTimer, 0x10);

// One track of a parsed MIDI file. Size from the stride of
// MidiPlayer::tracks.
struct MidiTrack
{
    // Set when the track is rewound.
    int active;
    // Time of the next event, from the delta times read by ReadVarLen.
    int delta;
    // The length of `data`.
    DWORD length;
    // The last status byte, for running status.
    BYTE runningStatus;
    // The track's event data.
    BYTE *data;
    // Read position in `data`.
    BYTE *cursor;
    // `cursor` and `delta` at the loop start, saved by controller 2 and
    // restored by controller 4. The start of the data after a rewind.
    BYTE *loopCursor;
    int loopDelta;
};
ASSERT_SIZE(MidiTrack, 0x20);

// The state of one of the 16 MIDI channels. Size from the stride of
// MidiPlayer::channels.
struct MidiChannel
{
    // One bit per note that is on, note n at bit n % 8 of byte n / 8.
    BYTE notes[0x10];
    BYTE program;
    // Controllers 0, 10, 91 and 93.
    BYTE bank;
    BYTE pan;
    BYTE reverb;
    BYTE chorus;
    // Channel volume (controller 7) before scaling.
    BYTE volume;
    // `volume` scaled by volumeScale, as last sent.
    BYTE scaledVolume;
};
ASSERT_SIZE(MidiChannel, 0x17);

// Plays MIDI files on a MidiDevice, driven by its own timer. The same
// layout as th07's.
class MidiPlayer : public MmTimer
{
public:
    MidiPlayer();
    ~MidiPlayer();

    virtual void OnTimer();

    // Frees the MIDI file loaded into a slot.
    void UnloadFile(int slot);
    // Loads a MIDI file into a slot, stopping playback first if that slot
    // is playing. Returns -1 on failure, otherwise 0.
    int LoadFile(int slot, char *path);
    // Sends every channel's volume, scaled and offset by `offset`, unless
    // unk_2d4 is set.
    void SendVolume(int offset);
    // Stops playback and closes the device. Returns -1 if nothing is
    // loaded, otherwise 0.
    int Stop();
    // Unprepares and frees a header from `headers`.
    int ReleaseHeader(LPMIDIHDR header);
    // Plays the loaded tracks on the MIDI mapper, ticking every 1 ms.
    // Returns -1 if nothing is loaded, otherwise 0.
    int Play();
    // Rewinds the tracks.
    void Rewind();
    // Starts fading the music out over `duration` ms. Returns 0.
    int FadeOut(int duration);
    // Frees the parsed tracks.
    void FreeTracks();
    // Parses the file in a slot into tracks. Returns -1 if the slot is
    // empty, otherwise 0.
    int ParseFile(int slot);
    // Loads and parses a MIDI file through the scratch slot 31. Returns -1
    // if it cannot be loaded, otherwise 0.
    int Load(char *path);
    // Handles the next event of a track and reads the time of the one
    // after.
    void ProcessEvent(MidiTrack *track);
    // Stops, parses slot `slot` and plays it.
    void Restart(int slot)
    {
        this->Stop();
        this->ParseFile(slot);
        this->Play();
    }

    // The slot of the file being played, -1 for none. LoadFile stops
    // playback before it replaces that file.
    int playingSlot;
    // Long messages being sent; released by ReleaseHeader.
    LPMIDIHDR headers[0x20];
    // The entry of `headers` the next long message goes in.
    int nextHeader;
    // MIDI files loaded by LoadFile, freed by UnloadFile.
    void *fileData[0x20];
    // The number of entries in `tracks`.
    int trackCount;
    // From the file header: the format and the time division.
    int format;
    int division;
    // Microseconds per quarter note, from the tempo meta event.
    int tempo;
    // Timer ticks since the start.
    unsigned __int64 ticks;
    // The time, in ms, played at earlier tempos; added to the time computed
    // from `ticks`, which restart at each tempo change.
    unsigned __int64 baseTime;
    // Parsed tracks, `trackCount` of them.
    MidiTrack *tracks;
    MidiDevice device;
    unsigned char unk_144[0x10];
    MidiChannel channels[0x10];
    // Added to every note number.
    char transpose;
    // Scales every channel's volume.
    float volumeScale;
    // The volume last sent while fading, volumeScale * 128.
    int lastVolume;
    int unk_2d0;
    int unk_2d4;
    int unk_2d8;
    int unk_2dc;
    // Set while fading out.
    int fading;
    // In ms, set by FadeOut.
    int fadeDuration;
    // Ticks of the fade so far.
    int fadeElapsed;
    // tempo, ticks and baseTime at the loop start (controller 2).
    int loopTempo;
    unsigned __int64 loopTicks;
    unsigned __int64 loopBaseTime;
};
ASSERT_SIZE(MidiPlayer, 0x300);
