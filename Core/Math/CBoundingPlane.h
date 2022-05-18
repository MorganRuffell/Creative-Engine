

#pragma once

#include "VectorMath.h"

namespace CMath
{
    class CBoundingPlane
    {
    public:

        CBoundingPlane() {}
        CBoundingPlane( CVector3 normalToPlane, float distanceFromOrigin ) : m_repr(normalToPlane, distanceFromOrigin) {}
        CBoundingPlane( CVector3 pointOnPlane, CVector3 normalToPlane );
        CBoundingPlane( float A, float B, float C, float D ) : m_repr(A, B, C, D) {}
        CBoundingPlane( const CBoundingPlane& plane ) : m_repr(plane.m_repr) {}
        explicit CBoundingPlane( Vector4 plane ) : m_repr(plane) {}

        INLINE operator Vector4() const { return m_repr; }

        // Returns the direction the plane is facing.  (Warning:  might not be normalized.)
        CVector3 GetNormal( void ) const { return CVector3(XMVECTOR(m_repr)); }

        // Returns the point on the plane closest to the origin
        CVector3 GetPointOnPlane( void ) const { return -GetNormal() * m_repr.GetW(); }

        // Distance from 3D point
        CScalar DistanceFromPoint( CVector3 point ) const
        {
            return Dot(point, GetNormal()) + m_repr.GetW();
        }

        // Distance from homogeneous point 
        CScalar DistanceFromPoint(Vector4 point) const
        {
            return Dot(point, m_repr);
        }

        // Most efficient way to transform a plane.  (Involves one quaternion-vector rotation and one dot product.)
        friend CBoundingPlane operator* ( const CMOrthoTransformation& xform, CBoundingPlane plane )
        {
            CVector3 normalToPlane = xform.GetRotation() * plane.GetNormal();
            float distanceFromOrigin = plane.m_repr.GetW() - Dot(normalToPlane, xform.GetTranslation());
            return CBoundingPlane(normalToPlane, distanceFromOrigin);
        }

        // Less efficient way to transform a plane (but handles affine transformations.)
        friend CBoundingPlane operator* ( const CMatrix4& mat, CBoundingPlane plane )
        {
            return CBoundingPlane( Transpose(Invert(mat)) * plane.m_repr );
        }

    private:

        Vector4 m_repr;
    };

    
    inline CBoundingPlane::CBoundingPlane( CVector3 pointOnPlane, CVector3 normalToPlane )
    {
        // Guarantee a normal.  This constructor isn't meant to be called frequently, but if it is, we can change this.
        normalToPlane = Normalize(normalToPlane);	
        m_repr = Vector4(normalToPlane, -Dot(pointOnPlane, normalToPlane));
    }


    inline CBoundingPlane PlaneFromPointsCCW( CVector3 A, CVector3 B, CVector3 C )
    {
        return CBoundingPlane( A, Cross(B - A, C - A) );
    }
} 
