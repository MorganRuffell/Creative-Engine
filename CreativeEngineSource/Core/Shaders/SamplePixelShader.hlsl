
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 Normal : NORMAL;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(input.Normal, 1.0f);
} 
