// The title menu. Only what is known so far.

#pragma once

#include "common.h"
#include "unsorted.h"

// The 0x3c bytes of App's config, from controllerMapping, as one block
// to copy (0x38 in th07).
struct ConfigCopy
{
    unsigned char unk_0[0x1e];
    BYTE colorMode16Bits;
    unsigned char unk_1f[0x3];
    BYTE windowed;
    unsigned char unk_23[0x19];
};
ASSERT_SIZE(ConfigCopy, 0x3c);

// The button mapping the key config's reset entry restores.
extern ControllerMapping g_DefaultControllerMapping;
