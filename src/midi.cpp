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

void CALLBACK MmTimer::DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MmTimer *timer = (MmTimer *)user;
    timer->OnTimer();
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

MidiPlayer::MidiPlayer()
{
    int i;
    int j;

    this->tracks = NULL;
    this->division = 0;
    this->tempo = 0;
    this->trackCount = 0;
    this->transpose = 0;
    this->volumeScale = 0.0f;
    this->lastVolume = 0;
    this->unusedAfterConstruct2d0 = 0;
    this->suppressVolume = 0;
    this->unusedAfterConstruct2d8 = 0;
    this->framesSinceRewind = 0;
    this->fading = 0;
    for (i = 0; i < 0x20; i++)
    {
        this->fileData[i] = NULL;
    }
    for (j = 0; j < 0x20; j++)
    {
        this->headers[j] = NULL;
    }
    this->playingSlot = -1;
    this->nextHeader = 0;
}

MidiPlayer::~MidiPlayer()
{
    int i;

    this->Stop();
    this->FreeTracks();
    for (i = 0; i < 0x20; i++)
    {
        this->UnloadFile(i);
    }
}

int MidiPlayer::LoadFile(int slot, char *path)
{
    if (this->playingSlot == slot)
    {
        this->Stop();
    }
    this->UnloadFile(slot);
    this->fileData[slot] = LoadFileData(path, NULL, 0);
    if (this->fileData[slot] == NULL)
    {
        // "\rv\r\n" in the original.
        LogPrintf(&g_Log, "error : MIDI File が読み込めない %s \rv\r\n", path);
        return -1;
    }
    return 0;
}

void MidiPlayer::UnloadFile(int slot)
{
    g_MemoryManager.Free(this->fileData[slot]);
    this->fileData[slot] = NULL;
}

void MidiPlayer::FreeTracks()
{
    int i;

    for (i = 0; i < this->trackCount; i++)
    {
        g_MemoryManager.Free(this->tracks[i].data);
    }
    g_MemoryManager.Free(this->tracks);
    this->tracks = NULL;
    this->trackCount = 0;
}

int MidiPlayer::ParseFile(int slot)
{
    int i;
    BYTE *cursor;
    BYTE *file;
    BYTE *track;
    DWORD headerLength;
    MidiChunk hdr;
    BYTE *header;
    DWORD trackLen;

    this->FreeTracks();
    cursor = (BYTE *)this->fileData[slot];
    file = cursor;
    if (cursor == NULL)
    {
        DebugPrint("error : まだMIDIが読み込まれていないのに再生しようとしている\r\n");
        return -1;
    }
    memcpy(&hdr, cursor, sizeof(MidiChunk));
    cursor += sizeof(MidiChunk);
    headerLength = SwapBytes32(hdr.length);
    header = cursor;
    cursor += headerLength;
    this->format = SwapBytes16(*(WORD *)header);
    this->division = SwapBytes16(*(WORD *)(header + 4));
    this->trackCount = SwapBytes16(*(WORD *)(header + 2));
    this->tracks = (MidiTrack *)g_MemoryManager.Alloc(this->trackCount * sizeof(MidiTrack), "midi");
    memset(this->tracks, 0, this->trackCount * sizeof(MidiTrack));
    for (i = 0; i < this->trackCount; i++)
    {
        track = cursor;
        cursor += sizeof(MidiChunk);
        trackLen = SwapBytes32(((MidiChunk *)track)->length);
        this->tracks[i].length = trackLen;
        this->tracks[i].data = (BYTE *)g_MemoryManager.Alloc(trackLen, "midi");
        this->tracks[i].active = 1;
        memcpy(this->tracks[i].data, cursor, trackLen);
        cursor += trackLen;
    }
    this->tempo = 1000000;
    this->playingSlot = slot;
    DebugPrint(" midi open %d\n", slot);
    return 0;
}

