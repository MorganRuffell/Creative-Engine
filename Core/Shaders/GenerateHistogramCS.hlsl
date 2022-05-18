#include "PostEffectsRS.hlsli"

Texture2D<uint> LumaBuf : register( t0 );
RWByteAddressBuffer Histogram : register( u0 );

groupshared uint g_TileHistogram[256];

cbuffer CB0 : register(b0)
{
    uint kBufferHeight;
}

[RootSignature(PostEffects_RootSig)]
[numthreads( 16, 16, 1 )]
void main( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
    g_TileHistogram[GI] = 0;

    GroupMemoryBarrierWithGroupSync();

    for (uint2 ST = DTid.xy; ST.y < kBufferHeight; ST.y += 16)
    {
        uint QuantizedLogLuma = LumaBuf[ST];
        InterlockedAdd( g_TileHistogram[QuantizedLogLuma], 1 );
    }

    GroupMemoryBarrierWithGroupSync();

    Histogram.InterlockedAdd( GI * 4, g_TileHistogram[GI] );
}
