
#pragma once

#include "CPixelBuffer.h"

class CGPURamAllocator;

class CGPUDepthBuffer : public CPixelBuffer
{
public:
    CGPUDepthBuffer(_In_ float ClearDepth = 0.0f, _In_ uint8_t ClearStencil = 0 )
        : m_ClearDepth(ClearDepth), m_ClearStencil(ClearStencil) 
    {
        m_hDSV[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_hDSV[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_hDSV[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_hDSV[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_hDepthSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        m_hStencilSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }


public:

    // Get pre-created CPU-visible descriptor handles
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return m_hDSV[0]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return m_hDSV[1]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return m_hDSV[2]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return m_hDSV[3]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return m_hDepthSRV; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return m_hStencilSRV; }

    float GetClearDepth() const { return m_ClearDepth; }
    uint8_t GetClearStencil() const { return m_ClearStencil; }

public:

    void Create( _In_ const std::wstring& Name, _In_ uint32_t Width, _In_ uint32_t Height, _In_ DXGI_FORMAT Format,
        D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );

    void Create( _In_ const std::wstring& Name, _In_ uint32_t Width, _In_ uint32_t Height, _In_ DXGI_FORMAT Format,
        CGPURamAllocator& Allocator );

    void Create(_In_ const std::wstring& Name, _In_ uint32_t Width, _In_ uint32_t Height, _In_ uint32_t NumSamples, _In_ DXGI_FORMAT Format,
        _Inout_ D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );

    void Create( _In_ const std::wstring& Name, _In_ uint32_t Width, _In_ uint32_t Height, _In_ uint32_t NumSamples, _In_ DXGI_FORMAT Format,
        _Inout_ CGPURamAllocator& Allocator );


protected:

    void CreateDerivedViews( ID3D12Device8* Device, DXGI_FORMAT Format );

    float m_ClearDepth;
    uint8_t m_ClearStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hDSV[4];
    D3D12_CPU_DESCRIPTOR_HANDLE m_hDepthSRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hStencilSRV;
};
