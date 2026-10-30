// Text drawn with the ASCII font, the pause and continue menus and the
// popups, as th07's ascii.cpp. The first object of the game, from
// 0x402000; its inline copies run to about 0x4071a0.

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "player.h"
#include "screeneffect.h"
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
        if (!g_GameManager.deathbombFreeze)
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
    this->summaryPopup2.pos.x -= g_GameManager.gaugeMin * 56.0f / -10000.0f;
    this->summaryPopup3.pos.x += g_GameManager.gaugeMax * 56.0f / 10000.0f;
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

void AsciiText::AddPopup2(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup2 >= 3)
    {
        this->nextPopup2 = 0;
    }
    popup = &this->popups[this->nextPopup2 + 0x2d0];
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
    this->nextPopup2++;
}

void AsciiText::Func403460(D3DXVECTOR3 *pos, int count, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextNumberPopup >= 0x80)
    {
        this->nextNumberPopup = 0;
    }
    popup = &this->numberPopups[this->nextNumberPopup];
    popup->active = 1;
    n = 0;
    if (value > 0)
    {
        popup->digits[n] = 0xf;
        n++;
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
        popup->digits[n] = 0xe;
        n++;
    }
    if (count > 0)
    {
        while (count != 0)
        {
            popup->digits[n] = count % 10;
            n++;
            count /= 10;
        }
    }
    else
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digits[n] = 0xd;
    n++;
    popup->digitCount = n;
    popup->color = color;
    popup->timer = 0;
    popup->pos = *pos;
    popup->pos.x += g_GameManager.arena[0];
    popup->pos.y += g_GameManager.arena[1];
    popup->scaleX = this->scaleX;
    popup->scaleY = this->scaleY;
    this->nextNumberPopup++;
}

void AsciiText::Func403600(D3DXVECTOR3 *pos, int count, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextNumberPopup >= 0x80)
    {
        this->nextNumberPopup = 0;
    }
    popup = &this->numberPopups[this->nextNumberPopup];
    popup->active = 1;
    n = 0;
    if (value > 0)
    {
        popup->digits[n] = 0xf;
        n++;
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
        popup->digits[n] = 0xe;
        n++;
    }
    if (count > 0)
    {
        while (count != 0)
        {
            popup->digits[n] = count % 10;
            n++;
            count /= 10;
        }
    }
    else
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digits[n] = 0xd;
    n++;
    popup->digitCount = n;
    popup->color = color;
    popup->timer = 0x58;
    popup->pos = *pos;
    popup->pos.x += n * 3.5f + g_GameManager.arena[0];
    popup->pos.y += g_GameManager.arena[1];
    popup->scaleX = this->scaleX;
    popup->scaleY = this->scaleY;
    this->nextNumberPopup++;
}

