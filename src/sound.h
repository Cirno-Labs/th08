// Sound effects and the wave music.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// A command PlayQueuedSounds carries out over several frames, for the
// streaming wave music. The same as th07's.
struct SoundCommand
{
    // 0 ends the queue.
    int type;
    int arg;
    // How far the command has got.
    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);

// An entry of thbgm.fmt: where a track is in thbgm.dat and its format.
// The same as th07's.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // CWaveFile::ResetFile limits the track to `loopEnd` bytes and, when
    // looping, skips its first `loopStart`. SoundPlayer::PlayQueuedSounds
    // fills the buffer with Loops() as the repeat flag.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;

    BOOL Loops()
    {
        return this->loopEnd != 0;
    }
};
ASSERT_SIZE(BgmFormat, 0x34);

// Plays the sound effects and music (at 0x18b8a68), as th07's. The queue
// of sound effects grew from 5 to 12 entries, each with up to 0x80
// arguments, and the music and sound effects got their own volumes.
class SoundPlayer
{
public:
    // Zeroes everything; no sound is loaded. Only g_SoundPlayer's static
    // initializer calls it; its copy is among the inline copies at the
    // end of the sound object (0x45e320). th07's was a plain method.
    SoundPlayer();
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure.
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound.
    int Release();
    // Stops the wave music: ends the notification thread and releases
    // the stream.
    void StopBgm();
    // The index in bgmFormat of the track named as the file name of
    // `path`, or 0 if there is none.
    int FindTrackIndex(char *path);
    // Loads the wave file at `path` into soundBuffers[`index`]. Returns
    // 0, also when there is no sound, or -1 on failure.
    int LoadSound(int index, char *path);
    // Loads the track list at `path` (thbgm.fmt) into bgmFormat. Returns
    // 0 or -1.
    int LoadBgmFormat(char *path);
    // Starts streaming the wave music from the file at `path`
    // (thbgm.dat), as bgmFormat's first track, after stopping what
    // plays, and starts the thread that streams it. Returns 0 or -1.
    int LoadBgm(char *path);
    // Switches the wave music to the track named as the file name of
    // `path`. Returns 0, or -1 when no music is loaded.
    int SwitchBgmTrack(char *path);
    // Reads the track named as the file name of `path` out of bgmPath
    // into preload slot `slot`, unless it is there already. Does nothing
    // unless g_App.flags.preloadBgm is set. Returns 0 or -1.
    int PreloadBgm(int slot, char *path);
    // Starts the wave music from preload slot `slot`, as LoadBgm does,
    // or switches the stream to its track without g_App.flags.preloadBgm.
    // Returns 0 or -1.
    int PlayPreloadedBgm(int slot);
    // Loads the sound effects ("data/wav/se_plst00.wav", ...).
    int LoadSounds();
    // Carries out the commands' next stages and plays the queued sound
    // effects. Returns the first command's type: WinMain calls it until
    // it returns 0.
    int PlayQueuedSounds();
    // Fades the wave music out over `seconds`. An out-of-line copy at the
    // end of the sound object (0x45e3c0).
    void FadeOutBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeOut(seconds);
        }
    }
    // Fades the wave music in over `seconds`. New in th08; inline, the
    // copy is at 0x406ac0.
    void FadeInBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeIn(seconds);
        }
    }
    // The same for fades 4 (down) and 3 (up). New in th08; inline, the
    // copies are at 0x406b40 and 0x406bb0.
    void FadeDownBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeDown(seconds);
        }
    }
    void FadeUpBgm(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeUp(seconds);
        }
    }
    // Queues the "Pause" command (6), as th07 did by hand. Inline; the
    // copy is at 0x406c30.
    void Pause()
    {
        this->QueueSoundCommand(6, 0, "Pause");
    }
    // Queues the "UnPause" command (7). Inline; the copy is at 0x406c50.
    void UnPause()
    {
        this->QueueSoundCommand(7, 0, "UnPause");
    }
    // Frees preload slot `slot`'s data, if any. An out-of-line copy at
    // the end of the sound object (0x45e370). Not in th07.
    void FreePreloadedBgm(int slot)
    {
        if (this->bgmPreloadData[slot] != NULL)
        {
            g_MemoryTracker.Free(this->bgmPreloadData[slot]);
            this->bgmPreloadData[slot] = NULL;
        }
    }
    // Queues sound effect `idx` with `arg` to start on the next update:
    // a sound already queued collects up to 0x80 arguments, and up to 12
    // sounds are queued. th07's queued 5, without arguments.
    void QueueSound(int idx, int arg);
    // QueueSound with the pan for horizontal position `x`: -1000 at 0,
    // the left edge, to 1000 at 384. Not in th07.
    void QueueSoundPanned(int idx, float x);
    // Queues a command of `type` with `arg` and `path` in the first free
    // entry of `commands`.
    void QueueSoundCommand(int type, int arg, char *path);
    // Steps the BGM's fades, if there is a BGM: UpdateFade, then
    // UpdateFadeIn, UpdateFadeDown and UpdateFadeUp. In App's object
    // (0x447764); th07's App::OnUpdate called UpdateFade itself.
    void UpdateBgmFades();

    IDirectSound8 *dsound;
    DWORD pad_004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set from g_SoundInfos[].unk_6 when a sound is queued (nothing reads
    // either); -1 at first.
    int unk_408[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD pad_61c;
    // The sound effects to play, or -1.
    int queue[0xc];
    // How many arguments each queued sound has, and the arguments. Not
    // in th07.
    int queueArgCounts[0xc];
    int queueArgs[0xc][0x80];
    // The thbgm.fmt entries of the preloaded tracks, their data, and
    // its size.
    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];
    // The preloaded track playing.
    int bgmPlaying;
    // The contents of thbgm.fmt.
    BgmFormat *bgmFormat;
    // The commands for the wave music, in order.
    SoundCommand commands[0x20];
    // The file names of the preloaded tracks.
    char bgmNames[0x10][0x100];
    // The file the wave music streams from.
    char bgmPath[0x100];
    // The wave music playing; NULL when stopped.
    CStreamingSound *bgm;
    // Signalled as the wave music plays.
    HANDLE notifyEvent;
    DWORD pad_5210;
    // Added to every BgmFormat offset by CWaveFile::ResetFile (as
    // th07's).
    DWORD bgmFileOffset;
    // The music and sound effect volumes in percent, 100 at first, then
    // copied from the config (App+0x13f, App+0x140). Not in th07:
    // CSound::SetVolume scales by bgmVolume.
    int bgmVolume;
    int seVolume;
    DWORD bgmBufferVolume;
};
ASSERT_SIZE(SoundPlayer, 0x5224);
extern SoundPlayer g_SoundPlayer;
