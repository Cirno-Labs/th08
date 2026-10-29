// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

class AsciiText
{
public:
    // Adds AsciiText's update and draw elements, as th07's. Returns 0,
    // or -1 if adding the update element fails (0x402780).
    static int RegisterChain();
    // Removes the update element and the first draw element (0x4028f0).
    static void RemoveChain();
    // Queues `text` to be drawn at `pos`, with printf-style formatting
    // (0x402a30).
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // Queues `text` to be drawn at `pos` (0x402920).
    void AddString(D3DXVECTOR3 *pos, char *text);
    // Draws the queued strings (0x402b20).
    void DrawStrings();

    // Inline; /Os files call the copy at 0x4398e8.
    void SetColor(D3DCOLOR color)
    {
        this->color = color;
    }
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
