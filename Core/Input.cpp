
#include "pch.h"
#include "EngineCore.h"
#include "EngineInput.h"

#ifdef _HIGH_PERFORMANCE

#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define USE_KEYBOARD_MOUSE

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#endif

namespace CECore
{
    extern HWND CreativeBaseWindow;
}

namespace 
{
    bool s_Buttons[2][EngineInput::kNumDigitalInputs];
    float s_HoldDuration[EngineInput::kNumDigitalInputs] = { 0.0f };
    float s_Analogs[EngineInput::kNumAnalogInputs];
    float s_AnalogsTC[EngineInput::kNumAnalogInputs];


#ifdef USE_KEYBOARD_MOUSE

    IDirectInput8A* s_DI;
    IDirectInputDevice8A* s_Keyboard;
    IDirectInputDevice8A* s_Mouse;

    DIMOUSESTATE2 s_MouseState;
    
    unsigned char s_Keybuffer[256];
    unsigned char s_DXKeyMapping[EngineInput::kNumKeys]; // map DigitalInput enum to DirectX 9 input keycode

#endif

    inline float FilterAnalogInput( int val, int deadZone )
    {
        if (val < 0)
        {
            if (val > -deadZone)
                return 0.0f;
            else
                return (val + deadZone) / (32768.0f - deadZone);
        }
        else
        {
            if (val < deadZone)
                return 0.0f;
            else
                return (val - deadZone) / (32767.0f - deadZone);
        }
    }


#ifdef USE_KEYBOARD_MOUSE

#endif

    static bool blockMouse = false;
    static bool blockKeyboard = false;

    typedef HRESULT(__stdcall* GetDeviceStateT)(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData);
    
    //Method that allows ImGui to hook into 
    HRESULT __stdcall hookGetDeviceState(_Inout_ IDirectInputDevice8* pThis, _In_ DWORD cbData, _In_ LPVOID lpvData)
    {
        HRESULT result = pThis->GetDeviceState(cbData, lpvData);

        if (result == DI_OK)
        {
            if (cbData == sizeof(DIMOUSESTATE))
            {
                if (((LPDIMOUSESTATE)lpvData)->rgbButtons[0] != 0)
                {
                    // This is the LMB
                }

                if (((LPDIMOUSESTATE)lpvData)->rgbButtons[1] != 0)
                {
                    // This is the RMB
                }
            }

            if (cbData == sizeof(DIMOUSESTATE2))
            {
                if (((LPDIMOUSESTATE)lpvData)->rgbButtons[0] != 0)
                {
                    // This is the LMB
                }

                if (((LPDIMOUSESTATE)lpvData)->rgbButtons[1] != 0)
                {
                    // This is the RMB
                }
            }

            if (blockMouse)
            {
                ((LPDIMOUSESTATE)lpvData)->rgbButtons[0] = 0;
                ((LPDIMOUSESTATE)lpvData)->rgbButtons[1] = 0;
            }

        }
        

    };

    typedef HRESULT(__stdcall* GetDeviceDataT)(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rbdod, LPDWORD pdwInOut, DWORD dwFlags);
    GetDeviceDataT pGetDeviceData = nullptr;

    HRESULT hookGetDeviceData(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgbdod, LPDWORD pdwInOut, DWORD dwFlags)
    {
        HRESULT result = pGetDeviceData(pThis, cbObjectData, rgbdod, pdwInOut, dwFlags);

        if (result == DI_OK)
        {
            for (int i = 0; i < *pdwInOut; i++)
            {
                if (LOBYTE(rgbdod[i].dwData) > 0)
                {
                    if (rgbdod[i].dwOfs == DIK_W)
                    {
                        //[W] key is pressed
                    }

                }

                if (LOBYTE(rgbdod[i].dwData) == 0)
                {
                    if (rgbdod[i].dwOfs == DIK_W)
                    {
                        //[W] key is released
                    }

                }
            }
            if (blockKeyboard)
            {
                *pdwInOut = 0; // Set arary size to 0
            }
        }


    }


