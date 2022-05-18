#pragma once

#include "SamplerManager.h"

class D3D12_SAMPLER_DESC1;
class CCommandSignature;
class CRootSignature;
class CComputePSO;
class CGraphicsPSO;

namespace CGraphics
{
    void InitializeCommonState(void);
    void DestroyCommonState(void);

    extern D3D12_SAMPLER_DESC1 SamplerLinearWrapDesc;
    extern D3D12_SAMPLER_DESC1 SamplerAnisoWrapDesc;
    extern D3D12_SAMPLER_DESC1 SamplerShadowDesc;
    extern D3D12_SAMPLER_DESC1 SamplerLinearClampDesc;
    extern D3D12_SAMPLER_DESC1 SamplerVolumeWrapDesc;
    extern D3D12_SAMPLER_DESC1 SamplerPointClampDesc;
    extern D3D12_SAMPLER_DESC1 SamplerPointBorderDesc;
    extern D3D12_SAMPLER_DESC1 SamplerLinearBorderDesc;

    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

    extern D3D12_RASTERIZER_DESC RasterizerDefault;
    extern D3D12_RASTERIZER_DESC RasterizerDefaultMsaa;
    extern D3D12_RASTERIZER_DESC RasterizerDefaultCw;
    extern D3D12_RASTERIZER_DESC RasterizerDefaultCwMsaa;
    extern D3D12_RASTERIZER_DESC RasterizerTwoSided;
    extern D3D12_RASTERIZER_DESC RasterizerTwoSidedMsaa;
    extern D3D12_RASTERIZER_DESC RasterizerShadow;
    extern D3D12_RASTERIZER_DESC RasterizerShadowCW;
    extern D3D12_RASTERIZER_DESC RasterizerShadowTwoSided;

    extern D3D12_BLEND_DESC BlendNoColorWrite;		// XXX
    extern D3D12_BLEND_DESC BlendDisable;			// 1, 0
    extern D3D12_BLEND_DESC BlendPreMultiplied;		// 1, 1-SrcA
    extern D3D12_BLEND_DESC BlendTraditional;		// SrcA, 1-SrcA
    extern D3D12_BLEND_DESC BlendAdditive;			// 1, 1
    extern D3D12_BLEND_DESC BlendTraditionalAdditive;// SrcA, 1

    extern D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
    extern D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
    extern D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;
    extern D3D12_DEPTH_STENCIL_DESC DepthStateReadOnlyReversed;
    extern D3D12_DEPTH_STENCIL_DESC DepthStateTestEqual;

    extern CCommandSignature DispatchIndirectCommandSignature;
    extern CCommandSignature DrawIndirectCommandSignature;

    enum EDefaultTexture
    {
        kMagenta2D,  // Useful for indicating missing textures
        kBlackOpaque2D,
        kBlackTransparent2D,
        kWhiteOpaque2D,
        kWhiteTransparent2D,
        kDefaultNormalMap,
        kBlackCubeMap,

        kNumDefaultTextures
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture( EDefaultTexture texID );

    extern CRootSignature g_CommonRS;
    extern CComputePSO g_GenerateMipsLinearPSO[4];
    extern CComputePSO g_GenerateMipsGammaPSO[4];
    extern CGraphicsPSO g_DownsampleDepthPSO;
}
