#pragma once

#include <wrl/client.h>


// DirectX Includes
#include <d3d11.h>
#include <d3d12.h>
#include "combaseapi.h"
#include <cassert>

using Microsoft::WRL::ComPtr;


class CDirectXCore
{

public:


	ComPtr<ID3D11Device> g_device;
	ComPtr<IDXGISwapChain> g_SwapChain;
	ComPtr<ID3D11DeviceContext> g_deviceContext;




	//For drawing graphics on the screen!
	ComPtr<ID3D11RenderTargetView> g_RenderTargetView;
	D3D11_VIEWPORT g_viewport;



};

