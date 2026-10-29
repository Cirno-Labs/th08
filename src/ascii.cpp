// Text drawn with the ASCII font, the pause and continue menus and the
// popups, as th07's ascii.cpp. The first object of the game, from
// 0x402000; its inline copies run to about 0x4071a0.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "chain.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

AsciiText::AsciiText()
{
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    AsciiPopup *entry;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        entry = text->popups;
        if (!g_GameManager.unk_3dbac_10)
        {
            for (i = 0; i < 0x2d3; i++, entry++)
            {
                if (!entry->active)
                {
                    continue;
                }
                entry->pos.y -= 0.5f * g_App.gameSpeed;
                entry->timer++;
                if (entry->timer > 60)
                {
                    entry->active = 0;
                }
            }
            entry = text->numberPopups;
            for (i = 0; i < 0x80; i++, entry++)
            {
                if (!entry->active)
                {
                    continue;
                }
                entry->timer++;
                if (entry->timer > 90)
                {
                    entry->active = 0;
                }
            }
        }
    }
    else if (g_GameManager.isPaused)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_GameManager.isInContinueMenu)
    {
        text->continueMenu.UpdateContinueMenu();
    }
    text->RunScripts();
    if (g_GameManager.IsDemo())
    {
        if (text->demoIcon.scriptIndex == 0)
        {
            text->anm->StartScript(&text->demoIcon, 11);
        }
        g_SpriteEngine->Step(&text->demoIcon);
    }
    else
    {
        text->demoIcon.scriptIndex = 0;
    }
    text->frameCounter++;
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->ClearStrings();
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    if (text->demoIcon.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->demoIcon);
    }
    return 1;
}

int AsciiText::OnDraw2(AsciiText *text)
{
    text->DrawPopups();
    return 1;
}

void AsciiText::Initialize()
{
    memset(&this->digit, 0, sizeof(this->digit));
    memset(&this->digit2, 0, sizeof(this->digit2));
    memset(&this->glyph, 0, sizeof(this->glyph));
    memset(this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(this->pauseMenu));
    memset(&this->continueMenu, 0, sizeof(this->continueMenu));
    memset(this->popups, 0, sizeof(this->popups));
    memset(this->numberPopups, 0, sizeof(this->numberPopups));
    this->stringCount = 0;
    this->inArena = 0;
    this->highlight = 0;
    this->nextPopup = 0;
    this->nextPopup2 = 0;
    this->unusedAfterInit829c = 0;
    this->color = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->digit.flags |= 0x1800;
    this->digit2.flags |= 0x1800;
    this->anm->InitSprite(&this->digit, 0);
    this->anm->InitSprite(&this->digit2, 0x88);
    this->anm->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->SetGlyphWidth(13);
}

void AsciiText::StartScripts()
{
    this->anm->StartScript(&this->summaryPopup, 5);
    this->anm->StartScript(&this->summaryPopup3, 7);
    this->anm->StartScript(&this->summaryPopup2, 6);
    this->anm->StartScript(&this->summaryPopup4, 8);
    this->anm->StartScript(&this->unk_127c, 4);
    this->anm->StartScript(&this->unk_1520, 9);
    this->anm->StartScript(&this->nearbyLabels[0], 10);
    this->anm->StartScript(&this->nearbyLabels[1], 10);
    this->anm->StartScript(&this->nearbyLabels[2], 10);
    this->anm->StartScript(&this->nearbyLabels[3], 10);
    this->summaryPopup2.pos.x -= g_GameManager.unk_3ddf8 * 56.0f / -10000.0f;
    this->summaryPopup3.pos.x += g_GameManager.unk_3ddfa * 56.0f / 10000.0f;
    this->SetSummaryPendingInterrupt(this->GetSummaryPendingInterrupt());
}

