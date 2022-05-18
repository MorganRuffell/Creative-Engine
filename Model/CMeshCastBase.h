#pragma once
#include <FileUtility.h>

//Base class for Mesh Casting -- A Process
//that is used for importing meshes -- regardless of file type
class CMeshCastBase
{
public:

    CUtility::CSmartByteVector VB;
    CUtility::CSmartByteVector IB;
    CUtility::CSmartByteVector DepthVB;

public:

    uint32_t primCount;

public:

    union
    {
        uint32_t hash;
        struct {
            uint32_t psoFlags : 16;
            uint32_t index32 : 1;
            uint32_t materialIdx : 15;
        };
    };

};

