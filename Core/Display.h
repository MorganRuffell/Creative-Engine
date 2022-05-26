
#pragma once

#include "pch.h"
#include <cstdint>

class CGraphicsContext;

namespace Display
{
    void Initialize(HWND window);
    HMONITOR GetPrimaryMonitorHandle(HWND window);
    void Shutdown(void);

    void InitGUI(const HWND& hwnd);
    void UpdateUI();
    void DrawGUI();

    void UpdateGizmo();


    static bool LoadUITextureFromFile(const char* filename, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, ID3D12Resource** out_tex_resource, int* out_width, int* out_height);

    void InitCreativeIcon();
    void InitUIIcon(_In_ const char* UI, _In_ int i, D3D12_GPU_DESCRIPTOR_HANDLE* GPUDescHandle);

    static ID3D12DescriptorHeap*                   g_SRVDescHeap;
    static D3D12_GPU_DESCRIPTOR_HANDLE             UILogoGPU_CBVSRVUAV_HANDLE;

   


    static int IconWidth;
    static int IconHeight;


    static std::recursive_mutex g_imguiMutex;

    void Resize(uint32_t width, uint32_t height);
    void Present(void);
}

namespace CGraphics
{
    static HANDLE g_SwapChainWritableObject;

    static uint64_t s_FrameIndex = 0;
    static UINT g_CurrentBuffer = 0;

    extern uint32_t g_DisplayWidth;
    extern uint32_t g_DisplayHeight;
    extern bool g_bEnableHDROutput;

    // Returns the number of elapsed frames since application start
    uint64_t GetFrameCount(void);

    // The amount of time elapsed during the last completed frame.  The CPU and/or
    // GPU may be idle during parts of the frame.  The frame time measures the time
    // between calls to present each frame.
    float GetFrameTime(void);

    // The total number of frames per second
    float GetFrameRate(void);

    extern bool g_bEnableHDROutput;
}
