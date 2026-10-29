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

MmTimer::MmTimer()
{
    timeGetDevCaps(&this->caps, sizeof(this->caps));
    this->timerId = 0;
}

MmTimer::~MmTimer()
{
    this->Stop();
    timeEndPeriod(this->caps.wPeriodMin);
}

void MmTimer::OnTimer()
{
}

UINT MmTimer::Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user)
{
    this->Stop();
    timeBeginPeriod(this->caps.wPeriodMin);
    if (callback != NULL)
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, callback, user, TIME_PERIODIC);
    }
    else
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, DefaultCallback, (DWORD_PTR)this, TIME_PERIODIC);
    }
    return this->timerId;
}

int MmTimer::Stop()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->caps.wPeriodMin);
    this->timerId = 0;
    return 1;
}

WORD SwapBytes16(WORD value)
{
    union
    {
        WORD word;
        BYTE bytes[2];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[1];
    swapped.bytes[1] = ((BYTE *)&value)[0];
    return swapped.word;
}

DWORD ReadVarLen(BYTE **cursor)
{
    DWORD value = 0;
    BYTE byte;

    do
    {
        byte = **cursor;
        (*cursor)++;
        value = (value << 7) + (byte & 0x7f);
    } while (byte & 0x80);
    return value;
}
