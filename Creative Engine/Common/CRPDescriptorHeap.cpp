#include "CRPDescriptorHeap.h"

CRPDescriptorHeap::CRPDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, int NumberOfDescriptors)
	: CDescriptorHeap(GraphicsCard, HeapType, NumberOfDescriptors,true)
{
	CurrentDescriptorIndex = 0;
}

CRPDescriptorHeap::~CRPDescriptorHeap()
{
}

void CRPDescriptorHeap::Reset()
{
	CurrentDescriptorIndex = 0;
}

CResourceDescriptorDelegate CRPDescriptorHeap::GetHeapHandleBlock(int count)
{
	int newHandleID = 0;
	int MemoryBlockEnd = CurrentDescriptorIndex + count;

	if (MemoryBlockEnd < Local_MaxDescriptors)
	{
		newHandleID = CurrentDescriptorIndex;
		CurrentDescriptorIndex = MemoryBlockEnd;
	}
	else
	{
		throw new CGraphicsException;
		//If this is thrown we've run out of delegates! Need to increase heap size
	}

	CResourceDescriptorDelegate newHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuDelegate = CPUData;
	CpuDelegate.ptr += newHandleID * Local_DescriptorSize;
	newHandle.SetCPUHandle(CpuDelegate);


	D3D12_GPU_DESCRIPTOR_HANDLE GpuDelegate = GPUData;
	GpuDelegate.ptr += newHandleID * Local_DescriptorSize;
	newHandle.SetGPUHandle(GpuDelegate);

	newHandle.SetHeapIndex(newHandleID);

	return newHandle;
}
