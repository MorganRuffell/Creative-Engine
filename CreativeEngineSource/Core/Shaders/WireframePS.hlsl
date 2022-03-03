struct WireParameters
{
    float4 meshColor;
    float4 edgeColor;
    float2 windowSize;
};

ConstantBuffer<WireParamaters> WireframeValues : register(c1);

struct PS_INPUT
{
    float4 vertexColor : COLOR0;
    noperspective float3 edgeDistance : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float dmin = min(input.edgeDistance[0], input.edgeDistance[1]);
    dmin = min(dmin, input.edgeDistance[2]);
    float blend = smoothstep(0.0f, 1.0f, dmin);
    output.pixelColor = lerp(WireframeValues.edgeColor, input.vertexColor, blend);
    output.pixelColor.a = dmin;
    return output;
}
