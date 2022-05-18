
#include "DoFCommon.hlsli"

StructuredBuffer<uint> WorkQueue : register(t5);
RWTexture2D<float3> DstColor : register(u0);

[RootSignature(DoF_RootSig)]
[numthreads( 1, 16, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID )
{
    uint TileCoord = WorkQueue[Gid.x];
    uint2 Tile = uint2(TileCoord & 0xFFFF, TileCoord >> 16);
    uint2 st = Tile * 16 + GTid.xy;

    DstColor[st] = float3(1, 0, 0);
}
