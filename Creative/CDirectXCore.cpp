#include "CDirectXCore.h"
#include <iostream>

bool CDirectXCore::LoadContent()
{

}

bool CDirectXCore::Intialize(HWND hWnd, HINSTANCE hInst)
{
	std::cout << " Beginning DirectX Intitalization" << std::endl;

	Window = hWnd;
	Instance = hInst;

	RECT Background;
	GetClientRect(hWnd, &Background);
	UINT nWidth = Background.right - Background.left;
	UINT nHeight = Background.bottom - Background.top;

	
	// To do: Check the volatility of this code!

	CreativeDirectXSupportedFeatures->FeatureLevel;
	
	CreativeDirectXSupportedFeatures->NumberOfFeatureLevels;

	CreativeDirectXDriverTypes->driverTypes;
	CreativeDirectXDriverTypes->numDriverTypes;

	// end

	UINT Flags = 0;

#ifdef _DEBUG

	Flags = D3D11_CREATE_DEVICE_DEBUG;

#endif // _DEBUG

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		DXAdapter,
		CreativeDirectXDriverTypes->driverTypes[0],
		NULL,
		Flags,
		CreativeDirectXSupportedFeatures->FeatureLevels,
		CreativeDirectXSupportedFeatures->NumberOfFeatureLevels,
		D3D11_SDK_VERSION,
		&SetupSwapChain(hWnd, nWidth, nHeight, 1, 0),
		&DXSwapChain,
		&GraphicsCard,
		&CreativeDirectXSupportedFeatures->FeatureLevel,
		&GraphicsCardController
	);

	GraphicsCardController->OMSetRenderTargets(1, &RenderTargetView, NULL);
	DirectXViewport.Width = (float) nWidth;
	DirectXViewport.Height = (float) nHeight;
	DirectXViewport.MinDepth = 0.0f;
	DirectXViewport.MaxDepth = 1.0f;
	DirectXViewport.TopLeftX = 0;
	DirectXViewport.TopLeftY = 0;

	GraphicsCardController->RSSetViewports(1, &DirectXViewport);

	return LoadContent();


}

DXGI_SWAP_CHAIN_DESC CDirectXCore::SetupSwapChain(HWND hWnd, UINT nWidth, UINT nHeight, int DoUseAntiAlising)
{
	DXGI_SWAP_CHAIN_DESC swapChainController;
	ZeroMemory(&swapChainController, sizeof(swapChainController));
	swapChainController.BufferCount = 2;
	swapChainController.BufferDesc.Width = nWidth;
	swapChainController.BufferDesc.Height = nHeight;
	swapChainController.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController.BufferDesc.RefreshRate.Numerator = 60;
	swapChainController.BufferDesc.RefreshRate.Denominator = 1;
	swapChainController.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController.OutputWindow = (HWND)hWnd;
	swapChainController.SampleDesc.Count = DoUseAntiAlising;
	swapChainController.SampleDesc.Quality = 0;
	swapChainController.Windowed = true;

	return swapChainController;
}

DXGI_SWAP_CHAIN_DESC CDirectXCore::SetupSwapChain(HWND hWnd, UINT nWidth, UINT nHeight, int DoUseAntiAlising, int AntiAilisngQuality)
{
	DXGI_SWAP_CHAIN_DESC swapChainController;
	ZeroMemory(&swapChainController, sizeof(swapChainController));
	swapChainController.BufferCount = 2;
	swapChainController.BufferDesc.Width = nWidth;
	swapChainController.BufferDesc.Height = nHeight;
	swapChainController.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController.BufferDesc.RefreshRate.Numerator = 60;
	swapChainController.BufferDesc.RefreshRate.Denominator = 1;
	swapChainController.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController.OutputWindow = (HWND)hWnd;
	swapChainController.SampleDesc.Count = DoUseAntiAlising;
	swapChainController.SampleDesc.Quality = AntiAilisngQuality;
	swapChainController.Windowed = true;

	return swapChainController;
}


void CDirectXCore::Tick(float DeltaTime)
{
}

void CDirectXCore::Tick()
{
}

void CDirectXCore::DeIntialize()
{
}

void CDirectXCore::Paused()
{
}

void CDirectXCore::UnPaused()
{
}

