

#pragma once

#include "VectorMath.h"

namespace CMath
{
    class CPlaneCollider
    {
    public:

        CPlaneCollider() {}
        CPlaneCollider( CVector3 normalToPlane, float distanceFromOrigin ) : m_repr(normalToPlane, distanceFromOrigin) {}
        CPlaneCollider( CVector3 pointOnPlane, CVector3 normalToPlane );
        CPlaneCollider( float A, float B, float C, float D ) : m_repr(A, B, C, D) {}
        CPlaneCollider( const CPlaneCollider& plane ) : m_repr(plane.m_repr) {}
        explicit CPlaneCollider( CVector4 plane ) : m_repr(plane) {}

        INLINE operator CVector4() const { return m_repr; }

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
        CScalar DistanceFromPoint(CVector4 point) const
        {
            return Dot(point, m_repr);
        }

        // Most efficient way to transform a plane.  (Involves one quaternion-vector rotation and one dot product.)
        friend CPlaneCollider operator* ( const CMOrthoTransformation& xform, CPlaneCollider plane )
        {
            CVector3 normalToPlane = xform.GetRotation() * plane.GetNormal();
            float distanceFromOrigin = plane.m_repr.GetW() - Dot(normalToPlane, xform.GetTranslation());
            return CPlaneCollider(normalToPlane, distanceFromOrigin);
        }

        // Less efficient way to transform a plane (but handles affine transformations.)
        friend CPlaneCollider operator* ( const CMatrix4& mat, CPlaneCollider plane )
        {
            return CPlaneCollider( Transpose(Invert(mat)) * plane.m_repr );
        }

    private:

        CVector4 m_repr;
    };

    inline CPlaneCollider::CPlaneCollider( CVector3 pointOnPlane, CVector3 normalToPlane )
    {
        // Guarantee a normal.  This constructor isn't meant to be called frequently, but if it is, we can change this.
        normalToPlane = Normalize(normalToPlane);	
        m_repr = CVector4(normalToPlane, -Dot(pointOnPlane, normalToPlane));
    }

    inline CPlaneCollider PlaneFromPointsCCW( CVector3 A, CVector3 B, CVector3 C )
    {
        return CPlaneCollider( A, Cross(B - A, C - A) );
    }


} 
