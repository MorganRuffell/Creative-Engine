#include "CStageDescriptorHeap.h"
#include <memory>


CStageDescriptorHeap::CStageDescriptorHeap(ComPtr<ID3D12Device> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors)
	:CDescriptorHeap(GraphicsCard,heapType, numDescriptors, false)
{
	m_CurrentDescriptorIndex = 0;
	m_ActiveHandleCount = 0;
}

CStageDescriptorHeap::~CStageDescriptorHeap()
{
	if (m_ActiveHandleCount != 0)
	{
		throw new CGraphicsException;
	}

	FreeDescriptors.clear();
}

CResourceDescriptorDelegate CStageDescriptorHeap::GetNewHeapHandle()
{
	int newHandleID = 0;


	if (m_CurrentDescriptorIndex < Local_MaxDescriptors)
	{
		newHandleID = m_CurrentDescriptorIndex;
		m_CurrentDescriptorIndex++;
	}
	else if (FreeDescriptors.size() > 0)
	{
		newHandleID = FreeDescriptors.back();
		FreeDescriptors.pop_back();
	}
	else
	{
		throw new CGraphicsException;
		//If this is thrown, you need to increase heap size out of delegates!
	}


	CResourceDescriptorDelegate Delegate;
	D3D12_CPU_DESCRIPTOR_HANDLE* CPUHandle = &CPUData;
	D3D12_GPU_DESCRIPTOR_HANDLE* GPUHandle = &GPUData;

	CPUHandle->ptr += newHandleID * Local_DescriptorSize;
	Delegate.SetCPUHandle(*CPUHandle);
	Delegate.SetGPUHandle(*GPUHandle);
	Delegate.SetHeapIndex(newHandleID);
	m_ActiveHandleCount++;

	return CResourceDescriptorDelegate();
}

void CStageDescriptorHeap::UnbindHeapDelegate(CResourceDescriptorDelegate handle)
{
	FreeDescriptors.push_back(handle.GetHeapIndex());

	if (m_ActiveHandleCount == 0)
	{
		throw new CGraphicsException;
	}

	m_ActiveHandleCount--;
}
