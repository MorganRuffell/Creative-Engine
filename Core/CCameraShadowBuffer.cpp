

#include "pch.h"
#include "CCameraShadowBuffer.h"
#include "CGPURamAllocator.h"
#include "CCommandContext.h"

void CCameraShadowBuffer::Create( const std::wstring& Name, uint32_t Width, uint32_t Height, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr )
{
    CGPUDepthBuffer::Create( Name, Width, Height, DXGI_FORMAT_D16_UNORM, VidMemPtr );

    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)Width;
    m_Viewport.Height = (float)Height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    // Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
    m_Scissor.left = 1;
    m_Scissor.top = 1;
    m_Scissor.right = (LONG)Width - 2;
    m_Scissor.bottom = (LONG)Height - 2;
}

void CCameraShadowBuffer::Create( const std::wstring& Name, uint32_t Width, uint32_t Height, CGPURamAllocator& Allocator )
{
    CGPUDepthBuffer::Create( Name, Width, Height, DXGI_FORMAT_D16_UNORM, Allocator );

    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = (float)Width;
    m_Viewport.Height = (float)Height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    // Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
    m_Scissor.left = 1;
    m_Scissor.top = 1;
    m_Scissor.right = (LONG)Width - 2;
    m_Scissor.bottom = (LONG)Height - 2;
}

void CCameraShadowBuffer::BeginRendering( CGraphicsContext& Context )
{
    Context.TransitionResource(*this, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    Context.ClearDepth(*this);
    Context.SetDepthStencilTarget(GetDSV());
    Context.SetViewportAndScissor(m_Viewport, m_Scissor);
}

void CCameraShadowBuffer::EndRendering( CGraphicsContext& Context )
{
    Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
