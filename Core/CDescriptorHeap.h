

#pragma once

#include <mutex>
#include <vector>
#include <queue>
#include <string>
#include "CDescriptorHeapBase.h"

// This is an unbounded resource descriptor allocator.  It is intended to provide space for CPU-visible
// resource descriptors as resources are created.  For those that need to be made shader-visible, they
// will need to be copied to a DescriptorHeap or a DynamicDescriptorHeap.
class CDescriptorAllocator
{
public:
    CDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) : 
        m_Type(Type), m_CurrentHeap(nullptr), m_DescriptorSize(0)
    {
        m_CurrentHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate( uint32_t Count );

    static void DestroyAll(void);

protected:

    static const uint32_t sm_NumDescriptorsPerHeap = 256;
    static std::mutex sm_AllocationMutex;
    static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool;
    static ID3D12DescriptorHeap* RequestNewHeap( D3D12_DESCRIPTOR_HEAP_TYPE Type );

    D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
    ID3D12DescriptorHeap* m_CurrentHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
    uint32_t m_DescriptorSize;
    uint32_t m_RemainingFreeHandles;
};

/// <summary>
/// I've structed the bespoke implementation of descriptor heaps in a pattern similar
/// to the node you'd see in a node based data structure (Like a binary tree or a linkedlist). They 
/// </summary>
class CDescriptorHeap : public CDescriptorHeapBase
{
public:

    CDescriptorHeap(void) {}
    ~CDescriptorHeap(void) { Destroy(); }

    void Create( const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount );
    void Destroy(void) { m_DescriptorHeap = nullptr; }

    bool HasAvailableSpace( uint32_t Count ) const { return Count <= m_NumFreeDescriptors; }
    CDescriptorHandle Alloc( uint32_t Count = 1 );

    CDescriptorHandle operator[] (uint32_t arrayIdx) const { return m_FirstHandle + arrayIdx * m_DescriptorSize; }

    uint32_t GetOffsetOfHandle(const CDescriptorHandle& DHandle ) {
        return (uint32_t)(DHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr()) / m_DescriptorSize; }

    bool IsHandleValid( const CDescriptorHandle& DHandle ) const;

    ID3D12DescriptorHeap* GetHeap() const { return m_DescriptorHeap.Get(); }

    uint32_t GetDescriptorSize(void) const { return m_DescriptorSize; }

    unsigned long Release()
    {
        unsigned long ref = 0;

        GetHeap()->Release();

        return ref;
    }

private:

    ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

    uint32_t m_NumFreeDescriptors;

    CDescriptorHandle m_NextFreeHandle;
};
