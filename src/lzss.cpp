// The LZSS coder, from 0x473cb0.

#include "lzss.h"

// The next input byte, or -1 past the end.
#define GET_BYTE(c) \
    { \
        if (in - data >= size) \
        { \
            c = -1; \
        } \
        else \
        { \
            c = *in; \
            in++; \
        } \
    }

// Appends one bit to the output.
#define PUT_BIT(bit) \
    { \
        if (bit) \
        { \
            currentByte |= mask; \
        } \
        mask >>= 1; \
        if (mask == 0) \
        { \
            *dst = currentByte; \
            dst++; \
            checksum += currentByte; \
            currentByte = 0; \
            mask = 0x80; \
        } \
    }

// Appends the low `count` bits of `value`, most significant first.
// Needs a DWORD `bitMask`.
#define PUT_BITS(value, count) \
    { \
        bitMask = 1 << ((count) - 1); \
        while (bitMask != 0) \
        { \
            PUT_BIT(bitMask & (value)); \
            bitMask >>= 1; \
        } \
    }

void *LzssCompress(BYTE *data, int size, int *outSize)
{
    HGLOBAL hMem;
    DWORD currentByte;
    // The position of the match found last.
    int position;
    BYTE *dst;
    int replaceLen;
    int i;
    BYTE mask;
    BYTE *in;
    // The length of the match found last.
    int len;
    // The sum of the output bytes; unused.
    DWORD checksum;
    DWORD bitMask;
    // The window position being coded.
    int wndPos;
    int ch;
    int lookAheadBytes;

    mask = 0x80;
    currentByte = 0;
    checksum = 0;
    hMem = GlobalAlloc(GMEM_FIXED, size * 2);
    if (hMem == NULL)
    {
        return NULL;
    }
    in = data;
    dst = (BYTE *)hMem;
    *outSize = 0;
    LzssClearWindow();
    wndPos = 1;
    for (i = 0; i < 0x12; i++)
    {
        GET_BYTE(ch);
        if (ch == -1)
        {
            break;
        }
        g_LzssWindow[wndPos + i] = ch;
    }
    lookAheadBytes = i;
    LzssTreeInitNode(wndPos);
    len = 0;
    position = 0;
    while (lookAheadBytes > 0)
    {
        if (len > lookAheadBytes)
        {
            len = lookAheadBytes;
        }
        if (len <= 2)
        {
            replaceLen = 1;
            PUT_BIT(1);
            PUT_BITS(g_LzssWindow[wndPos], 8);
        }
        else
        {
            PUT_BIT(0);
            PUT_BITS(position, 13);
            PUT_BITS(len - 3, 4);
            replaceLen = len;
        }
        for (i = 0; i < replaceLen; i++)
        {
            LzssDeleteNode((wndPos + 0x12) & 0x1fff);
            GET_BYTE(ch);
            if (ch == -1)
            {
                lookAheadBytes--;
            }
            else
            {
                g_LzssWindow[(wndPos + 0x12) & 0x1fff] = ch;
            }
            wndPos = (wndPos + 1) & 0x1fff;
            if (lookAheadBytes != 0)
            {
                len = LzssInsertNode(wndPos, &position);
            }
        }
    }
    PUT_BIT(0);
    PUT_BITS(0, 13);
    *outSize = dst - (BYTE *)hMem;
    return hMem;
}
