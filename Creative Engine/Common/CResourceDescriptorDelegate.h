#pragma once
#include "CGraphicsBase.h"

class CResourceDescriptorDelegate : public CGraphicsBase
{
public:

	CResourceDescriptorDelegate()
	{
		LocalCPUHandle.ptr = NULL;
		LocalGPUHandle.ptr = NULL;
		HeapIndex = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return LocalCPUHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return LocalGPUHandle; }
	int GetHeapIndex() { return HeapIndex; }

public:

	void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle) { LocalCPUHandle = CPUHandle; }
	void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle) { LocalGPUHandle = GPUHandle; }
	void SetHeapIndex(int n_HeapIndex) { HeapIndex = n_HeapIndex; }

	bool IsCPUValid() { return LocalCPUHandle.ptr != NULL; }
	bool IsGPUValid() { return LocalGPUHandle.ptr != NULL; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE LocalCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE LocalGPUHandle;
	int HeapIndex;
};

