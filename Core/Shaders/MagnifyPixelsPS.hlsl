#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

struct Constants
{
    float ScaleFactor;
};

Texture2D<float3> ColorTex : register(t0);
SamplerState PointSampler : register(s1);
ConstantBuffer<Constants> MagnifyConstantValues : register(b0);

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
    float2 ScaledUV = MagnifyConstantValues.ScaleFactor * (uv - 0.5) + 0.5;
    return ColorTex.SampleLevel(PointSampler, ScaledUV, 0);
}
