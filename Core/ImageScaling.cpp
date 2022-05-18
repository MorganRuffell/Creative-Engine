#include "pch.h"
#include "ImageScaling.h"
#include "BufferManager.h"
#include "CCommandContext.h"

#include "CompiledShaders/ScreenQuadPresentVS.h"

#include "CompiledShaders/BilinearUpsamplePS.h"
#include "CompiledShaders/BicubicHorizontalUpsamplePS.h"
#include "CompiledShaders/BicubicVerticalUpsamplePS.h"
#include "CompiledShaders/BicubicUpsampleCS.h"
#include "CompiledShaders/BicubicUpsampleFast16CS.h"
#include "CompiledShaders/BicubicUpsampleFast24CS.h"
#include "CompiledShaders/BicubicUpsampleFast32CS.h"
#include "CompiledShaders/SharpeningUpsamplePS.h"

#include "CompiledShaders/LanczosHorizontalPS.h"
#include "CompiledShaders/LanczosVerticalPS.h"
#include "CompiledShaders/LanczosCS.h"
#include "CompiledShaders/LanczosFast16CS.h"
#include "CompiledShaders/LanczosFast24CS.h"
#include "CompiledShaders/LanczosFast32CS.h"

using namespace CGraphics;

namespace CGraphics
{
    extern CRootSignature s_PresentRS;
}

namespace ImageScaling
{
    CGraphicsPSO SharpeningUpsamplePS(L"Image Scaling: Sharpen Upsample PSO");
    CGraphicsPSO BicubicHorizontalUpsamplePS(L"Image Scaling: Bicubic Horizontal Upsample PSO");
    CGraphicsPSO BicubicVerticalUpsamplePS(L"Image Scaling: Bicubic Vertical Upsample PSO");
    CGraphicsPSO BilinearUpsamplePS(L"Image Scaling: Bilinear Upsample PSO");
    CGraphicsPSO LanczosHorizontalPS(L"Image Scaling: Lanczos Horizontal PSO");
    CGraphicsPSO LanczosVerticalPS(L"Image Scaling: Lanczos Vertical PSO");

    enum { kDefaultCS, kFast16CS, kFast24CS, kFast32CS, kNumCSModes };
    CComputePSO LanczosCS[kNumCSModes];
    CComputePSO BicubicCS[kNumCSModes];

    CETFloat BicubicUpsampleWeight("Graphics/Display/Image Scaling/Bicubic Filter Weight", -0.5f, -1.0f, -0.25f, 0.25f);
    CETFloat SharpeningSpread("Graphics/Display/Image Scaling/Sharpness Sample Spread", 1.0f, 0.7f, 2.0f, 0.1f);
    CETFloat SharpeningRotation("Graphics/Display/Image Scaling/Sharpness Sample Rotation", 45.0f, 0.0f, 90.0f, 15.0f);
    CETFloat SharpeningStrength("Graphics/Display/Image Scaling/Sharpness Strength", 0.10f, 0.0f, 1.0f, 0.01f);
    CETBoolean ForcePixelShader("Graphics/Display/Image Scaling/Prefer Pixel Shader", false);

