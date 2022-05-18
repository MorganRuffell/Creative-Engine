

#include "pch.h"
#include "LinearAllocator.h"
#include "CDirectX12Core.h"
#include "CCommandListManager.h"
#include <thread>

using namespace CGraphics;
using namespace std;

ELinearAllocatorType CLinearAllocatorManager::sm_AutoType = kGpuExclusive;

CLinearAllocatorManager::CLinearAllocatorManager()
{
    m_AllocationType = sm_AutoType;
    sm_AutoType = (ELinearAllocatorType)(sm_AutoType + 1);
    ASSERT(sm_AutoType <= kNumAllocatorTypes);
}

CLinearAllocatorManager CLinearAllocator::sm_BlockManager[2];

ClinearAllocationBlock* CLinearAllocatorManager::FetchBlock()
{
    lock_guard<mutex> LockGuard(m_Mutex);

    while (!m_UsedBlocks.empty() && g_CommandManager.IsFenceComplete(m_UsedBlocks.front().first))
    {
        m_AvaliableBlocks.push(m_UsedBlocks.front().second);
        m_UsedBlocks.pop();
    }

    ClinearAllocationBlock* PagePtr = nullptr;

    if (!m_AvaliableBlocks.empty())
    {
        PagePtr = m_AvaliableBlocks.front();
        m_AvaliableBlocks.pop();
    }
    else
    {
        PagePtr = AllocateNewBlock();
        m_BlockPool.emplace_back(PagePtr);
    }

    return PagePtr;
}

void CLinearAllocatorManager::DiscardBlocks( uint64_t FenceValue, const vector<ClinearAllocationBlock*>& UsedPages )
{
    lock_guard<mutex> LockGuard(m_Mutex);
    for (auto iter = UsedPages.begin(); iter != UsedPages.end(); ++iter)
        m_UsedBlocks.push(make_pair(FenceValue, *iter));
}

void CLinearAllocatorManager::DestroyLargeBlocks( uint64_t FenceValue, const vector<ClinearAllocationBlock*>& LargeBlocks )
{
    lock_guard<mutex> LockGuard(m_Mutex);

    while (!m_DeletionQueue.empty() && g_CommandManager.IsFenceComplete(m_DeletionQueue.front().first))
    {
        delete m_DeletionQueue.front().second;
        m_DeletionQueue.pop();
    }

    for (auto iter = LargeBlocks.begin(); iter != LargeBlocks.end(); ++iter)
    {
        (*iter)->Unmap();
        m_DeletionQueue.push(make_pair(FenceValue, *iter));
    }
}

ClinearAllocationBlock* CLinearAllocatorManager::AllocateNewBlock( size_t PageSize  )
{
    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDesc;
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_RESOURCE_STATES DefaultUsage;

    if (m_AllocationType == kGpuExclusive)
    {
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        ResourceDesc.Width = PageSize == 0 ? kGpuAllocatorBlockSize : PageSize;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    else
    {
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        ResourceDesc.Width = PageSize == 0 ? kCpuAllocatorBlockSize : PageSize;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    ID3D12Resource2* pBuffer;
    ASSERT_SUCCEEDED( g_Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
        &ResourceDesc, DefaultUsage, nullptr, MY_IID_PPV_ARGS(&pBuffer)) );

    pBuffer->SetName(L"LinearAllocator Page");

    return new ClinearAllocationBlock(pBuffer, DefaultUsage);
}

void CLinearAllocator::CleanupUsedBlocks( uint64_t FenceID )
{
    if (m_CurrentBlock == nullptr)
        return;

    m_UsedBlocks.push_back(m_CurrentBlock);
    m_CurrentBlock = nullptr;
    m_CurOffset = 0;

    sm_BlockManager[m_AllocationType].DiscardBlocks(FenceID, m_UsedBlocks);
    m_UsedBlocks.clear();

    sm_BlockManager[m_AllocationType].DestroyLargeBlocks(FenceID, m_LargeBlockList);
    m_LargeBlockList.clear();
}

SDynAlloc CLinearAllocator::AllocateLargeMemoryBlock(size_t SizeInBytes)
{
    ClinearAllocationBlock* OneOff = sm_BlockManager[m_AllocationType].AllocateNewBlock(SizeInBytes);
    m_LargeBlockList.push_back(OneOff);

    SDynAlloc ret(*OneOff, 0, SizeInBytes);
    ret.DataPtr = OneOff->m_CpuVirtualAddress;
    ret.GpuAddress = OneOff->m_GpuVirtualAddress;

    return ret;
}

SDynAlloc CLinearAllocator::Allocate(size_t SizeInBytes, size_t Alignment)
{
    const size_t AlignmentMask = Alignment - 1;

    // If a memory block is not a power of two, refuse to allow allocation
    ASSERT((AlignmentMask & Alignment) == 0);

    // Align the allocation - Memory is aligned on GPU to save space on the VRAM
    const size_t AlignedSize = CMath::AlignUpWithMask(SizeInBytes, AlignmentMask);

    if (AlignedSize > m_BlockSize)
        return AllocateLargeMemoryBlock(AlignedSize);

    m_CurOffset = CMath::AlignUp(m_CurOffset, Alignment);

    if (m_CurOffset + AlignedSize > m_BlockSize)
    {
        ASSERT(m_CurrentBlock != nullptr);
        m_UsedBlocks.push_back(m_CurrentBlock);
        m_CurrentBlock = nullptr;
    }

    if (m_CurrentBlock == nullptr)
    {
        m_CurrentBlock = sm_BlockManager[m_AllocationType].FetchBlock();
        m_CurOffset = 0;
    }

    SDynAlloc Return(*m_CurrentBlock, m_CurOffset, AlignedSize);
    Return.DataPtr = (uint8_t*)m_CurrentBlock->m_CpuVirtualAddress + m_CurOffset;
    Return.GpuAddress = m_CurrentBlock->m_GpuVirtualAddress + m_CurOffset;

    m_CurOffset += AlignedSize;

    return Return;
}
