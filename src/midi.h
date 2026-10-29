// MIDI playback through WinMM.

#pragma once

#include <windows.h>
#include <mmsystem.h>

#include "common.h"

// Swaps the bytes of a big-endian 16-bit value from a MIDI file.
WORD SwapBytes16(WORD value);
