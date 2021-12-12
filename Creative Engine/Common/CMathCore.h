#pragma once

#include <DirectXMath.h>
#include "CreativeMacros.h"

#define SFloat2 DirectX::XMFLOAT2
#define SFloat3 DirectX::XMFLOAT3
#define SFloat4 DirectX::XMFLOAT4
#define SMatrix DirectX::XMMATRIX

// This is the core of the Math class inside of Creative -- it cannot be copied or moved.
class CRMathAPI CMathCore
{
protected:

	CMathCore(CMathCore const&) = delete;
	CMathCore& operator= (CMathCore const&) = delete;
	CMathCore& operator= (CMathCore&& Value) = delete;
	CMathCore(CMathCore&& a) = delete;
};