DWORD SwapBytes32(DWORD value)
{
    union
    {
        DWORD dword;
        BYTE bytes[4];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[3];
    swapped.bytes[1] = ((BYTE *)&value)[2];
    swapped.bytes[2] = ((BYTE *)&value)[1];
    swapped.bytes[3] = ((BYTE *)&value)[0];
    return swapped.dword;
}

int MidiPlayer::Load(char *path)
{
    if (this->LoadFile(0x1f, path) != 0)
    {
        return -1;
    }
    this->ParseFile(0x1f);
    this->UnloadFile(0x1f);
    return 0;
}

void MidiPlayer::Rewind()
{
    int i;
    MidiTrack *track;

    track = this->tracks;
    this->volumeScale = 1.0f;
    this->framesSinceRewind = 0;
    this->fading = 0;
    this->ticks = 0;
    this->baseTime = 0;
    for (i = 0; i < this->trackCount; i++, track++)
    {
        track->cursor = track->data;
        track->loopCursor = track->cursor;
        track->active = 1;
        track->delta = ReadVarLen(&track->cursor);
    }
}

int MidiPlayer::Play()
{
    if (this->tracks == NULL)
    {
        return -1;
    }
    this->Rewind();
    this->device.Open(MIDI_MAPPER);
    this->Start(1, NULL, 0);
    DebugPrint(" midi play\n");
    return 0;
}

int MidiPlayer::Stop()
{
    int i;

    if (this->tracks == NULL)
    {
        return -1;
    }
    for (i = 0; i < 0x20; i++)
    {
        if (this->headers[i] != NULL)
        {
            this->ReleaseHeader(this->headers[i]);
        }
    }
    MmTimer::Stop();
    this->device.Close();
    this->playingSlot = -1;
    return 0;
}

int MidiPlayer::ReleaseHeader(LPMIDIHDR header)
{
    MMRESULT result;

    if (header == NULL)
    {
        DebugPrint("error :\r\n");
    }
    if (this->device.handle == 0)
    {
        DebugPrint("error :\r\n");
    }
    for (int i = 0; i < 0x20; i++)
    {
        if (this->headers[i] == header)
        {
            this->headers[i] = NULL;
            goto found;
        }
    }
    return -1;

found:
    result = midiOutUnprepareHeader(this->device.handle, header, sizeof(MIDIHDR));
    if (result != MMSYSERR_NOERROR)
    {
        DebugPrint("error :\r\n");
    }
    g_MemoryManager.Free(header->lpData);
    g_MemoryManager.Free(header);
    return 0;
}

int MidiPlayer::FadeOut(int duration)
{
    this->volumeScale = 0.0f;
    this->fadeDuration = duration;
    this->fadeElapsed = 0;
    this->framesSinceRewind = 0;
    this->fading = 1;
    return 0;
}

void MidiPlayer::OnTimer()
{
    int i;
    unsigned __int64 time;
    int anyPlaying;

    anyPlaying = 0;
    time = this->baseTime + this->ticks * this->division * 1000 / this->tempo;
    if (this->fading != 0)
    {
        if (this->fadeElapsed < this->fadeDuration)
        {
            this->volumeScale = 1.0f - (float)this->fadeElapsed / (float)this->fadeDuration;
            if ((int)(this->volumeScale * 128.0f) != this->lastVolume)
            {
                this->SendVolume(0);
            }
            this->lastVolume = (int)(this->volumeScale * 128.0f);
            this->fadeElapsed++;
        }
        else
        {
            this->volumeScale = 0.0f;
            return;
        }
    }
    for (i = 0; i < this->trackCount; i++)
    {
        if (this->tracks[i].active)
        {
            anyPlaying = 1;
            while (this->tracks[i].active && this->tracks[i].delta <= time)
            {
                this->ProcessEvent(&this->tracks[i]);
                time = this->baseTime + this->ticks * this->division * 1000 / this->tempo;
            }
        }
    }
    this->ticks++;
    if (!anyPlaying)
    {
        this->Rewind();
    }
}

void MidiPlayer::ProcessEvent(MidiTrack *track)
{
    int deltaTime;
    int i;
    BYTE byte2;
    int newVolume;
    BYTE channel;
    BYTE event;
    BYTE byte1;
    BYTE runStatus;
    int numBytes;

    runStatus = *track->cursor;
    if (runStatus < 0x80)
    {
        runStatus = track->runningStatus;
    }
    else
    {
        track->cursor++;
    }
    event = runStatus & 0xf0;
    channel = runStatus & 0x0f;
    switch (event)
    {
    case 0xf0:
        if (runStatus == 0xf0)
        {
            LPMIDIHDR header;

            if (this->headers[this->nextHeader] != NULL)
            {
                this->ReleaseHeader(this->headers[this->nextHeader]);
            }
            this->headers[this->nextHeader] = (LPMIDIHDR)g_MemoryManager.Alloc(sizeof(MIDIHDR), "midiHDR");
            header = this->headers[this->nextHeader];
            numBytes = ReadVarLen(&track->cursor);
            memset(header, 0, sizeof(MIDIHDR));
            header->lpData = (LPSTR)g_MemoryManager.Alloc(numBytes + 1, "midiHDR->lpData");
            header->lpData[0] = 0xf0;
            header->dwFlags = 0;
            header->dwBufferLength = numBytes + 1;
            for (i = 0; i < numBytes; i++)
            {
                header->lpData[i + 1] = *track->cursor;
                track->cursor++;
            }
            if (this->device.SendLongMsg(header) != 0)
            {
                g_MemoryManager.Free(header->lpData);
                g_MemoryManager.Free(header);
                this->headers[this->nextHeader] = NULL;
            }
            this->nextHeader++;
            this->nextHeader %= 0x20;
        }
        else if (runStatus == 0xff)
        {
            BYTE metaEvent;
            int bpm;

            metaEvent = *track->cursor;
            track->cursor++;
            numBytes = ReadVarLen(&track->cursor);
            if (metaEvent == 0x2f)
            {
                // End of track.
                track->active = 0;
                return;
            }
            if (metaEvent == 0x51)
            {
                // Set tempo: restart the ticks at the new tempo.
                this->baseTime += this->ticks * this->division * 1000 / this->tempo;
                this->ticks = 0;
                this->tempo = 0;
                // Adds the old value once too many: a big-endian read would
                // be (tempo << 8) + byte.
                for (i = 0; i < numBytes; i++)
                {
                    this->tempo = (this->tempo << 8) + *track->cursor + this->tempo;
                    track->cursor++;
                }
                // Computed and never used.
                bpm = 60000000 / this->tempo;
                break;
            }
            track->cursor += numBytes;
        }
        break;
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xe0:
        byte1 = *track->cursor;
        track->cursor++;
        byte2 = *track->cursor;
        track->cursor++;
        break;
    case 0xc0:
    case 0xd0:
        byte1 = *track->cursor;
        track->cursor++;
        byte2 = 0;
        break;
    }
    switch (event)
    {
    case 0x90:
        if (byte2 != 0)
        {
            byte1 += this->transpose;
            this->channels[channel].notes[byte1 >> 3] |= (BYTE)(1 << (byte1 & 7));
            break;
        }
        // A note on with velocity 0 is a note off.
    case 0x80:
        byte1 += this->transpose;
        this->channels[channel].notes[byte1 >> 3] &= (BYTE)~(1 << (byte1 & 7));
        break;
    case 0xc0:
        this->channels[channel].program = byte1;
        break;
    case 0xb0:
        switch (byte1)
        {
        case 0:
            this->channels[channel].bank = byte2;
            break;
        case 7:
            this->channels[channel].volume = byte2;
            newVolume = (float)byte2 * this->volumeScale;
            if (newVolume < 0)
            {
                newVolume = 0;
            }
            else if (newVolume > 0x7f)
            {
                newVolume = 0x7f;
            }
            this->channels[channel].scaledVolume = newVolume;
            byte2 = newVolume;
            break;
        case 91:
            this->channels[channel].reverb = byte2;
            break;
        case 93:
            this->channels[channel].chorus = byte2;
            break;
        case 10:
            this->channels[channel].pan = byte2;
            break;
        case 2:
        {
            // Loop start.
            MidiTrack *saved;

            saved = this->tracks;
            for (i = 0; i < this->trackCount; i++, saved++)
            {
                saved->loopCursor = saved->cursor;
                saved->loopDelta = saved->delta;
            }
            this->loopTempo = this->tempo;
            this->loopTicks = this->ticks;
            this->loopBaseTime = this->baseTime;
            break;
        }
        case 4:
        {
            // Loop end: back to the loop start.
            MidiTrack *restored;

            restored = this->tracks;
            for (i = 0; i < this->trackCount; i++, restored++)
            {
                restored->cursor = restored->loopCursor;
                restored->delta = restored->loopDelta;
            }
            this->tempo = this->loopTempo;
            this->ticks = this->loopTicks;
            this->baseTime = this->loopBaseTime;
            break;
        }
        }
        break;
    }
    if (runStatus < 0xf0)
    {
        this->device.SendShortMsg(runStatus, byte1, byte2);
    }
    track->runningStatus = runStatus;
    deltaTime = ReadVarLen(&track->cursor);
    track->delta += deltaTime;
}

void MidiPlayer::SendVolume(int offset)
{
    int cc;
    int i;
    int chStatus;
    int data2;
    int volume;

    if (this->suppressVolume != 0)
    {
        return;
    }
    cc = 7;
    for (i = 0; i < 0x10; i++)
    {
        chStatus = (BYTE)(i + 0xb0);
        volume = (int)(this->channels[i].volume * this->volumeScale) + offset;
        if (volume < 0)
        {
            volume = 0;
        }
        else if (volume > 0x7f)
        {
            volume = 0x7f;
        }
        data2 = (BYTE)volume;
        this->device.SendShortMsg(chStatus, cc, data2);
    }
}

void MidiTimer::OnTimer()
{
    QueryPerformanceCounter(&g_MidiTimerLastCounter);
}

void MidiTimer::StartTimer()
{
    this->Start(16, NULL, 0);
}

void MidiTimer::StopTimer()
{
    this->Stop();
}
