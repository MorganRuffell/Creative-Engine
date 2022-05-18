
#include "pch.h"

#include "CPostEffects.h"
#include "EngineCore.h"
#include "CCommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Display.h"
#include "BufferManager.h"
#include "CMotionBlurSettings.h"
#include "CDOFSettings.h"
#include "FXAA.h"
#include "Math/Random.h"

//All of the compiled PostEffects Shaders
#include "CompiledShaders/ToneMapCS.h"
#include "CompiledShaders/ToneMap2CS.h"
#include "CompiledShaders/ToneMapHDRCS.h"
#include "CompiledShaders/ToneMapHDR2CS.h"
#include "CompiledShaders/ApplyBloomCS.h"
#include "CompiledShaders/ApplyBloom2CS.h"
#include "CompiledShaders/DebugLuminanceLdrCS.h"
#include "CompiledShaders/DebugLuminanceLdr2CS.h"
#include "CompiledShaders/GenerateHistogramCS.h"
#include "CompiledShaders/DebugDrawHistogramCS.h"
#include "CompiledShaders/AdaptExposureCS.h"
#include "CompiledShaders/DownsampleBloomCS.h"
#include "CompiledShaders/DownsampleBloomAllCS.h"
#include "CompiledShaders/UpsampleAndBlurCS.h"
#include "CompiledShaders/BlurCS.h"
#include "CompiledShaders/BloomExtractAndDownsampleHdrCS.h"
#include "CompiledShaders/BloomExtractAndDownsampleLdrCS.h"
#include "CompiledShaders/ExtractLumaCS.h"
#include "CompiledShaders/AverageLumaCS.h"
#include "CompiledShaders/CopyBackPostBufferCS.h"

using namespace CGraphics;

namespace SSAO
{
    extern CETBoolean DebugDraw;
}

namespace FXAA
{
    extern CETBoolean DebugDraw;
}

namespace CDOFSettings
{
    extern CETBoolean Enable;
    extern CETStaticEnum DebugMode;
}

namespace CGraphics
{
    extern CETFloat g_HDRPaperWhite;
    extern CETFloat g_MaxDisplayLuminance;
}

namespace CGlobalPostEffects
{
    const float kInitialMinLog = -12.0f;
    const float kInitialMaxLog = 4.0f;

    CETBoolean EnableHDR("Graphics/HDR/Enable", true);
    CETBoolean EnableAdaptation("Graphics/HDR/Adaptive Exposure", true);

    ExponentialCETFloat MinExposure("Graphics/HDR/Min Exposure", 1.0f / 64.0f, -8.0f, 0.0f, 0.25f);
    ExponentialCETFloat MaxExposure("Graphics/HDR/Max Exposure", 64.0f, 0.0f, 8.0f, 0.25f);

    CETFloat TargetLuminance("Graphics/HDR/Key", 0.08f, 0.01f, 0.99f, 0.01f);
    CETFloat AdaptationRate("Graphics/HDR/Adaptation Rate", 0.05f, 0.01f, 1.0f, 0.01f);
    ExponentialCETFloat Exposure("Graphics/HDR/Exposure", 2.0f, -8.0f, 8.0f, 0.25f);
    CETBoolean DrawHistogram("Graphics/HDR/Draw Histogram", false);

    CETBoolean BloomEnable("Graphics/Bloom/Enable", true);
    CETFloat BloomThreshold("Graphics/Bloom/Threshold", 4.0f, 0.0f, 8.0f, 0.1f);		// The threshold luminance above which a pixel will start to bloom
    CETFloat BloomStrength("Graphics/Bloom/Strength", 0.1f, 0.0f, 2.0f, 0.05f);		// A modulator controlling how much bloom is added back into the image
    CETFloat BloomUpsampleFactor("Graphics/Bloom/Scatter", 0.65f, 0.0f, 1.0f, 0.05f);	// Controls the "focus" of the blur.  High values spread out more causing a haze.
    CETBoolean HighQualityBloom("Graphics/Bloom/High Quality", true);					// High quality blurs 5 octaves of bloom; low quality only blurs 3.

