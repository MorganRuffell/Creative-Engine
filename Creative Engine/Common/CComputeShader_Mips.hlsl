// Ruffell Interactive limited 2021

#define WIDTH_HEIGHT_EVEN 0    
#define WIDTH_ODD_HEIGHT_EVEN 1 
#define WIDTH_EVEN_HEIGHT_ODD 2 
#define WIDTH_HEIGHT_ODD 3      
#define GenerateMips_RootSignature \
     "RootFlags(0), " \
     "RootConstants(b0, num32BitConstants = 6), " \
     "DescriptorTable( SRV(t0, numDescriptors = 1) )," \
     "DescriptorTable( UAV(u0, numDescriptors = 4) )," \
     "StaticSampler(s0," \
     "addressU = TEXTURE_ADDRESS_CLAMP," \
     "addressV = TEXTURE_ADDRESS_CLAMP," \
     "addressW = TEXTURE_ADDRESS_CLAMP," \
     "filter = FILTER_MIN_MAG_MIP_LINEAR)"

//Texture Registers
Texture2D<float4> SourceMap : register( t0 );

//Unsigned Registers
RWTexture2D<float4> OutputMipMapLevel0 : register( u0 );
RWTexture2D<float4> OutputMipMapLevel1 : register( u1 );
RWTexture2D<float4> OutputMipMapLevel2 : register( u2 );

//Sampler Registers
SamplerState LinearClampSampler : register(s0);

//Shared Group floats -- These are used for RGBA channels, and to reduce data races
groupshared float ThreadSafe_Red[64];
groupshared float ThreadSafe_Green[64];
groupshared float ThreadSafe_Blue[64];
groupshared float ThreadSafe_Alpha[64];

struct ComputeShaderInput
{
	uint3 GroupID : SV_GroupID; 
	uint3 GroupThreadID  : SV_GroupThreadID;
	uint3 DispatchThreadID   : SV_DispatchThreadID;
	uint3 GroupIndex         : SV_GroupIndex;
};

cbuffer GenerateMipsCB : register(b0)
{
    uint SrcMipLevel; // Texture level of source mip
    uint NumMipLevels; // Number of OutMips to write: [1-4]
    uint SrcDimension;  // Width and height of the source texture are even or odd.
    bool IsSRGB;        // Must apply gamma correction to sRGB textures.
    float2 TexelSize; // 1.0 / OutMip1.Dimensions
}

void SeperateColoursInMemoryBanks(int index, double4 Colour)
{
    ThreadSafe_Red[index] = Colour.r;
    ThreadSafe_Green[index] = Colour.g;
    ThreadSafe_Blue[index] = Colour.b;
    ThreadSafe_Alpha[index] = Colour.a;
}

float4 LoadColourFromMemory(int index)
{
    float4 result; 

    if (index < 0)
    {
        result = ThreadSafe_Red[64], ThreadSafe_Red[64], ThreadSafe_Red[64], ThreadSafe_Alpha[64];
        return result
    }
    else
    {
        result = ThreadSafe_Red[index],
            ThreadSafe_Green[index],
            ThreadSafe_Blue[index],
            ThreadSafe_Alpha[index];

        return result;
    }
}

float3 ConvertToLinear(float3 ValueToConvert) //Changes an SRGB value to linear equv
{
    return ValueToConvert < 0.04 ? ValueToConvert / 12.92 : pow((ValueToConvert + 0.055) / 1.055, 2.4);
}

float3 ConvertToSRGB(float3 ValueToConvert)
{
    return ValueToConvert < 0.0031308 ? 12.92 * ValueToConvert : 1.055 * pow(abs(ValueToConvert), 1.0 / 2.4) - 0.055;
}

float4 PackColour(float4 Input)
{
    if (IsSRGB)
    {
        return float(ConvertToSRGB(Input.rgb), Input.a);
    }
    else
    {
        return Input;
    }
}

[RootSignature(GenerateMipsComputeShader_RootSignature)]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput CSI)
{
    float4 SourceZero = (float4) 0;

    switch (SrcDimension)
    {
    case WIDTH_HEIGHT_EVEN:

    }
}