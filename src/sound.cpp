// The file with SoundPlayer, the sound effects and wave music, from
// 0x45c190.

#include "dsutil.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
SoundInfo g_SoundInfos[0x2e] = {
    {0, -1900, 0}, {0, -2100, 0}, {1, -1200, 5}, {1, -1500, 5},
    {2, -1100, 100}, {3, -700, 100}, {4, -700, 100}, {5, -1900, 50},
    {6, -2200, 50}, {7, -2400, 50}, {8, -1100, 100}, {9, -1100, 100},
    {10, -1500, 10}, {11, -1500, 10}, {12, -1000, 100}, {5, -1100, 50},
    {13, -1300, 50}, {14, -1400, 50}, {15, -900, 100}, {16, -400, 100},
    {17, -880, 0}, {18, -1500, 0}, {5, -300, 20}, {6, -1800, 20},
    {7, -1800, 20}, {19, -1100, 50}, {20, -1300, 50}, {21, -1500, 50},
    {22, -500, 140}, {23, -500, 100}, {24, -1100, 20}, {25, -800, 90},
    {24, -1200, 20}, {19, -500, 50}, {26, -800, 100}, {27, -800, 100},
    {28, -800, 100}, {29, -700, 0}, {30, -300, 100}, {31, -800, 100},
    {32, -800, 100}, {33, -200, 100}, {34, 0, 100}, {34, -600, 100},
    {35, -800, 0}, {8, -100, 100},
};
char *g_SoundPaths[36] = {
    "se_plst00.wav",
    "se_enep00.wav",
    "se_pldead00.wav",
    "se_power0.wav",
    "se_power1.wav",
    "se_tan00.wav",
    "se_tan01.wav",
    "se_tan02.wav",
    "se_ok00.wav",
    "se_cancel00.wav",
    "se_select00.wav",
    "se_gun00.wav",
    "se_cat00.wav",
    "se_lazer00.wav",
    "se_lazer01.wav",
    "se_enep01.wav",
    "se_nep00.wav",
    "se_damage00.wav",
    "se_item00.wav",
    "se_kira00.wav",
    "se_kira01.wav",
    "se_kira02.wav",
    "se_extend.wav",
    "se_timeout.wav",
    "se_graze.wav",
    "se_powerup.wav",
    "se_pause.wav",
    "se_cardget.wav",
    "se_option.wav",
    "se_damage01.wav",
    "se_timeout2.wav",
    "se_opshow.wav",
    "se_ophide.wav",
    "se_invalid.wav",
    "se_slash.wav",
    "se_item01.wav",
};
SoundPlayer g_SoundPlayer;
#endif // TH08_EXE

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

char *FindRiffChunk(char *data, char *name, int *chunkSize, unsigned int size);

int SoundPlayer::Initialize(HWND hwnd)
{
    DSBUFFERDESC params;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    WAVEFORMATEX formatInfo;
    int m;
    int j;

    memset(this, 0, sizeof(*this));
    for (j = 0; j < 0x80; j++)
    {
        this->queuedSound[j] = -1;
    }
    for (m = 0; m < 0xc; m++)
    {
        this->queue[m] = -1;
    }
    this->manager = new CSoundManager();
    if (this->manager->Initialize(hwnd, DSSCL_PRIORITY, 2, 44100, 16) < 0)
    {
        LogPrintf(&g_Log, "DirectSound オブジェクトの初期化が失敗したよ\r\n");
        if (this->manager != NULL)
        {
            delete this->manager;
            this->manager = NULL;
        }
        return -1;
    }
    this->dsound = this->manager->GetDirectSound();
    this->notifyThread = NULL;
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    params.dwBufferBytes = 0x8000;
    memset(&formatInfo, 0, sizeof(formatInfo));
    formatInfo.cbSize = 0;
    formatInfo.wFormatTag = WAVE_FORMAT_PCM;
    formatInfo.nChannels = 2;
    formatInfo.nSamplesPerSec = 44100;
    formatInfo.nAvgBytesPerSec = 44100 * 4;
    formatInfo.nBlockAlign = 4;
    formatInfo.wBitsPerSample = 16;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->silence, NULL) < 0)
    {
        return -1;
    }
    if (this->silence->Lock(0, 0x8000, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        return -1;
    }
    memset(audioPtr_1, 0, 0x8000);
    this->silence->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    this->silence->Play(0, 0, DSBPLAY_LOOPING);
    this->bgmVolume = 100;
    this->seVolume = 100;
    SetTimer(hwnd, 0, 250, NULL);
    this->hwnd = hwnd;
    LogPrintf(&g_Log, "DirectSound は正常に初期化されました\r\n");
    return 0;
}

