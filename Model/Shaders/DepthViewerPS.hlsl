
#include "Common.hlsli"

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TexCoord0;
};

Texture2D<float4>	texDiffuse		: register(t0);

[RootSignature(Renderer_RootSig)]
void main(VSOutput vsOutput)
{
    if (texDiffuse.Sample(defaultSampler, vsOutput.uv).a < 0.5)
        discard;
}
