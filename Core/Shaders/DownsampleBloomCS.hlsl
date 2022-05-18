#include "PostEffectsRS.hlsli"

Texture2D<float3> BloomBuf : register( t0 );
RWTexture2D<float3> Result1 : register( u0 );
RWTexture2D<float3> Result2 : register( u1 );
SamplerState BiLinearClamp : register( s0 );

cbuffer cb0 : register(b0)
{
    float2 g_inverseDimensions;
}

groupshared float3 g_Tile[64];    // 8x8 input pixels

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint GI : SV_GroupIndex, uint3 Did : SV_DispatchThreadID )
{
    // Check for power of 2.
    uint parity = Did.x | Did.y;

    // Store the first downsampled quad per thread
    float2 centerUV = (float2(Did.xy) * 2.0f + 1.0f) * g_inverseDimensions;
    float3 avgPixel = BloomBuf.SampleLevel(BiLinearClamp, centerUV, 0.0f);
    g_Tile[GI] = avgPixel;

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 1) == 0)
    {
        avgPixel = 0.25f * (avgPixel + g_Tile[GI+1] + g_Tile[GI+8] + g_Tile[GI+9]);
        g_Tile[GI] = avgPixel;
        Result1[Did.xy >> 1] = avgPixel;
    }

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 3) == 0)
    {
        avgPixel = avgPixel + g_Tile[GI+2] + g_Tile[GI+16] + g_Tile[GI+18];
        g_Tile[GI] = avgPixel;
    }

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 7) == 0)
    {
        avgPixel = 0.0625f * (avgPixel + g_Tile[GI+4] + g_Tile[GI+32] + g_Tile[GI+36]);
        Result2[Did.xy >> 3] = avgPixel;
    }
}