int PauseMenu::UpdatePauseMenu()
{
    int i;

    if (WAS_PRESSED(8) && this->state != 4)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 4;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
        this->background.pendingInterrupt = 1;
    }
    if (WAS_PRESSED(0x200) && this->state != 9)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 9;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
    }
    if (!g_GameManager.IsReplay() && WAS_PRESSED(0x4000) && this->state != 9)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 10;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
    }
    switch (this->state)
    {
    case 0:
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            g_AsciiText.anm->StartScript(&this->sprites[i], i + 0xc);
        }
        for (i = 0; i < 4; i++)
        {
            this->sprites[i].pendingInterrupt = 1;
        }
        if (g_GameManager.IsInSpellCardPractice() && g_GameManager.practiceSpellCard >= 0xcd)
        {
            g_AsciiText.anm->SetSprite(&this->sprites[7], 0x120);
        }
        else
        {
            g_AsciiText.anm->SetSprite(&this->sprites[7], g_GameManager.difficulty + 0x11b);
        }
        if (!g_GameManager.IsInPractice())
        {
            this->sprites[8].ClearShown();
        }
        if (!g_GameManager.savedConfig->scoreDisabled)
        {
            this->sprites[9].ClearShown();
        }
        if (g_GameManager.IsReplay())
        {
            this->sprites[3].scriptPos = NULL;
        }
        this->state++;
        this->stateFrames = 0;
        if ((g_App.stateFlags >> 1) & 1)
        {
            g_AsciiText.anm2->StartScript(&this->background, 0);
            if (g_SpriteEngine->SetPendingCopy(3, 0x20, 0x10, 0x180, 0x1c0, this->background.spriteData->startX,
                                               this->background.spriteData->startY,
                                               this->background.spriteData->height,
                                               this->background.spriteData->width) != 0)
            {
                this->state = 0;
                return 0;
            }
            this->background.pos.x = 32.0f;
            this->background.pos.y = 16.0f;
            this->background.pos.z = 0.0f;
        }
    case 1:
        this->sprites[1].color = 0xffffffff;
        this->sprites[3].color = 0xff505050;
        this->sprites[2].color = 0xff505050;
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[2].posOffset = this->sprites[3].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (!g_GameManager.IsReplay())
            {
                if (WAS_PRESSED(0x10))
                {
                    this->state = 3;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (WAS_PRESSED(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 4;
                this->stateFrames = 0;
                this->background.pendingInterrupt = 1;
            }
        }
        break;
    case 2:
        this->sprites[3].color = 0xff505050;
        this->sprites[1].color = 0xff505050;
        this->sprites[2].color = 0xffffffff;
        this->sprites[1].posOffset = this->sprites[3].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_GameManager.IsReplay())
            {
                if (WAS_PRESSED(0x20))
                {
                    this->state = 1;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (WAS_PRESSED(0x20))
            {
                this->state = 3;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 6;
                this->stateFrames = 0;
            }
        }
        break;
    case 3:
        this->sprites[2].color = 0xff505050;
        this->sprites[1].color = 0xff505050;
        this->sprites[3].color = 0xffffffff;
        this->sprites[1].posOffset = this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[3].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 8;
                this->stateFrames = 0;
            }
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_GameManager.isPaused = 0;
            for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
            {
                this->sprites[i].ClearShown();
            }
            g_SoundPlayer.UnPause();
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
        break;
    case 5:
    case 7:
        this->sprites[5].color = 0xffff8080;
        this->sprites[6].color = 0xff505050;
        this->sprites[5].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[6].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                if (this->state == 5)
                {
                    this->state = 6;
                }
                else
                {
                    this->state = 8;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 4; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 5)
                {
                    this->state = 9;
                }
                else
                {
                    this->state = 10;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 6:
    case 8:
        this->sprites[5].color = 0xff505050;
        this->sprites[6].color = 0xffff8080;
        this->sprites[5].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[6].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                if (this->state == 6)
                {
                    this->state = 5;
                }
                else
                {
                    this->state = 7;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                for (; i < 7; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 6)
                {
                    this->state = 2;
                }
                else
                {
                    this->state = 3;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 9:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_App.scene = 1;
            g_GameManager.isPaused = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            ResultScreen::RegisterChain(2);
        }
        break;
    case 10:
        if (this->stateFrames >= 20)
        {
            if (!g_GameManager.IsInSpellCardPractice() && !g_GameManager.IsInPractice() && g_GameManager.difficulty != 4)
            {
                this->state = 0;
                g_App.scene = 10;
                g_GameManager.isPaused = 0;
                g_App.lastTotalPlayTimeTick = timeGetTime();
            }
            else
            {
                if (g_GameManager.IsInSpellCardPractice() && !Func439916(g_GameManager.practiceSpellCard))
                {
                    g_SoundPlayer.UnPause();
                    g_SoundPlayer.FadeInBgm(2.0f);
                }
                else
                {
                    g_App.StopMusic();
                }
                g_App.scene = 11;
                g_Gui.Func438f58();
                g_GameManager.isPaused = 0;
                g_App.lastTotalPlayTimeTick = timeGetTime();
                return 0;
            }
        }
    }
    for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void PauseMenu::DrawPauseMenu()
{
    DWORD i;

    if (g_GameManager.isPaused)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_GameManager.arena[0];
        g_App.viewport.Y = g_GameManager.arena[1];
        g_App.viewport.Width = g_GameManager.arena[2];
        g_App.viewport.Height = g_GameManager.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.stateFlags >> 1) & 1 && this->state != 0)
        {
            Sprite sprite = this->background;

            sprite.flags |= 0x2000;
            g_SpriteEngine->DrawUnrotated(&sprite);
        }
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

int ContinueMenu::UpdateContinueMenu()
{
    int i;

    if (g_GameManager.IsInPractice() && !((g_GameManager.stateFlags >> 14) & 1))
    {
        g_GameManager.isInContinueMenu = 0;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
        g_App.scene = 6;
        return 1;
    }
    if (g_GameManager.IsReplay())
    {
        g_GameManager.isInContinueMenu = 0;
        g_App.scene = 7;
        g_GameManager.stats->guiScore = g_GameManager.stats->score;
        return 1;
    }
    switch (this->state)
    {
    case 0:
        if (this->stateFrames == 0)
        {
            if (!g_GameManager.IsInSpellCardPractice() && g_GameManager.difficulty < 4 &&
                (g_GameManager.GetUnk28() >= 11 || g_GameManager.stage == 7))
            {
                g_GameManager.isInContinueMenu = 0;
                g_GameManager.stats->guiScore = g_GameManager.stats->score;
                if (g_GameManager.difficulty >= 4)
                {
                    g_App.scene = 6;
                }
                else
                {
                    g_GameManager.stateFlags &= ~0x10;
                    g_App.scene = 9;
                }
                return 1;
            }
            if (g_GameManager.IsInSpellCardPractice() && !Func439916(g_GameManager.practiceSpellCard))
            {
                g_SoundPlayer.FadeDownBgm(1.0f);
            }
            else
            {
                g_SoundPlayer.Pause();
            }
            for (i = 0; i < 3; i++)
            {
                g_AsciiText.anm->StartScript(&this->sprites[i], i + 0x16);
                this->sprites[i].pendingInterrupt = 1;
            }
            g_Gui.timesAnm->StartScript(&this->sprites[3], 1);
            g_Gui.timesAnm->SetSprite(&this->sprites[3], g_GameManager.GetUnk28());
            if ((g_App.stateFlags >> 1) & 1)
            {
                g_AsciiText.anm2->StartScript(&this->background, 0);
                if (g_SpriteEngine->SetPendingCopy(3, 0x20, 0x10, 0x180, 0x1c0, this->background.spriteData->startX,
                                                   this->background.spriteData->startY,
                                                   this->background.spriteData->height,
                                                   this->background.spriteData->width) != 0)
                {
                    this->state = 0;
                    return 0;
                }
                this->background.pos.x = 32.0f;
                this->background.pos.y = 16.0f;
                this->background.pos.z = 0.0f;
            }
            g_App.AddTotalPlayTime();
        }
        if (this->stateFrames > 8)
        {
            break;
        }
        if (!g_GameManager.IsInSpellCardPractice() && g_GameManager.difficulty < 4)
        {
            this->state += 2;
        }
        else
        {
            this->state += !g_SpellCard.Func405260() && g_GameManager.IsInSpellCardPractice() ? 1 : 2;
        }
        this->stateFrames = 0;
        if (this->state == 2)
        {
            goto choosingNo;
        }
    case 1:
        this->sprites[1].color = 0xffff8080;
        this->sprites[2].color = 0xff505050;
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                if (!g_GameManager.IsInSpellCardPractice() && g_GameManager.difficulty < 4)
                {
                    this->sprites[3].pendingInterrupt = 1;
                    this->state = 3;
                    this->stateFrames = 0;
                }
                else
                {
                    if (g_GameManager.IsInSpellCardPractice() && !Func439916(g_GameManager.practiceSpellCard))
                    {
                        g_GameManager.isInContinueMenu = 0;
                        g_SoundPlayer.UnPause();
                        g_SoundPlayer.FadeUpBgm(1.0f);
                    }
                    else
                    {
                        g_App.StopMusic();
                    }
                    g_App.scene = 11;
                    g_Gui.Func438f58();
                    g_GameManager.isInContinueMenu = 0;
                    g_App.lastTotalPlayTimeTick = timeGetTime();
                    return 0;
                }
            }
        }
        break;
    case 2:
    choosingNo:
        this->sprites[2].color = 0xffff8080;
        this->sprites[1].color = 0xff505050;
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[1].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 30)
        {
            if (WAS_PRESSED(0x10) || WAS_PRESSED(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (WAS_PRESSED(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 4; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 4;
                this->stateFrames = 0;
            }
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_GameManager.isInContinueMenu = 0;
            g_App.scene = 6;
            for (i = 0; i < 4; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_GameManager.stats->guiScore = g_GameManager.stats->score;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames == 15)
        {
            g_GameManager.AddUnk28(1);
            g_Gui.timesAnm->SetSprite(&this->sprites[3], g_GameManager.GetUnk28());
        }
        if (this->stateFrames == 60)
        {
            this->background.pendingInterrupt = 1;
            for (i = 0; i < 4; i++)
            {
                this->sprites[i].pendingInterrupt = 3;
            }
        }
        if (this->stateFrames >= 90)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_GameManager.isInContinueMenu = 0;
            for (i = 0; i < 4; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_GameManager.stats->continuesUsed++;
            g_GameManager.stats->guiScore = g_GameManager.stats->continuesUsed;
            g_GameManager.stats->scoreIncrement = 0;
            g_GameManager.stats->score = g_GameManager.stats->guiScore;
            g_GameManager.SetLives(g_GameManager.savedConfig->lifeCount);
            g_GameManager.SetBombs(g_Player.shotType->bombs);
            g_GameManager.stats->graze = 0;
            g_GameManager.stats->stageClearPoints = 0;
            g_GameManager.stats->pointItemsTotal = 0;
            g_GameManager.SetPower(0);
            g_GameManager.stats->checksumKeyStage = 0;
            g_GameManager.stats->checksumKeyPoints = 100;
            g_App.redrawFrames = 8;
            // The shot type counters are indexed by character here.
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[0], 999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].plays[0], 999999);
            IncrementCounterCapped(
                (DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].plays[g_GameManager.character + 1],
                999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].plays[g_GameManager.character + 1],
                                   999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[g_GameManager.difficulty].continues,
                                   999999);
            IncrementCounterCapped((DWORD *)&g_GameManager.plst.counts[6].continues, 999999);
            g_SoundPlayer.UnPause();
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 0;
        }
    }
    for (i = 0; i < 4; i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void IncrementCounterCapped(DWORD *count, DWORD max)
{
    if (*count < max)
    {
        *count = *count + 1;
    }
}

void ContinueMenu::DrawContinueMenu()
{
    int i;

    if (g_GameManager.isInContinueMenu)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.viewport.X = g_GameManager.arena[0];
        g_App.viewport.Y = g_GameManager.arena[1];
        g_App.viewport.Width = g_GameManager.arena[2];
        g_App.viewport.Height = g_GameManager.arena[3];
        g_App.d3dDevice->SetViewport(&g_App.viewport);
        if ((g_App.stateFlags >> 1) & 1 && (this->state != 0 || this->stateFrames > 2))
        {
            g_SpriteEngine->DrawUnrotated(&this->background);
        }
        if (!g_GameManager.IsInSpellCardPractice() && g_GameManager.difficulty < 4)
        {
            for (i = 0; i < 4; i++)
            {
                if (this->sprites[i].IsShown())
                {
                    g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
                }
            }
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                if (this->sprites[i].IsShown())
                {
                    g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
                }
            }
        }
    }
}

