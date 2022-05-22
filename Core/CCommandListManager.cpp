

#include "pch.h"
#include "CCommandListManager.h"

CCommandQueue::CCommandQueue(D3D12_COMMAND_LIST_TYPE Type) :
    m_Type(Type),
    m_CommandQueue(nullptr),
    m_pFence(nullptr),
    m_NextFenceValue((uint64_t)Type << 56 | 1),
    m_LastCompletedFenceValue((uint64_t)Type << 56),
    m_AllocatorPool(Type)
{
}

CCommandQueue::~CCommandQueue()
{
    Shutdown();
}

void CCommandQueue::Shutdown()
{
    if (m_CommandQueue == nullptr)
        return;

    m_AllocatorPool.Shutdown();

    CloseHandle(m_FenceEventHandle);

    m_pFence->Release();
    m_pFence = nullptr;

    m_CommandQueue->Release();
    m_CommandQueue = nullptr;
}

CCommandListManager::CCommandListManager() :
    m_Device(nullptr),
    m_GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
    m_ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
    m_CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY),
    m_BundleQueue(D3D12_COMMAND_LIST_TYPE_BUNDLE),
    m_UIQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)
{
}

CCommandListManager::~CCommandListManager()
{
    Shutdown();
}

void CCommandListManager::Shutdown()
{
    m_GraphicsQueue.Shutdown();
    m_UIQueue.Shutdown();
    m_ComputeQueue.Shutdown();
    m_CopyQueue.Shutdown();
    m_BundleQueue.Shutdown();
}

void CCommandQueue::Create(ID3D12Device8* pDevice)
{
    ASSERT(pDevice != nullptr);
    ASSERT(!IsReady());
    ASSERT(m_AllocatorPool.Size() == 0);

    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = m_Type;
    QueueDesc.NodeMask = 1;
    pDevice->CreateCommandQueue(&QueueDesc, MY_IID_PPV_ARGS(&m_CommandQueue));
    m_CommandQueue->SetName(L"CommandListManager::m_CommandQueue");

    ASSERT_SUCCEEDED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_pFence)));
    m_pFence->SetName(L"CommandListManager::m_pFence");
    m_pFence->Signal((uint64_t)m_Type << 56);

    m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    ASSERT(m_FenceEventHandle != NULL);

    m_AllocatorPool.Create(pDevice);

    ASSERT(IsReady());
}

void CCommandListManager::Create(ID3D12Device8* pDevice)
{
    ASSERT(pDevice != nullptr);

    m_Device = pDevice;

    m_GraphicsQueue.Create(pDevice);
    m_ComputeQueue.Create(pDevice);
    m_CopyQueue.Create(pDevice);
    m_UIQueue.Create(pDevice);
}

void CCommandListManager::CreateNewCommandList( D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator )
{
    switch (Type)
    {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: 
            *Allocator = m_GraphicsQueue.RequestAllocator(); 
            break;
        case D3D12_COMMAND_LIST_TYPE_BUNDLE: 
            *Allocator = m_BundleQueue.RequestAllocator();
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE: 
            *Allocator = m_ComputeQueue.RequestAllocator(); 
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY: 
            *Allocator = m_CopyQueue.RequestAllocator(); 
            break;
    }
    
    ASSERT_SUCCEEDED( m_Device->CreateCommandList(1, Type, *Allocator, nullptr, MY_IID_PPV_ARGS(List)) );
    (*List)->SetName(L"CommandList");
}

void CCommandListManager::CreateUICommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
{
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		*Allocator = m_UIQueue.RequestAllocator();
		break;
    default:
        throw new std::exception;
        break;
	}

	ASSERT_SUCCEEDED(m_Device->CreateCommandList(1, Type, *Allocator, nullptr, MY_IID_PPV_ARGS(List)));
	(*List)->SetName(L"UI CommandList");


}

uint64_t CCommandQueue::ExecuteCommandList( ID3D12CommandList* List )
{
    std::lock_guard<std::mutex> LockGuard(m_FenceMutex);

    assert(List != nullptr);

    ASSERT_SUCCEEDED(((ID3D12GraphicsCommandList*)List)->Close());

    m_CommandQueue->ExecuteCommandLists(1, &List);

    m_CommandQueue->Signal(m_pFence, m_NextFenceValue);
    return m_NextFenceValue++;
}

uint64_t CCommandQueue::IncrementFence(void)
{
    std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
    m_CommandQueue->Signal(m_pFence, m_NextFenceValue);
    return m_NextFenceValue++;
}

bool CCommandQueue::IsFenceComplete(uint64_t FenceValue)
{
    if (FenceValue > m_LastCompletedFenceValue)
        m_LastCompletedFenceValue = std::max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());

    return FenceValue <= m_LastCompletedFenceValue;
}

namespace CGraphics
{
    extern CCommandListManager g_CommandManager;
}

void CCommandQueue::StallForFence(uint64_t FenceValue)
{
    CCommandQueue& Producer = CGraphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
    m_CommandQueue->Wait(Producer.m_pFence, FenceValue);
}

void CCommandQueue::StallForProducer(CCommandQueue& Producer)
{
    ASSERT(Producer.m_NextFenceValue > 0);
    m_CommandQueue->Wait(Producer.m_pFence, Producer.m_NextFenceValue - 1);
}

void CCommandQueue::WaitForFence(uint64_t FenceValue)
{
    if (IsFenceComplete(FenceValue))
        return;

    {
        std::lock_guard<std::mutex> LockGuard(m_EventMutex);

        m_pFence->SetEventOnCompletion(FenceValue, m_FenceEventHandle);
        WaitForSingleObject(m_FenceEventHandle, INFINITE);
        m_LastCompletedFenceValue = FenceValue;
    }
}

void CCommandListManager::WaitForFence(uint64_t FenceValue)
{
    CCommandQueue& Producer = CGraphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
    Producer.WaitForFence(FenceValue);
}

ID3D12CommandAllocator* CCommandQueue::RequestAllocator()
{
    uint64_t CompletedFence = m_pFence->GetCompletedValue();

    return m_AllocatorPool.RequestAllocator(CompletedFence);
}

void CCommandQueue::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
{
    m_AllocatorPool.DiscardAllocator(FenceValue, Allocator);
}
