#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>

/// <summary>
/// Similar to what you'd have in Unreal when you have a pool of actors that we instantiate as part of when
/// we are firing bullets out of a gun. 
/// 
/// We are doing the same with the command allocators in creative, recall that command allocators are the backing memory that the GPU sees
/// the command list is what the CPU interacts with.
/// </summary>
class CCommandAllocatorPool
{
public:
    CCommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type);
    ~CCommandAllocatorPool();

    void Create(_Inout_ ID3D12Device8* pDevice);

    
    void Shutdown();

    ID3D12Device8* GetDevice()
    {
        return m_Device.Get();
    }

    ID3D12CommandAllocator* RequestAllocator(_In_ uint64_t CompletedFenceValue);
    void DiscardAllocator(_In_ uint64_t FenceValue, _In_ ID3D12CommandAllocator* Allocator);

    inline size_t Size() { return m_AllocatorPool.size(); }

private:

	void Destroy()
	{
		m_Device.Detach();
	}

    const D3D12_COMMAND_LIST_TYPE m_cCommandListType;

private:

    ComPtr<ID3D12Device8>                                               m_Device;
    std::vector<ID3D12CommandAllocator*>                                m_AllocatorPool;
    std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>>            m_ReadyAllocators;

    std::mutex                                                          m_AllocatorMutex;
};
