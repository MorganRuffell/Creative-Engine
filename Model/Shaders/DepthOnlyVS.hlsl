#include "Common.hlsli"

#ifdef ENABLE_SKINNING
//#undef ENABLE_SKINNING
#endif

cbuffer MeshConstants : register(b0)
{
    float4x4 WorldMatrix;   // Object to world
    float3x3 WorldIT;       // Object normal to world normal
};

cbuffer GlobalConstants : register(b1)
{
    float4x4 ViewProjMatrix;
}

#ifdef ENABLE_SKINNING
struct Joint
{
    float4x4 PosMatrix;
    float4x3 NrmMatrix; // Inverse-transpose of PosMatrix
};

StructuredBuffer<Joint> Joints : register(t20);
#endif

struct VSInput
{
    float3 position : POSITION;
#ifdef ENABLE_ALPHATEST
    float2 uv0 : TEXCOORD0;
#endif
#ifdef ENABLE_SKINNING
    uint4 jointIndices : BLENDINDICES;
    float4 jointWeights : BLENDWEIGHT;
#endif
};

struct VSOutput
{
    float4 position : SV_POSITION;
#ifdef ENABLE_ALPHATEST
    float2 uv0 : TEXCOORD0;
#endif
};

[RootSignature(Renderer_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    float4 position = 0;
    float4 weights = 0;
    float4x4 skinPosMat;

    if (WaveIsFirstLane())
    {
#ifdef ENABLE_SKINNING

        position = float4(vsInput.position, 1.0);
        weights = vsInput.jointWeights / dot(vsInput.jointWeights, 1);

        skinPosMat =
            Joints[vsInput.jointIndices.x].PosMatrix * weights.x +
            Joints[vsInput.jointIndices.y].PosMatrix * weights.y +
            Joints[vsInput.jointIndices.z].PosMatrix * weights.z +
            Joints[vsInput.jointIndices.w].PosMatrix * weights.w;

        position = mul(skinPosMat, position);
#endif
    }

    float3 worldPos = mul(WorldMatrix, position).xyz;
    vsOutput.position = mul(ViewProjMatrix, float4(worldPos, 1.0));

#ifdef ENABLE_ALPHATEST
    vsOutput.uv0 = vsInput.uv0;
#endif

    return vsOutput;
}
