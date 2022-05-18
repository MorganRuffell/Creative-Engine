#pragma once

#include "pch.h"

/// <summary>
/// This is similar to the Esram allocator created by microsoft in the raytracing sample
/// It acts as a stack based data structure. 
/// This allocates memory on the GPU.
/// 
/// We use this for all rendering code, the best way to think about this is that it is similar to a (c#) list that
/// will expand with each element added.
/// 
/// </summary>
class CGPURamAllocator
{
public:
    CGPURamAllocator() {}

    void PushStack() {}
    void PopStack() {}

    D3D12_GPU_VIRTUAL_ADDRESS AllocateNewMemory(_In_ size_t size, _In_ size_t align, _In_ const std::wstring& bufferName )
    {
        (size); 
        (align); 
        (bufferName);
        return D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    //This is an integer pointer, ANYWHERE there is the words void in the arguments inside a method, is where it allows for anything
    intptr_t SizeOfFreeSpace( void ) const
    {
        return 0;
    }

};