void AsciiText::DrawPopups()
{
    AsciiPopup *p = this->popups;
    int count;
    float dx2;
    float dy;
    int alpha;
    int i;
    BYTE *digitPtr;
    D3DXVECTOR3 unusedPos;

    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    for (i = 0; i < 0x2d3; i++, p++)
    {
        if (!p->active)
        {
            continue;
        }
        this->digit.pos.x = p->pos.x - (p->digitCount << 2);
        this->digit.pos.y = p->pos.y;
        this->digit.color = p->color;
        dx2 = g_Player.pos.x - p->pos.x;
        dy = g_Player.pos.y - p->pos.y;
        alpha = dx2 * dx2 + dy * dy;
        if (alpha > 0x1000)
        {
            alpha = 0xd0;
        }
        else if (alpha > 0x400)
        {
            alpha = ((alpha - 0x400) * 0x80) / 0xc00 + 0x50;
        }
        else
        {
            alpha = 0x50;
        }
        this->digit.scaleY = this->scaleX;
        this->digit.scaleX = this->scaleY;
        digitPtr = p->digits + p->digitCount - 1;
        for (count = p->digitCount; count > 0; count--)
        {
            if (p->timer < 0x34)
            {
                this->digit.spriteData = this->anm->GetSprite(*digitPtr);
                this->digit.alpha = alpha;
            }
            else if (p->timer < 0x38)
            {
                this->digit.spriteData = this->anm->GetSprite(*digitPtr + 0xb);
                this->digit.alpha = alpha;
            }
            else
            {
                this->digit.spriteData = this->anm->GetSprite(*digitPtr + 0x15);
                this->digit.alpha = alpha;
            }
            this->digit.spriteWidth = this->digit.spriteData->width;
            g_SpriteEngine->DrawUnrotated(&this->digit);
            this->digit.pos.x += 8.0f;
            digitPtr--;
        }
    }
    if (this->ringAlpha > 0)
    {
        float rect[4];
        D3DCOLOR tint;

        ((BYTE *)&tint)[3] = this->ringAlpha;
        ((BYTE *)&tint)[2] = 0;
        ((BYTE *)&tint)[1] = 0;
        ((BYTE *)&tint)[0] = 0;
        rect[0] = 32.0f;
        rect[1] = 16.0f;
        rect[2] = g_Player.pos.x + 32.0f - this->ringRadius + g_SpriteEngine->shakeX;
        rect[3] = 464.0f;
        if (rect[2] > rect[0])
        {
            DrawColorRect(rect, tint);
        }
        rect[0] = g_Player.pos.x + 32.0f + this->ringRadius + g_SpriteEngine->shakeX;
        rect[1] = 16.0f;
        rect[2] = 416.0f;
        rect[3] = 464.0f;
        if (rect[2] > rect[0])
        {
            DrawColorRect(rect, tint);
        }
        rect[0] = g_Player.pos.x + 32.0f - this->ringRadius + g_SpriteEngine->shakeX;
        if (rect[0] < 32.0f)
        {
            rect[0] = 32.0f;
        }
        rect[1] = 16.0f;
        rect[2] = g_Player.pos.x + 32.0f + this->ringRadius + g_SpriteEngine->shakeX;
        if (rect[2] > 416.0f)
        {
            rect[2] = 416.0f;
        }
        rect[3] = g_Player.pos.y + 16.0f - this->ringRadius + g_SpriteEngine->shakeY;
        if (rect[3] > rect[1])
        {
            DrawColorRect(rect, tint);
        }
        rect[1] = g_Player.pos.y + 16.0f + this->ringRadius + g_SpriteEngine->shakeY;
        rect[3] = 464.0f;
        if (rect[3] > rect[1])
        {
            DrawColorRect(rect, tint);
        }
        g_EtamaAnm->StartScript(&this->ring, 0x69);
        this->ring.scaleX = this->ringRadius / 63.0f;
        this->ring.scaleY = this->ring.scaleX;
        this->ring.pos = g_Player.pos;
        this->ring.pos.x += 32.0f;
        this->ring.pos.y += 16.0f;
        this->ring.alpha = this->ringAlpha;
        g_SpriteEngine->DrawUnrotated(&this->ring);
    }
    p = this->numberPopups;
    for (i = 0; i < 0x80; i++, p++)
    {
        if (!p->active)
        {
            continue;
        }
        this->digit2.pos.x = p->pos.x - p->digitCount * 3.5f;
        this->digit2.pos.y = p->pos.y;
        this->digit2.color = p->color;
        dx2 = g_Player.pos.x - p->pos.x;
        dy = g_Player.pos.y - p->pos.y;
        alpha = dx2 * dx2 + dy * dy;
        if (alpha > 0x1000)
        {
            alpha = 0xd0;
        }
        else if (alpha > 0x400)
        {
            alpha = ((alpha - 0x400) * 0x80) / 0xc00 + 0x50;
        }
        else
        {
            alpha = 0x50;
        }
        this->digit2.scaleY = p->scaleX;
        this->digit2.scaleX = p->scaleY;
        digitPtr = p->digits + p->digitCount - 1;
        for (count = p->digitCount; count > 0; count--)
        {
            this->digit2.spriteData = this->anm->GetSprite(*digitPtr + 0x88);
            this->digit2.alpha = alpha;
            this->digit2.spriteWidth = this->digit2.spriteData->width;
            g_SpriteEngine->DrawUnrotated(&this->digit2);
            this->digit2.pos.x += 7.0f * p->scaleX;
            digitPtr--;
        }
    }
    g_SpriteEngine->shakeY = 0.0f;
    g_SpriteEngine->shakeX = 0.0f;
    if (this->summaryPopup.IsShown())
    {
        int div;

        this->summaryPopup4.pos.x =
            g_GameManager.GetUnk22() * 112.0f / 2.0f / 10000.0f + this->summaryPopup.pos.x + 64.0f;
        g_SpriteEngine->DrawRotatedNoRound(&this->summaryPopup4);
        this->unk_127c.pos.x = g_GameManager.GetUnk22() * 80.0f / 2.0f / 10000.0f + this->summaryPopup.pos.x + 64.0f;
        this->unk_127c.pos.y = this->summaryPopup4.pos.y - 7.0f;
        this->unk_127c.pos.z = this->summaryPopup4.pos.z;
        this->unk_127c.alpha = this->summaryPopup.alpha;
        if (g_GameManager.IsUnk22AtLow1())
        {
            this->unk_127c.colorRgb[2] = 0x70;
            this->unk_127c.colorRgb[1] = 0x70;
            this->unk_127c.colorRgb[0] = 0xff;
        }
        else if (g_GameManager.IsUnk22AtLow2())
        {
            this->unk_127c.colorRgb[2] = 0xb0;
            this->unk_127c.colorRgb[1] = 0xb0;
            this->unk_127c.colorRgb[0] = 0xff;
        }
        else if (g_GameManager.IsUnk22AtHigh1())
        {
            this->unk_127c.colorRgb[2] = 0xff;
            this->unk_127c.colorRgb[1] = 0x70;
            this->unk_127c.colorRgb[0] = 0x70;
        }
        else if (g_GameManager.IsUnk22AtHigh2())
        {
            this->unk_127c.colorRgb[2] = 0xff;
            this->unk_127c.colorRgb[1] = 0xb0;
            this->unk_127c.colorRgb[0] = 0xb0;
        }
        else
        {
            this->unk_127c.colorRgb[2] = 0xff;
            this->unk_127c.colorRgb[1] = 0xff;
            this->unk_127c.colorRgb[0] = 0xff;
        }
        this->summaryPopup.color = this->unk_127c.color;
        g_SpriteEngine->DrawUnrotated(&this->summaryPopup);
        g_SpriteEngine->DrawUnrotated(&this->summaryPopup2);
        g_SpriteEngine->DrawUnrotated(&this->summaryPopup3);
        this->Func405e10(&this->unk_127c.pos, g_GameManager.GetUnk22(), this->unk_127c.color);
        div = 10000000;
        count = g_GameManager.stats->maxPointValue;
        alpha = 0;
        this->unk_127c.pos.x = this->summaryPopup.pos.x + 62.0f - 14.0f;
        this->unk_127c.pos.y = this->summaryPopup.pos.y + 3.0f + 8.0f;
        for (i = 0; i < 8; i++)
        {
            alpha += count / div;
            if (alpha != 0)
            {
                this->anm->SetSprite(&this->unk_127c, count / div + 0x88);
                g_SpriteEngine->DrawUnrotated(&this->unk_127c);
                this->unk_127c.pos.x += 7.0f;
            }
            count = count % div;
            div /= 10;
        }
    }
}

