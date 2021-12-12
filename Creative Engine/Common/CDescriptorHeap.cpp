#include "CDescriptorHeap.h"
#include "pch.h"

CDescriptorHeap::CDescriptorHeap(ComPtr<ID3D12Device> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors, bool isReferencedByShader)
{
	HeapType = heapType;
	Local_MaxDescriptors = numDescriptors;
	IsReferencedByShader = isReferencedByShader;

	D3D12_DESCRIPTOR_HEAP_DESC HeapController;
	HeapController.NumDescriptors = Local_MaxDescriptors;
	HeapController.Type = HeapType;
	HeapController.Flags = IsReferencedByShader ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (IsReferencedByShader && GraphicsCard != nullptr)
	{
		GPUData = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		HRESULT x = GraphicsCard->CreateDescriptorHeap(&HeapController, IID_PPV_ARGS(&mDescriptorHeap));
		if (FAILED(x))
		{

		}

		CPUData = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Local_DescriptorSize = GraphicsCard->GetDescriptorHandleIncrementSize(HeapType);
	}
	else
	{
		throw new CGraphicsException;
	}	
}

CDescriptorHeap::~CDescriptorHeap()
{
	mDescriptorHeap->Release();
	if (mDescriptorHeap != NULL)
	{
		throw new CGraphicsException;
	}
}
