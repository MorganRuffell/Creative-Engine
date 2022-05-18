#pragma once

#include "VectorMath.h"
#include "Math/CMFrustum.h"

namespace CMath
{
    class CBaseCamera
    {
    public:

        void Update();

        // Public functions for controlling where the camera is and its orientation
        void SetEyeAtUp( _In_ CVector3 eye, _In_ CVector3 at, _In_ CVector3 up );
        void SetLookDirection(_In_ CVector3 forward, _In_ CVector3 up );
        void SetRotation( _In_ CQuarternion basisRotation );
        void SetPosition( _In_ CVector3 worldPos );

    public:
        //These use the raw mathematical transforms to allow for transformation
        void SetTransform( const _In_ CMAffineTransform& ATransform );
        void SetTransform( const _In_ CMOrthoTransformation& OrthoTransform );


        // Accessors for reading the various matrices and frustrum -- Frustrum is the mathematics behind creatives camera
        const CMatrix4& GetViewMatrix() const { return m_ViewMatrix; }
        const CMatrix4& GetViewProjMatrix() const { return m_ViewProjMatrix; }

        const CMatrix4& GetReprojectionMatrix() const { return m_ReprojectMatrix; }
        const CMatrix4& GetProjMatrix() const { return m_ProjMatrix; }

    public:

        //We need to get specific three point vectors to be able to allow the viewporet camera function
        const CQuarternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
        const CVector3 GetRightVec() const { return m_Basis.GetX(); }
        const CVector3 GetUpVec() const { return m_Basis.GetY(); }
        const CVector3 GetForwardVec() const { return -m_Basis.GetZ(); }
        const CVector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

        

    public:

        const CMFrustum& GetViewSpaceFrustum() const { return m_FrustumVS; }
        const CMFrustum& GetWorldSpaceFrustum() const { return m_FrustumWS; }

    protected:

        CBaseCamera() : m_CameraToWorld(kIdentity), m_Basis(kIdentity) {}

        void SetProjMatrix(_In_ const CMatrix4& ProjMat ) { m_ProjMatrix = ProjMat; }

        CMOrthoTransformation m_CameraToWorld;

        CMatrix3 m_Basis;           //Cache Matrix -- DO NOT USE IN CALCULATIONS THE VALUES ARE UB

        // Transforms homogeneous coordinates from world space to view space.  
        CMatrix4 m_ViewMatrix;		

        // The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
        CMatrix4 m_ProjMatrix;		        
        CMatrix4 m_ViewProjMatrix;	        
        CMatrix4 m_PreviousViewProjMatrix;
        CMatrix4 m_ReprojectMatrix;

        CMFrustum m_FrustumVS;		// View-space view frustum
        CMFrustum m_FrustumWS;		// World-space view frustum

    };

    class CViewportCamera : public CBaseCamera
    {
    public:
        CViewportCamera();

        // Controls the view-to-projection matrix
        void SetPerspectiveMatrix(_In_ float verticalFovRadians, _In_ float aspectHeightOverWidth, _In_ float nearZClip, _In_ float farZClip );

        void SetFOV(_In_ float verticalFovInRadians ) { m_VerticalFOV = verticalFovInRadians; UpdateProjMatrix(); }

        void SetAspectRatio(_In_ float heightOverWidth ) { m_AspectRatio = heightOverWidth; UpdateProjMatrix(); }

        void SetZRange(_In_ float nearZ, _In_ float farZ) { m_NearClip = nearZ; m_FarClip = farZ; UpdateProjMatrix(); }
        void ReverseZ(_In_ bool enable ) { m_ReverseZ = enable; UpdateProjMatrix(); }

        float GetFOV() const { return m_VerticalFOV; }
        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }
        float GetClearDepth() const { return m_ReverseZ ? 0.0f : 1.0f; }

        HWND GetMainWindow() const { return m_window; }
        void SetMainWindow(_In_ HWND newWindow) { m_window = newWindow; }
    private:

        void UpdateProjMatrix( void );

        float m_VerticalFOV;	// Field of view angle in radians
        float m_AspectRatio;
        float m_NearClip;
        float m_FarClip;
        bool m_ReverseZ;		// Invert near and far clip distances so that Z=1 at the near plane
        bool m_InfiniteZ;       // Move the far plane to infinity

        HWND m_window;          //Reference to the window to which the camera belongs.

    };

    inline void CBaseCamera::SetEyeAtUp(_In_ CVector3 eye, _In_ CVector3 at, _In_ CVector3 up )
    {
        SetLookDirection(at - eye, up);
        SetPosition(eye);
    }

    inline void CBaseCamera::SetPosition(_In_ CVector3 worldPos )
    {
        m_CameraToWorld.SetTranslation( worldPos );
    }

    inline void CBaseCamera::SetTransform(_In_ const CMAffineTransform& xform )
    {
        // By using these functions, we rederive an orthogonal transform.
        SetLookDirection(-xform.GetZ(), xform.GetY());
        SetPosition(xform.GetTranslation());
    }

    inline void CBaseCamera::SetRotation(_In_ CQuarternion basisRotation )
    {
        m_CameraToWorld.SetRotation(Normalize(basisRotation));
        m_Basis = CMatrix3(m_CameraToWorld.GetRotation());
    }

    inline CViewportCamera::CViewportCamera() : m_ReverseZ(true), m_InfiniteZ(false)
    {
        SetPerspectiveMatrix( XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f );
    }

    inline void CViewportCamera::SetPerspectiveMatrix(_In_ float verticalFovRadians, _In_ float aspectHeightOverWidth, _In_ float nearZClip, _In_ float farZClip )
    {
        m_VerticalFOV = verticalFovRadians;
        m_AspectRatio = aspectHeightOverWidth;
        m_NearClip = nearZClip;
        m_FarClip = farZClip;

        UpdateProjMatrix();

        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }

} // namespace Math
