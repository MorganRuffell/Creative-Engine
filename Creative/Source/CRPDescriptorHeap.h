#pragma once
#include "CDescriptorHeap.h"
#include "CResourceDescriptorDelegate.h"


class CRGraphicsAPI CRPDescriptorHeap : public CDescriptorHeap
{
public:

	CRPDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, int NumberOfDescriptors);

	~CRPDescriptorHeap() final;

	void Reset();
	CResourceDescriptorDelegate GetHeapHandleBlock(int count);

private:

	int CurrentDescriptorIndex;

};

