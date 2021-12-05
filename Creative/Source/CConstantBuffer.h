#pragma once
#include "CGPUResource.h"
#include "CResourceDescriptorDelegate.h"

class CRGraphicsAPI CConstantBuffer : public CGPUResource
{
public:	
	//We need to create a descriptor heap handle class.
	CConstantBuffer(ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES usageState, int bufferSize, CResourceDescriptorDelegate* constantBufferViewhandle);
	~CConstantBuffer() override;

	void SetConstantBufferData(const void* BufferData, int bufferSize);
	CResourceDescriptorDelegate* GetConstantBufferViewHandle() { return LocalConstantBufferViewHandle; }

private:

	void* LocalMappedBuffer;
	int LocalBufferSize;
	CResourceDescriptorDelegate* LocalConstantBufferViewHandle;
};

