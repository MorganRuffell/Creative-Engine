#include "pch.h"
#include "CDynamicDescriptorHeap.h"
#include "CCommandContext.h"
#include "CDirectX12Core.h"
#include "CCommandListManager.h"
#include "RootSignature.h"

using namespace CGraphics;

//
// DynamicDescriptorHeap Implementation
//
// These are dynamically allocated, to allow for a pool of them to exist.

//This is a mutex, this is a concurrency primitive that is used to allow concurrent code by locking the code underneath it
//when used in a lock guard
std::mutex CDynamicDescriptorHeap::sm_Mutex; 

//Dynamic Array of Descriptor Heaps
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> CDynamicDescriptorHeap::sm_DescriptorHeapPool[2];

//These are queues, these are basically arrays that function in the same way 
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> CDynamicDescriptorHeap::sm_RetiredDescriptorHeaps[2];
std::queue<ID3D12DescriptorHeap*> CDynamicDescriptorHeap::sm_AvailableDescriptorHeaps[2];

ID3D12DescriptorHeap* CDynamicDescriptorHeap::RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
{
    //Once this is invoked all code beneath it, will be executed concurrently, once this reaches the end
    //of this scope, it is released! Allowing for these variables to be accessed from all threads concurrently
    std::lock_guard<std::mutex> LockGuard(sm_Mutex);

    uint32_t idx = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;

    while (!sm_RetiredDescriptorHeaps[idx].empty() && g_CommandManager.IsFenceComplete(sm_RetiredDescriptorHeaps[idx].front().first))
    {
        sm_AvailableDescriptorHeaps[idx].push(sm_RetiredDescriptorHeaps[idx].front().second);
        sm_RetiredDescriptorHeaps[idx].pop();
    }

    if (!sm_AvailableDescriptorHeaps[idx].empty())
    {
        ID3D12DescriptorHeap* HeapPtr = sm_AvailableDescriptorHeaps[idx].front();
        sm_AvailableDescriptorHeaps[idx].pop();
        return HeapPtr;
    }
    else
    {
        D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
        HeapDesc.Type = HeapType;
        HeapDesc.NumDescriptors = kNumDescriptorsPerHeap;
        HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        HeapDesc.NodeMask = 1;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> HeapPtr;
        ASSERT_SUCCEEDED(g_Device->CreateDescriptorHeap(&HeapDesc, MY_IID_PPV_ARGS(&HeapPtr)));
        sm_DescriptorHeapPool[idx].emplace_back(HeapPtr);
        return HeapPtr.Get();
    }
}

void CDynamicDescriptorHeap::DiscardDescriptorHeaps( D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValue, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps )
{
    uint32_t idx = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;
    std::lock_guard<std::mutex> LockGuard(sm_Mutex);

    //This is using iterators, I'm using this because I want to directly access the beginning and end of the collection.
    //Most of this exists as 'free' functions inside of the <algorithim> header
    for (auto iter = UsedHeaps.begin(); iter != UsedHeaps.end(); ++iter)
    {
        sm_RetiredDescriptorHeaps[idx].push(std::make_pair(FenceValue, *iter));

    }
}

void CDynamicDescriptorHeap::RetireCurrentHeap( void )
{
    // Don't retire unused heaps.
    if (m_CurrentOffset == 0)
    {
        ASSERT(m_CurrentHeapPtr == nullptr);
        return;
    }

    ASSERT(m_CurrentHeapPtr != nullptr);

    m_RetiredHeaps.push_back(m_CurrentHeapPtr);
    m_CurrentHeapPtr = nullptr;
    m_CurrentOffset = 0;
}

void CDynamicDescriptorHeap::RetireUsedHeaps( uint64_t fenceValue )
{
    DiscardDescriptorHeaps(m_DescriptorType, fenceValue, m_RetiredHeaps);
    m_RetiredHeaps.clear();
}

