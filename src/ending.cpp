// The ending's file, 0x428890 to 0x429df0, as th07's ending.cpp. Its
// inline copies of MemoryManager::Register and Unregister follow.

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "screeneffect.h"
#include "unsorted.h"

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}

void Ending::DrawFade()
{
    int a;
    float rect[4];

    rect[0] = 0.0f;
    rect[1] = 0.0f;
    rect[2] = 640.0f;
    rect[3] = 480.0f;
    switch (this->fadeType)
    {
    case 1:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 2:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xff000000;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 3:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 4:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xffffffff;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 0:
        this->fadeColor = 0;
        break;
    }
    if ((this->fadeColor & 0xff000000) != 0)
    {
        DrawColorRect(rect, this->fadeColor);
    }
}

#define SKIP_LINE()                                                                                                    \
    while (*this->cursor != '\n' && *this->cursor != '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }                                                                                                                  \
    while (*this->cursor == '\n' || *this->cursor == '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }

int Ending::RunScript()
{
    int half = 0;
    float secs4;
    int m;
    int bottom2;
    int num4;
    int spr2;
    int a2;
    int sprite2;
    int k2;
    int lineLen3 = 0;
    char text3[0x44];

    memset(text3, 0, sizeof(text3));
    if (this->waitTimer > 0)
    {
        this->waitTimer--;
        if (this->waitMinFrames != 0)
        {
            this->waitMinFrames--;
        }
        else if (WAS_PRESSED(0x1001) || (this->canSkip != 0 && (g_Buttons & 0x100) != 0))
        {
            this->waitTimer = 0;
        }
        if (this->waitTimer <= 0)
        {
            for (k2 = 0; k2 < 0xf; k2++)
            {
                this->sprites[k2].SetPendingInterrupt(2);
            }
            this->line = 0;
        }
        else
        {
            goto done;
        }
    }
    if (this->lineTimer > 0)
    {
        this->lineTimer--;
        if (this->lineMinFrames != 0)
        {
            this->lineMinFrames--;
        }
        else if (WAS_PRESSED(0x1001) || (this->canSkip != 0 && (g_Buttons & 0x100) != 0))
        {
            this->lineTimer = 0;
        }
        goto done;
    }
    while (TRUE)
    {
        switch (*this->cursor)
        {
        case '@':
            this->cursor++;
            switch (*this->cursor)
            {
            case 'b':
                if (g_SpriteEngine->LoadSurface(0, this->cursor + 1) != 0)
                {
                    return -1;
                }
                break;
            case 'a':
                this->cursor++;
                a2 = this->ReadInt();
                sprite2 = this->ReadInt();
                spr2 = this->ReadInt();
                this->anm->StartScriptAtOrigin(&this->sprites[a2], sprite2);
                this->anm->SetSprite(&this->sprites[a2], spr2);
                break;
            case 'V':
                this->cursor++;
                num4 = this->ReadInt();
                bottom2 = this->ReadInt();
                this->scrollSpeed = (float)num4 / (float)bottom2;
                break;
            case 'v':
                this->cursor++;
                this->scrollY = this->ReadInt();
                break;
            case 'F':
                if (this->Load(this->cursor + 1) != 0)
                {
                    return -1;
                }
                lineLen3 = 0;
                half = 0;
                this->canSkip = this->nextCanSkip;
            case 'R':
                for (m = 0; m < 0x10; m++)
                {
                    this->sprites[m].scriptIndex = 0;
                }
                break;
            case 'm':
                g_App.LoadMidi(0, this->cursor + 1);
                g_App.PlayMidi(0, 0);
                break;
            case 'M':
                this->cursor++;
                secs4 = this->ReadInt();
                g_App.FadeOutMusic(secs4);
                break;
            case 's':
                this->cursor++;
                this->lineDelay = this->ReadInt();
                this->lineDelayFast = this->ReadInt();
                break;
            case 'c':
                this->cursor++;
                this->textColor = this->ReadInt();
                break;
            case 'r':
                this->cursor++;
                this->waitTimer = this->ReadInt();
                this->waitMinFrames = this->ReadInt();
                this->lineTimer = 0;
                this->lineMinFrames = 0;
                SKIP_LINE();
                goto done;
            case 'w':
                this->cursor++;
                this->lineTimer = this->ReadInt();
                this->lineMinFrames = this->ReadInt();
                SKIP_LINE();
                goto done;
            case '0':
                this->cursor++;
                this->fadeType = 1;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '1':
                this->cursor++;
                this->fadeType = 2;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '2':
                this->cursor++;
                this->fadeType = 3;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case '3':
                this->cursor++;
                this->fadeType = 4;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt();
                break;
            case 'z':
                return -1;
            }
            SKIP_LINE();
            break;
        case '\0':
        case '\n':
        case '\r':
            if (lineLen3 != 0)
            {
                g_SpriteEngine->RenderText(&this->sprites[this->line], this->textColor, 0xffffffff, text3);
                this->sprites[this->line].SetPendingInterrupt(1);
            }
            while (*this->cursor == '\n' || *this->cursor == '\0' || *this->cursor == '\r')
            {
                this->cursor++;
            }
            if ((g_Buttons & 0x1001) != 0)
            {
                this->lineTimer = this->lineDelayFast;
                this->lineMinFrames = this->lineDelayFast;
            }
            else
            {
                this->lineTimer = this->lineDelay;
                // lineDelayFast, not lineDelay (th07's).
                this->lineMinFrames = this->lineDelayFast;
            }
            this->line++;
            goto done;
        default:
            text3[lineLen3] = this->cursor[0];
            text3[lineLen3 + 1] = this->cursor[1];
            lineLen3 += 2;
            this->cursor += 2;
            break;
        }
    }
done:
    this->timer++;
    this->scrollY -= this->scrollSpeed;
    if (this->scrollY <= 0.0f)
    {
        this->scrollY = 0.0f;
        this->scrollSpeed = 0.0f;
    }
    return 0;
}
