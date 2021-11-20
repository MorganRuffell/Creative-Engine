// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <array>

// DirectX Includes
#include <d3d11.h>
#include <d3d12.h>
#include "combaseapi.h"
#include <cassert>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ID3D11Device* g_device;
IDXGISwapChain* g_SwapChain;
ID3D11DeviceContext* g_deviceContext;

float g_AspectRatio;

//For drawing graphics on the screen!
ID3D11RenderTargetView* g_RenderTargetView;
D3D11_VIEWPORT g_viewport;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void SetupBackBuffer(HRESULT result);
void SetupViewport(DXGI_SWAP_CHAIN_DESC SwapChainInstance);

//Stable

bool ReleaseGlobalResources()
{
    g_device->Release();
    g_deviceContext->Release();
	g_RenderTargetView->Release();
    g_SwapChain->Release();

    if (g_device == nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

    MSG msg;

    //Main Program run loop.
    while (true)
    {
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

        if (msg.message == WM_QUIT)
        {
            break;
        }

        //Break if user presses Esc Key
        if (GetAsyncKeyState(VK_ESCAPE)) break;
  
        ID3D11RenderTargetView* NewRenderTargetView[] = {g_RenderTargetView};
        g_deviceContext->OMSetRenderTargets(1, NewRenderTargetView, nullptr);

        //This must be a carray of 4 elements for the color
        float RGBA_COLOR[4] = { 1,0,0,1 };

        g_deviceContext->ClearRenderTargetView(g_RenderTargetView, RGBA_COLOR);
        g_deviceContext->RSSetViewports(1, &g_viewport);
        g_SwapChain->Present(0, 0);

    }

    int x = (ReleaseGlobalResources() == true) ? 0 : 1;
   
      
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   //D3d11 code here
   RECT rectangle;
   GetClientRect(hWnd, &rectangle);

   // Attach D3d to the window -- We're attaching DirectX12 
   //D3D_FEATURE_LEVEL DX11 = D3D_FEATURE_LEVEL_11_0;
   D3D_FEATURE_LEVEL DX12 = D3D_FEATURE_LEVEL_12_0;

   DXGI_SWAP_CHAIN_DESC DirectSwapchain;

   ZeroMemory(&DirectSwapchain, sizeof(DXGI_SWAP_CHAIN_DESC)); //Zero the memory, basically kill everything inside the memory
   DirectSwapchain.BufferCount = 1; // You can increment graphics buffers if you want to later on.
   DirectSwapchain.OutputWindow = hWnd;
   DirectSwapchain.Windowed = true;
   DirectSwapchain.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
   DirectSwapchain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   DirectSwapchain.BufferDesc.Width = rectangle.right - rectangle.left;
   DirectSwapchain.BufferDesc.Height = rectangle.bottom - rectangle.top;
   DirectSwapchain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   DirectSwapchain.SampleDesc.Count = 1;

   g_AspectRatio = DirectSwapchain.BufferDesc.Width / (float)DirectSwapchain.BufferDesc.Height;

   //New instance of HResult
   HRESULT result;


   result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &DX12, 1, D3D11_SDK_VERSION, &DirectSwapchain, &g_SwapChain, &g_device, 0, &g_deviceContext);

   assert(!FAILED(result));

   SetupBackBuffer(result);
   SetupViewport(DirectSwapchain);

   return TRUE;
}

void SetupBackBuffer(HRESULT result)
{
    ID3D11Resource* BackBuffer;
    result == g_SwapChain->GetBuffer(0, __uuidof(BackBuffer), (void**)&BackBuffer);
    result == g_device->CreateRenderTargetView(BackBuffer, NULL, &g_RenderTargetView);

    assert(!FAILED(result));

    //We have to release this so that memory does not leak.
    BackBuffer->Release();
}

void SetupViewport(DXGI_SWAP_CHAIN_DESC SwapChainInstance)
{
	g_viewport.Width = SwapChainInstance.BufferDesc.Width;
	g_viewport.Height = SwapChainInstance.BufferDesc.Height;
	g_viewport.TopLeftY = g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0;
	g_viewport.MaxDepth = 1;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
