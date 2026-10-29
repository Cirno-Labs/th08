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
