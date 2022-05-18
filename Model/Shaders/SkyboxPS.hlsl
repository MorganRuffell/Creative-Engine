#include "Common.hlsli"

struct PixelShaderConstants 
{
    float TextureLevel;
};

ConstantBuffer<PixelShaderConstants> PSConstants : register (b0);
TextureCube<float3> radianceIBLTexture      : register(t10);

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 viewDir : TEXCOORD3;
};

[RootSignature(Renderer_RootSig)]
float4 main(VSOutput vsOutput) : SV_Target0
{
    return float4(radianceIBLTexture.SampleLevel(defaultSampler, vsOutput.viewDir, PSConstants.TextureLevel), 1);
}
