#include "CConstantBuffer.h"

CConstantBuffer::CConstantBuffer(ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES usageState, int bufferSize, CResourceDescriptorDelegate* constantBufferViewhandle)
	:CGPUResource(Resource, usageState)
{
	
	LocalGPUAddress = Resource->GetGPUVirtualAddress();
	LocalBufferSize = bufferSize;
	LocalConstantBufferViewHandle = constantBufferViewhandle;

	LocalMappedBuffer = NULL;
	LocalResource->Map(0, NULL, reinterpret_cast<void**>(&LocalMappedBuffer));

}

CConstantBuffer::~CConstantBuffer()
{
	LocalResource->Unmap(0, NULL);
}

void CConstantBuffer::SetConstantBufferData(const void* BufferData, int bufferSize)
{
	assert(bufferSize <= LocalBufferSize);
	memcpy(LocalMappedBuffer, BufferData, bufferSize);
}
