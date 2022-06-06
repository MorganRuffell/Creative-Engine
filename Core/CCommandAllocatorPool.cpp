

#include "pch.h"
#include "CCommandAllocatorPool.h"

CCommandAllocatorPool::CCommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type) :
    m_cCommandListType(Type),
    m_Device(nullptr)
{
}

CCommandAllocatorPool::~CCommandAllocatorPool()
{
    Shutdown();
}

void CCommandAllocatorPool::Create(ID3D12Device8 * pDevice)
{
    m_Device.Attach(pDevice);
}

void CCommandAllocatorPool::Shutdown()
{
    Destroy();

    for (size_t i = 0; i < m_AllocatorPool.size(); ++i)
        m_AllocatorPool[i]->Release();

    m_AllocatorPool.clear();
}

ID3D12CommandAllocator * CCommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceValue)
{
    //We lock the context with a mutex -- You have to rememeber that GPUs are going to work concurrently
    //this means that we have to use a mutex to lock an index of a function -- I'm using a std::lockguard which automatically
    //Unlocks when it leaves the method context

    std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

    ID3D12CommandAllocator* pAllocator = nullptr;

    if (!m_ReadyAllocators.empty())
    {
        std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

        if (AllocatorPair.first <= CompletedFenceValue)
        {
            pAllocator = AllocatorPair.second;
            ASSERT_SUCCEEDED(pAllocator->Reset());
            m_ReadyAllocators.pop();
        }
    }

    // If no allocator's were ready to be reused, create a new one
    if (pAllocator == nullptr)
    {
        ASSERT_SUCCEEDED(m_Device->CreateCommandAllocator(m_cCommandListType, MY_IID_PPV_ARGS(&pAllocator)));
        wchar_t AllocatorName[32];
        swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_AllocatorPool.size());
        pAllocator->SetName(AllocatorName);
        m_AllocatorPool.push_back(pAllocator);
    }

    return pAllocator;
}

void CCommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator * Allocator)
{
    std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);

    // That fence value indicates we are free to reset the allocator
    m_ReadyAllocators.push(std::make_pair(FenceValue, Allocator));
}
