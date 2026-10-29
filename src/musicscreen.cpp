// cflags: /Os
// The music screen's object, from 0x448a65 right after App's: built with
// /Os like it, but without /Ob1, so inlines such as
// Sprite::SetPendingInterrupt are called.

#include <stdlib.h>
#include <string.h>

#include "ascii.h"
#include "musicscreen.h"
#include "unsorted.h"

int MusicScreen::UpdateIntro()
{
    int i;

    if (this->stateFrames == 0)
    {
        for (i = 0; i < 0x1f; i++)
        {
            if (this->cursor == i)
            {
                this->titles[i].SetPendingInterrupt(1);
            }
            else
            {
                this->titles[i].SetPendingInterrupt(2);
            }
        }
        for (i = 0; i < 7; i++)
        {
            this->comment[i].SetPendingInterrupt(1);
        }
    }
    if (this->stateFrames >= 8)
    {
        this->state = 1;
    }
    return 0;
}