    void KbmBuildKeyMapping()
    {
        s_DXKeyMapping[EngineInput::kKey_escape] = 1;
        s_DXKeyMapping[EngineInput::kKey_1] = 2;
        s_DXKeyMapping[EngineInput::kKey_2] = 3;
        s_DXKeyMapping[EngineInput::kKey_3] = 4;
        s_DXKeyMapping[EngineInput::kKey_4] = 5;
        s_DXKeyMapping[EngineInput::kKey_5] = 6;
        s_DXKeyMapping[EngineInput::kKey_6] = 7;
        s_DXKeyMapping[EngineInput::kKey_7] = 8;
        s_DXKeyMapping[EngineInput::kKey_8] = 9;
        s_DXKeyMapping[EngineInput::kKey_9] = 10;
        s_DXKeyMapping[EngineInput::kKey_0] = 11;
        s_DXKeyMapping[EngineInput::kKey_minus] = 12;
        s_DXKeyMapping[EngineInput::kKey_equals] = 13;
        s_DXKeyMapping[EngineInput::kKey_back] = 14;
        s_DXKeyMapping[EngineInput::kKey_tab] = 15;
        s_DXKeyMapping[EngineInput::kKey_q] = 16;
        s_DXKeyMapping[EngineInput::kKey_w] = 17;
        s_DXKeyMapping[EngineInput::kKey_e] = 18;
        s_DXKeyMapping[EngineInput::kKey_r] = 19;
        s_DXKeyMapping[EngineInput::kKey_t] = 20;
        s_DXKeyMapping[EngineInput::kKey_y] = 21;
        s_DXKeyMapping[EngineInput::kKey_u] = 22;
        s_DXKeyMapping[EngineInput::kKey_i] = 23;
        s_DXKeyMapping[EngineInput::kKey_o] = 24;
        s_DXKeyMapping[EngineInput::kKey_p] = 25;
        s_DXKeyMapping[EngineInput::kKey_lbracket] = 26;
        s_DXKeyMapping[EngineInput::kKey_rbracket] = 27;
        s_DXKeyMapping[EngineInput::kKey_return] = 28;
        s_DXKeyMapping[EngineInput::kKey_lcontrol] = 29;
        s_DXKeyMapping[EngineInput::kKey_a] = 30;
        s_DXKeyMapping[EngineInput::kKey_s] = 31;
        s_DXKeyMapping[EngineInput::kKey_d] = 32;
        s_DXKeyMapping[EngineInput::kKey_f] = 33;
        s_DXKeyMapping[EngineInput::kKey_g] = 34;
        s_DXKeyMapping[EngineInput::kKey_h] = 35;
        s_DXKeyMapping[EngineInput::kKey_j] = 36;
        s_DXKeyMapping[EngineInput::kKey_k] = 37;
        s_DXKeyMapping[EngineInput::kKey_l] = 38;
        s_DXKeyMapping[EngineInput::kKey_semicolon] = 39;
        s_DXKeyMapping[EngineInput::kKey_apostrophe] = 40;
        s_DXKeyMapping[EngineInput::kKey_grave] = 41;
        s_DXKeyMapping[EngineInput::kKey_lshift] = 42;
        s_DXKeyMapping[EngineInput::kKey_backslash] = 43;
        s_DXKeyMapping[EngineInput::kKey_z] = 44;
        s_DXKeyMapping[EngineInput::kKey_x] = 45;
        s_DXKeyMapping[EngineInput::kKey_c] = 46;
        s_DXKeyMapping[EngineInput::kKey_v] = 47;
        s_DXKeyMapping[EngineInput::kKey_b] = 48;
        s_DXKeyMapping[EngineInput::kKey_n] = 49;
        s_DXKeyMapping[EngineInput::kKey_m] = 50;
        s_DXKeyMapping[EngineInput::kKey_comma] = 51;
        s_DXKeyMapping[EngineInput::kKey_period] = 52;
        s_DXKeyMapping[EngineInput::kKey_slash] = 53;
        s_DXKeyMapping[EngineInput::kKey_rshift] = 54;
        s_DXKeyMapping[EngineInput::kKey_multiply] = 55;
        s_DXKeyMapping[EngineInput::kKey_lalt] = 56;
        s_DXKeyMapping[EngineInput::kKey_space] = 57;
        s_DXKeyMapping[EngineInput::kKey_capital] = 58;
        s_DXKeyMapping[EngineInput::kKey_f1] = 59;
        s_DXKeyMapping[EngineInput::kKey_f2] = 60;
        s_DXKeyMapping[EngineInput::kKey_f3] = 61;
        s_DXKeyMapping[EngineInput::kKey_f4] = 62;
        s_DXKeyMapping[EngineInput::kKey_f5] = 63;
        s_DXKeyMapping[EngineInput::kKey_f6] = 64;
        s_DXKeyMapping[EngineInput::kKey_f7] = 65;
        s_DXKeyMapping[EngineInput::kKey_f8] = 66;
        s_DXKeyMapping[EngineInput::kKey_f9] = 67;
        s_DXKeyMapping[EngineInput::kKey_f10] = 68;
        s_DXKeyMapping[EngineInput::kKey_numlock] = 69;
        s_DXKeyMapping[EngineInput::kKey_scroll] = 70;
        s_DXKeyMapping[EngineInput::kKey_numpad7] = 71;
        s_DXKeyMapping[EngineInput::kKey_numpad8] = 72;
        s_DXKeyMapping[EngineInput::kKey_numpad9] = 73;
        s_DXKeyMapping[EngineInput::kKey_subtract] = 74;
        s_DXKeyMapping[EngineInput::kKey_numpad4] = 75;
        s_DXKeyMapping[EngineInput::kKey_numpad5] = 76;
        s_DXKeyMapping[EngineInput::kKey_numpad6] = 77;
        s_DXKeyMapping[EngineInput::kKey_add] = 78;
        s_DXKeyMapping[EngineInput::kKey_numpad1] = 79;
        s_DXKeyMapping[EngineInput::kKey_numpad2] = 80;
        s_DXKeyMapping[EngineInput::kKey_numpad3] = 81;
        s_DXKeyMapping[EngineInput::kKey_numpad0] = 82;
        s_DXKeyMapping[EngineInput::kKey_decimal] = 83;
        s_DXKeyMapping[EngineInput::kKey_f11] = 87;
        s_DXKeyMapping[EngineInput::kKey_f12] = 88;
        s_DXKeyMapping[EngineInput::kKey_numpadenter] = 156;
        s_DXKeyMapping[EngineInput::kKey_rcontrol] = 157;
        s_DXKeyMapping[EngineInput::kKey_divide] = 181;
        s_DXKeyMapping[EngineInput::kKey_sysrq] = 183;
        s_DXKeyMapping[EngineInput::kKey_ralt] = 184;
        s_DXKeyMapping[EngineInput::kKey_pause] = 197;
        s_DXKeyMapping[EngineInput::kKey_home] = 199;
        s_DXKeyMapping[EngineInput::kKey_up] = 200;
        s_DXKeyMapping[EngineInput::kKey_pgup] = 201;
        s_DXKeyMapping[EngineInput::kKey_left] = 203;
        s_DXKeyMapping[EngineInput::kKey_right] = 205;
        s_DXKeyMapping[EngineInput::kKey_end] = 207;
        s_DXKeyMapping[EngineInput::kKey_down] = 208;
        s_DXKeyMapping[EngineInput::kKey_pgdn] = 209;
        s_DXKeyMapping[EngineInput::kKey_insert] = 210;
        s_DXKeyMapping[EngineInput::kKey_delete] = 211;
        s_DXKeyMapping[EngineInput::kKey_lwin] = 219;
        s_DXKeyMapping[EngineInput::kKey_rwin] = 220;
        s_DXKeyMapping[EngineInput::kKey_apps] = 221;

    }

