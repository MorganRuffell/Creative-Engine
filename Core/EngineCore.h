
#pragma once

#include "pch.h"
#include "CDirectX12Core.h"

using namespace CGraphics;

namespace CECore
{
    extern bool gIsSupending;

   
    class CreativeEngineApplication
    {

    public:
        virtual void Startup(_In_ HWND window) = 0;

        virtual void Cleanup( void ) = 0;

        virtual bool IsDone( void );

        virtual void Update( float deltaT ) = 0;

        virtual void RenderScene() = 0;

        virtual void RenderUI( class GraphicsContext& ) {};

        virtual bool RequiresRaytracingSupport() const { return false; }

    };
}

namespace CECore
{
    int RunApplication( _In_ CreativeEngineApplication& app, _Inout_ const wchar_t* className, _In_ HINSTANCE hInst, _In_ int nCmdShow );
}


#define CREATE_APPLICATION(CreativeEngineApplication) \
    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) \
    { \
        return CECore::RunApplication( CreativeEngineApplication(), L"Creative Engine", hInstance, nCmdShow ); \
    }
