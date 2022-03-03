#include "CPyramid.h"

unsigned short CPyramid::GetIndCount()
{
    return sizeof(PyramidIndicies) / sizeof (unsigned short);
}

unsigned int CPyramid::GetVertexCount()
{
    return sizeof(PyramidVertexes) / sizeof(unsigned int);
}

UINT CPyramid::MeshVertexBufferSize()
{
    return sizeof(PyramidVertexes);
}

UINT CPyramid::MeshIndexBufferSize()
{
    return sizeof(PyramidIndicies);
}
