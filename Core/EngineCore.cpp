#include "pch.h"
#include "EngineCore.h"
#include "CDirectX12Core.h"

#include "../ImGUI/imgui.h"
#include "../ImGUI/imgui_impl_win32.h"
#include "../ImGUI/imgui_impl_dx12.h"


#include "CInternalWindowsTime.h"
#include "EngineInput.h"
#include "BufferManager.h"
#include "CCommandContext.h"
#include "CPostEffects.h"
#include "Display.h"
#include "Util/CommandLineArg.h"
#include <shellapi.h>
#include <wincodec.h>
#include <iostream>


using namespace Microsoft::WRL::Wrappers;

#pragma comment(lib, "runtimeobject.lib") 

namespace CECore
{
    using namespace CGraphics;

    static CDirectX12Core* Core;

    bool gIsSupending = false;

    void InitializeApplication( CreativeEngineApplication& engine, HWND MainWindow )
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        CommandLineArgs::Initialize(argc, argv);

        CGraphics::Initialize(engine.RequiresRaytracingSupport(), MainWindow);
        CInternalWindowsTime::Initialize();
        EngineInput::Initialize();
        EngineTuning::Initialize();

        engine.Startup(MainWindow);
    }

    void TerminateApplication( CreativeEngineApplication& game )
    {
        g_CommandManager.IdleGPU();

        game.Cleanup();

        EngineInput::Shutdown();
    }

    bool UpdateApplication( CreativeEngineApplication& game )
    {
        EngineProfiling::Update();

        float DeltaTime = CGraphics::GetFrameTime();
    
        EngineInput::Update(DeltaTime);
        EngineTuning::Update(DeltaTime);
        
        game.Update(DeltaTime);

        game.RenderScene();

        CGlobalPostEffects::Render();

       
        Display::Present();

        return !game.IsDone();
    }

    
    // Default implementation to be overridden by the application
    bool CreativeEngineApplication::IsDone( void )
    {
        return EngineInput::IsFirstPressed(EngineInput::kKey_escape);
    }

    
    HWND CreativeBaseWindow = nullptr;
    HWND CreativeSplashScreen = nullptr;

    LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );


    int RunApplication( CreativeEngineApplication& Engine, const wchar_t* className, HINSTANCE hInst, int nCmdShow )
    {
        if (!XMVerifyCPUSupport())
            return 1;

        // I'm forcing this to use multithreading, this prepares the CPU and GPU beforehand.
        RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
        ASSERT_SUCCEEDED(InitializeWinRT);
    

        // Create window
       /* RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);*/

        WNDCLASSEX _GraphicsWindow = { sizeof(WNDCLASSEX), CS_VREDRAW, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL,L"Creative Engine Viewport", NULL };

        //Might load icon
        _GraphicsWindow.hIconSm = LoadIcon(_GraphicsWindow.hInstance, MAKEINTRESOURCE(IDB_PNG1));
        _GraphicsWindow.hIcon = LoadIcon(_GraphicsWindow.hInstance, MAKEINTRESOURCE(IDB_PNG1));
        
        ::RegisterClassEx(&_GraphicsWindow);

        CreativeBaseWindow = ::CreateWindow(_GraphicsWindow.lpszClassName, L"Creative Engine V0.1 -- ALPHA -- DX12", WS_OVERLAPPEDWINDOW, 100, 100, 1680, 900, NULL, NULL, _GraphicsWindow.hInstance, NULL);
        
        ASSERT(CreativeBaseWindow != 0);

        InitializeApplication(Engine, CreativeBaseWindow);

        std::cout << "Application Initalized" << std::endl;
        
        ImGui::StyleColorsLight();
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ShowWindow(CreativeBaseWindow, SW_SHOWMAXIMIZED);

        UpdateWindow(CreativeBaseWindow);
        // main do while loop
        while(UpdateApplication(Engine))
        {
            UpdateWindow(CreativeBaseWindow);



            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (msg.message == WM_QUIT)
                break;



        }
        
        TerminateApplication(Engine);

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

        CGraphics::Shutdown();
        return 0;
    }

   

    //--------------------------------------------------------------------------------------
    // Called every time the application receives a message
    //--------------------------------------------------------------------------------------
    LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
    {
        switch( message )
        {
        case WM_SIZE:
            Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
        }

        return 0;
    }

}
