#pragma once

#include <memory>

#include <wrl/client.h>
#include "CObject.h"

// DirectX Includes
#include <d3d11.h>
#include <d3d12.h>
#include "combaseapi.h"
#include <cassert>

using Microsoft::WRL::ComPtr;


struct CRCoreAPI SDirectXDriverTypes
{
	D3D_DRIVER_TYPE driverTypes[4]
	{
		D3D_DRIVER_TYPE_HARDWARE, 
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
		D3D_DRIVER_TYPE_SOFTWARE
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);
};

struct CRCoreAPI SDirectXFeatureLevels
{
	D3D_FEATURE_LEVEL FeatureLevel;
	D3D_FEATURE_LEVEL FeatureLevels[5]
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_10_1
	};

	UINT NumberOfFeatureLevels = ARRAYSIZE(FeatureLevels);
};

struct CRCoreAPI SCreativeBaseColors
{
	float CR_BASE_Black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float CR_BASE_White[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float CR_BASE_Red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float CR_BASE_Green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float CR_BASE_Blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
};

class CRCoreAPI CDirectXCore : public CObject
{
public:

	CDirectXCore()
	{
		Window = NULL;
		Instance = NULL;
		&RenderTargetView = NULL;
		&DXSwapChain = NULL;
		&GraphicsCardController = NULL;
		&GraphicsCard = NULL;
	}

	~CDirectXCore()
	{

	}

public:

	std::unique_ptr<SCreativeBaseColors> BaseColors = std::make_unique<SCreativeBaseColors>();
	std::unique_ptr<SDirectXFeatureLevels> CreativeDirectXSupportedFeatures = std::make_unique<SDirectXFeatureLevels>();
	std::unique_ptr<SDirectXDriverTypes> CreativeDirectXDriverTypes = std::make_unique<SDirectXDriverTypes>();



public:

	ComPtr<ID3D11Device> GraphicsCard;
	ComPtr<ID3D11DeviceContext> GraphicsCardController;
	ComPtr<ID3D11ClassLinkage> ClassLinkage;
	ComPtr<IDXGISwapChain> DXSwapChain;
	IDXGIAdapter* DXAdapter;

	//For drawing graphics on the screen!
	ComPtr<ID3D11RenderTargetView> RenderTargetView;
	D3D11_VIEWPORT DirectXViewport;
	HWND Window;
	HINSTANCE Instance;

private:

	ID3D11ClassLinkage* BaseClassLinkage;


public:

	// Inherited via CObject
	virtual bool Intialize(HWND hWnd, HINSTANCE hInst) override;

	DXGI_SWAP_CHAIN_DESC SetupSwapChain(HWND hWnd, UINT nWidth, UINT nHeight, int DoUseAntiAlising);
	DXGI_SWAP_CHAIN_DESC SetupSwapChain(HWND hWnd, UINT nWidth, UINT nHeight, int DoUseAntiAlising, int AntiAilisngQuality);

	virtual void Tick(float DeltaTime) override;
	virtual void Tick() override;
	virtual void DeIntialize() override;

public:

	void Paused();
	void UnPaused();

	bool LoadContent();
};



