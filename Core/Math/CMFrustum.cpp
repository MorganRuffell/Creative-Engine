

#include "pch.h"
#include "CMFrustum.h"
#include "CViewportCamera.h"

using namespace CMath;

void CMFrustum::ConstructPerspectiveFrustum( float HTan, float VTan, float NearClip, float FarClip )
{
    const float NearX = HTan * NearClip;
    const float NearY = VTan * NearClip;
    const float FarX = HTan * FarClip;
    const float FarY = VTan * FarClip;

    // Define the frustum corners
    m_FrustumCorners[ kNearLowerLeft  ] = CVector3(-NearX, -NearY, -NearClip);	// Near lower left
    m_FrustumCorners[ kNearUpperLeft  ] = CVector3(-NearX,  NearY, -NearClip);	// Near upper left
    m_FrustumCorners[ kNearLowerRight ] = CVector3( NearX, -NearY, -NearClip);	// Near lower right
    m_FrustumCorners[ kNearUpperRight ] = CVector3( NearX,  NearY, -NearClip);	// Near upper right
    m_FrustumCorners[ kFarLowerLeft   ] = CVector3( -FarX,  -FarY,  -FarClip);	// Far lower left
    m_FrustumCorners[ kFarUpperLeft   ] = CVector3( -FarX,   FarY,  -FarClip);	// Far upper left
    m_FrustumCorners[ kFarLowerRight  ] = CVector3(  FarX,  -FarY,  -FarClip);	// Far lower right
    m_FrustumCorners[ kFarUpperRight  ] = CVector3(  FarX,   FarY,  -FarClip);	// Far upper right

    const float NHx = RecipSqrt( 1.0f + HTan * HTan );
    const float NHz = -NHx * HTan;
    const float NVy = RecipSqrt( 1.0f + VTan * VTan );
    const float NVz = -NVy * VTan;

    // Define the bounding planes
    m_FrustumPlanes[kNearPlane]		= CPlaneCollider( 0.0f, 0.0f, -1.0f, -NearClip );
    m_FrustumPlanes[kFarPlane]		= CPlaneCollider( 0.0f, 0.0f,  1.0f,   FarClip );
    m_FrustumPlanes[kLeftPlane]		= CPlaneCollider(  NHx, 0.0f,   NHz,      0.0f );
    m_FrustumPlanes[kRightPlane]	= CPlaneCollider( -NHx, 0.0f,   NHz,      0.0f );
    m_FrustumPlanes[kTopPlane]		= CPlaneCollider( 0.0f, -NVy,   NVz,      0.0f );
    m_FrustumPlanes[kBottomPlane]	= CPlaneCollider( 0.0f,  NVy,   NVz,      0.0f );
}

void CMFrustum::ConstructOrthographicFrustum( float Left, float Right, float Top, float Bottom, float Front, float Back )
{
    // Define the frustum corners
    m_FrustumCorners[ kNearLowerLeft  ] = CVector3(Left,   Bottom,	-Front);	// Near lower left
    m_FrustumCorners[ kNearUpperLeft  ] = CVector3(Left,   Top,		-Front);	// Near upper left
    m_FrustumCorners[ kNearLowerRight ] = CVector3(Right,  Bottom,	-Front);	// Near lower right
    m_FrustumCorners[ kNearUpperRight ] = CVector3(Right,  Top,		-Front);	// Near upper right
    m_FrustumCorners[ kFarLowerLeft   ] = CVector3(Left,   Bottom,	 -Back);	// Far lower left
    m_FrustumCorners[ kFarUpperLeft   ] = CVector3(Left,   Top,		 -Back);	// Far upper left
    m_FrustumCorners[ kFarLowerRight  ] = CVector3(Right,  Bottom,	 -Back);	// Far lower right
    m_FrustumCorners[ kFarUpperRight  ] = CVector3(Right,  Top,		 -Back);	// Far upper right

    // Define the bounding planes
    m_FrustumPlanes[kNearPlane]		= CPlaneCollider(  0.0f,  0.0f, -1.0f, -Front );
    m_FrustumPlanes[kFarPlane]		= CPlaneCollider(  0.0f,  0.0f,  1.0f,   Back );
    m_FrustumPlanes[kLeftPlane]		= CPlaneCollider(  1.0f,  0.0f,  0.0f,  -Left );
    m_FrustumPlanes[kRightPlane]	= CPlaneCollider( -1.0f,  0.0f,  0.0f,  Right );
    m_FrustumPlanes[kTopPlane]		= CPlaneCollider(  0.0f, -1.0f,  0.0f, Bottom );
    m_FrustumPlanes[kBottomPlane]	= CPlaneCollider(  0.0f,  1.0f,  0.0f,   -Top );
}


CMFrustum::CMFrustum( const CMatrix4& ProjMat )
{
    const float* ProjMatF = (const float*)&ProjMat;

    const float RcpXX = 1.0f / ProjMatF[ 0];
    const float RcpYY = 1.0f / ProjMatF[ 5];
    const float RcpZZ = 1.0f / ProjMatF[10];

    // Identify if the projection is perspective or orthographic by looking at the 4th row.
    if (ProjMatF[3] == 0.0f && ProjMatF[7] == 0.0f && ProjMatF[11] == 0.0f && ProjMatF[15] == 1.0f)
    {
        // Orthographic
        float Left	 = (-1.0f - ProjMatF[12]) * RcpXX;
        float Right	 = ( 1.0f - ProjMatF[12]) * RcpXX;
        float Top	 = ( 1.0f - ProjMatF[13]) * RcpYY;
        float Bottom = (-1.0f - ProjMatF[13]) * RcpYY;
        float Front	 = ( 0.0f - ProjMatF[14]) * RcpZZ;
        float Back   = ( 1.0f - ProjMatF[14]) * RcpZZ;

        // Check for reverse Z here.  The bounding planes need to point into the frustum.
        if (Front < Back)
            ConstructOrthographicFrustum( Left, Right, Top, Bottom, Front, Back );
        else
            ConstructOrthographicFrustum( Left, Right, Top, Bottom, Back, Front );
    }
    else
    {
        // Perspective
        float NearClip, FarClip;

        if (RcpZZ > 0.0f)	// Reverse Z
        {
            FarClip = ProjMatF[14] * RcpZZ;
            NearClip = FarClip / (RcpZZ + 1.0f);
        }
        else
        {
            NearClip = ProjMatF[14] * RcpZZ;
            FarClip = NearClip / (RcpZZ + 1.0f);
        }

        ConstructPerspectiveFrustum( RcpXX, RcpYY, NearClip, FarClip );
    }
}
