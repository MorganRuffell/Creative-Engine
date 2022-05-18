//Motion Blur Final Pass 
//Pixel Shader PS6.2
//Morgan Ruffell - BSc Computer Games Technology -- Final Blur Pass -- PS6.2

#include "CommonRS.hlsli"
#include "PixelPacking_Velocity.hlsli"

#define MAX_SAMPLE_COUNT  30
#define STEP_SIZE         3.2

struct BufferValues
{
    float2 RcpBufferDim;	
};

Texture2D<packed_velocity_t> VelocityBuffer     : register(t0);	        // full resolution motion vectors
Texture2D<float4> PrepBuffer                    : register(t1);		            // 1/4 resolution pre-weighted blurred color samples
SamplerState LinearSampler                      : register(s1);                    // Bilinear w/ black border
ConstantBuffer<BufferValues> BufferValueCNST    : register(b0);

[RootSignature(Common_RootSig)] // We use common root signature, this is used in vfx and is only used in vfx pass
float4 main( float4 position : SV_Position ) : SV_Target0
{
    uint2 st = uint2(position.xy);
    float2 uv = position.xy * BufferValueCNST.RcpBufferDim;

    float2 Velocity = UnpackVelocity(VelocityBuffer[st]).xy;

    float Speed = length(Velocity);

    if (Speed < 4.0)
        discard;

    float4 accum = 0;

    float halfSampleCount = min(MAX_SAMPLE_COUNT * 0.5, Speed * 0.5 / STEP_SIZE);

    float2 deltaUV, uvw1, uvw2 = 0.0f;

    if (WaveIsFirstLane())
    {
        // Accumulate low-res, pre-weighted samples, summing their weights in alpha.
        deltaUV = Velocity / Speed * BufferValueCNST.RcpBufferDim * STEP_SIZE;
        uvw1 = uv; //Works per UV channel of a 3D object
        uvw2 = uv;

        // First accumulate the whole samples
        for (int i = halfSampleCount - 1; i > 0; i -= 1)
        {
            accum += PrepBuffer.SampleLevel(LinearSampler, uvw1 += deltaUV, 0);
            accum += PrepBuffer.SampleLevel(LinearSampler, uvw2 -= deltaUV, 0);
        }
    }

    

    // This is almost the same as 'frac(halfSampleCount)' replaces 0 with 1.
    float remainder = 1 + halfSampleCount - ceil(halfSampleCount);

    // Then accumulate all the samples
    deltaUV *= remainder;
    accum += PrepBuffer.SampleLevel(LinearSampler, uvw1 + deltaUV, 0) * remainder;
    accum += PrepBuffer.SampleLevel(LinearSampler, uvw2 - deltaUV, 0) * remainder;

    return accum * (saturate(Speed / 32.0) / (accum.a + 1.0));
}


