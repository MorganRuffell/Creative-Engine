#include "CVertexBuffer.h"

CVertexBuffer::CVertexBuffer (ComPtr<ID3D12Resource> resource, ComPtr<ID3D12Device8>& GraphicsCard, D3D12_RESOURCE_STATES usageState, int vertexStride, int bufferSize)
	:CGPUResource(resource,usageState)
{
	if (resource == nullptr)
	{
		ComPtr<ID3D12Resource> VertexBufferResource = NULL;
		D3D12_RESOURCE_DESC* VertexBufferDesc = {};

		VertexBufferDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		VertexBufferDesc->Alignment = 0;
		VertexBufferDesc->Width = bufferSize;
		VertexBufferDesc->Height = 1;
		VertexBufferDesc->DepthOrArraySize = 1;
		VertexBufferDesc->MipLevels = 1;
		VertexBufferDesc->Format = DXGI_FORMAT_UNKNOWN;
		VertexBufferDesc->SampleDesc.Count = 1;
		VertexBufferDesc->SampleDesc.Quality = 0;
		VertexBufferDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		VertexBufferDesc->Flags = D3D12_RESOURCE_FLAG_NONE;
		
		D3D12_HEAP_PROPERTIES* DefaultHeapProperties = {};

		DefaultHeapProperties->Type = D3D12_HEAP_TYPE_DEFAULT;
		DefaultHeapProperties->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		DefaultHeapProperties->MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		DefaultHeapProperties->CreationNodeMask = 0;
		DefaultHeapProperties->VisibleNodeMask = 0;


		HRESULT x = GraphicsCard->CreateCommittedResource(DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, VertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&VertexBufferResource));
		if (FAILED(x))
		{
			throw new CGraphicsException;
		}

		LocalGPUAddress = VertexBufferResource->GetGPUVirtualAddress();
		LocalVertexBufferView.StrideInBytes = vertexStride;
		LocalVertexBufferView.SizeInBytes = bufferSize;
		LocalVertexBufferView.BufferLocation = LocalGPUAddress;

	}
	
	LocalGPUAddress = resource->GetGPUVirtualAddress();
	LocalVertexBufferView.StrideInBytes = vertexStride;
	LocalVertexBufferView.SizeInBytes = bufferSize;
	LocalVertexBufferView.BufferLocation = LocalGPUAddress;
}

CVertexBuffer::~CVertexBuffer()
{

}

void CVertexBuffer::SetVertexBufferView(D3D12_VERTEX_BUFFER_VIEW* VertexBufferView)
{
	if (VertexBufferView == nullptr)
	{
		throw new CGraphicsException;
	}


	LocalVertexBufferView = *VertexBufferView;
}