    void BilinearScale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source)
    {
        Context.SetPipelineState(BilinearUpsamplePS);
        Context.TransitionResource(dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
        Context.TransitionResource(source, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        Context.SetRenderTarget(dest.GetRTV());
        Context.SetViewportAndScissor(0, 0, dest.GetWidth(), dest.GetHeight());
        Context.SetDynamicDescriptor(0, 0, source.GetSRV());
        Context.Draw(3);
    }

    void BicubicScale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source)
    {
        // On Windows it is illegal to have a UAV of a swap chain buffer. In that case we
        // must fall back to the slower, two-pass pixel shader.
        bool bDestinationUAV = dest.GetUAV().ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

        // Draw or dispatch
        if (bDestinationUAV && !ForcePixelShader)
        {
            CComputeContext& cmpCtx = Context.GetCComputeContext();
            cmpCtx.SetRootSignature(s_PresentRS);

            const float scaleX = (float)source.GetWidth() / (float)dest.GetWidth();
            const float scaleY = (float)source.GetHeight() / (float)dest.GetHeight();
            cmpCtx.SetConstants(1, scaleX, scaleY, (float)BicubicUpsampleWeight);

            uint32_t tileWidth, tileHeight, shaderMode;

            if (source.GetWidth() * 16 <= dest.GetWidth() * 13 &&
                source.GetHeight() * 16 <= dest.GetHeight() * 13)
            {
                tileWidth = 16;
                tileHeight = 16;
                shaderMode = kFast16CS;
            }
            else if (source.GetWidth() * 24 <= dest.GetWidth() * 21 &&
                source.GetHeight() * 24 <= dest.GetHeight() * 21)
            {
                tileWidth = 32; // For some reason, occupancy drops with 24x24, reducing perf
                tileHeight = 24;
                shaderMode = kFast24CS;
            }
            else if (source.GetWidth() * 32 <= dest.GetWidth() * 29 &&
                source.GetHeight() * 32 <= dest.GetHeight() * 29)
            {
                tileWidth = 32;
                tileHeight = 32;
                shaderMode = kFast32CS;
            }
            else
            {
                tileWidth = 16;
                tileHeight = 16;
                shaderMode = kDefaultCS;
            }

            cmpCtx.SetPipelineState(BicubicCS[shaderMode]);
            cmpCtx.TransitionResource(source, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            cmpCtx.TransitionResource(dest, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            cmpCtx.SetDynamicDescriptor(0, 0, source.GetSRV());
            cmpCtx.SetDynamicDescriptor(3, 0, dest.GetUAV());

            cmpCtx.Dispatch2D(dest.GetWidth(), dest.GetHeight(), tileWidth, tileHeight);
        }
        else
        {
            Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            Context.SetRenderTarget(g_HorizontalBuffer.GetRTV());
            Context.SetViewportAndScissor(0, 0, dest.GetWidth(), source.GetHeight());
            Context.SetPipelineState(BicubicHorizontalUpsamplePS);
            Context.SetConstants(1, source.GetWidth(), source.GetHeight(), (float)BicubicUpsampleWeight);
            Context.Draw(3);

            Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
            Context.SetRenderTarget(dest.GetRTV());
            Context.SetViewportAndScissor(0, 0, dest.GetWidth(), dest.GetHeight());
            Context.SetPipelineState(BicubicVerticalUpsamplePS);
            Context.SetConstants(1, dest.GetWidth(), source.GetHeight(), (float)BicubicUpsampleWeight);
            Context.SetDynamicDescriptor(0, 0, g_HorizontalBuffer.GetSRV());
            Context.Draw(3);
        }
    }

    void BilinearSharpeningScale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source)
    {
        Context.SetPipelineState(SharpeningUpsamplePS);
        Context.TransitionResource(dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
        Context.SetRenderTarget(dest.GetRTV());
        Context.SetViewportAndScissor(0, 0, dest.GetWidth(), dest.GetHeight());
        float TexelWidth = 1.0f / source.GetWidth();
        float TexelHeight = 1.0f / source.GetHeight();
        float X = CMath::Cos((float)SharpeningRotation / 180.0f * 3.14159f) * (float)SharpeningSpread;
        float Y = CMath::Sin((float)SharpeningRotation / 180.0f * 3.14159f) * (float)SharpeningSpread;
        const float WA = (float)SharpeningStrength;
        const float WB = 1.0f + 4.0f * WA;
        float Constants[] = { X * TexelWidth, Y * TexelHeight, Y * TexelWidth, -X * TexelHeight, WA, WB };
        Context.SetConstantArray(1, _countof(Constants), Constants);
        Context.Draw(3);
    }

    void LanczosScale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source)
    {
        // Constants
        const float srcWidth = (float)source.GetWidth();
        const float srcHeight = (float)source.GetHeight();

        // On Windows it is illegal to have a UAV of a swap chain buffer. In that case we
        // must fall back to the slower, two-pass pixel shader.
        bool bDestinationUAV = dest.GetUAV().ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

        // Draw or dispatch
        if (bDestinationUAV && !ForcePixelShader)
        {
            CComputeContext& cmpCtx = Context.GetCComputeContext();
            cmpCtx.SetRootSignature(s_PresentRS);

            const float scaleX = srcWidth / (float)dest.GetWidth();
            const float scaleY = srcHeight / (float)dest.GetHeight();
            cmpCtx.SetConstants(1, scaleX, scaleY);

            uint32_t tileWidth, tileHeight, shaderMode;

            if (source.GetWidth() * 16 <= dest.GetWidth() * 13 &&
                source.GetHeight() * 16 <= dest.GetHeight() * 13)
            {
                tileWidth = 16;
                tileHeight = 16;
                shaderMode = kFast16CS;
            }
            else if (source.GetWidth() * 24 <= dest.GetWidth() * 21 &&
                     source.GetHeight() * 24 <= dest.GetHeight() * 21)
            {
                tileWidth = 32; // For some reason, occupancy drops with 24x24, reducing perf
                tileHeight = 24;
                shaderMode = kFast24CS;
            }
            else if (source.GetWidth() * 32 <= dest.GetWidth() * 29 &&
                     source.GetHeight() * 32 <= dest.GetHeight() * 29)
            {
                tileWidth = 32;
                tileHeight = 32;
                shaderMode = kFast32CS;
            }
            else
            {
                tileWidth = 16;
                tileHeight = 16;
                shaderMode = kDefaultCS;
            }

            cmpCtx.SetPipelineState(LanczosCS[shaderMode]);
            cmpCtx.TransitionResource(source, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            cmpCtx.TransitionResource(dest, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            cmpCtx.SetDynamicDescriptor(0, 0, source.GetSRV());
            cmpCtx.SetDynamicDescriptor(3, 0, dest.GetUAV());

            cmpCtx.Dispatch2D(dest.GetWidth(), dest.GetHeight(), tileWidth, tileHeight);
        }
        else
        {
            Context.SetRootSignature(s_PresentRS);
            Context.SetConstants(1, srcWidth, srcHeight);
            Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            Context.TransitionResource(source, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(dest, D3D12_RESOURCE_STATE_RENDER_TARGET);

            Context.SetDynamicDescriptor(0, 0, source.GetSRV());

            Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            Context.SetPipelineState(LanczosHorizontalPS);
            Context.SetRenderTarget(g_HorizontalBuffer.GetRTV());
            Context.SetViewportAndScissor(0, 0, g_HorizontalBuffer.GetWidth(), source.GetHeight());
            Context.Draw(3);

            Context.TransitionResource(g_HorizontalBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            Context.SetPipelineState(LanczosVerticalPS);
            Context.SetRenderTarget(dest.GetRTV());
            Context.SetViewportAndScissor(0, 0, dest.GetWidth(), dest.GetHeight());
            Context.SetDynamicDescriptor(0, 0, g_HorizontalBuffer.GetSRV());
            Context.Draw(3);
        }
    }
}


void ImageScaling::Initialize(DXGI_FORMAT DestFormat)
{

	BilinearUpsamplePS.SetRootSignature(s_PresentRS);
	BilinearUpsamplePS.SetRasterizerState(RasterizerTwoSided);
	BilinearUpsamplePS.SetBlendState(BlendDisable);
	BilinearUpsamplePS.SetDepthStencilState(DepthStateDisabled);
	BilinearUpsamplePS.SetSampleMask(0xFFFFFFFF);
	BilinearUpsamplePS.SetInputLayout(0, nullptr);
	BilinearUpsamplePS.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	BilinearUpsamplePS.SetVertexShader(g_pScreenQuadPresentVS, sizeof(g_pScreenQuadPresentVS));
	BilinearUpsamplePS.SetPixelShader(g_pBilinearUpsamplePS, sizeof(g_pBilinearUpsamplePS));
	BilinearUpsamplePS.SetRenderTargetFormat(DestFormat, DXGI_FORMAT_UNKNOWN);
	BilinearUpsamplePS.Finalize();

    BicubicHorizontalUpsamplePS = BilinearUpsamplePS;
    BicubicHorizontalUpsamplePS.SetPixelShader(g_pBicubicHorizontalUpsamplePS, sizeof(g_pBicubicHorizontalUpsamplePS) );
    BicubicHorizontalUpsamplePS.SetRenderTargetFormat(g_HorizontalBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
    BicubicHorizontalUpsamplePS.Finalize();

    BicubicVerticalUpsamplePS = BilinearUpsamplePS;
    BicubicVerticalUpsamplePS.SetPixelShader(g_pBicubicVerticalUpsamplePS, sizeof(g_pBicubicVerticalUpsamplePS) );
    BicubicVerticalUpsamplePS.Finalize();

    BicubicCS[kDefaultCS].SetRootSignature(s_PresentRS);
    BicubicCS[kDefaultCS].SetComputeShader(g_pBicubicUpsampleCS, sizeof(g_pBicubicUpsampleCS));
    BicubicCS[kDefaultCS].Finalize();
    BicubicCS[kFast16CS].SetRootSignature(s_PresentRS);
    BicubicCS[kFast16CS].SetComputeShader(g_pBicubicUpsampleFast16CS, sizeof(g_pBicubicUpsampleFast16CS));
    BicubicCS[kFast16CS].Finalize();
    BicubicCS[kFast24CS].SetRootSignature(s_PresentRS);
    BicubicCS[kFast24CS].SetComputeShader(g_pBicubicUpsampleFast24CS, sizeof(g_pBicubicUpsampleFast24CS));
    BicubicCS[kFast24CS].Finalize();
    BicubicCS[kFast32CS].SetRootSignature(s_PresentRS);
    BicubicCS[kFast32CS].SetComputeShader(g_pBicubicUpsampleFast32CS, sizeof(g_pBicubicUpsampleFast32CS));
    BicubicCS[kFast32CS].Finalize();

    SharpeningUpsamplePS = BilinearUpsamplePS;
    SharpeningUpsamplePS.SetPixelShader(g_pSharpeningUpsamplePS, sizeof(g_pSharpeningUpsamplePS) );
    SharpeningUpsamplePS.Finalize();

    LanczosHorizontalPS = BicubicHorizontalUpsamplePS;
    LanczosHorizontalPS.SetPixelShader(g_pLanczosHorizontalPS, sizeof(g_pLanczosHorizontalPS) );
    LanczosHorizontalPS.Finalize();

    LanczosVerticalPS = BilinearUpsamplePS;
    LanczosVerticalPS.SetPixelShader(g_pLanczosVerticalPS, sizeof(g_pLanczosVerticalPS) );
    LanczosVerticalPS.Finalize();

    LanczosCS[kDefaultCS].SetRootSignature(s_PresentRS);
    LanczosCS[kDefaultCS].SetComputeShader(g_pLanczosCS, sizeof(g_pLanczosCS));
    LanczosCS[kDefaultCS].Finalize();

    LanczosCS[kFast16CS].SetRootSignature(s_PresentRS);
    LanczosCS[kFast16CS].SetComputeShader(g_pLanczosFast16CS, sizeof(g_pLanczosFast16CS));
    LanczosCS[kFast16CS].Finalize();

    LanczosCS[kFast24CS].SetRootSignature(s_PresentRS);
    LanczosCS[kFast24CS].SetComputeShader(g_pLanczosFast24CS, sizeof(g_pLanczosFast24CS));
    LanczosCS[kFast24CS].Finalize();

    LanczosCS[kFast32CS].SetRootSignature(s_PresentRS);
    LanczosCS[kFast32CS].SetComputeShader(g_pLanczosFast32CS, sizeof(g_pLanczosFast32CS));
    LanczosCS[kFast32CS].Finalize();
}

void ImageScaling::Upscale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source, EScalingType tech)
{
    Context.SetRootSignature(s_PresentRS);
    Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context.TransitionResource(source, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(0, 0, source.GetSRV());

    switch (tech)
    {
    case kBicubic: return BicubicScale(Context, dest, source);
    case kSharpening: return BilinearSharpeningScale(Context, dest, source);
    case kBilinear: return BilinearScale(Context, dest, source);
    case kLanczos: return LanczosScale(Context, dest, source);
    }
}
