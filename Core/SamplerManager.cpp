

#include "pch.h"
#include "SamplerManager.h"
#include "CDirectX12Core.h"
#include "Hash.h"
#include <map>

using namespace std;
using namespace CGraphics;

namespace
{
    map< size_t, D3D12_CPU_DESCRIPTOR_HANDLE > s_SamplerCache;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12_SAMPLER_DESC1::CreateDescriptor()
{
    size_t hashValue = CUtility::HashState(this);
    auto iter = s_SamplerCache.find(hashValue);
    if (iter != s_SamplerCache.end())
    {
        return iter->second;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Handle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    g_Device->CreateSampler(this, Handle);
    return Handle;
}

void D3D12_SAMPLER_DESC1::CreateDescriptor( D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    ASSERT(Handle.ptr != 0 && Handle.ptr != -1);
    g_Device->CreateSampler(this, Handle);
}