CDynamicDescriptorHeap::CDynamicDescriptorHeap(CCommandContext& OwningContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
    : m_OwningContext(OwningContext), m_DescriptorType(HeapType)
{
    m_CurrentHeapPtr = nullptr;
    m_CurrentOffset = 0;
    m_DescriptorSize = CGraphics::g_Device->GetDescriptorHandleIncrementSize(HeapType);
}

CDynamicDescriptorHeap::~CDynamicDescriptorHeap()
{
}

void CDynamicDescriptorHeap::CleanupUsedHeaps( uint64_t fenceValue )
{
    RetireCurrentHeap();
    RetireUsedHeaps(fenceValue);
    m_GraphicsHandleCache.ClearCache();
    m_ComputeHandleCache.ClearCache();
}

inline ID3D12DescriptorHeap* CDynamicDescriptorHeap::GetHeapPointer()
{
    if (m_CurrentHeapPtr == nullptr)
    {
        ASSERT(m_CurrentOffset == 0);
        m_CurrentHeapPtr = RequestDescriptorHeap(m_DescriptorType);
        m_FirstHandle = CDescriptorHandle(
            m_CurrentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
            m_CurrentHeapPtr->GetGPUDescriptorHandleForHeapStart());
    }

    return m_CurrentHeapPtr;
}

uint32_t CDynamicDescriptorHeap::CDescriptorHandleCache::ComputeStagedSize()
{
    // Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
    uint32_t NeededSpace = 0;
    uint32_t RootIndex;
    uint32_t StaleParams = m_StaleRootParamsBitMap;
    while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
    {
        StaleParams ^= (1 << RootIndex);

        uint32_t MaxSetHandle;
        ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
            "Root entry marked as stale but has no stale descriptors");

        NeededSpace += MaxSetHandle + 1;
    }
    return NeededSpace;
}

void CDynamicDescriptorHeap::CDescriptorHandleCache::CopyAndBindStaleTables(
    D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize,
    CDescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
    void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
    uint32_t StaleParamCount = 0;
    uint32_t TableSize[CDescriptorHandleCache::kMaxNumDescriptorTables];
    uint32_t RootIndices[CDescriptorHandleCache::kMaxNumDescriptorTables];
    uint32_t NeededSpace = 0;
    uint32_t RootIndex;

    // Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
    uint32_t StaleParams = m_StaleRootParamsBitMap;
    while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
    {
        RootIndices[StaleParamCount] = RootIndex;
        StaleParams ^= (1 << RootIndex);

        uint32_t MaxSetHandle;
        ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
            "Root entry marked as stale but has no stale descriptors");

        NeededSpace += MaxSetHandle + 1;
        TableSize[StaleParamCount] = MaxSetHandle + 1;

        ++StaleParamCount;
    }

    ASSERT(StaleParamCount <= CDescriptorHandleCache::kMaxNumDescriptorTables,
        "We're only equipped to handle so many descriptor tables");

    m_StaleRootParamsBitMap = 0;

    static const uint32_t kMaxDescriptorsPerCopy = 16;
    UINT NumDestDescriptorRanges = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
    UINT pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

    UINT NumSrcDescriptorRanges = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
    UINT pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

    for (uint32_t i = 0; i < StaleParamCount; ++i)
    {
        RootIndex = RootIndices[i];
        (CmdList->*SetFunc)(RootIndex, DestHandleStart);

        CDescTableCache& RootDescTable = m_RootDescriptorTable[RootIndex];

        D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandles = RootDescTable.TableStart;
        uint64_t SetHandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
        D3D12_CPU_DESCRIPTOR_HANDLE CurDest = DestHandleStart;
        DestHandleStart += TableSize[i] * DescriptorSize;

        unsigned long SkipCount;
        while (_BitScanForward64(&SkipCount, SetHandles))
        {
            // Skip over unset descriptor handles
            SetHandles >>= SkipCount;
            SrcHandles += SkipCount;
            CurDest.ptr += SkipCount * DescriptorSize;

            unsigned long DescriptorCount;
            _BitScanForward64(&DescriptorCount, ~SetHandles);
            SetHandles >>= DescriptorCount;

            // If we run out of temp room, copy what we've got so far
            if (NumSrcDescriptorRanges + DescriptorCount > kMaxDescriptorsPerCopy)
            {
                g_Device->CopyDescriptors(
                    NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
                    NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
                    Type);

                NumSrcDescriptorRanges = 0;
                NumDestDescriptorRanges = 0;
            }

            // Setup destination range
            pDestDescriptorRangeStarts[NumDestDescriptorRanges] = CurDest;
            pDestDescriptorRangeSizes[NumDestDescriptorRanges] = DescriptorCount;
            ++NumDestDescriptorRanges;

            // Setup source ranges (one descriptor each because we don't assume they are contiguous)
            for (uint32_t j = 0; j < DescriptorCount; ++j)
            {
                pSrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandles[j];
                pSrcDescriptorRangeSizes[NumSrcDescriptorRanges] = 1;
                ++NumSrcDescriptorRanges;
            }

            // Move the destination pointer forward by the number of descriptors we will copy
            SrcHandles += DescriptorCount;
            CurDest.ptr += DescriptorCount * DescriptorSize;
        }
    }

    g_Device->CopyDescriptors(
        NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
        NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
        Type);
}
    
