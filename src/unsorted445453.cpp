// cflags: /Os
// The file from 0x445453: the App chain callbacks, DirectInput set-up,
// the frame timer, the config and the music, as in th07's
// unsorted437903.cpp. Built with /Os: its functions are laid out without
// alignment.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unsorted.h"

App::App()
{
    memset(this, 0, sizeof(App));
    this->unk_1a4 |= 0x40;
    this->unk_1a4 |= 0x100;
}
