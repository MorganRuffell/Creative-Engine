#pragma once
#include "CCameraCore.h"

class CBasicCamera : public CCameraCore
{
    //To Do: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/UWP/D3D12PipelineStateCache/src/SimpleCamera.cpp


public:
	CBasicCamera();
    ~CBasicCamera();

    void BeginRoll(SFloat3 position) override;

    void Reset() override;

public:

    //DeltaTime is seconds elapsed
    void Update(float DeltaTime) override;

    void CalculateMovementAndLookDirection(DirectX::XMFLOAT3& Move, float moveInterval);
    void ProcessCameraRotationInput(float rotateInterval);
    void SetMinandMaxCameraRotation();
    void ProcessCameraMovementInput(DirectX::XMFLOAT3& Move);

public:

    SMatrix GetViewMatrix();
    SMatrix GetProjectionMatrix(float fov, float aspectRatio, float nearPlane = 1.0f, float farPlane = 1000.0f);

    void SetMoveSpeed(float unitsPerSecond);
    void SetTurnSpeed(float radiansPerSecond);

    void OnKeyDown(WPARAM key);
    void OnKeyUp(WPARAM key);


protected:


    struct SimpleCameraKeysPressed
    { 
        bool w;
        bool a;
        bool s;
        bool d;

        bool left;
        bool right;
        bool up;
        bool down;
    };

    SFloat3 Local_position;

    float Local_yaw;                // Relative to the +z axis.
    float Local_pitch;                // Relative to the xz plane.
    float Local_roll;

    SFloat3 Local_lookDirection;
    SFloat3 Local_upDirection;
    float Local_moveSpeed;            // Speed at which the camera moves, in units per second.
    float Local_turnSpeed;            // Speed at which the camera turns, in radians per second.


    SimpleCameraKeysPressed Input;

};

