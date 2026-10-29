// A frame counter that can run in fractions of a frame.

#pragma once

#include <windows.h>

#include "common.h"

struct Timer
{
    // Starts counting from 0. Inline; the copy is at 0x4065a0.
    Timer()
    {
        this->Initialize();
    }

    // Starts counting from 0 again, as the constructor.
    void Initialize()
    {
        this->current = 0;
        this->previous = -999;
        this->subFrame = 0.0f;
    }

    // Sets the count to `frames`, with no fraction and no previous count
    // (-999).
    void Set(int frames)
    {
        this->current = frames;
        this->subFrame = 0.0f;
        this->previous = -999;
    }
    // Set, as an assignment. Inline; the copy is at 0x4065f0.
    void operator=(int frames)
    {
        this->Set(frames);
    }
    // Advances by `frames`, in fractions of a frame while the game runs
    // slow; first by one more frame, with no fraction or previous count,
    // while bit 0x20 of g_App.stateFlags is set.
    void Add(int frames);
    // The same backwards.
    void Sub(int frames);
    // Add, as th06's operator+=.
    void operator+=(int frames)
    {
        this->Add(frames);
    }
    // Advances by one frame through App::TickTimer. Inline; defined in
    // unsorted.h.
    void Tick();
    // Tick, returning the new count. Inline in th08, defined in
    // unsorted.h; the copy is at 0x406660.
    int NextTick();
    void operator++()
    {
        this->NextTick();
    }
    // The same as a postfix. Inline; the copy is at 0x406640.
    void operator++(int)
    {
        this->NextTick();
    }
    // Sub(1), as a postfix. Inline; the copy is at 0x418110.
    void operator--(int)
    {
        this->Sub(1);
    }

    // The count with its fraction.
    float AsFloat()
    {
        return (float)this->current + this->subFrame;
    }
    // The count without its fraction.
    int AsFrames()
    {
        return this->current;
    }
    // Whether the count changed since the last Tick or Set.
    BOOL HasTicked()
    {
        return this->current != this->previous;
    }
    BOOL HasTickedTo(int frames)
    {
        return this->current != this->previous && this->current == frames;
    }
    // Whether it has just ticked to a multiple of `n`. New in th08.
    BOOL HasTickedEvery(int n)
    {
        return this->current != this->previous && this->current % n == 0;
    }
    BOOL operator==(int frames)
    {
        return this->current == frames;
    }
    // Inline; the copy at 0x441830 is the item object's.
    BOOL operator!=(int frames)
    {
        return this->current != frames;
    }
    BOOL operator<(int frames)
    {
        return this->current < frames;
    }
    BOOL operator>(int frames)
    {
        return this->current > frames;
    }
    BOOL operator>=(int frames)
    {
        return this->current >= frames;
    }
    BOOL operator<=(int frames)
    {
        return this->current <= frames;
    }
    // The count modulo `n`.
    int operator%(int n)
    {
        return this->current % n;
    }

    // `current` before the last change.
    int previous;
    float subFrame;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
