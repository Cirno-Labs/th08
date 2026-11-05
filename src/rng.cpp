// Rng, 0x43ecc0 to 0x43edb0, as th06's rng.cpp (g_Random is defined in
// unsorted.cpp): split at a plausible seam (nothing in the binary shows
// it).

#include "unsorted.h"

WORD Rng::GetRandomU16()
{
    WORD a;

    a = (this->seed ^ 0x9630) - 0x6553;
    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xffff;
    this->generated++;
    return this->seed;
}

DWORD Rng::GetRandomU32()
{
    return (this->GetRandomU16() << 16) | this->GetRandomU16();
}

float Rng::GetRandomF32ZeroToOne()
{
    return this->GetRandomU32() / 4294967296.0f;
}

float Rng::GetRandomF32MinusOneToOne()
{
    return this->GetRandomU32() / 2147483648.0f - 1.0f;
}