void AsciiText::Func405e10(D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    int absValue;
    int len;
    float offset;

    len = 4;
    if (value < 0)
    {
        len++;
    }
    absValue = abs(value);
    if (absValue >= 10000)
    {
        len += 3;
    }
    else if (absValue >= 1000)
    {
        len += 2;
    }
    else
    {
        len += 1;
    }
    offset = len * 3.5f - 3.5f - 4.0f;
    this->unk_127c.pos = *pos;
    this->unk_127c.pos.x -= offset;
    this->unk_127c.color = color;
    if (value < 0)
    {
        this->anm->SetSprite(&this->unk_127c, 0x94);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
    }
    if (absValue >= 10000)
    {
        this->anm->SetSprite(&this->unk_127c, 0x89);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, 0x93);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->unk_127c.scaleX = 0.8f;
        this->unk_127c.scaleY = 0.8f;
        this->unk_127c.pos.y += 2.0f;
        this->anm->SetSprite(&this->unk_127c, 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->anm->SetSprite(&this->unk_127c, 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
    }
    else if (absValue >= 1000)
    {
        len = absValue;
        this->anm->SetSprite(&this->unk_127c, len / 1000 + 0x88);
        len %= 1000;
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, len / 100 + 0x88);
        len %= 100;
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, 0x93);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->unk_127c.scaleX = 0.8f;
        this->unk_127c.scaleY = 0.8f;
        this->unk_127c.pos.y += 2.0f;
        this->anm->SetSprite(&this->unk_127c, len / 10 + 0x88);
        len %= 10;
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->anm->SetSprite(&this->unk_127c, len + 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
    }
    else
    {
        len = absValue;
        this->anm->SetSprite(&this->unk_127c, len / 100 + 0x88);
        len %= 100;
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
        this->anm->SetSprite(&this->unk_127c, 0x93);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->unk_127c.scaleX = 0.8f;
        this->unk_127c.scaleY = 0.8f;
        this->unk_127c.pos.y += 2.0f;
        this->anm->SetSprite(&this->unk_127c, len / 10 + 0x88);
        len %= 10;
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 5.0f;
        this->anm->SetSprite(&this->unk_127c, len + 0x88);
        g_SpriteEngine->DrawUnrotated(&this->unk_127c);
        this->unk_127c.pos.x += 7.0f;
    }
    this->unk_127c.scaleX = 1.0f;
    this->unk_127c.scaleY = 1.0f;
    this->unk_127c.pos.y -= 2.0f;
    this->anm->SetSprite(&this->unk_127c, 0x92);
    g_SpriteEngine->DrawUnrotated(&this->unk_127c);
}