int SoundPlayer::Release()
{
    int i;

    if (this->bgmFormat != NULL)
    {
        g_MemoryManager.Free(this->bgmFormat);
    }
    for (i = 0; i < 0x80; i++)
    {
        if (this->sounds[i] != NULL)
        {
            this->sounds[i]->Release();
            this->sounds[i] = NULL;
        }
        if (this->soundBuffers[i] != NULL)
        {
            this->soundBuffers[i]->Release();
            this->soundBuffers[i] = NULL;
        }
    }
    if (!this->manager)
    {
        return 0;
    }
    KillTimer(this->hwnd, 1);
    this->StopBgm();
    this->dsound = NULL;
    this->silence->Stop();
    if (this->silence != NULL)
    {
        this->silence->Release();
        this->silence = NULL;
    }
    if (this->bgm != NULL)
    {
        delete this->bgm;
        this->bgm = NULL;
    }
    if (this->manager != NULL)
    {
        delete this->manager;
        this->manager = NULL;
    }
    for (i = 0; i < 0x10; i++)
    {
        this->FreePreloadedBgm(i);
    }
    return 0;
}

int SoundPlayer::FindTrackIndex(char *path)
{
    char name[0x80];
    int i;
    char *p;

    i = 0;
    p = strrchr(path, '/');
    if (p == NULL)
    {
        p = strrchr(path, '\\');
    }
    if (p == NULL)
    {
        strcpy(name, path);
    }
    else
    {
        strcpy(name, p + 1);
    }
    while (this->bgmFormat[i].name[0] != '\0')
    {
        if (strcmp(this->bgmFormat[i].name, name) == 0)
        {
            break;
        }
        i++;
    }
    if (this->bgmFormat[i].name[0] == '\0')
    {
        i = 0;
    }
    return i;
}

