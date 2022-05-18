#pragma once

#include "CGPUBuffer.h"
#include <vector>
#include <queue>
#include <mutex>
#include <set>

// Unfortunately the api restricts the minimum size of a placed buffer resource to 64k
#define MINIMUM_BUFFER_SIZE (64 * 1024)

#if defined(PROFILE) || defined(_DEBUG)
#define INCREASE_BUDDY_COUNTER(A, B) (A = A + B);
#define DECREASE_BUDDY_COUNTER(A, B) (A = A - B);
#else
#define INCREASE_BUDDY_COUNTER(A, B)
#define DECREASE_BUDDY_COUNTER(A, B)
#endif


enum ECBlockAllocatorStrategy
{
	kPlacedResourceStrategy,
	kManualSubAllocationStrategy
};

struct CContigiousMemoryBlock
{
	CByteAddressBuffer* m_pBuffer;
	ComPtr<ID3D12Heap> m_pBackingHeap;

	size_t m_offset;
	size_t m_size;
	size_t m_unpaddedSize;
	uint64_t m_fenceValue;

	inline size_t GetOffset() const { return m_offset; }
	inline size_t GetSize() const { return m_size; }

	CContigiousMemoryBlock() : m_pBuffer(nullptr), m_pBackingHeap(nullptr), m_offset(0), m_size(0), m_unpaddedSize(0), m_fenceValue(0) {};

	CContigiousMemoryBlock(_In_ uint32_t heapOffset, _In_ uint32_t totalSize, _In_ uint32_t unpaddedSize);

	void InitPlaced(_Inout_ ID3D12Heap* pBackingHeap, _In_ uint32_t numElements, _In_ uint32_t elementSize, _Inout_ const void* initialData = nullptr);

	void InitFromResource(_Inout_ CByteAddressBuffer* pBuffer, _In_ uint32_t numElements, _In_ uint32_t elementSize, _Inout_ const void* initialData = nullptr);

	void Destroy();
};

class CContigiousAllocator
{
public:

	CContigiousAllocator(ECBlockAllocatorStrategy allocationStrategy, D3D12_HEAP_TYPE heapType, _In_ size_t maxBlockSize, _In_ size_t minBlockSize = MINIMUM_BUFFER_SIZE, _In_ size_t baseOffset = 0);

	void Initialize();

	void Destroy();

	CContigiousMemoryBlock* Allocate(_In_ uint32_t numElements, _In_ uint32_t elementSize, _Inout_ const void* initialData = nullptr);

	void Deallocate(_Inout_ CContigiousMemoryBlock* pBlock);

	inline bool IsOwner(_In_ const CContigiousMemoryBlock& block)
	{
		return block.GetOffset() >= m_baseOffset && block.GetSize() <= m_maxBlockSize;
	}

	inline void Reset()
	{
		// Clear the free blocks collection  
		m_freeBlocks.clear();

		// Initialize the pool with a free inner block of max inner block size  
		m_freeBlocks.resize(m_maxOrder + 1);
		m_freeBlocks[m_maxOrder].insert((size_t)0);
	}

	void CleanUpAllocations();

private:

	ComPtr<ID3D12Heap> m_pBackingHeap;
	CByteAddressBuffer m_BackingResource;

	const D3D12_HEAP_TYPE m_heapType;

	std::queue<CContigiousMemoryBlock*> m_deferredDeletionQueue;
	std::vector<std::set<size_t>> m_freeBlocks;

private:

	UINT m_maxOrder;
	const size_t m_baseOffset;
	const size_t m_maxBlockSize;
	const size_t m_minBlockSize;

	const ECBlockAllocatorStrategy m_allocationStrategy;

	inline size_t SizeToUnitSize(size_t size) const
	{
		return (size + (m_minBlockSize - 1)) / m_minBlockSize;
	}

	inline UINT UnitSizeToOrder(size_t size) const
	{
		return CMath::Log2(size); // Log2 rounds up fractions to next whole value
	}

	inline size_t GetContigiousOffset(const size_t& offset, const size_t& size)
	{
		return offset ^ size;
	}

	void DeallocateInternal(CContigiousMemoryBlock* pBlock);

	size_t OrderToUnitSize(UINT order) const { return ((size_t)1) << order; }
	size_t AllocateBlock(UINT order);
	void DeallocateBlock(size_t offset, UINT order);

#if defined(PROFILE) || defined(_DEBUG)
	size_t m_SpaceUsed;
	size_t m_InternalFragmentation;
#endif
};
