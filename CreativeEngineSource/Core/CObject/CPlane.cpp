#include "CPlane.h"

unsigned short CPlane::GetIndCount()
{
	return sizeof(PlaneIndicies) / sizeof(unsigned short);
}

unsigned int CPlane::GetVertexCount()
{
	return sizeof(PlaneVertexes) / sizeof(unsigned int);
}

UINT CPlane::MeshVertexBufferSize()
{
	return sizeof(PlaneVertexes);
}

UINT CPlane::MeshIndexBufferSize()
{
	return sizeof(PlaneIndicies);
}
