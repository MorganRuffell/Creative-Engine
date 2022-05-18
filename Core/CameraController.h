
#pragma once

#include "EngineCore.h"
#include "VectorMath.h"
#include "../Core/Math/CSphereCollider.h"

namespace CMath
{
    class CViewportCamera;
}

using namespace CMath;

class CViewportCameraController
{
public:
    // Assumes worldUp is not the X basis vector
    CViewportCameraController( CViewportCamera& camera ) : m_TargetCamera(camera) {}
    virtual ~CViewportCameraController() {}
    virtual void Update( float dt ) = 0;

    // Helper function
    static void ApplyMomentum( float& oldValue, float& newValue, float deltaTime );

protected:
    CViewportCamera& m_TargetCamera;

private:
    CViewportCameraController& operator=( const CViewportCameraController& ) {return *this;}
};

class CAdvancedViewportCamera : public CViewportCameraController
{
public:
    CAdvancedViewportCamera( CViewportCamera& camera, CVector3 worldUp );

    virtual void Update( float dt ) override;

    void SlowMovement( bool enable ) { m_FineMovement = enable; }
    void SlowRotation( bool enable ) { m_FineRotation = enable; }

    void EnableMomentum( bool enable ) { m_Momentum = enable; }

    void SetHeadingPitchAndPosition(float heading, float pitch, const CVector3& position);

private:

    CVector3 m_WorldUp;
    CVector3 m_WorldNorth;
    CVector3 m_WorldEast;

    float m_HorizontalLookSensitivity;
    float m_VerticalLookSensitivity;

    float m_MouseSensitivityX;
    float m_MouseSensitivityY;

    float m_CurrentHeading;
    float m_CurrentPitch;

    bool m_FineMovement;
    bool m_FineRotation;
    bool m_Momentum;

    /// <summary>
    /// Controls for 3D Space
    /// </summary>
	float m_LastPitch;
	float m_LastYaw;


    float m_LastForward;
    float m_LastStrafe;
    float m_LastAscent;
};

class CRotationalCamera : public CViewportCameraController
{
public:
    CRotationalCamera( CMath::CViewportCamera& camera, 
        CMath::CSphereCollider focus,
        CMath::CVector3 upVec = CMath::CVector3(CMath::kYUnitVector) );

    virtual void Update( float dt ) override;

    void EnableMomentum( bool enable ) { m_Momentum = enable; }

private:
    CRotationalCamera& operator=( const CRotationalCamera& ) {return *this;}

    CMath::CSphereCollider m_ModelBounds;
    CMath::CVector3 m_WorldUp;

    float m_JoystickSensitivityX;
    float m_JoystickSensitivityY;

    float m_MouseSensitivityX;
    float m_MouseSensitivityY;

    float m_CurrentHeading;
    float m_CurrentPitch;
    float m_CurrentCloseness;

    bool m_Momentum;

    float m_LastYaw;
    float m_LastPitch;
    float m_LastForward;
};
