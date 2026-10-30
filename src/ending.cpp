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

int Ending::Load(char *path)
{
    char *oldFile;

    oldFile = this->file;
    this->file = (char *)LoadFileData(path, NULL, 0);
    if (this->file == NULL)
    {
        this->file = oldFile;
        LogPrintf(&g_Log, "error : エンディングファイルが読み込めない、ファイルが破壊されています\r\n");
        return -1;
    }
    this->cursor = this->file;
    this->lineDelay = 8;
    this->lineTimer = 0;
    this->timer = 0;
    if (oldFile != NULL)
    {
        g_MemoryManager.Free(oldFile);
    }
    return 0;
}

int Ending::RegisterChain()
{
    Ending *ending = (Ending *)g_MemoryManager.Register(new Ending(), sizeof(Ending), "EndingInf");

    ending->calcChain = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainCallback)Ending::AddedCallback;
    ending->calcChain->deletedCallback = (ChainCallback)Ending::DeletedCallback;
    if (g_Chain.AddToCalcChain(ending->calcChain, 5) != 0)
    {
        return -1;
    }
    ending->drawChain = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawChain->arg = ending;
    g_Chain.AddToDrawChain(ending->drawChain, 4);
    return 0;
}

int Ending::OnUpdate(Ending *ending)
{
    int i;
    int extra = 0;

loop:
    if (ending->RunScript() != 0)
    {
        return 0;
    }
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Step(&ending->sprites[i]);
    }
    if (ending->canSkip != 0 && (g_Buttons & 0x100) != 0 && extra < 8)
    {
        extra++;
        goto loop;
    }
    return 1;
}

int Ending::OnDraw(Ending *ending)
{
    int i;

    g_SpriteEngine->CopySurfaceRectToBackBuffer(0, 0, 0, ending->scrollX, ending->scrollY, 640, 480);
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Draw(&ending->sprites[i]);
    }
    ending->DrawFade();
    return 1;
}

int Ending::AddedCallback(Ending *ending)
{
    char *script;
    int j;
    int si;
    int bit;
    // Unused.
    int unused = g_GameManager.character;

    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearVertexFormat();
    ClearScreen(0xffffffff);
    g_App.hideFps = 1;
    ending->anm = g_SpriteEngine->LoadAnm(0x18, "staff01.anm");
    if ((g_GameManager.stateFlags >> 4) & 1)
    {
        si = g_GameManager.character;
        // Final A (stage 6) and Final B (stage 7) are recorded in bits 15
        // and 14.
        bit = g_GameManager.stage != 7 ? 0x8000 : 0x4000;
        g_GameManager.plst.counts[g_GameManager.difficulty].clears++;
        g_GameManager.plst.counts[6].clears++;
        ending->canSkip = 0;
        ending->nextCanSkip = 0;
        if ((g_GameManager.stage == 7 && g_GameManager.AnyShotHasExtraOpen()) ||
            (g_GameManager.stage == 6 && g_GameManager.Func43c2c5()))
        {
            ending->nextCanSkip = 1;
        }
        if (g_GameManager.stats->continuesUsed == 0)
        {
            if ((g_GameManager.clrds[si].stagesClearedNoContinue[0] & bit) ||
                (g_GameManager.clrds[si].stagesClearedNoContinue[1] & bit) ||
                (g_GameManager.clrds[si].stagesClearedNoContinue[2] & bit) ||
                (g_GameManager.clrds[si].stagesClearedNoContinue[3] & bit))
            {
                ending->canSkip = 1;
            }
            g_GameManager.clrds[si].stagesClearedNoContinue[g_GameManager.difficulty] |= bit;
            g_GameManager.clrds[12].stagesClearedNoContinue[g_GameManager.difficulty] |= bit;
        }
        else if ((g_GameManager.clrds[si].stagesCleared[0] & bit) || (g_GameManager.clrds[si].stagesCleared[1] & bit) ||
                 (g_GameManager.clrds[si].stagesCleared[2] & bit) || (g_GameManager.clrds[si].stagesCleared[3] & bit))
        {
            ending->canSkip = 1;
        }
        g_GameManager.clrds[si].stagesCleared[g_GameManager.difficulty] |= bit;
        g_GameManager.clrds[12].stagesCleared[g_GameManager.difficulty] |= bit;
        g_GameManager.plst.bgmHeard[0x12] = 1;
        g_GameManager.plst.bgmHeard[0x13] = 1;
    }
    else
    {
        ending->canSkip = g_GameManager.clrds[g_GameManager.character].unk_20;
        g_GameManager.clrds[g_GameManager.character].unk_20 = 0;
        g_GameManager.plst.bgmHeard[0x12] = 0x12;
    }
    for (j = 0; j < 0xf; j++)
    {
        g_App.textAnm->StartScriptAtOrigin(&ending->sprites[j], j + 0x12);
        ending->sprites[j].pos = D3DXVECTOR3(64.0f, (float)j * 16.0f + 400.0f, 0.0f);
    }
    if (!((g_GameManager.stateFlags >> 4) & 1))
    {
        script = g_BadEndingFiles[g_GameManager.character];
    }
    else if (g_GameManager.stage != 7)
    {
        script = g_EndingFiles[g_GameManager.character];
    }
    else
    {
        script = g_FinalBEndingFiles[g_GameManager.character];
    }
    if (ending->Load(script) != 0)
    {
        return -1;
    }
    return 0;
}

int Ending::DeletedCallback(Ending *ending)
{
    g_SpriteEngine->ReleaseAnm(0x18);
    g_App.scene = 6;
    g_SpriteEngine->ReleaseSurface(0);
    g_MemoryManager.Free(ending->file);
    g_Chain.Remove(ending->drawChain);
    ending->drawChain = NULL;
    g_MemoryManager.Unregister(ending);
    delete ending;
    ending = NULL;
    g_App.hideFps = 0;
    return 0;
}
