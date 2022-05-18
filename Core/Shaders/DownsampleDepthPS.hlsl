#include "CommonRS.hlsli"

Texture2DMS<float> DepthBuffer : register(t0);

[RootSignature(Common_RootSig)]
float main( float4 position : SV_Position ) : SV_Depth
{
    return DepthBuffer.Load((int2)position.xy, 0);
}
