
#include "pch.h"
#include "CameraController.h"
#include "CViewportCamera.h"
#include "EngineInput.h"

using namespace CMath;
using namespace CECore;

CAdvancedViewportCamera::CAdvancedViewportCamera( CViewportCamera& camera, CVector3 worldUp ) : CViewportCameraController( camera )
{
    m_WorldUp = Normalize(worldUp);
    m_WorldNorth = Normalize(Cross(m_WorldUp, CVector3(kXUnitVector)));
    m_WorldEast = Cross(m_WorldNorth, m_WorldUp);

    m_HorizontalLookSensitivity = 2.0f;
    m_VerticalLookSensitivity = 2.0f;

    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = 1.0f;

    m_CurrentPitch = Sin(Dot(camera.GetForwardVec(), m_WorldUp));

    CVector3 forward = Normalize(Cross(m_WorldUp, camera.GetRightVec()));
    m_CurrentHeading = ATan2(-Dot(forward, m_WorldEast), Dot(forward, m_WorldNorth));

    m_FineMovement = false;
    m_FineRotation = false;
    m_Momentum = true;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
    m_LastForward = 0.0f;
    m_LastStrafe = 0.0f;
    m_LastAscent = 0.0f;
}

namespace CGraphics
{
    extern CETStaticEnum DebugZoom;
}

void CAdvancedViewportCamera::Update( float deltaTime )
{
    (deltaTime);

    float timeScale = CGraphics::DebugZoom == 0 ? 1.0f : CGraphics::DebugZoom == 1 ? 0.5f : 0.25f;

    float speedScale = (m_FineMovement ? 0.1f : 1.0f) * timeScale;
    float panScale = (m_FineRotation ? 0.5f : 1.0f) * timeScale;

    float yaw = EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogRightStickX ) * m_HorizontalLookSensitivity * panScale;
    float pitch = EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogRightStickY ) * m_VerticalLookSensitivity * panScale;
    float forward = EngineInput::MovementSpeed * speedScale * (
        EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogLeftStickY ) +
        (EngineInput::IsPressed( EngineInput::kKey_w ) ? deltaTime : 0.0f) +
        (EngineInput::IsPressed( EngineInput::kKey_s ) ? -deltaTime : 0.0f)
        );
    float strafe = EngineInput::MovementSpeed * speedScale * (
        EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogLeftStickX  ) +
        (EngineInput::IsPressed( EngineInput::kKey_d ) ? deltaTime : 0.0f) +
        (EngineInput::IsPressed( EngineInput::kKey_a ) ? -deltaTime : 0.0f)
        );
    float ascent = EngineInput::MovementSpeed * speedScale * (
        EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogRightTrigger ) -
        EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogLeftTrigger ) +
        (EngineInput::IsPressed( EngineInput::kKey_e ) ? deltaTime : 0.0f) +
        (EngineInput::IsPressed( EngineInput::kKey_q ) ? -deltaTime : 0.0f)
        );

    if (m_Momentum)
    {
        ApplyMomentum(m_LastYaw, yaw, deltaTime);
        ApplyMomentum(m_LastPitch, pitch, deltaTime);
        ApplyMomentum(m_LastForward, forward, deltaTime);
        ApplyMomentum(m_LastStrafe, strafe, deltaTime);
        ApplyMomentum(m_LastAscent, ascent, deltaTime);
    }

    // don't apply momentum to mouse inputs
    yaw += EngineInput::GetAnalogInput(EngineInput::kAnalogMouseX) * m_MouseSensitivityX;
    pitch += EngineInput::GetAnalogInput(EngineInput::kAnalogMouseY) * m_MouseSensitivityY;

    m_CurrentPitch += pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    m_CurrentHeading -= yaw;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 

    CMatrix3 orientation = CMatrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * CMatrix3::MakeYRotation( m_CurrentHeading ) * CMatrix3::MakeXRotation( m_CurrentPitch );
    CVector3 position = orientation * CVector3( strafe, ascent, -forward ) + m_TargetCamera.GetPosition();
    m_TargetCamera.SetTransform( CMAffineTransform( orientation, position ) );
    m_TargetCamera.Update();
}

