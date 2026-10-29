// The music room. Only what is known so far.

#pragma once

class MusicScreen
{
public:
    // Adds the music room's chain elements. Returns 0, or nonzero on
    // failure (0x449254).
    static int RegisterChain();
};
