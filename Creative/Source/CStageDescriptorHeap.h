#pragma once
#include "CDescriptorHeap.h"
#include "CResourceDescriptorDelegate.h"

class CStageDescriptorHeap : public CDescriptorHeap
{

public:

    CStageDescriptorHeap(ComPtr<ID3D12Device> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE heapType, int numDescriptors);
    ~CStageDescriptorHeap() final;

    CResourceDescriptorDelegate GetNewHeapHandle();
    void UnbindHeapDelegate(CResourceDescriptorDelegate handle);

public:

    std::vector<int> FreeDescriptors;
    int m_CurrentDescriptorIndex;
    int m_ActiveHandleCount;

};

