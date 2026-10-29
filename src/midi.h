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
