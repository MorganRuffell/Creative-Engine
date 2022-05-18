
#pragma once

#include "CPixelBuffer.h"
#include "CRGBAColour.h"
#include "CGPUBuffer.h"

class CGPURamAllocator;

class CRGBBuffer : public CPixelBuffer
{
public:
    CRGBBuffer( CRGBAColour ClearColor = CRGBAColour(0.0f, 0.0f, 0.0f, 0.0f)  )
        : m_ClearColor(ClearColor), m_NumMipMaps(0), m_FragmentCount(1), m_SampleCount(1)
    {
        m_RTVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_SRVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        for (int i = 0; i < _countof(m_UAVHandle); ++i)
            m_UAVHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    void CreateFromSwapChain( _In_ const std::wstring& Name, _Inout_ ID3D12Resource2* BaseResource );

    void Create(const std::wstring& Name, uint32_t Width, uint32_t Height,
        DXGI_FORMAT Format, _Inout_ D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
    
    void Create(const std::wstring& Name, uint32_t Width, uint32_t Height,
        DXGI_FORMAT Format, _Inout_ CGPURamAllocator& Allocator);

    void CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
        DXGI_FORMAT Format, _Inout_ D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
    
    void CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
        DXGI_FORMAT Format, _Inout_ CGPURamAllocator& Allocator);

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRVHandle; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return m_RTVHandle; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAVHandle[0]; }

    void SetClearColor( CRGBAColour ClearColor ) { m_ClearColor = ClearColor; }

    void SetMsaaMode( _In_ uint32_t NumColorSamples, _In_ uint32_t NumCoverageSamples )
    {
        ASSERT(NumCoverageSamples >= NumColorSamples);
        m_FragmentCount = NumColorSamples;
        m_SampleCount = NumCoverageSamples;
    }

    CRGBAColour GetClearColor(void) const { return m_ClearColor; }

    void GenerateMipMaps(CCommandContext& Context);

public:

    D3D12_RESOURCE_FLAGS CombineResourceFlags( void ) const
    {
        D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;

        if (Flags == D3D12_RESOURCE_FLAG_NONE && m_FragmentCount == 1)
            Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | Flags;
    }

    void CreateDerivedViews(_Inout_ ID3D12Device8* Device, _In_ DXGI_FORMAT Format, _In_ uint32_t ArraySize, _In_ uint32_t NumMips = 1);

    CRGBAColour m_ClearColor;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_UAVHandle[12];
    uint32_t m_NumMipMaps; // number of texture sublevels
    uint32_t m_FragmentCount;
    uint32_t m_SampleCount;
};
