#include "CWedge.h"

unsigned short CWedge::GetIndCount()
{
	return sizeof(WedgeIndicies) / sizeof(unsigned short);
}

unsigned int CWedge::GetVertexCount()
{
	return sizeof(WedgeVertexes) / sizeof(unsigned int);
}

UINT CWedge::MeshVertexBufferSize()
{
	return sizeof(WedgeVertexes);
}

UINT CWedge::MeshIndexBufferSize()
{
	return sizeof(WedgeIndicies);
}
