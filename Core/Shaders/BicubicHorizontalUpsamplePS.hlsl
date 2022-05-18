//--------------------------------------------------------------------------------------
// Simple bicubic filter
//
// http://en.wikipedia.org/wiki/Bicubic_interpolation
// http://http.developer.nvidia.com/GPUGems/gpugems_ch24.html
//
//--------------------------------------------------------------------------------------

#include "BicubicFilterFunctions.hlsli"
#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

struct Constants
{
    uint2 TextureSize;
    float kA;
};

ConstantBuffer<Constants> SampleConstants : register(b0);
Texture2D<float3> ColorTex : register(t0);


float3 GetColor(uint s, uint t)
{
#ifdef GAMMA_SPACE
    return ApplyDisplayProfile(ColorTex[uint2(s, t)], DISPLAY_PLANE_FORMAT);
#else
    return ColorTex[uint2(s, t)];
#endif
}

[RootSignature(Present_RootSig)]
float3 main(float4 position : SV_Position, float2 uv : TexCoord0) : SV_Target0
{
    float2 TextureSize, Fraction;
    int2 sizeCast;

    if (WaveIsFirstLane())
    {
        TextureSize = uv * SampleConstants.TextureSize + 0.5;
        Fraction = frac(TextureSize);
        sizeCast = int2(TextureSize.x, position.y);
    }

    uint MaxWidth = SampleConstants.TextureSize.x - 1;

    uint s0 = max(sizeCast.x - 2, 0);
    uint s1 = max(sizeCast.x - 1, 0);
    uint s2 = min(sizeCast.x + 0, MaxWidth);
    uint s3 = min(sizeCast.x + 1, MaxWidth);

    float4 W = GetBicubicFilterWeights(Fraction.x, SampleConstants.kA);
    float3 Color = 
        W.x * GetColor(s0, sizeCast.y) + 
        W.y * GetColor(s1, sizeCast.y) + 
        W.z * GetColor(s2, sizeCast.y) +
        W.w * GetColor(s3, sizeCast.y);

    return Color;
}
