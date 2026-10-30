// Text drawn with the ASCII font. Only what is known so far.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// A string queued by AsciiText::AddString, as th07's.
struct AsciiString
{
    char text[0x40];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    float scaleX;
    float scaleY;
    // Drawn with the second glyph set, in white.
    int highlight;
    // Drawn inside the play area's viewport.
    int inArena;
};
ASSERT_SIZE(AsciiString, 0x60);

// The pause menu, AsciiText's at 0x82a0, as th07's. Only what is known so
// far.
class PauseMenu
{
public:
    // While g_GameManager is paused (0x4037b0).
    int UpdatePauseMenu();
    // Draws it while paused (0x404750).
    void DrawPauseMenu();

    int state;
    // Frames since the state changed.
    int stateFrames;
    Sprite sprites[10];
    // The copy of the play area behind the menu.
    Sprite background;
};
ASSERT_SIZE(PauseMenu, 0x1d14);

// The continue menu, AsciiText's at 0x9fb4, as th07's.
class ContinueMenu
{
public:
    // While g_GameManager.isInContinueMenu is set (0x404890).
    int UpdateContinueMenu();
    // Draws it while in the continue menu (0x4052b0).
    void DrawContinueMenu();

    int state;
    int stateFrames;
    Sprite sprites[6];
    Sprite background;
};
ASSERT_SIZE(ContinueMenu, 0x1284);

// A number shown at a position for a while, as th07's (0x38 bytes now).
struct AsciiPopup
{
    // Least significant first, or the glyphs to draw.
    BYTE digits[0xc];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    Timer timer;
    float scaleX;
    float scaleY;
    BYTE active;
    BYTE digitCount;
    unsigned char pad_32[0x6];
};
ASSERT_SIZE(AsciiPopup, 0x38);

