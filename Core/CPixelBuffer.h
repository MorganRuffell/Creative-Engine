

#pragma once

#include "CGPUBaseResource.h"

class CGPURamAllocator;

class CPixelBuffer : public CGPUResource
{
public:
    CPixelBuffer() : m_Width(0), m_Height(0), m_ArraySize(0), m_Format(DXGI_FORMAT_UNKNOWN), m_BankRotation(0) {}

    // these are getters, they fetch buffer size, height and depth.
    uint32_t GetWidth(void) const { return m_Width; }
    uint32_t GetHeight(void) const { return m_Height; }
    uint32_t GetDepth(void) const { return m_ArraySize; }
    const DXGI_FORMAT& GetFormat(void) const { return m_Format; }

    // Has no effect on Desktop
    void SetBankRotation(_In_opt_ uint32_t RotationAmount )
    {
        (RotationAmount);
    }

    // Write the raw pixel buffer contents to a file
    void ExportToFile( _Inout_ const std::wstring& FilePath );

protected:

    D3D12_RESOURCE_DESC DescribeTex2D(uint32_t Width, uint32_t Height, uint32_t DepthOrArraySize, uint32_t NumMips, DXGI_FORMAT Format, UINT Flags);

    void AssociateWithResource(_Inout_ ID3D12Device8* Device, const std::wstring& Name, _Inout_ ID3D12Resource2* Resource, D3D12_RESOURCE_STATES CurrentState );

    void CreateTextureResource(_Inout_ ID3D12Device8* Device, const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc,
        D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );

    void CreateTextureResource(_Inout_ ID3D12Device8* Device, const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc,
        D3D12_CLEAR_VALUE ClearValue, CGPURamAllocator& Allocator );

    static DXGI_FORMAT GetBaseFormat( DXGI_FORMAT Format );
    static DXGI_FORMAT GetUAVFormat( DXGI_FORMAT Format );
    static DXGI_FORMAT GetDSVFormat( DXGI_FORMAT Format );
    static DXGI_FORMAT GetDepthFormat( DXGI_FORMAT Format );
    static DXGI_FORMAT GetStencilFormat( DXGI_FORMAT Format );
    static size_t BytesPerPixel( DXGI_FORMAT Format );

protected:

    uint32_t            m_Width;
    uint32_t            m_Height;
    uint32_t            m_ArraySize;

    DXGI_FORMAT         m_Format;
    uint32_t            m_BankRotation;
};
