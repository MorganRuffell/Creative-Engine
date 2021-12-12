#include "CConstantBuffer.h"
#include "pch.h"




CConstantBuffer::~CConstantBuffer()
	
{
	LocalResource->Unmap(0, NULL);
}

void CConstantBuffer::SetConstantBufferData(const void* BufferData, int bufferSize)
{
	assert(bufferSize <= LocalBufferSize);
	memcpy(LocalMappedBuffer, BufferData, bufferSize);
}
