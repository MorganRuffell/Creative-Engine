
#pragma once

#include "VectorMath.h"
#include "Transform.h"

namespace CMath
{
    class CAxisAlignedBox
    {
    public:
        CAxisAlignedBox() : m_min(FLT_MAX, FLT_MAX, FLT_MAX), m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
        CAxisAlignedBox(EZeroTag) : m_min(FLT_MAX, FLT_MAX, FLT_MAX), m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
        CAxisAlignedBox( CVector3 min, CVector3 max ) : m_min(min), m_max(max) {}

        void AddPoint( CVector3 point )
        {
            m_min = Min(point, m_min);
            m_max = Max(point, m_max);
        }

        void AddBoundingBox( const CAxisAlignedBox& box )
        {
            AddPoint(box.m_min);
            AddPoint(box.m_max);
        }

        CAxisAlignedBox Union( const CAxisAlignedBox& box )
        {
            return CAxisAlignedBox(Min(m_min, box.m_min), Max(m_max, box.m_max));
        }

        CVector3 GetMin() const { return m_min; }
        CVector3 GetMax() const { return m_max; }
        CVector3 GetCenter() const { return (m_min + m_max) * 0.5f; }
        CVector3 GetDimensions() const { return Max(m_max - m_min, CVector3(kZero)); }

    private:

        CVector3 m_min;
        CVector3 m_max;
    };

    class OrientedBox
    {
    public:
        OrientedBox() {}

        OrientedBox( const CAxisAlignedBox& box )
        {
            m_repr.SetBasis(CMatrix3::MakeScale(box.GetMax() - box.GetMin()));
            m_repr.SetTranslation(box.GetMin());
        }

        friend OrientedBox operator* (const CMAffineTransform& xform, const OrientedBox& obb )
        {
            return (OrientedBox&)(xform * obb.m_repr);
        }

        CVector3 GetDimensions() const { return m_repr.GetX() + m_repr.GetY() + m_repr.GetZ(); }
        CVector3 GetCenter() const { return m_repr.GetTranslation() + GetDimensions() * 0.5f; }

    private:
        CMAffineTransform m_repr;
    };

    INLINE OrientedBox operator* (const CMUniformTransform& xform, const OrientedBox& obb )
    {
        return CMAffineTransform(xform) * obb;
    }

    INLINE OrientedBox operator* (const CMUniformTransform& xform, const CAxisAlignedBox& aabb )
    {
        return CMAffineTransform(xform) * OrientedBox(aabb);
    }

} 
