
#pragma once

#include "glTF.h"
#include "../Core/Math/CSphereCollider.h"
#include "../Core/Math/CBoundingBox.h"

#include <cstdint>
#include <string>

namespace CBaseRenderer
{
    using namespace CMath;

    struct Primitive
    {
        CSphereCollider m_BoundsLS;  // local space bounds
        CSphereCollider m_BoundsOS;  // object space bounds
        CAxisAlignedBox m_BBoxLS;       // local space AABB
        CAxisAlignedBox m_BBoxOS;       // object space AABB
        CUtility::CSmartByteVector VB;
        CUtility::CSmartByteVector IB;
        CUtility::CSmartByteVector DepthVB;
        uint32_t primCount;
        union
        {
            uint32_t hash;
            struct {
                uint32_t psoFlags : 16;
                uint32_t index32 : 1;
                uint32_t materialIdx : 15;
            };
        };
        uint16_t vertexStride;
    };
}

void OptimizeMesh( CBaseRenderer::Primitive& outPrim, const glTF::Primitive& inPrim, const CMath::CMatrix4& localToObject );