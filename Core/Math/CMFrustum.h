

#pragma once

#include "CPlaneCollider.h"
#include "CSphereCollider.h"
#include "CBoundingBox.h"

namespace CMath
{
    class CMFrustum
    {
    public:
        CMFrustum() {}

        CMFrustum( const CMatrix4& ProjectionMatrix );

        enum ECornerID
        {
            kNearLowerLeft, 
            kNearUpperLeft, 
            kNearLowerRight, 
            kNearUpperRight,
            kFarLowerLeft,  
            kFarUpperLeft,  
            kFarLowerRight,  
            kFarUpperRight
        };

        enum EPlaneID
        {
            kNearPlane, 
            kFarPlane, 
            kLeftPlane, 
            kRightPlane, 
            kTopPlane, 
            kBottomPlane
        };

        CVector3         GetFrustumCorner( ECornerID id ) const   { return m_FrustumCorners[id]; }
        CPlaneCollider   GetFrustumPlane( EPlaneID id ) const     { return m_FrustumPlanes[id]; }

        // Test whether the bounding sphere intersects the frustum.  Intersection is defined as either being
        // fully contained in the frustum, or by intersecting one or more of the planes.
        bool IntersectSphere( CSphereCollider sphere ) const;

        bool IntersectBoundingBox(const CAxisAlignedBox& aabb) const;

        friend CMFrustum  operator* ( const CMOrthoTransformation& xform, const CMFrustum& frustum );	// Fast
        friend CMFrustum  operator* ( const CMAffineTransform& xform, const CMFrustum& frustum );		// Slow
        friend CMFrustum  operator* ( const CMatrix4& xform, const CMFrustum& frustum );				// Slowest (and most general)

    private:

        // Perspective frustum constructor (for pyramid-shaped frusta)
        void ConstructPerspectiveFrustum( float HTan, float VTan, float NearClip, float FarClip );

        // Orthographic frustum constructor (for box-shaped frusta)
        void ConstructOrthographicFrustum( float Left, float Right, float Top, float Bottom, float NearClip, float FarClip );

        CVector3 m_FrustumCorners[8];		// the corners of the frustum
        CPlaneCollider m_FrustumPlanes[6];			// the bounding planes
    };

    inline bool CMFrustum::IntersectSphere( CSphereCollider sphere ) const
    {
        float radius = sphere.GetRadius();
        for (int i = 0; i < 6; ++i)
        {
            if (m_FrustumPlanes[i].DistanceFromPoint(sphere.GetCenter()) + radius < 0.0f)
                return false;
        }
        return true;
    }

    inline bool CMFrustum::IntersectBoundingBox(const CAxisAlignedBox& aabb) const
    {
        for (int i = 0; i < 6; ++i)
        {
            CPlaneCollider p = m_FrustumPlanes[i];
            CVector3 farCorner = Select(aabb.GetMin(), aabb.GetMax(), p.GetNormal() > CVector3(kZero));
            if (p.DistanceFromPoint(farCorner) < 0.0f)
                return false;
        }

        return true;
    }

    inline CMFrustum operator* ( const CMOrthoTransformation& xform, const CMFrustum& frustum )
    {
        CMFrustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = xform * frustum.m_FrustumPlanes[i];

        return result;
    }

    inline CMFrustum operator* ( const CMAffineTransform& xform, const CMFrustum& frustum )
    {
        CMFrustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        CMatrix4 XForm = Transpose(Invert(CMatrix4(xform)));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = CPlaneCollider(XForm * CVector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

    inline CMFrustum operator* ( const CMatrix4& mtx, const CMFrustum& frustum )
    {
        CMFrustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = CVector3( mtx * frustum.m_FrustumCorners[i] );

        CMatrix4 XForm = Transpose(Invert(mtx));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = CPlaneCollider(XForm * CVector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

} 
