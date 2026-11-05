// The log helpers, 0x43ea00 to 0x43ecc0, as th06's log.cpp (g_AppLog is
// defined in unsorted.cpp): split at a plausible seam (nothing in the
// binary shows it).

#include <stdio.h>
#include <stdarg.h>
#include "unsorted.h"

int LogPrintf(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x2000];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}

int LogError(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    log->hasError = 1;
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}
