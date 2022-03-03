#pragma once

#include "CGraphicsCore.h"
#include "CDirectXTest.h"
#include <d3d12.h>
#include "../CObject/d3dx12.h"


// The min/max macros conflict with methods and variables
// Only use std::min and std::max defined in <algorithm>.
#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

using Microsoft::WRL::ComPtr;

namespace CScreenRotation
{
	// 0-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 90-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 180-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 270-degree Z-rotation
	static const DirectX::XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};

namespace CDisplayMetrics
{
	//Checks device specifications to see if this should support high resolutions or not.
	static const bool SupportHighResolutions = false;

	static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
	static const float WidthThreshold = 1920.0f;	// 1080p width.
	static const float HeightThreshold = 1080.0f;	// 1080p height.
};

struct CRCoreAPI CDirectXSupportedFeatures
{
	D3D12_FEATURE_DATA_ARCHITECTURE SupportData0 = {};
	D3D12_FEATURE_DATA_FORMAT_SUPPORT SupportData1 = {};
	D3D12_FEATURE_DATA_FEATURE_LEVELS SupportData2 = {};
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
    D3D12_FEATURE_DATA_D3D12_OPTIONS10 SupportData27 = {};
    D3D12_FEATURE_DATA_D3D12_OPTIONS11 SupportData28 = {};
    D3D12_FEATURE_DATA_D3D12_OPTIONS12 SupportData29 = {};
};

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

struct CRCoreAPI SCreativeDesciptorHeapsSizes {
	UINT RTVDescriptorHeapSize;
};

struct CRCoreAPI SceneConstantBuffer
{
	SFloat4 offset;
	float padding[60];
};

struct CRCoreAPI OutputSize
{
	float Width;
	float Height;


	void Size(float __widthArg, float __heightArg)
	{
		if (__widthArg < 0 || __heightArg < 0)
		{
			CException();
		}

		Width = __widthArg;
		Height = __heightArg;
	}

};

namespace CDirectX
{
	static constexpr UINT AmountOfRenderTargets = 3;	// Use triple buffering.

	class CRGraphicsAPI CDirectX12Core : public CGraphics::CGraphicsCore
	{

	public:

        ~CDirectX12Core()
        {
            delete(GraphicsTest);
        }


		std::unique_ptr<CGraphics::SCoreState> DirectCoreState;
		CDirectX::CDirectXTest* GraphicsTest;

	public:

        //1. DXGI_FORMAT_R8G8B8A8_UNORM
        //2. DXGI_FORMAT_D32_FLOAT

		CDirectX12Core(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, HWND Window);

		void GetVRAMInfo()
		{
			std::unique_ptr<IDXGIFactory4> Factory;
			std::unique_ptr<IDXGIAdapter4> Adapter;

			CreateDXGIFactory1(__uuidof(*Factory), (void**)&Factory);
			Factory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(&Adapter));

			DXGI_QUERY_VIDEO_MEMORY_INFO acceleratorMemoryInfo;
			Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &acceleratorMemoryInfo);

