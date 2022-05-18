#include "CommonRS.hlsli"
#include "PixelPacking_Velocity.hlsli"


#define USE_LINEAR_Z

Texture2D<float> DepthBuffer : register(t0);
RWTexture2D<packed_velocity_t> VelocityBuffer : register(u0);

cbuffer CBuffer : register(b1)
{
    matrix CurToPrevXForm;
}

[RootSignature(Common_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 st = DTid.xy;
    float2 CurPixel = st + 0.5;
    float Depth = DepthBuffer[st];
#ifdef USE_LINEAR_Z
    float4 HPos = float4( CurPixel * Depth, 1.0, Depth );
#else
    float4 HPos = float4( CurPixel, Depth, 1.0 );
#endif
    float4 PrevHPos = mul( CurToPrevXForm, HPos );

    PrevHPos.xyz /= PrevHPos.w;

#ifdef USE_LINEAR_Z
    PrevHPos.z = PrevHPos.w;
#endif

    VelocityBuffer[st] = PackVelocity(PrevHPos.xyz - float3(CurPixel, Depth));
}
