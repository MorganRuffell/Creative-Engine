#include "Common.hlsli"

cbuffer VSConstants : register(b0)
{
	float4x4 modelToProjection;
	float4x4 modelToShadow;
	float3 ViewerPos;
};

cbuffer StartVertex : register(b1)
{
	uint materialIdx;
};

struct VSInput
{
	float3 position : POSITION;
	float2 texcoord0 : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 worldPos : WorldPos;
	float2 texCoord : TexCoord0;
	float3 viewDir : TexCoord1;
	float3 shadowCoord : TexCoord2;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 bitangent : Bitangent;
#if ENABLE_TRIANGLE_ID
	uint vertexID : TexCoord3;
#endif
};

[RootSignature(Renderer_RootSig)]
VSOutput main(VSInput vsInput, uint vertexID : SV_VertexID)
{
	//SM6 Waves and data
	uint LaneCount = WaveGetLaneCount();

	VSOutput vsOutput;


	vsOutput.position = mul(modelToProjection, float4(vsInput.position, 1.0));
	vsOutput.viewDir = vsInput.position - ViewerPos;

	vsOutput.worldPos = vsInput.position;
	vsOutput.shadowCoord = mul(modelToShadow, float4(vsInput.position, 1.0)).xyz;
	vsOutput.texCoord = vsInput.texcoord0;
	vsOutput.normal = vsInput.normal;
	vsOutput.tangent = vsInput.tangent;
	vsOutput.bitangent = vsInput.bitangent;


#if ENABLE_TRIANGLE_ID
	vsOutput.vertexID = materialIdx << 24 | (vertexID & 0xFFFF);
#endif


	return vsOutput;
}
