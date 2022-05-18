#pragma once

#include "CGPUDepthBuffer.h"

class CGPURamAllocator;
class CGraphicsContext;

class CCameraShadowBuffer : public CGPUDepthBuffer
{
public:
    CCameraShadowBuffer() {}
        
    void Create( const std::wstring& Name, uint32_t Width, uint32_t Height,
        D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN );
    void Create( const std::wstring& Name, uint32_t Width, uint32_t Height, CGPURamAllocator& Allocator );

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return GetDepthSRV(); }

    void BeginRendering( CGraphicsContext& context );
    void EndRendering( CGraphicsContext& context );

private:
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_Scissor;
};
