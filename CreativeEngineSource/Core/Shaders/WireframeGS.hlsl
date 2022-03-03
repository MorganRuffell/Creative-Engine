groupshared int index;

struct EdgeDetails
{
    float4 MeshColour;
    float4 EdgesColour;
    float2 windowSize;
};

ConstantBuffer<EdgeDetails> EdgeDetailsBuffer : register(b1); 

struct GS_INPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

struct GS_OUTPUT
{
    float4 vertexColor : COLOR0;

    //These floats do not require perspective, this is important because the texture coordinate we are use SOMETIMES will use perspective
    noperspective float3 edgeDistance : TEXCOORD0;

    //These are clipping position, this tracks the intersection
    float4 clipPosition : SV_POSITION;
};

[maxvertexcount(3)]
void GSMain(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[3];

    float2 pixel[3];
    //This unrolls the triangle strip -- similar to how an artist would unroll cloth.
    [unroll]
    for (index = 0; index < 3; ++index)
    {
        float2 ndc = input[index].clipPosition.xy / input[index].clipPosition.w;
        pixel[index] = 0.5f * EdgeDetailsBuffer.windowSize * (ndc + 1.0f);
    }

    int j0[3] = { 2, 0, 1 }, j1[3] = { 1, 2, 0 };
    float edgeDistance[3];

    [unroll]
    for (index = 0; index < 3; ++index)
    {
        float2 diff = pixel[index] - pixel[j1[index]];
        float2 edge = pixel[j0[index]] - pixel[j1[index]];
        float edgeLength = length(edge);
        if (edgeLength > 0.0f)
        {
            //To calculate the edge length we get the dot product of the difference between pixels (Which we store in a float2)
            //, divided by the length of the edge
            edgeDistance[index] = abs(dot(diff, float2(edge.y, -edge.x)) / edgeLength);
        }
        else
        {
            edgeDistance[index] = 0.0f;
        }

        output[index].vertexColor = input[index].vertexColor;
        output[index].clipPosition = input[index].clipPosition;
    }

    //Recall the lecture on Affinite coordinates, we are getting the edge distance.
    output[0].edgeDistance = float3(edgeDistance[0], 0.0f, 0.0f);
    output[1].edgeDistance = float3(0.0f, edgeDistance[1], 0.0f);
    output[2].edgeDistance = float3(0.0f, 0.0f, edgeDistance[2]);

    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[2]);

    //Once we've appended (added) everything we Restart the triangle strip. -- I'm using these instead of lists as I like them more.
    stream.RestartStrip();
}