    void KbmZeroInputs()
    {
        memset(&s_MouseState, 0, sizeof(DIMOUSESTATE2));
        memset(s_Keybuffer, 0, sizeof(s_Keybuffer));
    }

    void KbmInitialize()
    {
        KbmBuildKeyMapping();

        LPDIRECTINPUTDEVICE8 lpdiMouse;
        

        if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_DI, nullptr)))
            ASSERT(false, "DirectInput8 initialization failed.");

        if (FAILED(s_DI->CreateDevice(GUID_SysKeyboard, &s_Keyboard, nullptr)))
            ASSERT(false, "Keyboard CreateDevice failed.");
        if (FAILED(s_Keyboard->SetDataFormat(&c_dfDIKeyboard)))
            ASSERT(false, "Keyboard SetDataFormat failed.");
        if (FAILED(s_Keyboard->SetCooperativeLevel(CECore::CreativeBaseWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            ASSERT(false, "Keyboard SetCooperativeLevel failed.");


        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = 10;

        if (FAILED(s_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
            ASSERT(false, "Keyboard set buffer size failed.");

       if (FAILED(s_DI->CreateDevice(GUID_SysMouse, &s_Mouse, nullptr)))
            ASSERT(false, "Mouse CreateDevice failed.");
        if (FAILED(s_Mouse->SetDataFormat(&c_dfDIMouse2)))
            ASSERT(false, "Mouse SetDataFormat failed.");
        if (FAILED(s_Mouse->SetCooperativeLevel(CECore::CreativeBaseWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            ASSERT(false, "Mouse SetCooperativeLevel failed.");


        KbmZeroInputs();
    }

    void KbmShutdown()
    {
        if (s_Keyboard)
        {
            s_Keyboard->Unacquire();
            s_Keyboard->Release();
            s_Keyboard = nullptr;
        }
        if (s_Mouse)
        {
            s_Mouse->Unacquire();
            s_Mouse->Release();
            s_Mouse = nullptr;
        }
        if (s_DI)
        {
            s_DI->Release();
            s_DI = nullptr;
        }
    }

    void KbmUpdate()
    {
        HWND foreground = GetForegroundWindow();
        bool visible = IsWindowVisible(foreground) != 0;

        if (foreground != CECore::CreativeBaseWindow // wouldn't be able to acquire
            || !visible)
        {
            KbmZeroInputs();
        }
        else
        {
            s_Mouse->Acquire();
            s_Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_MouseState);
            s_Keyboard->Acquire();
            s_Keyboard->GetDeviceState(sizeof(s_Keybuffer), s_Keybuffer);
        }
    }


}

void EngineInput::Initialize()
{
    ZeroMemory(s_Buttons, sizeof(s_Buttons) );
    ZeroMemory(s_Analogs, sizeof(s_Analogs) );

#ifdef USE_KEYBOARD_MOUSE
    if (EngineInput::IsAnyPressed())
    {
        KbmInitialize();
    }
#endif
}

void EngineInput::Shutdown()
{
#ifdef USE_KEYBOARD_MOUSE
    KbmShutdown();
#endif
}

void EngineInput::Update( float frameDelta )
{
    memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));
    memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
    memset(s_Analogs, 0, sizeof(s_Analogs));


#ifdef USE_KEYBOARD_MOUSE


    if (EngineInput::IsAnyPressed())
    {
        KbmUpdate();

        for (uint32_t i = 0; i < kNumKeys; ++i)
        {
            s_Buttons[0][i] = (s_Keybuffer[s_DXKeyMapping[i]] & 0x80) != 0;
        }

        if (s_MouseState.rgbButtons[2])
        {
            s_Analogs[kAnalogMouseX] = (float)s_MouseState.lX * ViewportSpeed;
            s_Analogs[kAnalogMouseY] = (float)s_MouseState.lY * -ViewportSpeed;
        }

        if (s_MouseState.lZ > 0)
        {
            s_Analogs[kAnalogMouseScroll] = 1.0f;
        }
            
        else if (s_MouseState.lZ < 0)
        {
            s_Analogs[kAnalogMouseScroll] = -1.0f;
        }
    }

    if (EngineInput::IsPressed(kKey_add))
    {
        ViewportSpeed += 0.0001;
        MovementSpeed += 100.0f;
    }

    if (EngineInput::IsPressed(kKey_subtract))
    {
        if (ViewportSpeed >= 0.0001)
        {
            ViewportSpeed -= 0.0001;
            MovementSpeed -= 100.0f;

        }
        else
        {

        }
    }


    // Update time duration for buttons pressed
    for (uint32_t i = 0; i < kNumDigitalInputs; ++i)
    {
        if (s_Buttons[0][i])
        {
            if (!s_Buttons[1][i])
                s_HoldDuration[i] = 0.0f;
            else
                s_HoldDuration[i] += frameDelta;
        }
    }

    for (uint32_t i = 0; i < kNumAnalogInputs; ++i)
    {
        s_AnalogsTC[i] = s_Analogs[i] * frameDelta;
    }

    


#endif

    
}

bool EngineInput::IsAnyPressed( void )
{
    return s_Buttons[0] != 0;
}

bool EngineInput::IsPressed( DigitalInput di )
{
    return s_Buttons[0][di];
}

bool EngineInput::IsFirstPressed( DigitalInput di )
{
    return s_Buttons[0][di] && !s_Buttons[1][di];
}

bool EngineInput::IsReleased( DigitalInput di )
{
    return !s_Buttons[0][di];
}

bool EngineInput::IsFirstReleased( DigitalInput di )
{
    return !s_Buttons[0][di] && s_Buttons[1][di];
}

float EngineInput::GetDurationPressed( DigitalInput di )
{
    return s_HoldDuration[di];
}

float EngineInput::GetAnalogInput( AnalogInput ai )
{
    return s_Analogs[ai];
}

float EngineInput::GetTimeCorrectedAnalogInput( AnalogInput ai )
{
    return s_AnalogsTC[ai];
}
