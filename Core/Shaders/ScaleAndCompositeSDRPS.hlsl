#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

struct Constants
{
    float2 UVOffset;
};

ConstantBuffer<Constants> UV : register(b0);
Texture2D<float3> MainBuffer : register(t0);
Texture2D<float4> OverlayBuffer : register(t1);

SamplerState BilinearFilter : register(s0);

float3 SampleColor(float2 uv)
{
    return MainBuffer.SampleLevel(BilinearFilter, uv, 0);
}

float3 ScaleBuffer(float2 uv)
{
    return 1.4 * SampleColor(uv) - 0.1 * (
        SampleColor(uv + float2(+UV.UVOffset.x, +UV.UVOffset.y)) +
        SampleColor(uv + float2(+UV.UVOffset.x, -UV.UVOffset.y)) +
        SampleColor(uv + float2(-UV.UVOffset.x, +UV.UVOffset.y)) +
        SampleColor(uv + float2(-UV.UVOffset.x, -UV.UVOffset.y))
        );
}

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
    float3 MainColor = ApplyDisplayProfile(ScaleBuffer(uv), DISPLAY_PLANE_FORMAT);
    float4 OverlayColor = OverlayBuffer[(int2)position.xy];
    return OverlayColor.rgb + MainColor.rgb * (1.0 - OverlayColor.a);
}
