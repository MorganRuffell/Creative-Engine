#include "CStaticMesh.h"
#include "CException.h"

CStaticMesh::CStaticMesh()
{

}

CStaticMesh::~CStaticMesh()
{
}


void CStaticMesh::DrawIndicies(CRE::CVertex* Vertexes)
{
	for (UINT i = 0; i < MeshIndexBufferSize(); i++)
	{

	}

}


UINT CStaticMesh::MeshVertexBufferSize()
{
	return 0;
}

UINT CStaticMesh::MeshIndexBufferSize()
{
	return 0;
}

