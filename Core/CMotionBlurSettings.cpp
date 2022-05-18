

#include "pch.h"
#include "CMotionBlurSettings.h"
#include "CViewportCamera.h"
#include "BufferManager.h"
#include "CDirectX12Core.h"
#include "GraphicsCommon.h"
#include "CCommandContext.h"
#include "CViewportCamera.h"
#include "CTAAEffects.h"
#include "CPostEffects.h"
#include "CInternalWindowsTime.h"

#include "CompiledShaders/ScreenQuadCommonVS.h"
#include "CompiledShaders/CameraMotionBlurPrePassCS.h"
#include "CompiledShaders/CameraMotionBlurPrePassLinearZCS.h"
#include "CompiledShaders/MotionBlurPrePassCS.h"
#include "CompiledShaders/MotionBlurFinalPassCS.h"
#include "CompiledShaders/MotionBlurFinalPassPS.h"
#include "CompiledShaders/CameraVelocityCS.h"
#include "CompiledShaders/TemporalBlendCS.h"
#include "CompiledShaders/BoundNeighborhoodCS.h"

using namespace CGraphics;
using namespace CMath;

namespace CMotionBlurSettings
{
    CETBoolean Enable("Graphics/Motion Blur/Enable", false);

    CComputePSO s_CameraMotionBlurPrePassCS[2] = { {L"Motion Blur: Camera Motion Blur Pre-Pass CS"}, { L"Motion Blur: Camera Motion Blur Pre-Pass Linear Z CS" } };
    CComputePSO s_MotionBlurPrePassCS(L"Motion Blur: Motion Blur Pre-Pass CS");
    CComputePSO s_MotionBlurFinalPassCS(L"Motion Blur: Motion Blur Final Pass CS");
    CGraphicsPSO s_MotionBlurFinalPassPS(L"Motion Blur: Motion Blur Final Pass PS");
    CComputePSO s_CameraVelocityCS[2] = { { L"Motion Blur: Camera Velocity CS" },{ L"Motion Blur: Camera Velocity Linear Z CS" } };
}

void CMotionBlurSettings::Initialize( void )
{
#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(g_CommonRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(s_MotionBlurFinalPassCS, g_pMotionBlurFinalPassCS);
    }
    else
    {
        s_MotionBlurFinalPassPS.SetRootSignature(g_CommonRS);
        s_MotionBlurFinalPassPS.SetRasterizerState( RasterizerTwoSided );
        s_MotionBlurFinalPassPS.SetBlendState( BlendPreMultiplied );
        s_MotionBlurFinalPassPS.SetDepthStencilState( DepthStateDisabled );
        s_MotionBlurFinalPassPS.SetSampleMask(0xFFFFFFFF);
        s_MotionBlurFinalPassPS.SetInputLayout(0, nullptr);
        s_MotionBlurFinalPassPS.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        s_MotionBlurFinalPassPS.SetVertexShader( g_pScreenQuadCommonVS, sizeof(g_pScreenQuadCommonVS) );
        s_MotionBlurFinalPassPS.SetPixelShader( g_pMotionBlurFinalPassPS, sizeof(g_pMotionBlurFinalPassPS) );
        s_MotionBlurFinalPassPS.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
        s_MotionBlurFinalPassPS.Finalize();

    }
    CreatePSO( s_CameraMotionBlurPrePassCS[0], g_pCameraMotionBlurPrePassCS );
    CreatePSO( s_CameraMotionBlurPrePassCS[1], g_pCameraMotionBlurPrePassLinearZCS );
    CreatePSO( s_MotionBlurPrePassCS, g_pMotionBlurPrePassCS );
    CreatePSO( s_CameraVelocityCS[0], g_pCameraVelocityCS );
    CreatePSO( s_CameraVelocityCS[1], g_pCameraVelocityCS );

#undef CreatePSO
}

void CMotionBlurSettings::Shutdown( void )
{
}

