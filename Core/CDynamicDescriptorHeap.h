
#pragma once

#include "CDescriptorHeap.h"
#include "CDescriptorHeapBase.h"
#include "RootSignature.h"
#include <vector>
#include <queue>

namespace CGraphics
{
    extern ID3D12Device8* g_Device;
}

// We can dynamically create descriptor heaps, 
class CDynamicDescriptorHeap : public CDescriptorHeapBase
{
public:
    CDynamicDescriptorHeap(CCommandContext& OwningContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
    ~CDynamicDescriptorHeap();

    static void DestroyAll(void)
    {
        sm_DescriptorHeapPool[0].clear();
        sm_DescriptorHeapPool[1].clear();
    }

    void CleanupUsedHeaps( uint64_t fenceValue );

public:

    void SetGraphicsDescriptorHandles( UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
    {
        m_GraphicsHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
    }

    void SetComputeDescriptorHandles( UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
    {
        m_ComputeHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
    }


public:

    D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect( D3D12_CPU_DESCRIPTOR_HANDLE Handles );

public:

    void ParseGraphicsRootSignature( const CRootSignature& RootSig )
    {
        m_GraphicsHandleCache.ParseRootSignature(m_DescriptorType, RootSig);
    }

    void ParseComputeRootSignature( const CRootSignature& RootSig )
    {
        m_ComputeHandleCache.ParseRootSignature(m_DescriptorType, RootSig);
    }


public:

    inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
    {
        if (m_GraphicsHandleCache.m_StaleRootParamsBitMap != 0)
            CopyAndBindStagedTables(m_GraphicsHandleCache, CmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
    }

    inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
    {
        if (m_ComputeHandleCache.m_StaleRootParamsBitMap != 0)
            CopyAndBindStagedTables(m_ComputeHandleCache, CmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
    }



private:

    // Static members

    static const uint32_t kNumDescriptorsPerHeap = 2048;
    static std::mutex sm_Mutex;

private:

    static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool[2];
    static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sm_RetiredDescriptorHeaps[2];
    static std::queue<ID3D12DescriptorHeap*> sm_AvailableDescriptorHeaps[2];

    static ID3D12DescriptorHeap* RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
    static void DiscardDescriptorHeaps( D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps );

    CCommandContext& m_OwningContext;
    ID3D12DescriptorHeap* m_CurrentHeapPtr;
    const D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorType;

    uint32_t m_CurrentOffset;


    std::vector<ID3D12DescriptorHeap*> m_RetiredHeaps;

    struct CDescTableCache
    {
        CDescTableCache() : AssignedHandlesBitMap(0) {}
        uint32_t AssignedHandlesBitMap;
        D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
        uint32_t TableSize;
    };

    struct CDescriptorHandleCache
    {
        CDescriptorHandleCache()
        {
            ClearCache();
        }

        void ClearCache()
        {
            m_RootDescriptorTablesBitMap = 0;
            m_StaleRootParamsBitMap = 0;
            m_MaxCachedDescriptors = 0;
        }

        uint32_t m_RootDescriptorTablesBitMap;
        uint32_t m_StaleRootParamsBitMap;
        uint32_t m_MaxCachedDescriptors;

        static const uint32_t kMaxNumDescriptors = 512;
        static const uint32_t kMaxNumDescriptorTables = 14;

        uint32_t ComputeStagedSize();
        void CopyAndBindStaleTables( D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize, CDescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
            void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

        CDescTableCache m_RootDescriptorTable[kMaxNumDescriptorTables];
        D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCache[kMaxNumDescriptors];

        void UnbindAllValid();
        void StageDescriptorHandles( UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] );
        void ParseRootSignature( D3D12_DESCRIPTOR_HEAP_TYPE Type, const CRootSignature& RootSig );
    };

    CDescriptorHandleCache m_GraphicsHandleCache;
    CDescriptorHandleCache m_ComputeHandleCache;

    bool HasSpace( uint32_t Count )
    {
        return (m_CurrentHeapPtr != nullptr && m_CurrentOffset + Count <= kNumDescriptorsPerHeap);
    }

    void RetireCurrentHeap(void);
    void RetireUsedHeaps( uint64_t fenceValue );
    ID3D12DescriptorHeap* GetHeapPointer();

    CDescriptorHandle Allocate( UINT Count )
    {
        CDescriptorHandle ret = m_FirstHandle + m_CurrentOffset * m_DescriptorSize;
        m_CurrentOffset += Count;
        return ret;
    }

    void CopyAndBindStagedTables( CDescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
        void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE) );

    void UnbindAllValid( void );
};