int AsciiText::RegisterChain()
{
    AsciiText *text = &g_AsciiText;

    g_AsciiCalcChain.SetCallback((ChainCallback)AsciiText::OnUpdate);
    g_AsciiCalcChain.addedCallback = (ChainCallback)AsciiText::AddedCallback;
    g_AsciiCalcChain.deletedCallback = (ChainCallback)AsciiText::DeletedCallback;
    g_AsciiCalcChain.arg = text;
    if (g_Chain.AddToCalcChain(&g_AsciiCalcChain, 1) != 0)
    {
        return -1;
    }
    g_AsciiDrawChain.SetCallback((ChainCallback)AsciiText::OnDraw);
    g_AsciiDrawChain.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain, 0x14);
    g_AsciiDrawChain2.SetCallback((ChainCallback)AsciiText::OnDraw2);
    g_AsciiDrawChain2.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain2, 0xe);
    return 0;
}

int AsciiText::AddedCallback(AsciiText *text)
{
    memset(text, 0, sizeof(AsciiText));
    text->anm = g_SpriteEngine->PreloadAnm(1, "ascii.anm");
    if (text->anm == NULL)
    {
        return -1;
    }
    text->anm2 = g_SpriteEngine->PreloadAnm(3, "capture.anm");
    if (text->anm2 == NULL)
    {
        return -1;
    }
    text->Initialize();
    text->StartScripts();
    return 0;
}

int AsciiText::DeletedCallback(AsciiText *text)
{
    g_SpriteEngine->ReleaseAnm(1);
    g_SpriteEngine->ReleaseAnm(3);
    return 0;
}

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcChain);
    g_Chain.Remove(&g_AsciiDrawChain);
}

void AsciiText::AddString(D3DXVECTOR3 *pos, char *text)
{
    AsciiString *str;

    if (this->stringCount >= 0x100)
    {
        return;
    }
    str = &this->strings[this->stringCount];
    this->stringCount++;
    strcpy(str->text, text);
    str->pos = *pos;
    str->color = this->color;
    str->scaleX = this->scaleX;
    str->scaleY = this->scaleY;
    str->inArena = this->inArena;
    if (g_App.HasFlag0Or8())
    {
        str->highlight = this->highlight;
    }
    else
    {
        str->highlight = 0;
    }
}

void AsciiText::AddFormat(D3DXVECTOR3 *pos, char *fmt, ...)
{
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(pos, buf);
    va_end(args);
}

int AsciiText::AddFormatLen(D3DXVECTOR3 *pos, char *fmt, ...)
{
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(pos, buf);
    va_end(args);
    return strlen(buf);
}