    CRootSignature PostEffectsRS;
	CComputePSO ToneMapCS(L"Post Effects: Tone Map  CS");
	CComputePSO ToneMapHDRCS(L"Post Effects: Tone Map HDR CS");
	CComputePSO ApplyBloomCS(L"Post Effects: Apply Bloom CS");
	CComputePSO DebugLuminanceHdrCS(L"Post Effects: Debug Luminance HDR CS");
	CComputePSO DebugLuminanceLdrCS(L"Post Effects: Debug Luminance LDR CS");
	CComputePSO GenerateHistogramCS(L"Post Effects: Generate Histogram CS");
	CComputePSO DrawHistogramCS(L"Post Effects: Draw Histogram CS");
	CComputePSO AdaptExposureCS(L"Post Effects: Adapt Exposure CS");
	CComputePSO DownsampleBloom2CS(L"Post Effects: Downsample Bloom 2 CS");
	CComputePSO DownsampleBloom4CS(L"Post Effects: Downsample Bloom 4 CS");
	CComputePSO UpsampleAndBlurCS(L"Post Effects: Upsample and Blur CS");
	CComputePSO BlurCS(L"Post Effects: Blur CS");
	CComputePSO BloomExtractAndDownsampleHdrCS(L"Post Effects: Bloom Extract and Downsample HDR CS");
	CComputePSO BloomExtractAndDownsampleLdrCS(L"Post Effects: Bloom Extract and Downsample LDR CS");
	CComputePSO ExtractLumaCS(L"Post Effects: Extract Luma CS");
	CComputePSO AverageLumaCS(L"Post Effects: Average Luma CS");
	CComputePSO CopyBackPostBufferCS(L"Post Effects: Copy Back Post Buffer CS");

    CStructuredBuffer g_Exposure;

    void UpdateExposure(CComputeContext&);
    void BlurBuffer(CComputeContext&, CRGBBuffer buffer[2], const CRGBBuffer& lowerResBuf, float upsampleBlendFactor );
    void GenerateBloom(CComputeContext&);
    void ExtractLuma(CComputeContext&);
    void ProcessHDR(CComputeContext&);
    void ProcessLDR(CCommandContext&);
}

void CGlobalPostEffects::Initialize( void )
{
    PostEffectsRS.Reset(4, 2);
    PostEffectsRS.InitStaticSampler(0, SamplerLinearClampDesc);
    PostEffectsRS.InitStaticSampler(1, SamplerLinearBorderDesc);
    PostEffectsRS[0].InitAsConstants(0, 5);
    PostEffectsRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
    PostEffectsRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
    PostEffectsRS[3].InitAsConstantBuffer(1);
    PostEffectsRS.Finalize(L"Post Effects");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(PostEffectsRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        CreatePSO(ToneMapCS, g_pToneMap2CS);
        CreatePSO(ToneMapHDRCS, g_pToneMapHDR2CS);
        CreatePSO(ApplyBloomCS, g_pApplyBloom2CS);
        CreatePSO(DebugLuminanceLdrCS, g_pDebugLuminanceLdr2CS);
    }
    else
    {
        CreatePSO(ToneMapCS, g_pToneMapCS);
        CreatePSO(ToneMapHDRCS, g_pToneMapHDRCS);
        CreatePSO(ApplyBloomCS, g_pApplyBloomCS);
        CreatePSO(DebugLuminanceLdrCS, g_pDebugLuminanceLdrCS);
    }
    CreatePSO( GenerateHistogramCS, g_pGenerateHistogramCS );
    CreatePSO( DrawHistogramCS, g_pDebugDrawHistogramCS );
    CreatePSO( AdaptExposureCS, g_pAdaptExposureCS );
    CreatePSO( DownsampleBloom2CS, g_pDownsampleBloomCS );
    CreatePSO( DownsampleBloom4CS, g_pDownsampleBloomAllCS );
    CreatePSO( UpsampleAndBlurCS, g_pUpsampleAndBlurCS );
    CreatePSO( BlurCS, g_pBlurCS );
    CreatePSO( BloomExtractAndDownsampleHdrCS, g_pBloomExtractAndDownsampleHdrCS );
    CreatePSO( BloomExtractAndDownsampleLdrCS, g_pBloomExtractAndDownsampleLdrCS );
    CreatePSO( ExtractLumaCS, g_pExtractLumaCS );
    CreatePSO( AverageLumaCS, g_pAverageLumaCS );
    CreatePSO( CopyBackPostBufferCS, g_pCopyBackPostBufferCS );


#undef CreatePSO

    __declspec(align(16)) float initExposure[] =
    {
        Exposure, 1.0f / Exposure, Exposure, 0.0f,
        kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog)
    };
    g_Exposure.Create(L"Exposure", 8, 4, initExposure);

    FXAA::Initialize();
    CMotionBlurSettings::Initialize();
    CDOFSettings::Initialize();
}

