#pragma once

#include "CMathCore.h"
#include "CMeshBase.h"
#include "ShaderStructures.h"
#include "d3dx12.h"
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

//Base class for all meshes that do not move in creative.
class CRGraphicsAPI CStaticMesh : public CMeshBase
{
public:

	CStaticMesh();
	~CStaticMesh();

	virtual unsigned short GetIndCount() = 0;
	virtual unsigned int GetVertexCount() = 0;

	virtual CRE::CVertex* GetVertexes() = 0;
	virtual unsigned short* GetIndicies() = 0;

	void DrawIndicies(CRE::CVertex* Vertexes);


	virtual UINT MeshVertexBufferSize();
	virtual UINT MeshIndexBufferSize();

protected:

	UINT SizeOfVertexBuffer;
	UINT SizeOfIndexBuffer;

public:

	//Buffer used to upload data to the GPU about this static mesh 
	ComPtr<ID3D12Resource2> UploadBuffer;

	//Optional Buffer used as and when needed
	ComPtr<ID3D12Resource2> OptionalBuffer;
};

