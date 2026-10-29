// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

class AsciiText
{
public:
    // Queues `text` to be drawn at `pos`, with printf-style formatting
    // (0x402a30).
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // Draws the queued strings (0x402b20).
    void DrawStrings();

    void SetAlpha(int alpha)
    {
        this->alpha = alpha;
    }

    unsigned char unk_0[0x8264];
    // How many strings are queued.
    int stringCount;
    // The color text is queued with.
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE unk_rgb[3];
            BYTE alpha;
        };
    };
    // The scale of the characters added next.
    float scaleX;
    float scaleY;
};
extern AsciiText g_AsciiText;