class AsciiText
{
public:
    // Constructs the members, for g_AsciiText's static initialization
    // (0x402000).
    AsciiText();
    // Adds AsciiText's update element (priority 1) and its draw
    // elements (0x14 and 0xe), as th07's. Returns 0, or -1 if adding the
    // update element fails (0x402780).
    static int RegisterChain();
    // Removes the update element and the first draw element (0x4028f0).
    static void RemoveChain();
    // Initializes everything, as th07's: zeroes the sprites, strings,
    // menus and popups, sets the color to white and the scale to 1, sets
    // digit, digit2 (sprite 0x88, new) and glyph up and glyphWidth to 13
    // (th07 14) (0x402450).
    void Initialize();
    // Steps summaryPopup to summaryPopup4, summaryDigit, nearbyLabels[] and
    // summaryBombIcon for a frame, as th07's. Inline; the copy is at 0x406fd0.
    void RunScripts()
    {
        g_SpriteEngine->Step(&this->summaryPopup);
        g_SpriteEngine->Step(&this->summaryPopup2);
        g_SpriteEngine->Step(&this->summaryPopup3);
        g_SpriteEngine->Step(&this->summaryPopup4);
        g_SpriteEngine->Step(&this->summaryDigit);
        g_SpriteEngine->Step(&this->nearbyLabels[0]);
        g_SpriteEngine->Step(&this->nearbyLabels[1]);
        g_SpriteEngine->Step(&this->nearbyLabels[2]);
        g_SpriteEngine->Step(&this->nearbyLabels[3]);
        g_SpriteEngine->Step(&this->summaryBombIcon);
    }
    // Starts the scripts of the summary sprites (5 to 9, 4 for summaryDigit)
    // and nearbyLabels[] (10), moves summaryPopup2 and summaryPopup3 by
    // g_GameManager.gaugeMin and gaugeMax and resends
    // summaryPendingInterrupt (0x402600). th07's StartScripts.
    void StartScripts();
    // The update element's callback, as th07's: moves the popups up and
    // retires them after 60 frames (90 for numberPopups) unless the game
    // is paused, in the continue menu or holds still (bit 10 of
    // GameManager::stateFlags), runs the menus, the summary sprites and
    // demoIcon (script 11) while a demo plays, and counts frameCounter
    // up. Returns 1 (0x402200).
    static int OnUpdate(AsciiText *text);
    // The first draw element's callback: draws the queued strings and
    // empties the queue, draws the menus, then demoIcon while it runs a
    // script, as th07's. Returns 1 (0x4023d0).
    static int OnDraw(AsciiText *text);
    // The second draw element's callback: draws the popups. Returns 1
    // (0x402430).
    static int OnDraw2(AsciiText *text);
    // The update element's added callback: zeroes the object, loads
    // ascii.anm (slot 1) and capture.anm (slot 3), then initializes and
    // starts the sprites' scripts. Returns 0, or -1 if loading fails
    // (0x402830).
    static int AddedCallback(AsciiText *text);
    // The update element's deleted callback: frees ANM slots 1 and 3.
    // Returns 0 (0x4028c0).
    static int DeletedCallback(AsciiText *text);
    // Draws the queued strings (0x402b20).
    void DrawStrings();
    // Draws the popups (0x405420).
    void DrawPopups();
    // Draws `value` in hundredths at `pos` in `color` with summaryDigit: a
    // minus, the whole part, ascii.anm's sprite 0x93, two smaller
    // decimals (100.00 for 10000 and more), then sprite 0x92. For
    // DrawPopups' stats->gauge (0x405e10). New in th08.
    void DrawPercent(D3DXVECTOR3 *pos, int value, D3DCOLOR color);
    // Queues `text` to be drawn at `pos` (0x402920).
    void AddString(D3DXVECTOR3 *pos, char *text);
    // Queues `text` to be drawn at `pos`, with printf-style formatting
    // (0x402a30).
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // AddFormat that returns the length of the formatted text (0x402a80).
    int AddFormatLen(D3DXVECTOR3 *pos, char *fmt, ...);
    // Shows `value` rising from `pos` (play-area coordinates) in `color`
    // for 60 frames, in one of the first 0x2d0 popups, as th07's
    // (0x403200).
    void AddPopup(D3DXVECTOR3 *pos, int value, D3DCOLOR color);
    // Like AddPopup, in the last three popups, as th07's (0x403330).
    void AddPopup2(D3DXVECTOR3 *pos, int value, D3DCOLOR color);
    // AddNumberPopupBrief at `pos` in play-area coordinates, in the scale
    // text is added with (0x403460). New in th08.
    void AddNumberPopup(D3DXVECTOR3 *pos, int count, int value, D3DCOLOR color);
    // Queues `count` and `value` as a string of glyphs 0xf, `value`'s
    // digits, 0xe, `count`'s digits and 0xd at `pos`, in `color`, in the
    // next of numberPopups (0x403600). New in th08; DrawBanners shows the
    // first boss's with it.
    void AddNumberPopupBrief(D3DXVECTOR3 *pos, int count, int value, D3DCOLOR color);

