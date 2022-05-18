//NVidia Corporation -- Slight modifications by Morgan Ruffell

//--------------------------------------------------------------------------------------
// Simple bicubic filter
//
// http://en.wikipedia.org/wiki/Bicubic_interpolation
// http://http.developer.nvidia.com/GPUGems/gpugems_ch24.html
//
//--------------------------------------------------------------------------------------

#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"


struct Constants
{
    uint2 TextureSize;
    float A;
};

ConstantBuffer<Constants> ConstantParams : register(b0);
Texture2D<float3> ColorTex : register(t0);
SamplerState BilinearClamp : register(s0);

float W1(float x)
{
    return x * x * ((ConstantParams.A + 2) * x - (ConstantParams.A + 3)) + 1.0;
}

float W2(float x)
{
    return ConstantParams.A * (x * (x * (x - 5) + 8) - 4);
}

float4 GetWeights(float d1)
{
    return float4(W2(1.0 + d1), W1(d1), W1(1.0 - d1), W2(2.0 - d1));
}

float3 Cubic(float4 w, float3 c0, float3 c1, float3 c2, float3 c3)
{
    return c0 * w.x + c1 * w.y + c2 * w.z + c3 * w.w;
}

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
        TextureSize = uv * ConstantParams.TextureSize + 0.5;
        Fraction = frac(TextureSize);
        sizeCast = int2(TextureSize);

    }

    uint MaxWidth = ConstantParams.TextureSize.x - 1;
    uint MaxHeight = ConstantParams.TextureSize.y - 1;

    uint s0 = max(sizeCast.x - 2, 0);
    uint s1 = max(sizeCast.x - 1, 0);
    uint s2 = min(sizeCast.x + 0, MaxWidth);
    uint s3 = min(sizeCast.x + 1, MaxWidth);

    uint t0 = max(sizeCast.y - 2, 0);
    uint t1 = max(sizeCast.y - 1, 0);
    uint t2 = min(sizeCast.y + 0, MaxHeight);
    uint t3 = min(sizeCast.y + 1, MaxHeight);

    float4 Weights = GetWeights(Fraction.x);
    float3 c0 = Cubic(Weights, GetColor(s0, t0), GetColor(s1, t0), GetColor(s2, t0), GetColor(s3, t0));
    float3 c1 = Cubic(Weights, GetColor(s0, t1), GetColor(s1, t1), GetColor(s2, t1), GetColor(s3, t1));
    float3 c2 = Cubic(Weights, GetColor(s0, t2), GetColor(s1, t2), GetColor(s2, t2), GetColor(s3, t2));
    float3 c3 = Cubic(Weights, GetColor(s0, t3), GetColor(s1, t3), GetColor(s2, t3), GetColor(s3, t3));
    float3 Color = Cubic(GetWeights(Fraction.y), c0, c1, c2, c3);

#ifdef GAMMA_SPACE
    return Color;
#else
    return ApplyDisplayProfile(Color, DISPLAY_PLANE_FORMAT);
#endif
}