			MaximumAmountOfVRAM = acceleratorMemoryInfo.Budget / 1024 / 1024 / 1024;
			CurrentUsageOfVRAM = acceleratorMemoryInfo.CurrentUsage / 1024 / 1024 / 1024;
		}

		void SetDpi(float dpi, HWND window) override;
		void ValidateDevice() override;
		void Present() override;
		void WaitForGpu() override;

		
		float GetDpi() const
		{ 
			return m_effectiveDpi; 
		}

		bool IsDeviceRemoved() const				
		{ 
			return m_deviceRemoved; 
		}


	public:

		std::map<std::string, bool> SupportedMSAALevels;

        //https://vzout.com/c++/directx12_tutorial.html#hlsl-shaders Depth stencil buffers

	public:

		//
		// D3D Accessors.
		ID3D12Device10* GetD3DDevice() const
		{
			return GraphicsCard.Get();
		}
		
		IDXGISwapChain3* GetSwapChain() const
		{ 
			return SwapChain.Get(); 
		}

		ID3D12Resource*	GetRenderTarget() const	
		{ 
			return m_renderTargets[CurrentBuffer].Get(); 
		}
		ID3D12Resource*	GetDepthStencil() const
		{ 
			return m_depthStencil.Get(); 
		}
		ID3D12CommandQueue*	GetCommandQueue() const	
		{ 
			return PrimaryCommandQueue.Get(); 
		}

		ID3D12CommandAllocator*	GetDirectCommandAllocator() const	
		{ 
			return DirectCommandAllocators[CurrentBuffer].Get(); 
		}

		ID3D12CommandAllocator* GetComputeCommandAllocator() const
		{
			return ComputeCommandAllocators[CurrentBuffer].Get();
		}


		DXGI_FORMAT	GetBackBufferFormat() const			
		{ 
			return m_backBufferFormat; 
		}
		DXGI_FORMAT	GetDepthBufferFormat() const
		{ 
			return m_depthBufferFormat; 
		}

		D3D12_VIEWPORT GetScreenViewport() const 
		{ 
			return m_screenViewport; 
		}
		DirectX::XMFLOAT4X4	GetOrientationTransfromAsFloatMatrix() const 
		{ 
			return DisplayOrientation; 
		}
		UINT GetCurrentFrameIndex() const
		{ 
			return CurrentBuffer; 
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBuffer, m_rtvDescriptorSize);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

	public:

		bool WarpSupport;
		int UnsupportedFeatureCount;
		

	private:
		void CreateDeviceIndependentResources();
		void CreateDeviceResources(HWND window);

		bool CreateGraphicsCard();
		bool CreateBackupGraphicsCard();

		void CreateCommandQueues();
		bool CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC HeapDesc, ComPtr<ID3D12DescriptorHeap> Heap);


		void CreateWindowSizeDependentResources(HWND window);
		void UpdateRenderTargetSize();
		void MoveToNextFrame();

    public:

		bool GetHardwareAdapter(IDXGIAdapter4** ppAdapter);
		bool GetSoftwareAdapter(IDXGIAdapter4** ppAdapter);


        HMONITOR GetPrimaryMonitorHandle(HWND window);

		UINT CurrentBuffer;

		// Direct3D objects.
		
		ComPtr<ID3D12Resource2> m_renderTargets[AmountOfRenderTargets];
		ComPtr<ID3D12Resource2> m_depthStencil;
	
	private:

		//Direct Command Queue
		ComPtr<ID3D12CommandQueue> PrimaryCommandQueue;
		//Graphics Command Queue
		ComPtr<ID3D12CommandQueue> SecondaryCommandQueue;
		//Fallback Command Queue
		ComPtr<ID3D12CommandQueue> TertiaryCommandQueue;


		std::vector<ComPtr<ID3D12CommandQueue>> CommandQueueArray;

		ComPtr<ID3D12CommandAllocator> DirectCommandAllocators[AmountOfRenderTargets];
		ComPtr<ID3D12CommandAllocator> ComputeCommandAllocators[AmountOfRenderTargets];


		DXGI_FORMAT	m_backBufferFormat;
		DXGI_FORMAT	m_depthBufferFormat;
		D3D12_VIEWPORT m_screenViewport;
		UINT m_rtvDescriptorSize;
		bool m_deviceRemoved;

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence1> GraphicsFence;
		UINT64 m_fenceValues[AmountOfRenderTargets];
		HANDLE	m_fenceEvent;

    public:

		// Cached reference to the Window.
		HWND	                                        m_window;

        UINT m_backBufferWidth = 0;
        UINT m_backBufferHeight = 0;

		float m_dpi;

		// This is the DPI that will be reported back to the engine, it shows whether or not this is a high resolution screen like a workstation monitor.
		float	m_effectiveDpi;

		// Transforms used for display orientation.
		DirectX::XMFLOAT4X4	DisplayOrientation;

		// Inherited via CObject
		/*virtual bool Intialize(HWND MainWindow, HINSTANCE hInst) override;
		virtual void Tick(float DeltaTime) override;
		virtual void Tick() override;
		virtual void DeIntialize() override;*/
	};
}