    // Inline; /Os files call the copy at 0x4398e8.
    void SetColor(D3DCOLOR color)
    {
        this->color = color;
    }
    void SetHighlight(int highlight)
    {
        this->highlight = highlight;
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
    // Inline; the copy is at 0x407180.
    void SetGlyphWidth(int width)
    {
        this->glyphWidth = width;
    }
    // Empties the queue of strings. Inline; the copy is at 0x407160.
    void ClearStrings()
    {
        this->stringCount = 0;
    }
    // summaryPendingInterrupt, and setting it while sending summaryPopup
    // to summaryPopup4 the same interrupt, as th07's. Inline; the copies
    // are at 0x407140 and 0x4070b0.
    int GetSummaryPendingInterrupt()
    {
        return this->summaryPendingInterrupt;
    }
    void SetSummaryPendingInterrupt(int value)
    {
        this->summaryPopup.SetPendingInterrupt(value);
        this->summaryPopup2.SetPendingInterrupt(value);
        this->summaryPopup3.SetPendingInterrupt(value);
        this->summaryPopup4.SetPendingInterrupt(value);
        this->summaryPendingInterrupt = value;
    }

    // Moves nearbyLabels[i] to `pos`, as th07's. Inline; the copy at
    // 0x422be0 is in an /Od object.
    void SetNearbyLabelPos(int i, D3DXVECTOR3 *pos)
    {
        this->nearbyLabels[i].pos = *pos;
    }
    // Sets nearbyLabelDimmed[i], as th07's: 1 dims the label, 2 to 4
    // colour it by the boss's danger level. Inline; the copy is at
    // 0x42f2d0.
    void SetNearbyLabelDimmed(int i, int value)
    {
        this->nearbyLabelDimmed[i] = value;
    }
    // Sends nearbyLabels[i] `interrupt`. Inline; the copy at 0x422bb0
    // is in an /Od object.
    void SetNearbyLabelInterrupt(int i, short interrupt)
    {
        this->nearbyLabels[i].SetPendingInterrupt(interrupt);
    }

    // The glyph DrawStrings draws each character with, as th07's.
    Sprite glyph;
    // The digit the popups are drawn with, as th07's, and a second one
    // (sprite 0x88). New in th08.
    Sprite digit;
    Sprite digit2;
    // Sent summaryPendingInterrupt, as th07's summaryPopup.
    Sprite summaryPopup;
    Sprite summaryPopup2;
    Sprite summaryPopup3;
    Sprite summaryPopup4;
    Sprite summaryDigit;
    Sprite summaryBombIcon;
    Sprite nearbyLabels[4];
    // Whether nearbyLabels[i] is drawn dimmed, as th07's.
    int nearbyLabelDimmed[4];
    AsciiString strings[0x100];
    // How many strings are queued.
    int stringCount;
    // The color text is queued with.
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };
    // The scale of the characters added next.
    float scaleX;
    float scaleY;
    // Whether positions are relative to the play area, as th07's.
    int inArena;
    // Drawn with the second glyph set, as th07's.
    int highlight;
    // As th07's.
    int summaryPendingInterrupt;
    // The advance of a character, in pixels, as th07's.
    int glyphWidth;
    // The frames OnUpdate has run. New in th08.
    DWORD frameCounter;
    // ascii.anm and capture.anm, which the Gui also runs scripts from.
    AnmLoaded *anm;
    AnmLoaded *anm2;
    // As th07's nextPopup and nextPopup2, and the next of numberPopups.
    int nextPopup;
    int nextPopup2;
    int nextNumberPopup;
    int unusedAfterInit829c;
    PauseMenu pauseMenu;
    ContinueMenu continueMenu;
    Sprite demoIcon;
    AsciiPopup popups[0x2d3];
    // The strings AddNumberPopupBrief queues. New in th08.
    AsciiPopup numberPopups[0x80];
    // While ringAlpha is above 0, DrawPopups darkens the play area
    // outside a square of ringRadius around the player, by ringAlpha, and
    // draws ring (etama.anm's script 0x69) scaled to it. New in th08.
    float ringRadius;
    int ringAlpha;
    Sprite ring;
};
ASSERT_SIZE(AsciiText, 0x171b0);
extern AsciiText g_AsciiText;

// AsciiText's chain elements: the update element and the two draw
// elements, as th07's.
extern ChainElem g_AsciiCalcChain;
extern ChainElem g_AsciiTextElemDraw;
extern ChainElem g_AsciiDrawChain2;

// Adds 1 to `*count` unless it reached `max`, as th07's (which ignored
// `max`). ContinueMenu::UpdateContinueMenu calls it on six counters of
// g_GameManager (0x405280).
void IncrementCounterCapped(DWORD *count, DWORD max);
