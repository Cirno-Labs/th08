// cflags: /Os /Ob1
// The /Os file with GameManager, as th07's, from 0x4399ac to the Chain
// code at 0x43c710.

#include <string.h>

#include "chain.h"
#include "replay.h"
#include "unsorted.h"
BOOL GameManager::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (x - width / 2.0f > 384.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (y - height / 2.0f > 448.0f)
    {
        return FALSE;
    }
    return TRUE;
}