// Linear Z ends up being faster since we haven't officially decompressed the depth buffer.  You 
// would think that it might be slower to use linear Z because we have to convert it back to
// hyperbolic Z for the reprojection.  Nevertheless, the reduced bandwidth and decompress eliminate
// make Linear Z the better choice.  (The choice also lets you evict the depth buffer from ESRAM.)

void CMotionBlurSettings::GenerateCameraVelocityBuffer( CCommandContext& BaseContext, const CViewportCamera& camera, bool UseLinearZ )
{
    GenerateCameraVelocityBuffer(BaseContext, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), UseLinearZ);
}

void CMotionBlurSettings::GenerateCameraVelocityBuffer( CCommandContext& BaseContext, const CMatrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ)
{
    CComputeContext& Context = BaseContext.GetCComputeContext();

    Context.SetRootSignature(g_CommonRS);

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    float RcpHalfDimX = 2.0f / Width;
    float RcpHalfDimY = 2.0f / Height;
    float RcpZMagic = nearClip / (farClip - nearClip);

    CMatrix4 preMult = CMatrix4(
        CVector4( RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        CVector4( 0.0f, -RcpHalfDimY, 0.0f, 0.0f),
        CVector4( 0.0f, 0.0f, UseLinearZ ? RcpZMagic : 1.0f, 0.0f ),
        CVector4( -1.0f, 1.0f, UseLinearZ ? -RcpZMagic : 0.0f, 1.0f )
    );

    CMatrix4 postMult = CMatrix4(
        CVector4( 1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        CVector4( 0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f ),
        CVector4( 0.0f, 0.0f, 1.0f, 0.0f ),
        CVector4( 1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f ) );


    CMatrix4 CurToPrevXForm = postMult * reprojectionMatrix * preMult;

    Context.SetDynamicConstantBufferView(3, sizeof(CurToPrevXForm), &CurToPrevXForm);
    Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    CRGBBuffer& LinearDepth = g_LinearDepth[ CTAAEffects::GetFrameIndexMod2() ];
    if (UseLinearZ)
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    else
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetPipelineState(s_CameraVelocityCS[UseLinearZ ? 1 : 0]);
    Context.SetDynamicDescriptor(1, 0, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
    Context.SetDynamicDescriptor(2, 0, g_VelocityBuffer.GetUAV());
    Context.Dispatch2D(Width, Height);
}


void CMotionBlurSettings::RenderCameraBlur( CCommandContext& BaseContext, const CViewportCamera& camera, bool UseLinearZ )
{
    RenderCameraBlur(BaseContext, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), UseLinearZ);
}

void CMotionBlurSettings::RenderCameraBlur( CCommandContext& BaseContext, const CMatrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ)
{

    if (!Enable)
        return;

    CComputeContext& Context = BaseContext.GetCComputeContext();

    Context.SetRootSignature(g_CommonRS);

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    float RcpHalfDimX = 2.0f / Width;
    float RcpHalfDimY = 2.0f / Height;
    float RcpZMagic = nearClip / (farClip - nearClip);

    CMatrix4 preMult = CMatrix4(
        CVector4( RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        CVector4( 0.0f, -RcpHalfDimY, 0.0f, 0.0f),
        CVector4( 0.0f, 0.0f, UseLinearZ ? RcpZMagic : 1.0f, 0.0f ),
        CVector4( -1.0f, 1.0f, UseLinearZ ? -RcpZMagic : 0.0f, 1.0f )
    );

    CMatrix4 postMult = CMatrix4(
        CVector4( 1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f ),
        CVector4( 0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f ),
        CVector4( 0.0f, 0.0f, 1.0f, 0.0f ),
        CVector4( 1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f ) );

    CMatrix4 CurToPrevXForm = postMult * reprojectionMatrix * preMult;

    Context.SetDynamicConstantBufferView(3, sizeof(CurToPrevXForm), &CurToPrevXForm);

    CRGBBuffer& LinearDepth = g_LinearDepth[ CTAAEffects::GetFrameIndexMod2() ];
    if (UseLinearZ)
        Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    else
        Context.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    if (Enable)
    {
        Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.SetPipelineState(s_CameraMotionBlurPrePassCS[UseLinearZ ? 1 : 0]);
        Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetSRV());
        Context.SetDynamicDescriptor(1, 1, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
        Context.SetDynamicDescriptor(2, 0, g_MotionPrepBuffer.GetUAV());
        Context.SetDynamicDescriptor(2, 1, g_VelocityBuffer.GetUAV());
        Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        {
            Context.SetPipelineState(s_MotionBlurFinalPassCS);
            Context.SetConstants(0, 1.0f / Width, 1.0f / Height);

            Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            Context.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
            Context.SetDynamicDescriptor(1, 0, g_VelocityBuffer.GetSRV());
            Context.SetDynamicDescriptor(1, 1, g_MotionPrepBuffer.GetSRV());

            Context.Dispatch2D(Width, Height);

            Context.InsertUAVBarrier(g_SceneColorBuffer);
        }
        else
        {
            CGraphicsContext& GrContext = BaseContext.GetGraphicsContext();
            GrContext.SetRootSignature(g_CommonRS);
            GrContext.SetPipelineState(s_MotionBlurFinalPassPS);
            GrContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
            GrContext.TransitionResource(g_VelocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            GrContext.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            GrContext.SetDynamicDescriptor(1, 0, g_VelocityBuffer.GetSRV());
            GrContext.SetDynamicDescriptor(1, 1, g_MotionPrepBuffer.GetSRV());
            GrContext.SetConstants(0, 1.0f / Width, 1.0f / Height);
            GrContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
            GrContext.SetViewportAndScissor(0, 0, Width, Height);
            GrContext.Draw(3);
        }
    }
    else
    {
        Context.SetPipelineState(s_CameraVelocityCS[UseLinearZ ? 1 : 0]);
        Context.SetDynamicDescriptor(1, 0, UseLinearZ ? LinearDepth.GetSRV() : g_SceneDepthBuffer.GetDepthSRV());
        Context.SetDynamicDescriptor(2, 0, g_VelocityBuffer.GetUAV());
        Context.Dispatch2D(Width, Height);
    }
}

void CMotionBlurSettings::RenderObjectBlur( CCommandContext& BaseContext, CRGBBuffer& velocityBuffer )
{

    if (!Enable)
        return;

    uint32_t Width = g_SceneColorBuffer.GetWidth();
    uint32_t Height = g_SceneColorBuffer.GetHeight();

    CComputeContext& Context = BaseContext.GetCComputeContext();

    Context.SetRootSignature(g_CommonRS);

    Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetDynamicDescriptor(2, 0, g_MotionPrepBuffer.GetUAV());
    Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, velocityBuffer.GetSRV());

    Context.SetPipelineState(s_MotionBlurPrePassCS);
    Context.Dispatch2D(g_MotionPrepBuffer.GetWidth(), g_MotionPrepBuffer.GetHeight());

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        Context.SetPipelineState(s_MotionBlurFinalPassCS);

        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
        Context.SetDynamicDescriptor(1, 0, velocityBuffer.GetSRV());
        Context.SetDynamicDescriptor(1, 1, g_MotionPrepBuffer.GetSRV());
        Context.SetConstants(0, 1.0f / Width, 1.0f / Height);

        Context.Dispatch2D(Width, Height);

        Context.InsertUAVBarrier(g_SceneColorBuffer);
    }
    else
    {
        CGraphicsContext& GrContext = BaseContext.GetGraphicsContext();
        GrContext.SetRootSignature(g_CommonRS);
        GrContext.SetPipelineState(s_MotionBlurFinalPassPS);

        GrContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        GrContext.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GrContext.TransitionResource(g_MotionPrepBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        GrContext.SetDynamicDescriptor(1, 0, velocityBuffer.GetSRV());
        GrContext.SetDynamicDescriptor(1, 1, g_MotionPrepBuffer.GetSRV());
        GrContext.SetConstants(0, 1.0f / Width, 1.0f / Height);
        GrContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
        GrContext.SetViewportAndScissor(0, 0, Width, Height);

        GrContext.Draw(3);
    }
}

