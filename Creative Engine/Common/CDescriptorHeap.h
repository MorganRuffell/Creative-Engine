#pragma once
#include "CGraphicsBase.h"
#include "CException.h"

using Microsoft::WRL::ComPtr;

class CDescriptorHeap : public 
{
public:
	CDescriptorHeap(ComPtr<ID3D12Device> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors, bool isReferencedByShader)
		: CGPUResource(Resource, usageState)
	{

		LocalGPUAddress = Resource->GetGPUVirtualAddress();
		LocalBufferSize = bufferSize;
		LocalConstantBufferViewHandle = constantBufferViewhandle;

		LocalMappedBuffer = NULL;
		LocalResource->Map(0, NULL, reinterpret_cast<void**>(&LocalMappedBuffer));

	}
	virtual ~CDescriptorHeap();

	ComPtr<ID3D12DescriptorHeap> GetHeap() { return mDescriptorHeap; }

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return HeapType; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetHeapCPUStart() { return CPUData; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetHeapGPUStart() { return GPUData; }
	int GetMaxDescriptors() { return Local_MaxDescriptors; }
	int GetDescriptorSize() { return Local_DescriptorSize; }

protected:

	ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;

	D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
	D3D12_CPU_DESCRIPTOR_HANDLE CPUData;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUData;

	int Local_MaxDescriptors;
	int  Local_DescriptorSize;
	bool IsReferencedByShader;

};

