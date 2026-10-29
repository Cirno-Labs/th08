// The file from about 0x43d0c0: input, file I/O, the log, the random
// number generator and math helpers, as in th07's unsorted.cpp.

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#include "unsorted.h"

WORD InitJoystick()
{
    JOYINFOEX joyInfo;

    joyInfo.dwSize = sizeof(joyInfo);
    joyInfo.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
    {
        LogPrintf(&g_Log, "使えるパッドが存在しないようです、残念\r\n");
        return 1;
    }
    joyGetDevCapsA(JOYSTICKID1, &g_JoyCaps, sizeof(g_JoyCaps));
    return 0;
}
