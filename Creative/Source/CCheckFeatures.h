#pragma once
#include "CObject.h"
#include "CGraphicsBase.h"
#include <d3dcompiler.h>
#include "CException.h"
#include <algorithm>
#include "d3d11_4.h"
#include <cstdint>
#include "CObject.h"
#include "CCheckFeatures.h"
#include <d3dcompiler.h>
#include <iostream>
#include <mutex>
#include "CDescriptorHeap.h"
#include <d3d11_2.h>
#include <d3d11_3.h>
#include <d3d11_4.h>


struct CRCoreAPI CDirectXSupportedFeatures
{
	D3D12_FEATURE_DATA_ARCHITECTURE SupportData0 = {};
	D3D12_FEATURE_DATA_FEATURE_LEVELS SupportData1 = {};
	D3D12_FEATURE_DATA_FORMAT_SUPPORT SupportData2 = {};
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS SupportData3 = {};
	D3D12_FEATURE_DATA_FORMAT_INFO SupportData4 = {};
	D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT SupportData5 = {};
	D3D12_FEATURE_DATA_SHADER_MODEL SupportData6 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS1 SupportData7 = {};
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT SupportData8 = {};
	D3D12_FEATURE_DATA_ROOT_SIGNATURE SupportData9 = {};
	D3D12_FEATURE_DATA_ARCHITECTURE1 SupportData10 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS2 SupportData11 = {};
	D3D12_FEATURE_DATA_SHADER_CACHE SupportData12 = {};
	D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY SupportData13 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS3 SupportData14 = {};
	D3D12_FEATURE_DATA_EXISTING_HEAPS SupportData15 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS4 SupportData16 = {};
	D3D12_FEATURE_DATA_SERIALIZATION SupportData17 = {};
	D3D12_FEATURE_DATA_CROSS_NODE SupportData18 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 SupportData19 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 SupportData20 = {};
	D3D12_FEATURE_DATA_QUERY_META_COMMAND SupportData21 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 SupportData22 = {};
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT SupportData23 = {};
	D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES SupportData24 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS8 SupportData25 = {};
	D3D12_FEATURE_DATA_D3D12_OPTIONS9 SupportData26 = {};
};

class CCheckFeatures : public CObject
{
	CCheckFeatures()
	{
		HRESULT DeviceInit = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
		if (FAILED(DeviceInit))
		{
			HRESULT FallbackDeviceInit = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
			if (FAILED(FallbackDeviceInit))
			{
				throw CGraphicsException();
			}
		}

	}

	~CCheckFeatures()
	{


	}

public:
	
	void SystemCheck();
	void CheckGPUFeatures(CDirectXSupportedFeatures* SupportedFeatures);

	bool WarpSupport;


	int UnsupportedFeatureCount;
	int SupportedFeatureCount;
	D3D12_FEATURE Features;
	//std::vector<HRESULT> ComparisonFeatures;
	//std::forward_list<HRESULT> ComparisonFeaturesList;

public:
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<ID3D11Device> LegacyGraphicsCard;

public:
	ComPtr<IDXGIFactory7> LatestFactory;
	ComPtr<ID3D12Device8> GraphicsCard;

	ComPtr<IDXGIAdapter4> adapter = {};
	ComPtr<IDXGIFactory5> FallBackFactory;
	ComPtr<IDXGIFactory6> Factory;
};

