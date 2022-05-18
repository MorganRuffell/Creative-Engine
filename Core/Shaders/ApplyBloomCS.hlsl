#include "ShaderUtility.hlsli"
#include "PostEffectsRS.hlsli"
#include "PixelPacking.hlsli"

Texture2D<float3> Bloom : register( t0 );
#if SUPPORT_TYPED_UAV_LOADS
RWTexture2D<float3> SrcColor : register( u0 );
#else
RWTexture2D<uint> DstColor : register(u0);
Texture2D<float3> SrcColor : register(t2);
#endif
RWTexture2D<float> OutLuma : register( u1 );
SamplerState LinearSampler : register( s0 );

cbuffer CB0 : register(b0)
{
    float2 g_RcpBufferDim;
    float g_BloomStrength;
};

[RootSignature(PostEffects_RootSig)]
[numthreads( 32, 16, 1 )]
void main( uint3 Gid : SV_GroupID, uint GlobalIllumination : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DispatchedThread : SV_DispatchThreadID )
{
    //Applies bloom across the camera lens
    float2 TexCoord = (DispatchedThread.xy + 1.0) * g_RcpBufferDim;

    // Load LDR and bloom
    float3 ldrColor = SrcColor[DispatchedThread.xy] + g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);

#if SUPPORT_TYPED_UAV_LOADS
    SrcColor[DispatchedThread.xy] = ldrColor;
#else
    DstColor[DispatchedThread.xy] = Pack_R11G11B10_FLOAT(ldrColor);
#endif
    OutLuma[DispatchedThread.xy] = RGBToLogLuminance(ldrColor);
}