void CGlobalPostEffects::Shutdown( void )
{
    g_Exposure.Destroy();

    FXAA::Shutdown();
    CMotionBlurSettings::Shutdown();
    CDOFSettings::Shutdown();
}

void CGlobalPostEffects::BlurBuffer( CComputeContext& Context, CRGBBuffer buffer[2], const CRGBBuffer& lowerResBuf, float upsampleBlendFactor )
{
    // Set the shader constants
    uint32_t bufferWidth = buffer[0].GetWidth();
    uint32_t bufferHeight = buffer[0].GetHeight();
    Context.SetConstants(0, 1.0f / bufferWidth, 1.0f / bufferHeight, upsampleBlendFactor);

    Context.TransitionResource( buffer[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptor(1, 0, buffer[1].GetUAV());
    D3D12_CPU_DESCRIPTOR_HANDLE SRVs[2] = { buffer[0].GetSRV(), lowerResBuf.GetSRV() };
    Context.SetDynamicDescriptors(2, 0, 2, SRVs);

    Context.SetPipelineState(&buffer[0] == &lowerResBuf ? BlurCS : UpsampleAndBlurCS);

    Context.Dispatch2D(bufferWidth, bufferHeight);

    Context.TransitionResource( buffer[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

//--------------------------------------------------------------------------------------
// Bloom effect in CS path
//--------------------------------------------------------------------------------------
void CGlobalPostEffects::GenerateBloom( CComputeContext& Context )
{
    // We can generate a bloom buffer up to 1/4 smaller in each dimension without undersampling.  If only downsizing by 1/2 or less, a faster
    // shader can be used which only does one bilinear sample.

    uint32_t kBloomWidth = g_LumaLR.GetWidth();
    uint32_t kBloomHeight = g_LumaLR.GetHeight();

    ASSERT(kBloomWidth % 16 == 0 && kBloomHeight % 16 == 0, "Bloom buffer dimensions must be multiples of 16");


    Context.SetConstants(0, 1.0f / kBloomWidth, 1.0f / kBloomHeight, (float)BloomThreshold );
    Context.TransitionResource(g_aBloomUAV1[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    {

        Context.SetDynamicDescriptor(1, 0, g_aBloomUAV1[0].GetUAV());
        Context.SetDynamicDescriptor(1, 1, g_LumaLR.GetUAV());
        Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetSRV());
        Context.SetDynamicDescriptor(2, 1, g_Exposure.GetSRV());

        Context.SetPipelineState(EnableHDR ? BloomExtractAndDownsampleHdrCS : BloomExtractAndDownsampleLdrCS);
        Context.Dispatch2D(kBloomWidth, kBloomHeight);
    }

    Context.TransitionResource(g_aBloomUAV1[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(2, 0, g_aBloomUAV1[0].GetSRV());

    // The difference between high and low quality bloom is that high quality sums 5 octaves with a 2x frequency scale, and the low quality
    // sums 3 octaves with a 4x frequency scale.
    if (HighQualityBloom)
    {
        Context.TransitionResource(g_aBloomUAV2[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV4[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Set the UAVs
        D3D12_CPU_DESCRIPTOR_HANDLE UAVs[4] = {
            g_aBloomUAV2[0].GetUAV(), g_aBloomUAV3[0].GetUAV(), g_aBloomUAV4[0].GetUAV(), g_aBloomUAV5[0].GetUAV() };
        Context.SetDynamicDescriptors(1, 0, 4, UAVs);

        // Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
        Context.SetPipelineState(DownsampleBloom4CS);
        Context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

        Context.TransitionResource(g_aBloomUAV2[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV4[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        float upsampleBlendFactor = BloomUpsampleFactor;

        // Blur then upsample and blur four times
        BlurBuffer( Context, g_aBloomUAV5, g_aBloomUAV5[0], 1.0f );
        BlurBuffer( Context, g_aBloomUAV4, g_aBloomUAV5[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV3, g_aBloomUAV4[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV2, g_aBloomUAV3[1], upsampleBlendFactor  );
        BlurBuffer( Context, g_aBloomUAV1, g_aBloomUAV2[1], upsampleBlendFactor  );
    }
    else
    {
        // Set the UAVs
        D3D12_CPU_DESCRIPTOR_HANDLE UAVs[2] = { g_aBloomUAV3[0].GetUAV(), g_aBloomUAV5[0].GetUAV() };
        Context.SetDynamicDescriptors(1, 0, 2, UAVs);

        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
        Context.SetPipelineState(DownsampleBloom2CS);
        Context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

        Context.TransitionResource(g_aBloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_aBloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        float upsampleBlendFactor = BloomUpsampleFactor * 2.0f / 3.0f;

        // Blur then upsample and blur two times
        BlurBuffer( Context, g_aBloomUAV5, g_aBloomUAV5[0], 1.0f );
        BlurBuffer( Context, g_aBloomUAV3, g_aBloomUAV5[1], upsampleBlendFactor );
        BlurBuffer( Context, g_aBloomUAV1, g_aBloomUAV3[1], upsampleBlendFactor );
    }
}

void CGlobalPostEffects::ExtractLuma( CComputeContext& Context )
{

    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetConstants(0, 1.0f / g_LumaLR.GetWidth(), 1.0f / g_LumaLR.GetHeight());
    Context.SetDynamicDescriptor(1, 0, g_LumaLR.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetSRV());
    Context.SetDynamicDescriptor(2, 1, g_Exposure.GetSRV());
    Context.SetPipelineState(ExtractLumaCS);
    Context.Dispatch2D(g_LumaLR.GetWidth(), g_LumaLR.GetHeight());
}

void CGlobalPostEffects::UpdateExposure( CComputeContext& Context )
{

    if (!EnableAdaptation)
    {
        __declspec(align(16)) float initExposure[] =
        {
            Exposure, 1.0f / Exposure, Exposure, 0.0f,
            kInitialMinLog, kInitialMaxLog, kInitialMaxLog - kInitialMinLog, 1.0f / (kInitialMaxLog - kInitialMinLog)
        };
        Context.WriteBuffer(g_Exposure, 0, initExposure, sizeof(initExposure));
        Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        return;
    }
    
    // Generate an HDR histogram
    Context.TransitionResource(g_Histogram, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
    Context.ClearUAV(g_Histogram);
    Context.TransitionResource(g_LumaLR, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(1, 0, g_Histogram.GetUAV() );
    Context.SetDynamicDescriptor(2, 0, g_LumaLR.GetSRV() );
    Context.SetPipelineState(GenerateHistogramCS);
    Context.SetConstants(0, g_LumaLR.GetHeight());
    Context.Dispatch2D(g_LumaLR.GetWidth(), g_LumaLR.GetHeight(), 16, g_LumaLR.GetHeight());

    __declspec(align(16)) struct
    {
        float TargetLuminance;
        float AdaptationRate;
        float MinExposure;
        float MaxExposure;
        uint32_t PixelCount; 
    } constants =
    {
        TargetLuminance, AdaptationRate, MinExposure, MaxExposure,
        g_LumaLR.GetWidth() * g_LumaLR.GetHeight()
    };
    Context.TransitionResource(g_Histogram, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.SetDynamicDescriptor(1, 0, g_Exposure.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_Histogram.GetSRV());
    Context.SetDynamicConstantBufferView(3, sizeof(constants), &constants);
    Context.SetPipelineState(AdaptExposureCS);
    Context.Dispatch();
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void CGlobalPostEffects::ProcessHDR( CComputeContext& Context )
{

    if (BloomEnable)
    {
        GenerateBloom(Context);
        Context.TransitionResource(g_aBloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    else if (EnableAdaptation)
        ExtractLuma(Context);

    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    else
        Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_Exposure, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    Context.SetPipelineState(FXAA::DebugDraw ? DebugLuminanceHdrCS : (g_bEnableHDROutput ? ToneMapHDRCS : ToneMapCS));

    // Set constants
    Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(),
        (float)BloomStrength);
    Context.SetConstant(0, 3, (float)g_HDRPaperWhite / (float)g_MaxDisplayLuminance);
    Context.SetConstant(0, 4, (float)g_MaxDisplayLuminance);

    // Separate out SDR result from its perceived luminance
    if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
    else
    {
        Context.SetDynamicDescriptor(1, 0, g_PostEffectsBuffer.GetUAV());
        Context.SetDynamicDescriptor(2, 2, g_SceneColorBuffer.GetSRV());
    }
    Context.SetDynamicDescriptor(1, 1, g_LumaBuffer.GetUAV());

    // Read in original HDR value and blurred bloom buffer
    Context.SetDynamicDescriptor(2, 0, g_Exposure.GetSRV());
    Context.SetDynamicDescriptor(2, 1, BloomEnable ? g_aBloomUAV1[1].GetSRV() : GetDefaultTexture(kBlackOpaque2D));
    
    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Do this last so that the bright pass uses the same exposure as tone mapping
    UpdateExposure(Context);
}

void CGlobalPostEffects::ProcessLDR(CCommandContext& BaseContext)
{

    CComputeContext& Context = BaseContext.GetCComputeContext();

    bool bGenerateBloom = BloomEnable && !SSAO::DebugDraw;
    if (bGenerateBloom)
        GenerateBloom(Context);

    if (bGenerateBloom || FXAA::DebugDraw || SSAO::DebugDraw || !g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
    {
        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
            Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        else
            Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        Context.TransitionResource(g_aBloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Set constants
        Context.SetConstants(0, 1.0f / g_SceneColorBuffer.GetWidth(), 1.0f / g_SceneColorBuffer.GetHeight(),
            (float)BloomStrength);

        // Separate out SDR result from its perceived luminance
        if (g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
            Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
        else
        {
            Context.SetDynamicDescriptor(1, 0, g_PostEffectsBuffer.GetUAV());
            Context.SetDynamicDescriptor(2, 2, g_SceneColorBuffer.GetSRV());
        }
        Context.SetDynamicDescriptor(1, 1, g_LumaBuffer.GetUAV());

        // Read in original SDR value and blurred bloom buffer
        Context.SetDynamicDescriptor(2, 0, bGenerateBloom ? g_aBloomUAV1[1].GetSRV() : GetDefaultTexture(kBlackOpaque2D));

        Context.SetPipelineState(FXAA::DebugDraw ? DebugLuminanceLdrCS : ApplyBloomCS);
        Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

        Context.TransitionResource(g_LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
}

void CGlobalPostEffects::CopyBackPostBuffer( CComputeContext& Context )
{
    Context.SetRootSignature(PostEffectsRS);
    Context.SetPipelineState(CopyBackPostBufferCS);
    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(g_PostEffectsBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetUAV());
    Context.SetDynamicDescriptor(2, 0, g_PostEffectsBuffer.GetSRV());
    Context.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
}

void CGlobalPostEffects::Render( void )
{
    CComputeContext& Context = CComputeContext::Begin(L"Post Effects");

    Context.SetRootSignature(PostEffectsRS);

    Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    if (EnableHDR && !SSAO::DebugDraw && !(CDOFSettings::Enable && CDOFSettings::DebugMode >= 3))
        ProcessHDR(Context);
    else
        ProcessLDR(Context);

    bool bGeneratedLumaBuffer = EnableHDR || FXAA::DebugDraw || BloomEnable;
    if (FXAA::Enable)
        FXAA::Render(Context, bGeneratedLumaBuffer);

    if (!g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
        CopyBackPostBuffer(Context);


    Context.Finish();
}
