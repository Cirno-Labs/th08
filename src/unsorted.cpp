// The file from about 0x43d0c0: input, file I/O, the log, the random
// number generator and math helpers, as in th07's unsorted.cpp.

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#include "unsorted.h"

WORD InitJoystick()
{
    JOYINFOEX joyInfo;

    joyInfo.dwSize = sizeof(joyInfo);
    joyInfo.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
    {
        LogPrintf(&g_Log, "使えるパッドが存在しないようです、残念\r\n");
        return 1;
    }
    joyGetDevCapsA(JOYSTICKID1, &g_JoyCaps, sizeof(g_JoyCaps));
    return 0;
}

WORD ReadJoystick(WORD buttons)
{
    JOYINFOEX joyInfo;
    DWORD margin;
    int shotPressed;
    int diFire;
    HRESULT hr;
    DIJOYSTATE2 state;

    if (g_App.joystick == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        {
            return buttons;
        }
        shotPressed = SetButtonFromJoystick(&buttons, g_App.controllerMapping.shootButton, 1, joyInfo.dwButtons);
        if (g_App.GetShootFocuses())
        {
            if (shotPressed)
            {
                if (g_ShotHeldFrames < 20)
                {
                    g_ShotHeldFrames++;
                }
                if (g_ShotHeldFrames >= 10)
                {
                    buttons |= 4;
                }
            }
            else if (g_ShotHeldFrames > 10)
            {
                g_ShotHeldFrames -= 10;
                buttons |= 4;
            }
            else
            {
                g_ShotHeldFrames = 0;
            }
        }
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.bombButton, 2, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.focusButton, 4, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.menuButton, 8, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.upButton, 0x10, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.downButton, 0x20, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.leftButton, 0x40, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.rightButton, 0x80, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, g_App.controllerMapping.skipButton, 0x100, joyInfo.dwButtons);
        margin = (g_JoyCaps.wXmax - g_JoyCaps.wXmin) / 2 / 2;
        buttons |= ((g_JoyCaps.wXmin + g_JoyCaps.wXmax) / 2 + margin < joyInfo.dwXpos) ? 0x80 : 0;
        buttons |= (joyInfo.dwXpos < (g_JoyCaps.wXmin + g_JoyCaps.wXmax) / 2 - margin) ? 0x40 : 0;
        margin = (g_JoyCaps.wYmax - g_JoyCaps.wYmin) / 2 / 2;
        buttons |= ((g_JoyCaps.wYmin + g_JoyCaps.wYmax) / 2 + margin < joyInfo.dwYpos) ? 0x20 : 0;
        buttons |= (joyInfo.dwYpos < (g_JoyCaps.wYmin + g_JoyCaps.wYmax) / 2 - margin) ? 0x10 : 0;
        return buttons;
    }
    else
    {
        hr = g_App.joystick->Poll();
        if (FAILED(hr))
        {
            int retries = 0;

            DebugPrint("error : DIERR_INPUTLOST\r\n");
            hr = g_App.joystick->Acquire();
            while (hr == DIERR_INPUTLOST)
            {
                hr = g_App.joystick->Acquire();
                DebugPrint("error : DIERR_INPUTLOST %d\r\n", retries);
                retries++;
                if (retries >= 400)
                {
                    return buttons;
                }
            }
            return buttons;
        }
        memset(&state, 0, sizeof(state));
        hr = g_App.joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (FAILED(hr))
        {
            return buttons;
        }
        diFire = SetButtonFromDirectInput(&buttons, g_App.controllerMapping.shootButton, 1, state.rgbButtons);
        if (g_App.GetShootFocuses())
        {
            if (diFire)
            {
                if (g_ShotHeldFrames < 20)
                {
                    g_ShotHeldFrames++;
                }
                if (g_ShotHeldFrames >= 10)
                {
                    buttons |= 4;
                }
            }
            else if (g_ShotHeldFrames > 10)
            {
                g_ShotHeldFrames -= 10;
                buttons |= 4;
            }
            else
            {
                g_ShotHeldFrames = 0;
            }
        }
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.bombButton, 2, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.focusButton, 4, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.menuButton, 8, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.upButton, 0x10, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.downButton, 0x20, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.leftButton, 0x40, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.rightButton, 0x80, state.rgbButtons);
        SetButtonFromDirectInput(&buttons, g_App.controllerMapping.skipButton, 0x100, state.rgbButtons);
        buttons |= (state.lX > g_App.padThresholdX) ? 0x80 : 0;
        buttons |= (state.lX < -g_App.padThresholdX) ? 0x40 : 0;
        buttons |= (state.lY > g_App.padThresholdY) ? 0x20 : 0;
        buttons |= (state.lY < -g_App.padThresholdY) ? 0x10 : 0;
        return buttons;
    }
}

int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons)
{
    if (button < 0)
    {
        return 0;
    }
    *buttons |= (rgbButtons[button] & 0x80) ? bit : 0;
    return (rgbButtons[button] & 0x80) ? bit : 0;
}

