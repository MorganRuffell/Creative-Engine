#include "Common.hlsli"

struct ProjectionConstants
{
    float4x4 ProjInverse;
    float3x3 ViewInverse;
};

ConstantBuffer<ProjectionConstants> ProjectionConst: register(b0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 viewDir : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
VSOutput main(uint VertID : SV_VertexID)
{
    float2 ScreenUV = float2(uint2(VertID, VertID << 1) & 2);
    float4 ProjectedPos = float4(lerp(float2(-1, 1), float2(1, -1), ScreenUV), 0, 1);
    float4 PosViewSpace = mul(ProjectionConst.ProjInverse, ProjectedPos);

    VSOutput vsOutput;
    vsOutput.position = ProjectedPos;
    vsOutput.viewDir = mul(ProjectionConst.ViewInverse, PosViewSpace.xyz / PosViewSpace.w);

    return vsOutput;
}
