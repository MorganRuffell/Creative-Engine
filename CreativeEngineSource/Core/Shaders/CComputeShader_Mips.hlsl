// Ruffell Interactive limited 2021

#define BLOCK_SIZE 8

#define WIDTH_HEIGHT_EVEN 0    
#define WIDTH_ODD_HEIGHT_EVEN 1 
#define WIDTH_EVEN_HEIGHT_ODD 2 
#define WIDTH_HEIGHT_ODD 3      
#define WIDTH_HEIGHT_ODD_EVEN 4
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
Texture2D<float4> SourceMipMap : register( t0 );

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
        return result;
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
        return float4(ConvertToSRGB(Input.rgb), Input.a);
    }
    else
    {
        return Input;
    }
}

[RootSignature(GenerateMips_RootSignature)]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeShaderInput CSI)
{
    float4 SourceZero = (float4) 0;

    switch (SrcDimension)
    {
    case WIDTH_HEIGHT_EVEN:

        float2 UVSpaceZero = TexelSize * (CSI.DispatchThreadID.xy + 0.5);
        SourceZero = SourceMipMap.SampleLevel(LinearClampSampler, UVSpaceZero, SrcMipLevel);
        break;

    case WIDTH_HEIGHT_ODD_EVEN:

        float2 UVSpaceOne = TexelSize * (CSI.DispatchThreadID.xy + float2(0.25, 0.5));
        float2 Offset = TexelSize * float2( 0.5, 0.0);
        break;

    case WIDTH_EVEN_HEIGHT_ODD:

        float2 UVSpaceOne = TexelSize * (CSI.DispatchThreadID.xy + float2(0.5, 0.25));
        float2 Offset = TexelSize * float2(0.0, 0.5);

        SourceZero = (SourceMipMap.SampleLevel(LinearClampSampler, UVSpaceOne, SrcMipLevel) + SrcMip.SampleLevel(LinearClampSampler, UVSpaceOne, SrcMipLevel));
        break;

    case WIDTH_HEIGHT_ODD:

        float2 UVSpaceOne = TexelSize * (CSI.DispatchThreadID.xy + float2(0.25, 0.25));
        float2 Offset = TexelSize * 0.5;

        SourceZero = SourceMipMap.SampleLevel(LinearClampSamplerm UVSpaceOne, SrcMipLevel);
        SourceZero += SourceMipMap.SampleLevel(LinearClampSampler, UVSpaceOne + float2(Off.x, 0.0), SrcMipLevel);
        SourceZero += SourceMipMap.SampleLevel(LinearClampSampler, UVSpaceOne + float2(0.0, Off.y), SrcMipLevel);
        SourceZero += SourceMipMap.SampleLevel(LinearClampSampler, UVSpaceOne + float2(Off.x, Off.y), SrcMipLevel);
        SourceZero *= 0.25;
        break;
    }

    OutMip1[CSI.DispatchThreadID.xy] = PackColor(SourceZero);

    // A Constant can exit all threads concurrently.

    if (NumMipLevels == 1)
    {
        return;
    }

    SeperateColoursInMemoryBanks(CSI.GroupIndex, SourceZero);
    GroupMemoryBarrierWithGroupSync();

    if ((CSI.GroupIndex & 0x9) == 0)
    {
        float4 SourceOne = LoadColor(IN.GroupIndex + 0x01);
        float4 SourceTwo = LoadColor(IN.GroupIndex + 0x08);
        float4 SourceThree = LoadColor(IN.GroupIndex + 0x09);

        SourceZero = 0.25 * (SourceZero + SourceOne + SourceTwo + SourceThree);

        OutMip2[CSI.DispatchThreadID.xy / 2] = PackColour( SourceZero );
        SeperateColoursInMemoryBanks(CSI.GroupIndex, SourceZero);
    }

    if (NumMipLevels == 2)
        return;

    GroupMemoryBarrierWithGroupSync()

    // This bit mask (binary: 011011) checks that X and Y are multiples of four.
    if ((GI & 0x1B) == 0)
    {
        float4 Src2 = LoadColor(GI + 0x02);
        float4 Src3 = LoadColor(GI + 0x10);
        float4 Src4 = LoadColor(GI + 0x12);
        Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

        OutMip3[DTid.xy / 4] = PackColor(Src1);
        StoreColor(GI, Src1);
    }

    if (NumMipLevels == 3)
        return;

    GroupMemoryBarrierWithGroupSync();

    // This bit mask would be 111111 (X & Y multiples of 8), but only one
    // thread fits that criteria.
    if (GI == 0)
    {
        float4 Src2 = LoadColor(GI + 0x04);
        float4 Src3 = LoadColor(GI + 0x20);
        float4 Src4 = LoadColor(GI + 0x24);
        Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

        OutMip4[DTid.xy / 8] = PackColor(Src1);
    }
}

//Update this https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/GenerateMipsCS.hlsli