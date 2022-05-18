#define BitonicSort_RootSig \
    "RootFlags(0), " \
    "RootConstants(b0, num32BitConstants = 2)," \
    "DescriptorTable(SRV(t0, numDescriptors = 1))," \
    "DescriptorTable(UAV(u0, numDescriptors = 1))," \
    "RootConstants(b1, num32BitConstants = 2)"

ByteAddressBuffer g_CounterBuffer : register(t0);

cbuffer CB1 : register(b1)
{
    uint CounterOffset;
    uint NullItem; 
}


uint InsertOneBit( uint Value, uint OneBitMask )
{
    uint Mask = OneBitMask - 1;
    return (Value & ~Mask) << 1 | (Value & Mask) | OneBitMask;
}

bool ShouldSwap(uint A, uint B)
{
    return (A ^ NullItem) < (B ^ NullItem);
}

// Same as above, but only compares the upper 32-bit word.
bool ShouldSwap(uint2 A, uint2 B)
{
    return (A.y ^ NullItem) < (B.y ^ NullItem);
}
