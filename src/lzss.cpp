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

// The next input byte into currentByte (0 past the end), added to
// checksum.
#define FETCH_BYTE() \
    { \
        currentByte = *in; \
        if (in - data >= size) \
        { \
            currentByte = 0; \
        } \
        else \
        { \
            in++; \
        } \
        checksum += currentByte; \
    }

// The next input bit.
#define READ_BIT(bit) \
    { \
        if (mask == 0x80) \
        { \
            FETCH_BYTE(); \
        } \
        bit = currentByte & mask; \
        mask >>= 1; \
        if (mask == 0) \
        { \
            mask = 0x80; \
        } \
    }

// The next `count` input bits, most significant first. Needs a DWORD
// `bitMask`.
#define READ_BITS(value, count) \
    { \
        bitMask = 1 << ((count) - 1); \
        value = 0; \
        while (bitMask != 0) \
        { \
            if (mask == 0x80) \
            { \
                FETCH_BYTE(); \
            } \
            if (currentByte & mask) \
            { \
                value |= bitMask; \
            } \
            bitMask >>= 1; \
            mask >>= 1; \
            if (mask == 0) \
            { \
                mask = 0x80; \
            } \
        } \
    }

void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize)
{
    int wndPos;
    DWORD bitMask;
    int ch;
    // The sum of the input bytes; unused.
    DWORD checksum;
    int len;
    int size;
    DWORD bits;
    BYTE *in;
    BYTE mask;
    int i;
    // The position of a match.
    int position;
    BYTE *dst;
    DWORD currentByte;

    mask = 0x80;
    currentByte = 0;
    checksum = 0;
    size = dataSize;
    if (out == NULL)
    {
        out = (BYTE *)GlobalAlloc(GMEM_FIXED, outSize);
        if (out == NULL)
        {
            return NULL;
        }
    }
    in = data;
    dst = out;
    wndPos = 1;
    for (;;)
    {
        READ_BIT(bits);
        if (bits)
        {
            READ_BITS(bits, 8);
            *dst = bits;
            dst++;
            g_LzssWindow[wndPos] = bits;
            wndPos = (wndPos + 1) & 0x1fff;
        }
        else
        {
            READ_BITS(bits, 13);
            position = bits;
            if (position == 0)
            {
                break;
            }
            READ_BITS(bits, 4);
            len = bits + 2;
            for (i = 0; i <= len; i++)
            {
                ch = g_LzssWindow[(position + i) & 0x1fff];
                *dst = ch;
                dst++;
                g_LzssWindow[wndPos] = ch;
                wndPos = (wndPos + 1) & 0x1fff;
            }
        }
    }
    while (mask != 0x80)
    {
        READ_BIT(bits);
    }
    return out;
}