void CDynamicDescriptorHeap::CopyAndBindStagedTables( CDescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
    void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
    uint32_t NeededSize = HandleCache.ComputeStagedSize();
    if (!HasSpace(NeededSize))
    {
        RetireCurrentHeap();
        UnbindAllValid();
        NeededSize = HandleCache.ComputeStagedSize();
    }

    // This can trigger the creation of a new heap
    m_OwningContext.SetDescriptorHeap(m_DescriptorType, GetHeapPointer());
    HandleCache.CopyAndBindStaleTables(m_DescriptorType, m_DescriptorSize, Allocate(NeededSize), CmdList, SetFunc);
}

void CDynamicDescriptorHeap::UnbindAllValid( void )
{
    m_GraphicsHandleCache.UnbindAllValid();
    m_ComputeHandleCache.UnbindAllValid();
}

D3D12_GPU_DESCRIPTOR_HANDLE CDynamicDescriptorHeap::UploadDirect( D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    if (!HasSpace(1))
    {
        RetireCurrentHeap();
        UnbindAllValid();
    }

    m_OwningContext.SetDescriptorHeap(m_DescriptorType, GetHeapPointer());

    CDescriptorHandle DestHandle = m_FirstHandle + m_CurrentOffset * m_DescriptorSize;
    m_CurrentOffset += 1;

    g_Device->CopyDescriptorsSimple(1, DestHandle, Handle, m_DescriptorType);

    return DestHandle;
}

void CDynamicDescriptorHeap::CDescriptorHandleCache::UnbindAllValid()
{
    m_StaleRootParamsBitMap = 0;

    unsigned long TableParams = m_RootDescriptorTablesBitMap;
    unsigned long RootIndex;
    while (_BitScanForward(&RootIndex, TableParams))
    {
        TableParams ^= (1 << RootIndex);
        if (m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap != 0)
            m_StaleRootParamsBitMap |= (1 << RootIndex);
    }
}

void CDynamicDescriptorHeap::CDescriptorHandleCache::StageDescriptorHandles( UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
    ASSERT(((1 << RootIndex) & m_RootDescriptorTablesBitMap) != 0, "Root parameter is not a CBV_SRV_UAV descriptor table");
    ASSERT(Offset + NumHandles <= m_RootDescriptorTable[RootIndex].TableSize);

    CDescTableCache& TableCache = m_RootDescriptorTable[RootIndex];
    D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCache.TableStart + Offset;
    for (UINT i = 0; i < NumHandles; ++i)
        CopyDest[i] = Handles[i];
    TableCache.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
    m_StaleRootParamsBitMap |= (1 << RootIndex);
}

void CDynamicDescriptorHeap::CDescriptorHandleCache::ParseRootSignature( D3D12_DESCRIPTOR_HEAP_TYPE Type, const CRootSignature& RootSig )
{
    UINT CurrentOffset = 0;

    ASSERT(RootSig.m_NumParameters <= 16, "Maybe we need to support something greater");

    m_StaleRootParamsBitMap = 0;
    m_RootDescriptorTablesBitMap = (Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?
        RootSig.m_SamplerTableBitMap : RootSig.m_DescriptorTableBitMap);

    unsigned long TableParams = m_RootDescriptorTablesBitMap;
    unsigned long RootIndex;
    while (_BitScanForward(&RootIndex, TableParams))
    {
        TableParams ^= (1 << RootIndex);

        UINT TableSize = RootSig.m_DescriptorTableSize[RootIndex];
        ASSERT(TableSize > 0);

        CDescTableCache& RootDescriptorTable = m_RootDescriptorTable[RootIndex];
        RootDescriptorTable.AssignedHandlesBitMap = 0;
        RootDescriptorTable.TableStart = m_HandleCache + CurrentOffset;
        RootDescriptorTable.TableSize = TableSize;

        CurrentOffset += TableSize;
    }

    m_MaxCachedDescriptors = CurrentOffset;

    ASSERT(m_MaxCachedDescriptors <= kMaxNumDescriptors, "Exceeded user-supplied maximum cache size");
}
