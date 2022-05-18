
#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include "CCommandAllocatorPool.h"

/// <summary>
/// This is the command queue, recall that this is another wrapper around DirectX12 resources.
/// </summary>
class   CCommandQueue
{
    friend class CCommandListManager;
    friend class CCommandContext;

public:
    CCommandQueue(D3D12_COMMAND_LIST_TYPE Type);
    ~CCommandQueue();

    void Create(_Inout_ ID3D12Device8* pDevice);
    void Shutdown();

    inline bool IsReady()
    {
        return m_CommandQueue != nullptr;
    }

    uint64_t IncrementFence(void);
    bool IsFenceComplete(_In_ uint64_t FenceValue);
    void StallForFence(_In_ uint64_t FenceValue);
    void StallForProducer(_In_ CCommandQueue& Producer);
    void WaitForFence(_In_ uint64_t FenceValue);
    void WaitForIdle(void) { WaitForFence(IncrementFence()); }
    

    ID3D12CommandQueue* GetCommandQueue() { return m_CommandQueue; }

    uint64_t GetNextFenceValue() { return m_NextFenceValue; }
    uint64_t GetLastCompletedFenceValue() { return m_LastCompletedFenceValue; }

public:

    uint64_t ExecuteCommandList(_Inout_ ID3D12CommandList* List);

public:
    ID3D12CommandAllocator* RequestAllocator(void);
    void DiscardAllocator(_In_ uint64_t FenceValueForReset, _Inout_ ID3D12CommandAllocator* Allocator);

    ID3D12CommandQueue* m_CommandQueue;

    const D3D12_COMMAND_LIST_TYPE m_Type;

    CCommandAllocatorPool m_AllocatorPool;

public:

    std::mutex m_FenceMutex;
    std::mutex m_EventMutex;

public:

    // Lifetime of these objects is managed by the descriptor cache
    ID3D12Fence* m_pFence;

    uint64_t m_fenceValue;
    uint64_t m_NextFenceValue;
    uint64_t m_LastCompletedFenceValue;

    HANDLE m_FenceEventHandle;

};

class CCommandListManager
{
    friend class CCommandContext;

public:
    CCommandListManager();
    ~CCommandListManager();

    void Create(_Inout_ ID3D12Device8* pDevice);
    void Shutdown();

    CCommandQueue& GetGraphicsQueue(void) { return m_GraphicsQueue; }
    CCommandQueue& GetComputeQueue(void) { return m_ComputeQueue; }
    CCommandQueue& GetCopyQueue(void) { return m_CopyQueue; }

    CCommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        switch (Type)
        {
        case D3D12_COMMAND_LIST_TYPE_COMPUTE: return m_ComputeQueue;
        case D3D12_COMMAND_LIST_TYPE_COPY: return m_CopyQueue;
        default: return m_GraphicsQueue;
        }
    }

public:

    ID3D12CommandQueue* GetCommandQueue()
    {
        return m_GraphicsQueue.GetCommandQueue();
    }

    ID3D12CommandQueue* GetUICommandQueue()
    {
        return m_UIQueue.GetCommandQueue();
    }

public:

    void CreateNewCommandList(
        _In_  D3D12_COMMAND_LIST_TYPE Type,
        _Inout_ ID3D12GraphicsCommandList** List,
        _Inout_ ID3D12CommandAllocator** Allocator);

    void CreateUICommandList(_In_ D3D12_COMMAND_LIST_TYPE Type,
        _Inout_ ID3D12GraphicsCommandList** List,
        _Inout_ ID3D12CommandAllocator** Allocator);
    

    // Test to see if a fence has already been reached
    bool IsFenceComplete(_In_ uint64_t FenceValue)
    {
        return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
    }

    // The CPU will wait for a fence to reach a specified value
    void WaitForFence(_In_ uint64_t FenceValue);

    // The CPU will wait for all command queues to empty (so that the GPU is idle)
    void IdleGPU(void)
    {
        m_GraphicsQueue.WaitForIdle();
        m_ComputeQueue.WaitForIdle();
        m_CopyQueue.WaitForIdle();
    }

    
private:
    uint64_t m_FenceValue;

    uint64_t m_AmountofActiveCommandLists;


    ID3D12Device8* m_Device;

    CCommandQueue m_GraphicsQueue;
	CCommandQueue m_BundleQueue;
    CCommandQueue m_ComputeQueue;
    CCommandQueue m_CopyQueue;

    CCommandQueue m_UIQueue;

};
