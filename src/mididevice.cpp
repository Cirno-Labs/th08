// MidiDevice, 0x443c60 to 0x443df0, as th06's mididevice.cpp: split from
// midi.cpp at a plausible seam (nothing in the binary shows it).
// MidiPlayer::FadeOut (0x4447d0) is defined here too, before Close, as
// /OPT:ICF needs it for D3DX8's fold choices.

#include "midi.h"
#include "unsorted.h"

MidiDevice::MidiDevice()
{
    this->handle = 0;
    this->deviceId = 0;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

#ifdef TH08_EXE
// Unused: the original's object also opened a MIDI device before Close,
// from code /OPT:REF dropped: the hint/name table loads midiOutOpen
// before midiOutClose and midiOutReset, which decides WINMM's IAT order.
#pragma code_seg(".text$u")
MMRESULT UnusedOpenMidi(HMIDIOUT *handle, UINT deviceId)
{
    return midiOutOpen(handle, deviceId, 0, 0, CALLBACK_NULL);
}
#pragma code_seg()
#endif // TH08_EXE

// Keep this before MidiDevice::Close for D3DX8's SSE fold selection;
// the executable's /ORDER file preserves FadeOut's game address.
int MidiPlayer::FadeOut(int duration)
{
    this->volumeScale = 0.0f;
    this->fadeDuration = duration;
    this->fadeElapsed = 0;
    this->framesSinceRewind = 0;
    this->fading = 1;
    return 0;
}

int MidiDevice::Close()
{
    if (this->handle == 0)
    {
        return -1;
    }
    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;
    return 0;
}

int MidiDevice::Open(UINT deviceId)
{
    if (this->handle != 0)
    {
        if (this->deviceId != deviceId)
        {
            this->Close();
        }
        else
        {
            return 0;
        }
    }
    this->deviceId = deviceId;
    return midiOutOpen(&this->handle, deviceId, (DWORD_PTR)g_App.hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR;
}

int MidiDevice::SendLongMsg(LPMIDIHDR header)
{
    if (this->handle == 0)
    {
        return 0;
    }
    if (midiOutPrepareHeader(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
    {
        return 1;
    }
    return midiOutLongMsg(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR;
}

int MidiDevice::SendShortMsg(BYTE status, BYTE data1, BYTE data2)
{
    MidiShortMsg msg;

    if (this->handle == 0)
    {
        return 0;
    }
    msg.bytes[0] = status;
    msg.bytes[1] = data1;
    msg.bytes[2] = data2;
    return midiOutShortMsg(this->handle, msg.dword) != MMSYSERR_NOERROR;
}
