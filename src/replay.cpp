// The file with ReplayManager, around 0x451d90.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "enemy.h"
#include "lzss.h"
#include "replay.h"
#include "resultscreen.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

ReplayHeader *ValidateReplay(void *data, int size)
{
    BYTE *ptr2;
    int sum;
    BYTE *bytes;
    BYTE b;
    ReplayHeader *file;
    int i;
    ReplayHeader *decoded;

    file = (ReplayHeader *)data;
    if (file == NULL)
    {
        goto fail;
    }
    if (*(DWORD *)file->magic != *(DWORD *)"T8RP")
    {
        goto fail;
    }
    if (file->version != 6)
    {
        goto fail;
    }
    bytes = (BYTE *)&file->compressedSize;
    b = file->key;
    for (i = 0; i < file->size - 0x18; i++, bytes++)
    {
        *bytes -= b;
        b += 7;
    }
    ptr2 = &file->key;
    sum = 0x3f000318;
    for (i = 0; i < file->size - 0x15; i++, ptr2++)
    {
        sum += *ptr2;
    }
    if (sum != file->checksum)
    {
        goto fail;
    }
    decoded = (ReplayHeader *)g_MemoryManager.Alloc(file->dataSize + 0x68 + (size - file->size), GLOBAL_H);
    memcpy(decoded, data, 0x68);
    LzssDecompress((BYTE *)file + 0x68, file->compressedSize, (BYTE *)decoded + 0x68, file->dataSize);
    memcpy((BYTE *)decoded + file->dataSize + 0x68, (BYTE *)data + file->size, size - file->size);
    file = decoded;
    if (file->config[0x25] != 0)
    {
        goto failDecoded;
    }
    if (g_App.CheckVersionFile(file->exeVersion, file->exeSize, file->exeChecksum) != 0)
    {
        goto failDecoded;
    }
    g_MemoryManager.Free(data);
    return decoded;

fail:
    g_MemoryManager.Free(data);
    return NULL;

failDecoded:
    g_MemoryManager.Free(data);
    g_MemoryManager.Free(decoded);
    return NULL;
}
