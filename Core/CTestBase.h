#pragma once
#include "dxgi1_6.h"
#include <d3d12.h>
#include <wrl.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace CDirectX
{
	class CTestBase
	{
	public:

		int AcceleratorIndex = 0;

	public:


		ComPtr<IDXGIFactory4> FallbackFactory;
		ComPtr<IDXGIFactory1> BasicFactory;

		ComPtr<IDXGIAdapter1> adapter;


		ComPtr<ID3D12Device> BasicGraphicsCard;
	};
}

