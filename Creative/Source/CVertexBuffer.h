#include "CGPUResource.h"


class CRGraphicsAPI CVertexBuffer : public CGPUResource 
{

public:
	CVertexBuffer(ComPtr<ID3D12Resource> resource, ComPtr<ID3D12Device8>& GraphicsCard, D3D12_RESOURCE_STATES usageState, int vertexStride, int bufferSize);
	~CVertexBuffer() override;

public:

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() 
	{
		return LocalVertexBufferView;
	}

	void SetVertexBufferView(D3D12_VERTEX_BUFFER_VIEW* VertexBufferView);

private:

	D3D12_VERTEX_BUFFER_VIEW LocalVertexBufferView;
};