int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons)
{
    if (button < 0)
    {
        return 0;
    }
    DWORD mask = 1 << button;
    *buttons |= (joyButtons & mask) ? bit : 0;
    return (joyButtons & mask) ? bit : 0;
}

BYTE *GetPadButtons()
{
    JOYINFOEX joyInfo;
    DWORD mask;
    DWORD button;
    HRESULT hr;
    DIJOYSTATE2 state;

    memset(g_PadButtons, 0, sizeof(g_PadButtons));
    if (g_App.joystick == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        {
            return g_PadButtons;
        }
        mask = joyInfo.dwButtons;
        for (button = 0; button < 32; button++, mask >>= 1)
        {
            if (mask & 1)
            {
                g_PadButtons[button] = 0x80;
            }
        }
        return g_PadButtons;
    }
    else
    {
        hr = g_App.joystick->Poll();
        if (FAILED(hr))
        {
            int retries = 0;

            DebugPrint("error : DIERR_INPUTLOST\r\n");
            hr = g_App.joystick->Acquire();
            while (hr == DIERR_INPUTLOST)
            {
                hr = g_App.joystick->Acquire();
                retries++;
                if (retries >= 400)
                {
                    DebugPrint("error : DIERR_INPUTLOST %d\r\n", retries);
                    return g_PadButtons;
                }
            }
            return g_PadButtons;
        }
        g_App.joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (FAILED(hr))
        {
            return g_PadButtons;
        }
        memcpy(g_PadButtons, state.rgbButtons, sizeof(g_PadButtons));
        return g_PadButtons;
    }
}

// Sets `bit` in `buttons` while key `key` of `keyState` is down.
#define KEY(key, bit) buttons |= ((keyState[key] & 0x80) ? (bit) : 0)

WORD ReadButtons()
{
    WORD buttons = 0;
    BYTE keyState[0x100];

    if (g_App.keyboard == NULL)
    {
        GetKeyboardState(keyState);
        KEY(VK_UP, 0x10);
        KEY(VK_DOWN, 0x20);
        KEY(VK_LEFT, 0x40);
        KEY(VK_RIGHT, 0x80);
        KEY(VK_NUMPAD8, 0x10);
        KEY(VK_NUMPAD2, 0x20);
        KEY(VK_NUMPAD4, 0x40);
        KEY(VK_NUMPAD6, 0x80);
        KEY(VK_NUMPAD7, 0x50);
        KEY(VK_NUMPAD9, 0x90);
        KEY(VK_NUMPAD1, 0x60);
        KEY(VK_NUMPAD3, 0xa0);
        KEY(VK_HOME, 0x800);
        KEY('P', 0x800);
        KEY('D', 0x2000);
        KEY('Z', 0x1);
        KEY('X', 0x2);
        KEY(VK_SHIFT, 0x4);
        KEY(VK_ESCAPE, 0x8);
        KEY(VK_CONTROL, 0x100);
        KEY('Q', 0x200);
        KEY('S', 0x400);
        KEY('R', 0x4000);
        KEY(VK_RETURN, 0x1000);
    }
    else
    {
        HRESULT hr = g_App.keyboard->GetDeviceState(sizeof(keyState), keyState);
        buttons = 0;
        if (hr == DIERR_INPUTLOST)
        {
            g_App.keyboard->Acquire();
            return ReadJoystick(buttons);
        }
        if (hr != DI_OK)
        {
            g_App.keyboard->Acquire();
            return ReadJoystick(buttons);
        }
        KEY(DIK_UP, 0x10);
        KEY(DIK_DOWN, 0x20);
        KEY(DIK_LEFT, 0x40);
        KEY(DIK_RIGHT, 0x80);
        KEY(DIK_NUMPAD8, 0x10);
        KEY(DIK_NUMPAD2, 0x20);
        KEY(DIK_NUMPAD4, 0x40);
        KEY(DIK_NUMPAD6, 0x80);
        KEY(DIK_NUMPAD7, 0x50);
        KEY(DIK_NUMPAD9, 0x90);
        KEY(DIK_NUMPAD1, 0x60);
        KEY(DIK_NUMPAD3, 0xa0);
        KEY(DIK_HOME, 0x800);
        KEY(DIK_P, 0x800);
        KEY(DIK_D, 0x2000);
        KEY(DIK_Z, 0x1);
        KEY(DIK_X, 0x2);
        KEY(DIK_LSHIFT, 0x4);
        KEY(DIK_RSHIFT, 0x4);
        KEY(DIK_ESCAPE, 0x8);
        KEY(DIK_LCONTROL, 0x100);
        KEY(DIK_RCONTROL, 0x100);
        KEY(DIK_Q, 0x200);
        KEY(DIK_S, 0x400);
        KEY(DIK_RETURN, 0x1000);
        KEY(DIK_R, 0x4000);
    }
    return ReadJoystick(buttons);
}

void ResetKeyboard()
{
    BYTE keyState[0x100];
    int i;

    GetKeyboardState(keyState);
    for (i = 0; i < 0x100; i++)
    {
        keyState[i] &= 0x7f;
    }
    SetKeyboardState(keyState);
}

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "d:\\cygwin\\home\\zun\\prog\\th08\\global.h"