void AsciiText::DrawStrings()
{
    float advance;
    int idx;
    AsciiString *string;
    BYTE *c;
    D3DXVECTOR3 pos;
    int clipped = 1;

    string = this->strings;
    this->glyph.flags |= 1;
    this->glyph.flags |= 0x1800;
    for (idx = 0; idx < this->stringCount; idx++, string++)
    {
        this->glyph.pos = string->pos;
        c = (BYTE *)string->text;
        this->glyph.scaleY = string->scaleX;
        this->glyph.scaleX = string->scaleY;
        advance = this->glyphWidth * string->scaleX;
        if (clipped != string->inArena)
        {
            clipped = string->inArena;
            g_SpriteEngine->FlushQuadBatch();
            if (clipped != 0)
            {
                g_App.viewport.X = g_GameManager.arena[0];
                g_App.viewport.Y = g_GameManager.arena[1];
                g_App.viewport.Width = g_GameManager.arena[2];
                g_App.viewport.Height = g_GameManager.arena[3];
                g_App.d3dDevice->SetViewport(&g_App.viewport);
            }
            else
            {
                g_App.viewport.X = 0;
                g_App.viewport.Y = 0;
                g_App.viewport.Width = 640;
                g_App.viewport.Height = 480;
                g_App.d3dDevice->SetViewport(&g_App.viewport);
            }
        }
        while (*c != 0)
        {
            if (*c == '\n')
            {
                this->glyph.pos.y += 16.0f * string->scaleY;
                this->glyph.pos.x = string->pos.x;
            }
            else if (*c == ' ')
            {
                this->glyph.pos.x += advance;
            }
            else
            {
                if (string->highlight == 0)
                {
                    this->glyph.spriteData = this->anm->GetSprite(*c - 1);
                    this->glyph.color = string->color;
                }
                else
                {
                    this->glyph.spriteData = this->anm->GetSprite(*c + 0x8a);
                    this->glyph.color = 0xffffffff;
                }
                g_SpriteEngine->DrawUnrotated(&this->glyph);
                this->glyph.pos.x += advance;
            }
            c++;
        }
    }
    if (clipped != 0)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = 0;
        g_App.viewport.Y = 0;
        g_App.viewport.Width = 640;
        g_App.viewport.Height = 480;
        g_App.d3dDevice->SetViewport(&g_App.viewport);
    }
    for (idx = 0; idx < 4; idx++)
    {
        if (this->nearbyLabels[idx].pos.x >= 56.0f && this->nearbyLabels[idx].pos.x <= 392.0f)
        {
            advance = fabsf(this->nearbyLabels[idx].pos.x - 32.0f - g_Player.pos.x);
            this->nearbyLabels[idx].spriteData = this->anm->GetSprite(0x9d);
            switch (this->nearbyLabelDimmed[idx])
            {
            case 0:
            normal:
                this->nearbyLabels[idx].colorRgb[2] = 0xff;
                this->nearbyLabels[idx].colorRgb[1] = 0xff;
                this->nearbyLabels[idx].colorRgb[0] = 0xff;
                if (advance < 64.0f)
                {
                    this->nearbyLabels[idx].alpha = advance * 64.0f / 64.0f + 96.0f;
                }
                else
                {
                    this->nearbyLabels[idx].alpha = 0xa0;
                }
                break;
            case 1:
                this->nearbyLabels[idx].alpha = 0x80;
                this->nearbyLabels[idx].colorRgb[2] = 0xff;
                this->nearbyLabels[idx].colorRgb[1] = 0x40;
                this->nearbyLabels[idx].colorRgb[0] = 0x40;
                break;
            case 2:
                if (this->frameCounter % 8 == 0)
                {
                    this->nearbyLabels[idx].spriteData = this->anm->GetSprite(0x9e);
                    this->nearbyLabels[idx].alpha = 0xff;
                    this->nearbyLabels[idx].colorRgb[2] = 0xff;
                    this->nearbyLabels[idx].colorRgb[1] = 0xff;
                    this->nearbyLabels[idx].colorRgb[0] = 0xff;
                }
                else
                {
                    goto normal;
                }
                break;
            case 3:
                if (this->frameCounter % 4 == 0)
                {
                    this->nearbyLabels[idx].spriteData = this->anm->GetSprite(0x9e);
                    this->nearbyLabels[idx].alpha = 0xff;
                    this->nearbyLabels[idx].colorRgb[2] = 0xff;
                    this->nearbyLabels[idx].colorRgb[1] = 0xff;
                    this->nearbyLabels[idx].colorRgb[0] = 0xff;
                }
                else
                {
                    goto normal;
                }
                break;
            case 4:
                if (this->frameCounter % 2 == 0)
                {
                    this->nearbyLabels[idx].spriteData = this->anm->GetSprite(0x9e);
                    this->nearbyLabels[idx].alpha = 0xff;
                    this->nearbyLabels[idx].colorRgb[2] = 0xff;
                    this->nearbyLabels[idx].colorRgb[1] = 0xff;
                    this->nearbyLabels[idx].colorRgb[0] = 0xff;
                }
                else
                {
                    goto normal;
                }
                break;
            }
            g_SpriteEngine->DrawUnrotated(&this->nearbyLabels[idx]);
        }
    }
}

void AsciiText::AddPopup(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup >= 0x2d0)
    {
        this->nextPopup = 0;
    }
    popup = &this->popups[this->nextPopup];
    popup->active = 1;
    n = 0;
    if (value >= 0)
    {
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
    }
    else
    {
        popup->digits[n] = 10;
        n++;
    }
    if (n == 0)
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digitCount = n;
    popup->color = color;
    popup->timer = 0;
    popup->pos = *pos;
    popup->pos.x += g_GameManager.arena[0];
    popup->pos.y += g_GameManager.arena[1];
    this->nextPopup++;
}
