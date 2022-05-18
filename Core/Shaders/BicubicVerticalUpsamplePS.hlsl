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

ConstantBuffer<Constants> UpsampleConstants : register(b0);
Texture2D<float3> ColorTex : register(t0);

float3 GetColor(uint s, uint t)
{
    return ColorTex[uint2(s, t)];
}

[RootSignature(Present_RootSig)]
float3 main(float4 position : SV_Position, float2 uv : TexCoord0) : SV_Target0
{
    float2 TextureSize, Fraction;
    int2 sizeCast;

    if (WaveIsFirstLane())
    {
        TextureSize = uv * UpsampleConstants.TextureSize + 0.5;
        Fraction = frac(TextureSize);
        sizeCast = int2(position.x, TextureSize.y);
    }

    uint MaxHeight = UpsampleConstants.TextureSize.y - 1;

    uint t0 = max(sizeCast.y - 2, 0);
    uint t1 = max(sizeCast.y - 1, 0);
    uint t2 = min(sizeCast.y + 0, MaxHeight);
    uint t3 = min(sizeCast.y + 1, MaxHeight);

    float4 W = GetBicubicFilterWeights(Fraction.y, UpsampleConstants.kA);
    float3 Color =
        W.x * GetColor(sizeCast.x, t0) +
        W.y * GetColor(sizeCast.x, t1) +
        W.z * GetColor(sizeCast.x, t2) +
        W.w * GetColor(sizeCast.x, t3);

#ifdef GAMMA_SPACE
        return Color;
#else
        return ApplyDisplayProfile(Color, DISPLAY_PLANE_FORMAT);
#endif
}