BYTE *DecryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit)
{
    int unused = 0;
    int tail;
    BYTE *wp;
    BYTE *dest;
    int i;
    BYTE *line;
    BYTE *p;

    tail = (size % blockSize < blockSize / 4) ? size % blockSize : 0;
    p = data;
    dest = (BYTE *)g_MemoryManager.Alloc(size, GLOBAL_H);
    wp = dest;
    if (dest == NULL)
    {
        return data;
    }
    tail += size & 1;
    size -= tail;
    while (size > 0 && limit > 0)
    {
        if (size < blockSize)
        {
            blockSize = size;
        }
        line = wp;
        wp = wp + blockSize - 1;
        for (i = (blockSize + 1) / 2; i > 0; i--, p++)
        {
            *wp = *p ^ key;
            wp -= 2;
            key += step;
        }
        wp = line + blockSize - 2;
        for (i = blockSize / 2; i > 0; i--, p++)
        {
            *wp = *p ^ key;
            wp -= 2;
            key += step;
        }
        size -= blockSize;
        wp = line + blockSize;
        limit -= blockSize;
    }
    size += tail;
    if (size > 0)
    {
        memcpy(wp, p, size);
    }
    return dest;
}

BYTE *DecryptFileData(BYTE *data, int *size, int fileSize)
{
    DWORD j;
    BYTE *result;
    BYTE *p = data;

    if (p[0] == g_CryptMagic[0] - 0x20 && p[1] == g_CryptMagic[1] - 0x40 && p[2] == g_CryptMagic[2] - 0x60)
    {
        j = 0;
        while (p[3] != g_CryptParams[j].type - j * 16 - 0x10 && j < 8)
        {
            j++;
        }
        if (j >= 8)
        {
            return p;
        }
        result = DecryptData(p + 4, fileSize - 4, g_CryptParams[j].key, g_CryptParams[j].step,
                             g_CryptParams[j].blockSize, g_CryptParams[j].limit);
        g_MemoryManager.Free(data);
        return result;
    }
    return p;
}

BYTE *EncryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit)
{
    int unused = 0;
    BYTE *start;
    int tail;
    BYTE *wp;
    BYTE *dest;
    int i;
    BYTE *p;

    tail = (size % blockSize < blockSize / 4) ? size % blockSize : 0;
    p = data;
    dest = (BYTE *)g_MemoryManager.Alloc(size, GLOBAL_H);
    wp = dest;
    if (dest == NULL)
    {
        return data;
    }
    tail += size & 1;
    size -= tail;
    while (size > 0 && limit > 0)
    {
        if (size < blockSize)
        {
            blockSize = size;
        }
        start = p;
        p = p + blockSize - 1;
        for (i = (blockSize + 1) / 2; i > 0; i--, wp++)
        {
            *wp = *p ^ key;
            p -= 2;
            key += step;
        }
        p = start + blockSize - 2;
        for (i = blockSize / 2; i > 0; i--, wp++)
        {
            *wp = *p ^ key;
            p -= 2;
            key += step;
        }
        size -= blockSize;
        p = start + blockSize;
        limit -= blockSize;
    }
    size += tail;
    if (size > 0)
    {
        memcpy(wp, p, size);
    }
    return dest;
}

void *LoadFileData(char *path, int *size, int isExternal)
{
    int idx = -1;
    char *name;
    DWORD dataSize;
    void *buffer;
    HANDLE handle;

    g_App.EnterCriticalSection(2);
    if (isExternal == 0)
    {
        name = strrchr(path, '\\');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        name = strrchr(name, '/');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        dataSize = g_Archive.GetEntrySize(name);
        if (size != NULL)
        {
            *size = dataSize;
        }
        if (dataSize == 0)
        {
            LogError(&g_Log, "error : %s is not found in arcfile.\r\n", name);
            goto fail;
        }
        if (dataSize != 0)
        {
            DebugPrint("%s Decode ... \r\n", name);
            buffer = g_MemoryManager.Alloc(dataSize, path);
            if (buffer == NULL)
            {
                goto fail;
            }
            g_Archive.Decode(name, buffer);
            buffer = DecryptFileData((BYTE *)buffer, size, dataSize);
            goto done;
        }
    }
    DebugPrint("%s Load ... \r\n", path);
    handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        DebugPrint("error : %s is not found.\r\n", path);
        goto fail;
    }
    dataSize = GetFileSize(handle, NULL);
    buffer = g_MemoryManager.Alloc(dataSize, path);
    if (buffer == NULL)
    {
        DebugPrint("error : %s allocation error.\r\n", path);
        CloseHandle(handle);
        goto fail;
    }
    ReadFile(handle, buffer, dataSize, &dataSize, NULL);
    if (size != NULL)
    {
        *size = dataSize;
    }
    CloseHandle(handle);
    buffer = DecryptFileData((BYTE *)buffer, size, dataSize);
done:
    g_App.LeaveCriticalSection(2);
    return buffer;
fail:
    g_App.LeaveCriticalSection(2);
    return NULL;
}
