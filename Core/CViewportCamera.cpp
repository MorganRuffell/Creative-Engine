
#include "pch.h"
#include "CViewportCamera.h"
#include <cmath>

using namespace CMath;

void CBaseCamera::SetLookDirection( CVector3 forward, CVector3 up )
{
    // Given, but ensure normalization
    CScalar forwardLenSq = LengthSquare(forward);
    forward = Select(forward * RecipSqrt(forwardLenSq), -CVector3(kZUnitVector), forwardLenSq < CScalar(0.000001f));

    // Deduce a valid, orthogonal right vector
    CVector3 right = Cross(forward, up);
    CScalar rightLenSq = LengthSquare(right);
    right = Select(right * RecipSqrt(rightLenSq), CQuarternion(CVector3(kYUnitVector), -XM_PIDIV2) * forward, rightLenSq < CScalar(0.000001f));

    // Compute actual up vector
    up = Cross(right, forward);

    // Finish constructing basis
    m_Basis = CMatrix3(right, up, -forward);
    m_CameraToWorld.SetRotation(CQuarternion(m_Basis));
}

void CBaseCamera::Update()
{
    m_PreviousViewProjMatrix = m_ViewProjMatrix;

    m_ViewMatrix = CMatrix4(~m_CameraToWorld);
    m_ViewProjMatrix = m_ProjMatrix * m_ViewMatrix;
    m_ReprojectMatrix = m_PreviousViewProjMatrix * Invert(GetViewProjMatrix());

    m_FrustumVS = CMFrustum( m_ProjMatrix );
    m_FrustumWS = m_CameraToWorld * m_FrustumVS;
}


void CViewportCamera::UpdateProjMatrix( void )
{
    float Y = 1.0f / std::tanf( m_VerticalFOV * 0.5f );
    float X = Y * m_AspectRatio;

    float Q1, Q2;

    // ReverseZ puts far plane at Z=0 and near plane at Z=1.  This is never a bad idea, and it's
    // actually a great idea with F32 depth buffers to redistribute precision more evenly across
    // the entire range.  It requires clearing Z to 0.0f and using a GREATER variant depth test.
    // Some care must also be done to properly reconstruct linear W in a pixel shader from hyperbolic Z.
    if (m_ReverseZ)
    {
        if (m_InfiniteZ)
        {
            Q1 = 0.0f;
            Q2 = m_NearClip;
        }
        else
        {
            Q1 = m_NearClip / (m_FarClip - m_NearClip);
            Q2 = Q1 * m_FarClip;
        }
    }
    else
    {
        if (m_InfiniteZ)
        {
            Q1 = -1.0f;
            Q2 = -m_NearClip;
        }
        else
        {
            Q1 = m_FarClip / (m_NearClip - m_FarClip);
            Q2 = Q1 * m_NearClip;
        }
    }

    SetProjMatrix( CMatrix4(
        CVector4( X, 0.0f, 0.0f, 0.0f ),
        CVector4( 0.0f, Y, 0.0f, 0.0f ),
        CVector4( 0.0f, 0.0f, Q1, -1.0f ),
        CVector4( 0.0f, 0.0f, Q2, 0.0f )
        ) );
}
