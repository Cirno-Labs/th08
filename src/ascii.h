// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

class AnmLoaded;

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
    // AddFormat that returns the length of the formatted text (0x402a80).
    int AddFormatLen(D3DXVECTOR3 *pos, char *fmt, ...);
    // Queues `text` to be drawn at `pos` (0x402920).
    void AddString(D3DXVECTOR3 *pos, char *text);
    // Draws the queued strings (0x402b20).
    void DrawStrings();
    // Resets the text state, as th07's (0x402450).
    void Initialize();
    // Starts the scripts of a few of its sprites (0x402600). New in th08.
    void Func402600();
    // Queues `count` and `value` as a string of glyphs 0xf, `value`'s
    // digits, 0xe, `count`'s digits and 0xd at `pos`, in `color`
    // (0x403600). New in th08; DrawBanners shows the first boss's with
    // it.
    void Func403600(D3DXVECTOR3 *pos, int count, int value, D3DCOLOR color);

    // Inline; /Os files call the copy at 0x4398e8.
    void SetColor(D3DCOLOR color)
    {
        this->color = color;
    }
    void SetAlpha(int alpha)
    {
        this->alpha = alpha;
    }
    // Inline; the copy at 0x42f2f0 is in an /Od object.
    void SetScale(float x, float y)
    {
        this->scaleX = x;
        this->scaleY = y;
    }
    // Inline; the copy at 0x4398ff is the Gui object's.
    void SetInArena(int inArena)
    {
        this->inArena = inArena;
    }
    // Inline; the copy at 0x407180 is in an /Od object.
    void SetGlyphWidth(int width)
    {
        this->glyphWidth = width;
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
    // Whether positions are relative to the play area, as th07's.
    int inArena;
    int unk_8278;
    int unk_827c;
    // The advance of a character, in pixels, as th07's. (Named
    // g_StatisticsColumnWidth 0x4d50a0 when only its address was known.)
    int glyphWidth;
    int unk_8284;
    // ascii.anm and a second ANM, which the Gui also runs scripts from.
    AnmLoaded *anm;
    AnmLoaded *anm2;
};
extern AsciiText g_AsciiText;
