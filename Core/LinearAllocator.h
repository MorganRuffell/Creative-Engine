// This is a dynamic graphics memory allocator for DX12.  It's designed to work together with the CCommandContext and work concurrently
// There will be loads of command/graphics/compute contexts, each with its own linear allocators.  
// They act like the windows OS with a global memory pool by reserving a context-local memory page.  
// 
// When a command context is finished, it will receive a fence ID that indicates when it's safe to reclaim
// used resources.  The CleanupUsedPages() method must be invoked at this time so that the used pages can be
// scheduled for reuse after the fence has cleared.
// This is used in imgui to allocate the bakcing memory.

#pragma once

#include "CGPUBaseResource.h"
#include <vector>
#include <queue>
#include <mutex>

// Constant blocks must be multiples of 16 constants @ 16 bytes each - Refer to documentation on SM6 and HLSL
#define DEFAULT_ALIGN 256

// This manages dynamic allocation.
struct SDynAlloc
{
    SDynAlloc(CGPUResource& BaseResource, size_t ThisOffset, size_t ThisSize)
        : Buffer(BaseResource), Offset(ThisOffset), Size(ThisSize) {}

    CGPUResource& Buffer;	// The D3D buffer associated with this memory.
    size_t Offset;			// Offset from start of buffer resource
    size_t Size;			// Reserved size of this allocation
    void* DataPtr;			// The CPU-writeable address
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// The GPU-visible address
};

/// <summary>
/// Linear Allocation block are a resource that is allocated inside of the linear memory allocators
/// these work concurrently and derievbe from the GPU resource class.
/// 
/// </summary>
class ClinearAllocationBlock : public CGPUResource
{
public:
    ClinearAllocationBlock(ID3D12Resource2* pResource, D3D12_RESOURCE_STATES Usage) : CGPUResource()
    {
        m_pResource.Attach(pResource);
        m_UsageState = Usage;
        m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
        m_pResource->Map(0, nullptr, &m_CpuVirtualAddress);
    }

    ~ClinearAllocationBlock()
    {
        Unmap();
    }

    void Map(void)
    {
        if (m_CpuVirtualAddress == nullptr)
        {
            m_pResource->Map(0, nullptr, &m_CpuVirtualAddress);
        }
    }

    void Unmap(void)
    {
        if (m_CpuVirtualAddress != nullptr)
        {
            m_pResource->Unmap(0, nullptr);
            m_CpuVirtualAddress = nullptr;
        }
    }

    void* m_CpuVirtualAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};

enum ELinearAllocatorType
{
    kInvalidAllocator = -1,

    kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
    kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

    kNumAllocatorTypes
};

enum 
{
    kGpuAllocatorBlockSize = 0x10000,	// 64K
    kCpuAllocatorBlockSize = 0x200000	// 2MB
};

class CLinearAllocatorManager
{
public:

    CLinearAllocatorManager();
    ClinearAllocationBlock* FetchBlock( void );
    ClinearAllocationBlock* AllocateNewBlock( size_t PageSize = 0 );

    // Discarded pages will get recycled.  This is for fixed size pages.
    void DiscardBlocks( uint64_t FenceID, const std::vector<ClinearAllocationBlock*>& Blocks );

    // Freed pages will be destroyed once their fence has passed.  This is for single-use,
    // "large" pages.
    

    /// <summary>
    /// Passes the array of pages by reference, as we want to copy the memory location
    /// </summary>
    /// <param name="FenceID"> The integer that identifies the fence </param>
    /// <param name="Pages"> The array of pages</param>
    void DestroyLargeBlocks( uint64_t FenceID, const std::vector<ClinearAllocationBlock*>& Blocks );

    void Destroy( void ) { m_BlockPool.clear(); }

private:

    static ELinearAllocatorType sm_AutoType;

    ELinearAllocatorType m_AllocationType;

private:

    std::vector<std::unique_ptr<ClinearAllocationBlock> > m_BlockPool;

    //These are std::queues, we are using these over a vector because I want execution to occur based upon creation order
    //This is so that the UI is not drawn underneath the rendered image.
    std::queue<std::pair<uint64_t, ClinearAllocationBlock*> > m_UsedBlocks;
    std::queue<std::pair<uint64_t, ClinearAllocationBlock*> > m_DeletionQueue;

    //These are the linear allocation blocks that are avaliable within the executing program.
    std::queue<ClinearAllocationBlock*> m_AvaliableBlocks;

private:
    //The mutex that we are locking to allow this to function
    std::mutex m_Mutex;
};

class CLinearAllocator
{
public:

    CLinearAllocator(ELinearAllocatorType Type) : m_AllocationType(Type), m_BlockSize(0), m_CurOffset(~(size_t)0), m_CurrentBlock(nullptr)
    {
        ASSERT(Type > kInvalidAllocator && Type < kNumAllocatorTypes);
        m_BlockSize = (Type == kGpuExclusive ? kGpuAllocatorBlockSize : kCpuAllocatorBlockSize);
    }

    SDynAlloc Allocate( size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN );

    void CleanupUsedBlocks( uint64_t FenceID );

    static void DestroyAll( void )
    {
        sm_BlockManager[0].Destroy();
        sm_BlockManager[1].Destroy();
    }

private:

    SDynAlloc AllocateLargeMemoryBlock( size_t SizeInBytes );

    static CLinearAllocatorManager sm_BlockManager[2];

    ELinearAllocatorType m_AllocationType;

    size_t m_BlockSize;
    size_t m_CurOffset;

private:

    ClinearAllocationBlock* m_CurrentBlock;

private:

    std::vector<ClinearAllocationBlock*> m_UsedBlocks;
    std::vector<ClinearAllocationBlock*> m_LargeBlockList;
};