int SoundPlayer::LoadSound(int index, char *path)
{
    char *cursor;
    DSBUFFERDESC params;
    char *chunk;
    int size;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    char *wavBuf;
    WAVEFORMATEX formatInfo;
    int totalSize2;

    if (!this->manager)
    {
        return 0;
    }
    if (this->soundBuffers[index] != NULL)
    {
        this->soundBuffers[index]->Release();
        this->soundBuffers[index] = NULL;
    }
    wavBuf = (char *)LoadFileData(path, NULL, 0);
    cursor = wavBuf;
    if (cursor == NULL)
    {
        return -1;
    }
    if (strncmp(cursor, "RIFF", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    cursor += 4;
    totalSize2 = *(int *)cursor;
    cursor += 4;
    if (strncmp(cursor, "WAVE", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    cursor += 4;
    chunk = FindRiffChunk(cursor, "fmt ", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    formatInfo = *(WAVEFORMATEX *)chunk;
    chunk = FindRiffChunk(cursor, "data", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
    params.dwBufferBytes = size;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->soundBuffers[index], NULL) < 0)
    {
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    if (this->soundBuffers[index]->Lock(0, size, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    memcpy(audioPtr_1, chunk, sizeOne);
    if (size2 != 0)
    {
        memcpy(audioPtrTwo, chunk + sizeOne, size2);
    }
    this->soundBuffers[index]->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    g_MemoryManager.Free(wavBuf);
    return 0;
}

// Finds chunk `name` among the RIFF chunks at `data`, `size` bytes of
// them. Returns its contents and sets *chunkSize, or returns NULL.
char *FindRiffChunk(char *data, char *name, int *chunkSize, unsigned int size)
{
    while (size > 0)
    {
        *chunkSize = *(int *)(data + 4);
        if (strncmp(data, name, 4) == 0)
        {
            return data + 8;
        }
        size -= *chunkSize + 8;
        data += *chunkSize + 8;
    }
    return NULL;
}

int SoundPlayer::LoadBgmFormat(char *path)
{
    this->bgmFormat = (BgmFormat *)LoadFileData(path, NULL, 0);
    return this->bgmFormat != NULL ? 0 : -1;
}

int SoundPlayer::LoadBgm(char *path)
{
    DWORD notifySize;
    BgmFormat *format;
    HRESULT hr;
    INT nSamplesPerSec;
    DWORD nBlockAlign;

    strcpy(this->bgmPath, path);
    if (!this->manager)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return -1;
    }
    DebugPrint("Streming BGM Start\r\n");
    this->StopBgm();
    format = this->bgmFormat;
    nBlockAlign = format->format.nBlockAlign;
    nSamplesPerSec = format->format.nSamplesPerSec;
    notifySize = nSamplesPerSec * 4 * nBlockAlign / 16;
    notifySize -= notifySize % nBlockAlign;
    this->notifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->notifyThread = CreateThread(NULL, 0, BgmNotificationProc, g_App.hwnd, 0, &this->notifyThreadId);
    hr = this->manager->CreateStreaming(&this->bgm, path, DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2,
                                        GUID_NULL, 0x10, notifySize, this->notifyEvent, format);
    if (hr < 0)
    {
        DebugPrint("error : ストリーミング用サウンドバッファを作成出来ませんでした\r\n");
        return -1;
    }
    return 0;
}

int SoundPlayer::SwitchBgmTrack(char *path)
{
    int i;

    if (this->bgm == NULL)
    {
        return -1;
    }
    i = this->FindTrackIndex(path);
    this->bgm->GetWaveFile()->SwitchTrack(&this->bgmFormat[i]);
    DebugPrint("Streming BGM Reopen %d\r\n", i);
    return 0;
}

int SoundPlayer::PreloadBgm(int slot, char *path)
{
    int i;
    DWORD sizeRead;
    HANDLE file;
    void *data;

    if (this->bgmPreloadData[slot] != NULL && strcmp(path, this->bgmNames[slot]) == 0)
    {
        return 0;
    }
    strcpy(g_SoundPlayer.bgmNames[slot], path);
    if (!g_App.GetPreloadBgm())
    {
        return 0;
    }
    if (!this->manager)
    {
        return 0;
    }
    this->FreePreloadedBgm(slot);
    DebugPrint("Streming BGM PreLoad %d\r\n", slot);
    file = CreateFileA(this->bgmPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        DebugPrint("error : bgmfile is not find %s\r\n", this->bgmPath);
        return -1;
    }
    i = this->FindTrackIndex(path);
    SetFilePointer(file, this->bgmFormat[i].offset, NULL, FILE_BEGIN);
    data = g_MemoryManager.Alloc(this->bgmFormat[i].size, GLOBAL_H);
    if (data == NULL)
    {
        CloseHandle(file);
        DebugPrint("error : bgmfile is not find %s\r\n", this->bgmPath);
        return -1;
    }
    ReadFile(file, data, this->bgmFormat[i].size, &sizeRead, NULL);
    CloseHandle(file);
    this->bgmEntries[slot] = &this->bgmFormat[i];
    this->bgmPreloadData[slot] = data;
    this->bgmPreloadCursor[slot] = data;
    this->bgmPreloadSize[slot] = this->bgmEntries[slot]->size;
    return 0;
}

int SoundPlayer::PlayPreloadedBgm(int slot)
{
    DWORD notifySize;
    HRESULT hr;
    INT nSamplesPerSec;
    DWORD nBlockAlign;

    if (!this->manager)
    {
        return -1;
    }
    if (!g_App.musicMode)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return -1;
    }
    if (!g_App.GetPreloadBgm())
    {
        return this->SwitchBgmTrack(this->bgmNames[slot]);
    }
    if (this->bgmPreloadData[slot] == NULL)
    {
        return -1;
    }
    DebugPrint("Streming BGM Load no %d\r\n", slot);
    nBlockAlign = this->bgmEntries[slot]->format.nBlockAlign;
    nSamplesPerSec = this->bgmEntries[slot]->format.nSamplesPerSec;
    notifySize = nSamplesPerSec * 4 * nBlockAlign / 16;
    notifySize -= notifySize % nBlockAlign;
    this->notifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    this->notifyThread = CreateThread(NULL, 0, BgmNotificationProc, g_App.hwnd, 0, &this->notifyThreadId);
    hr = this->manager->CreateStreamingFromMemory(&this->bgm, this->bgmPreloadCursor[slot], this->bgmPreloadSize[slot],
                                   this->bgmEntries[slot], DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2,
                                   GUID_NULL, 0x10, notifySize, this->notifyEvent);
    if (hr < 0)
    {
        DebugPrint("error : ストリーミング用サウンドバッファを作成出来ませんでした\r\n");
        return -1;
    }
    DebugPrint("load comp\r\n");
    this->bgmPlaying = slot;
    return 0;
}

void SoundPlayer::StopBgm()
{
    if (this->bgm != NULL)
    {
        DebugPrint("Streming BGM stop\r\n");
        this->bgm->Stop();
        if (this->notifyThread != NULL)
        {
            PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
            DebugPrint("stop m_dwNotifyThreadID\r\n");
            while (WaitForSingleObject(this->notifyThread, 0x100) != WAIT_OBJECT_0)
            {
                PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
            }
            DebugPrint("stop comp\r\n");
            CloseHandle(this->notifyThread);
            CloseHandle(this->notifyEvent);
            this->notifyThread = NULL;
        }
        if (this->bgm != NULL)
        {
            delete this->bgm;
            this->bgm = NULL;
        }
    }
}

int SoundPlayer::LoadSounds()
{
    int i;

    if (!this->manager)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return 0;
    }
    for (i = 0; i < 0xc; i++)
    {
        this->queue[i] = -1;
    }
    for (i = 0; i < 0x24; i++)
    {
        if (this->LoadSound(i, g_SoundPaths[i]) != 0)
        {
            LogPrintf(&g_Log, "error : Sound ファイルが読み込めない データを確認 %s\r\n", g_SoundPaths[i]);
            return -1;
        }
    }
    for (i = 0; i < ARRAY_SIZE(g_SoundInfos); i++)
    {
        this->dsound->DuplicateSoundBuffer(this->soundBuffers[g_SoundInfos[i].buffer], &this->sounds[i]);
        this->sounds[i]->SetCurrentPosition(0);
        this->sounds[i]->SetVolume(g_SoundInfos[i].volume);
    }
    return 0;
}

void SoundPlayer::QueueSound(int idx, int arg)
{
    int i;
    int pri;

    pri = g_SoundInfos[idx].priority;
    for (i = 0; i < 0xc; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        if (this->queue[i] == idx)
        {
            if (this->queueArgCounts[i] < 0x80)
            {
                this->queueArgs[i][this->queueArgCounts[i]] = arg;
                this->queueArgCounts[i]++;
            }
            return;
        }
    }
    if (i >= 0xc)
    {
        return;
    }
    this->queue[i] = idx;
    this->queuedSound[idx] = pri;
    this->queueArgs[i][0] = arg;
    this->queueArgCounts[i]++;
}

void SoundPlayer::QueueSoundPanned(int idx, float x)
{
    int i;
    int pan;
    int pri;

    pri = g_SoundInfos[idx].priority;
    pan = (x - 192.0f) * 1000.0f / 192.0f;
    for (i = 0; i < 0xc; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        if (this->queue[i] == idx)
        {
            if (this->queueArgCounts[i] < 0x80)
            {
                this->queueArgs[i][this->queueArgCounts[i]] = pan;
                this->queueArgCounts[i]++;
            }
            return;
        }
    }
    if (i >= 0xc)
    {
        return;
    }
    this->queue[i] = idx;
    this->queuedSound[idx] = pri;
    this->queueArgs[i][0] = pan;
    this->queueArgCounts[i]++;
}

// Not matched yet: the original has a two-byte jump to the next
// instruction after case 7 that this code does not produce. Only the
// standalone exe builds it; the DLL keeps the original's.
#ifdef TH08_EXE
int SoundPlayer::PlayQueuedSounds()
{
    int sound;
    SoundCommand *command;
    int i;
    int arg;
    int advance;
    int j;

    if (!this->manager)
    {
        return 0;
    }
    command = this->commands;
again:
    advance = 0;
    switch (command->type)
    {
    case 8:
        if (this->bgm != NULL)
        {
            this->bgm->SetVolume(this->bgmVolume);
        }
        break;
    case 1:
        if (g_App.GetPreloadBgm())
        {
            DebugPrint("Sound : PreLoad Stage\r\n");
            if (command->stage == 0)
            {
                this->StopBgm();
                this->PreloadBgm(command->arg, command->path);
                advance = 1;
                break;
            }
        }
        else
        {
            DebugPrint("Sound : PreLoad Stage\r\n");
            this->PreloadBgm(command->arg, command->path);
            advance = 1;
            break;
        }
        command->stage++;
        goto done;
    case 2:
        if (g_App.GetPreloadBgm() && command->arg >= 0)
        {
            if (command->stage == 0)
            {
                DebugPrint("Sound : Load Stage\r\n");
                if (this->PlayPreloadedBgm(command->arg) != 0)
                {
                    break;
                }
            }
            else if (command->stage == 2)
            {
                DebugPrint("Sound : Reset Stage\r\n");
                if (this->bgm != NULL)
                {
                    if (this->bgm->Reset() < 0)
                    {
                        break;
                    }
                }
            }
            else if (command->stage == 5)
            {
                DebugPrint("Sound : Fill Buffer Stage\r\n");
                LPDIRECTSOUNDBUFFER buffer = this->bgm->GetBuffer(0);
                command->arg = this->bgm->GetWaveFile()->GetFormat()->loopEnd != 0;
                if (this->bgm->FillBufferWithSound(buffer, command->arg) < 0)
                {
                    break;
                }
            }
            else if (command->stage == 7)
            {
                DebugPrint("Sound : Play Stage\r\n");
                this->bgm->Play(0, DSBPLAY_LOOPING);
            }
            else if (command->stage >= 0x14)
            {
                break;
            }
        }
        else
        {
            if (this->bgm == NULL)
            {
                break;
            }
            if (command->stage == 0)
            {
                DebugPrint("Sound : Stop Stage\r\n");
                this->bgm->Stop();
            }
            else if (command->stage == 1)
            {
                if (this->bgm->isBuffering != 0)
                {
                    goto done;
                }
                DebugPrint("Sound : Recreate Stage\r\n");
                this->bgm->RecreateBuffers();
            }
            else if (command->stage == 2)
            {
                DebugPrint("Sound : ReOpen Stage\r\n");
                char *name = command->arg >= 0 ? this->bgmNames[command->arg] : command->path;
                int entry = this->FindTrackIndex(name);
                this->bgm->GetWaveFile()->SwitchTrack(&this->bgmFormat[entry]);
            }
            else if (command->stage == 3)
            {
                DebugPrint("Sound : Fill Buffer Stage\r\n");
                LPDIRECTSOUNDBUFFER buffer = this->bgm->GetBuffer(0);
                this->bgm->Reset();
                command->arg = this->bgm->GetWaveFile()->GetFormat()->loopEnd != 0;
                if (this->bgm->FillBufferWithSound(buffer, command->arg) < 0)
                {
                    break;
                }
            }
            else if (command->stage == 4)
            {
                DebugPrint("Sound : Play Stage\r\n");
                this->bgm->Play(0, DSBPLAY_LOOPING);
            }
            else if (command->stage >= 7)
            {
                break;
            }
        }
        command->stage++;
        goto done;
    case 4:
        if (this->bgm == NULL)
        {
            break;
        }
        if (command->stage == 0)
        {
            DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (command->stage == 1)
        {
            DebugPrint("Sound : Thread Stop Stage\r\n");
            if (this->notifyThread == NULL)
            {
                break;
            }
            PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
        }
        else if (command->stage == 2)
        {
            if (WaitForSingleObject(this->notifyThread, 0x100) != WAIT_OBJECT_0)
            {
                DebugPrint("Sound : Thread Stop Wait Stage\r\n");
                PostThreadMessageA(this->notifyThreadId, WM_QUIT, 0, 0);
                command->stage--;
            }
            else
            {
                this->notifyThread = NULL;
            }
        }
        else if (command->stage == 3)
        {
            DebugPrint("Sound : Handle Close Stage\r\n");
            CloseHandle(this->notifyThread);
            CloseHandle(this->notifyEvent);
            this->notifyThread = NULL;
            if (this->bgm != NULL)
            {
                delete this->bgm;
                this->bgm = NULL;
            }
        }
        else if (command->stage == 10)
        {
            break;
        }
        command->stage++;
        goto done;
    case 3:
        if (this->bgm == NULL)
        {
            break;
        }
        if (command->stage == 0)
        {
            DebugPrint("Sound : Stop Stage\r\n");
            this->bgm->Stop();
        }
        else if (command->stage == 1)
        {
            break;
        }
        command->stage++;
        goto done;
    case 5:
        DebugPrint("Sound : Fade Out Stage %d\r\n", command->arg);
        g_SoundPlayer.FadeOutBgm(command->arg);
        break;
    case 6:
        if (g_App.musicMode == 1)
        {
            if (this->bgm->isBuffering != 0)
            {
                DebugPrint("locked\n");
                goto done;
            }
            if (this->bgm != NULL)
            {
                this->bgm->PauseBuffer();
            }
        }
        break;
    case 7:
        if (g_App.musicMode == 1)
        {
            if (this->bgm->isBuffering != 0)
            {
                goto done;
            }
            if (this->bgm != NULL)
            {
                this->bgm->ResumeBuffer();
            }
        }
        break;
    default:
        goto done;
        break;
    }
    for (i = 0; i < 0x1f; i++, command++)
    {
        if (command->type == 0)
        {
            break;
        }
        *command = *(command + 1);
    }
    if (advance)
    {
        goto again;
    }
done:
    if (!g_App.playSounds)
    {
        return this->commands[0].type;
    }
    for (i = 0; i < 0xc; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        sound = this->queue[i];
        this->queue[i] = -1;
        arg = 0;
        for (j = 0; j < this->queueArgCounts[i]; j++)
        {
            arg += this->queueArgs[i][j];
        }
        arg /= this->queueArgCounts[i];
        this->queueArgCounts[i] = 0;
        if (this->sounds[sound] == NULL)
        {
            continue;
        }
        this->sounds[sound]->Stop();
        this->sounds[sound]->SetCurrentPosition(0);
        // The arguments' average is the pan.
        this->sounds[sound]->SetPan(arg);
        // As CSound::SetVolume, by g_SoundPlayer.seVolume with a cube.
        float scale = g_SoundPlayer.seVolume / 100.0f;
        if (g_SoundPlayer.seVolume != 0)
        {
            scale = (scale = 1.0f - scale) * scale * scale;
            scale = 1.0f - scale;
            this->sounds[sound]->SetVolume((LONG)((g_SoundInfos[sound].volume + 5000) * scale) - 5000);
        }
        else
        {
            this->sounds[sound]->SetVolume(DSBVOLUME_MIN);
        }
        this->sounds[sound]->Play(0, 0, 0);
    }
    return this->commands[0].type;
}
#endif // TH08_EXE

DWORD WINAPI BgmNotificationProc(LPVOID lpParameter)
{
    HRESULT hr;
    HWND hDlg = (HWND)lpParameter;
    MSG msg;
    DWORD dwResult;
    BOOL bDone = FALSE;
    BOOL bLooped = TRUE;

    while (!bDone)
    {
        dwResult = MsgWaitForMultipleObjects(1, &g_SoundPlayer.notifyEvent, FALSE, INFINITE, QS_ALLEVENTS);
        if (g_SoundPlayer.bgm == NULL)
        {
            bDone = TRUE;
        }
        switch (dwResult)
        {
        case WAIT_OBJECT_0 + 0:
            if (g_SoundPlayer.bgm != NULL && g_SoundPlayer.bgm->isPlaying)
            {
                g_SoundPlayer.bgm->isBuffering = 1;
                hr = g_SoundPlayer.bgm->HandleWaveStreamNotification(bLooped);
                g_SoundPlayer.bgm->isBuffering = 0;
            }
            break;
        case WAIT_OBJECT_0 + 1:
            while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    bDone = TRUE;
                }
            }
            break;
        }
    }
    DebugPrint("atention : ストリーミング用スレッドは終了しました。\r\n");
    return 0;
}

void SoundPlayer::QueueSoundCommand(int type, int arg, char *path)
{
    int i;

    for (i = 0; i < 0x1f; i++)
    {
        if (this->commands[i].type != 0)
        {
            continue;
        }
        this->commands[i].type = type;
        this->commands[i].arg = arg;
        strcpy(this->commands[i].path, path);
        this->commands[i].stage = 0;
        break;
    }
    DebugPrint("Sound Que Add %d\r\n", type);
}

EXE_INLINE SoundPlayer::SoundPlayer()
{
    int i;

    memset(this, 0, sizeof(*this));
    for (i = 0; i < 0x80; i++)
    {
        this->queuedSound[i] = -1;
    }
}
