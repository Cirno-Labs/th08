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

class MidiDevice
{
public:
    MidiDevice();

    HMIDIOUT handle;
    UINT deviceId;
};
