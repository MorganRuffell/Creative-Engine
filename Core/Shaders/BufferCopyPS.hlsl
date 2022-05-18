#include "PresentRS.hlsli"

Texture2D ColorTex : register(t0);
SamplerState BilinearSampler : register(s0);

struct Constants
{
    float2 RcpDestDim;
};

ConstantBuffer<Constants> CopyPSConsts : register(b0);

[RootSignature(Present_RootSig)]
float4 main( float4 position : SV_Position ) : SV_Target0
{
    //float2 UV = CopyPSConsts.saturate(RcpDestDim * position.xy);
    //return ColorTex.SampleLevel(BilinearSampler, UV, 0);
    return ColorTex[(int2)position.xy];
}
