#include "CCylinder.h"

unsigned short CCylinder::GetIndCount()
{
    return sizeof(CylinderIndicies) / sizeof(unsigned short);
}

unsigned int CCylinder::GetVertexCount()
{
    return sizeof(CylinderVertexes) / sizeof(unsigned int);
}

UINT CCylinder::MeshVertexBufferSize()
{
    return sizeof(CylinderVertexes);
}

UINT CCylinder::MeshIndexBufferSize()
{
    return sizeof(CylinderIndicies);
}
