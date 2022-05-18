#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

struct Constants 
{
    float2 UVOffset0;   // Four UV offsets to determine the sharpening tap locations
    float2 UVOffset1;   // We get two more offsets by negating these two.
    float WA, WB;       // Sharpness strength:  WB = 1.0 + Sharpness; 4*WA + WB = 1
};

Texture2D<float3> ColorTex : register(t0);
SamplerState BilinearClamp : register(s0);
ConstantBuffer<Constants> SharpeningConstants : register(b0);

float3 GetColor(float2 UV)
{
    float3 Color = ColorTex.SampleLevel(BilinearClamp, UV, 0);
#ifdef GAMMA_SPACE
    return ApplyDisplayProfile(Color, DISPLAY_PLANE_FORMAT);
#else
    return Color;
#endif
}

[RootSignature(Present_RootSig)]
float3 main(float4 position : SV_Position, float2 uv : TexCoord0) : SV_Target0
{
    float3 Color = SharpeningConstants.WB * GetColor(uv) - SharpeningConstants.WA * (
        GetColor(uv + SharpeningConstants.UVOffset0) + GetColor(uv - SharpeningConstants.UVOffset0) +
        GetColor(uv + SharpeningConstants.UVOffset1) + GetColor(uv - SharpeningConstants.UVOffset1));

#ifdef GAMMA_SPACE
    return Color;
#else
    return ApplyDisplayProfile(Color, DISPLAY_PLANE_FORMAT);
#endif
}

