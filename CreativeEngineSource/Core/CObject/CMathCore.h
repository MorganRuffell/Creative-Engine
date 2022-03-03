#pragma once

#include <DirectXMath.h>
#include "CreativeMacros.h"

#define SFloat2 DirectX::XMFLOAT2
#define SMatrix DirectX::XMMATRIX
#define SFloat3 DirectX::XMFLOAT3
#define SFloat4 DirectX::XMFLOAT4

//struct SFloat3 : public DirectX::XMFLOAT3
//{
//
//
//public:
//	SFloat3(float x, float y, float z)
//		: DirectX::XMFLOAT3(x,y,z)
//	{
//
//	}
//
//	template<typename T>
//	SFloat3(T x, T y, T z)
//		: DirectX::XMFLOAT3(x, y, z)
//	{
//
//	}
//
//
//
//	SFloat3 operator=(SFloat3 b)
//	{
//		this->x = b.x;
//		this->y = b.y;
//		this->z = b.z;
//	}
//
//	
//
//	const int AmountofComponents = 3;
//};

//struct SFloat4 : public DirectX::XMFLOAT4
//{
//	SFloat4(float x, float y, float z, float w)
//		: DirectX::XMFLOAT4(x, y, z, w)
//	{
//
//	}
//
//
//	const int AmountofComponents = 4;
//};


// This is the core of the Math class inside of Creative -- it cannot be copied or moved.
class CRMathAPI CMathCore
{
protected:

	CMathCore(CMathCore const&) = delete;
	CMathCore& operator= (CMathCore const&) = delete;
	CMathCore& operator= (CMathCore&& Value) = delete;
	CMathCore(CMathCore&& a) = delete;

public:

	CMathCore()
	{

	}


	bool Compare3PointVector(SFloat3 a, SFloat3 b)
	{
		if (a.x == b.x)
		{
			if (a.y == b.y)
			{
				if (a.z == b.z)
				{
					return true;
				}

				return false;
			}

			return false;
		}

	}

	bool Compare3PointVector(SFloat3 a, SFloat3 b, SFloat3 c)
	{
		if (a.x == b.x && a.x == c.x)
		{
			if (a.y == b.y && a.y == c.y)
			{
				if (a.z == b.z && a.z == c.z)
				{
					return true;
				}

				return false;
			}

			return false;
		}

	}

};

