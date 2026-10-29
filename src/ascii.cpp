// Text drawn with the ASCII font, the pause and continue menus and the
// popups, as th07's ascii.cpp. The first object of the game, from
// 0x402000; its inline copies run to about 0x4071a0.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "chain.h"
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
