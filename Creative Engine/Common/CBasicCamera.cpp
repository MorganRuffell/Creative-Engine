#include "CBasicCamera.h"
#include "pch.h"

CBasicCamera::CBasicCamera() :
	CCameraCore(Local_initialPosition),
	Local_position(Local_initialPosition),
	Local_yaw(DirectX::XM_PI),
	Local_pitch(0.0f),
	Local_lookDirection(0, 0, -1),
	Local_upDirection(0, 1, 0),
	Local_moveSpeed(20.0f),
	Local_turnSpeed(DirectX::XM_PIDIV2),
	Input{}
{

}

CBasicCamera::~CBasicCamera()
{

}


void CBasicCamera::BeginRoll(SFloat3 position)
{
	Local_position = position;
	Reset();
}

void CBasicCamera::Reset()
{
	Local_position = Local_initialPosition;
	Local_yaw = 3.1415f;
	Local_pitch = 0.0f;
	Local_lookDirection = { 0, 0, -1 };
}

void CBasicCamera::Update(float DeltaTime)
{
	SFloat3 Move(0, 0, 0);

	ProcessCameraMovementInput(Move);

	if (fabsf(Move.x) > 0.1f && fabsf(Move.z) > 0.1f)
	{
		DirectX::XMVECTOR vector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&Move));
		Move.x = DirectX::XMVectorGetX(vector);
		Move.z = DirectX::XMVectorGetZ(vector);
	}

	float moveInterval = Local_moveSpeed * DeltaTime;
	float rotateInterval = Local_turnSpeed * DeltaTime;

	ProcessCameraRotationInput(rotateInterval);
	CalculateMovementAndLookDirection(Move, moveInterval);
}

void CBasicCamera::CalculateMovementAndLookDirection(DirectX::XMFLOAT3& Move, float moveInterval)
{
	//Model Space movement
	float M_x = Move.x * -cosf(Local_yaw) - Move.z * sinf(Local_yaw);
	float M_z = Move.x * sinf(Local_yaw) - Move.z * cosf(Local_yaw);
	Local_position.x += M_x * moveInterval;
	Local_position.z += M_z * moveInterval;

	//Look Direction
	float RotationAngle = cosf(Local_pitch);
	Local_lookDirection.x = RotationAngle * sinf(Local_yaw);
	Local_lookDirection.y = sinf(Local_pitch);
	Local_lookDirection.z = RotationAngle * cosf(Local_yaw);
}

void CBasicCamera::ProcessCameraRotationInput(float rotateInterval)
{
	if (Input.left)
	{
		Local_yaw += rotateInterval;
	}

	if (Input.right)
	{
		Local_yaw -= rotateInterval;
	}

	if (Input.up)
	{
		Local_pitch += rotateInterval;
	}

	if (Input.down)
	{
		Local_pitch -= rotateInterval;
	}

	SetMinandMaxCameraRotation();
}

void CBasicCamera::SetMinandMaxCameraRotation()
{
	Local_pitch = min(Local_pitch, DirectX::XM_PIDIV4);
	Local_pitch = max(-DirectX::XM_PIDIV4, Local_pitch);
}

void CBasicCamera::ProcessCameraMovementInput(DirectX::XMFLOAT3& Move)
{
	if (Input.a)
	{
		Move.x -= 1.0f;
	}
	if (Input.d)
	{
		Move.x += 1.0f;

	}
	if (Input.w)
	{
		Move.z -= 1.0f;
	}
	if (Input.s)
	{
		Move.z += 1.0f;
	}
}

SMatrix CBasicCamera::GetViewMatrix()
{
	return DirectX::XMMatrixLookToRH(DirectX::XMLoadFloat3(&Local_position), DirectX::XMLoadFloat3(&Local_lookDirection), DirectX::XMLoadFloat3(&Local_upDirection));
}

SMatrix CBasicCamera::GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
}

void CBasicCamera::SetMoveSpeed(float unitsPerSecond)
{
	Local_moveSpeed = unitsPerSecond;
}

void CBasicCamera::SetTurnSpeed(float radiansPerSecond)
{
	//Rotation is mesured in radians not degrees. From a user perspective there will be conversion eventually.

	Local_turnSpeed = radiansPerSecond;
}

void CBasicCamera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		Input.w = true;
		break;
	case 'A':
		Input.a = true;
		break;
	case 'S':
		Input.s = true;
		break;
	case 'D':
		Input.d = true;
		break;
	case VK_LEFT:
		Input.left = true;
		break;
	case VK_RIGHT:
		Input.right = true;
		break;
	case VK_UP:
		Input.up = true;
		break;
	case VK_DOWN:
		Input.down = true;
		break;
	case VK_ESCAPE:
		Reset();
		break;
	}
}

void CBasicCamera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		Input.w = false;
		break;
	case 'A':
		Input.a = false;
		break;
	case 'S':
		Input.s = false;
		break;
	case 'D':
		Input.d = false;
		break;
	case VK_LEFT:
		Input.left = false;
		break;
	case VK_RIGHT:
		Input.right = false;
		break;
	case VK_UP:
		Input.up = false;
		break;
	case VK_DOWN:
		Input.down = false;
		break;
	}
}