void CAdvancedViewportCamera::SetHeadingPitchAndPosition(float heading, float pitch, const CVector3& position)
{
    m_CurrentHeading = heading;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 

    m_CurrentPitch = pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    CMatrix3 orientation =
        CMatrix3(m_WorldEast, m_WorldUp, -m_WorldNorth) * 
        CMatrix3::MakeYRotation( m_CurrentHeading ) *
        CMatrix3::MakeXRotation( m_CurrentPitch );

    m_TargetCamera.SetTransform( CMAffineTransform( orientation, position ) );
    m_TargetCamera.Update();
}


void CViewportCameraController::ApplyMomentum( float& oldValue, float& newValue, float deltaTime )
{
    float blendedValue;
    if (Abs(newValue) > Abs(oldValue))
        blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
    else
        blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
    oldValue = blendedValue;
    newValue = blendedValue;
}

CRotationalCamera::CRotationalCamera( CViewportCamera& camera, CMath::CSphereCollider focus, CVector3 worldUp ) : CViewportCameraController( camera )
{
    m_ModelBounds = focus;
    m_WorldUp = Normalize(worldUp);

    m_JoystickSensitivityX = 2.0f;
    m_JoystickSensitivityY = 2.0f;

    m_MouseSensitivityX = 1.0f;
    m_MouseSensitivityY = 1.0f;

    m_CurrentPitch = 0.0f;
    m_CurrentHeading = 0.0f;
    m_CurrentCloseness = 0.5f;

    m_Momentum = true;

    m_LastYaw = 0.0f;
    m_LastPitch = 0.0f;
}

void CRotationalCamera::Update( float deltaTime )
{
    (deltaTime);

    float timeScale = CGraphics::DebugZoom == 0 ? 1.0f : CGraphics::DebugZoom == 1 ? 0.5f : 0.25f;

    float yaw = EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogLeftStickX ) * timeScale * m_JoystickSensitivityX;
    float pitch = EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogLeftStickY ) * timeScale * m_JoystickSensitivityY;
    float closeness = EngineInput::GetTimeCorrectedAnalogInput( EngineInput::kAnalogRightStickY ) * timeScale;

    if (m_Momentum)
    {
        ApplyMomentum(m_LastYaw, yaw, deltaTime);
        ApplyMomentum(m_LastPitch, pitch, deltaTime);
    }

    // don't apply momentum to mouse inputs
    yaw += EngineInput::GetAnalogInput(EngineInput::kAnalogMouseX) * m_MouseSensitivityX;
    pitch += EngineInput::GetAnalogInput(EngineInput::kAnalogMouseY) * m_MouseSensitivityY;
    closeness += EngineInput::GetAnalogInput(EngineInput::kAnalogMouseScroll) * 0.1f;

    m_CurrentPitch += pitch;
    m_CurrentPitch = XMMin( XM_PIDIV2, m_CurrentPitch);
    m_CurrentPitch = XMMax(-XM_PIDIV2, m_CurrentPitch);

    m_CurrentHeading -= yaw;
    if (m_CurrentHeading > XM_PI)
        m_CurrentHeading -= XM_2PI;
    else if (m_CurrentHeading <= -XM_PI)
        m_CurrentHeading += XM_2PI; 

    m_CurrentCloseness += closeness;
    m_CurrentCloseness = Clamp(m_CurrentCloseness, 0.0f, 1.0f);

    CMatrix3 orientation = CMatrix3::MakeYRotation( m_CurrentHeading ) * CMatrix3::MakeXRotation( m_CurrentPitch );
    CVector3 position = orientation.GetZ() * (m_ModelBounds.GetRadius() * Lerp(3.0f, 1.0f, m_CurrentCloseness) + m_TargetCamera.GetNearClip());
    m_TargetCamera.SetTransform(CMAffineTransform(orientation, position + m_ModelBounds.GetCenter()));
    m_TargetCamera.Update();
}
