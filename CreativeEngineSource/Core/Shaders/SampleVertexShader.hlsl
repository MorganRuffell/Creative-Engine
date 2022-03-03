cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer ConstantBuffer : register(b1)
{
	float4x4 wvpMat;
}

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 Normal : NORMAL;
	//float2 texCoord: TEXCOORD;

};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
	float3 Normal : NORMAL;
	//float2 texCoord: TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	output.pos = pos;
	output.color = input.color;
	output.color = input.color;
	output.Normal = input.Normal;

	return output;
}
