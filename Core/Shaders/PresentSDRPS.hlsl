#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position ) : SV_Target0
{
    float3 LinearRGB = ColorTex[(int2)position.xy];
    return ApplyDisplayProfile(LinearRGB, DISPLAY_PLANE_FORMAT);
}
