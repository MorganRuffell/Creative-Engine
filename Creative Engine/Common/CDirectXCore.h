 #pragma once

#include <wrl/client.h>
#include <wrl/event.h>
#include <roapi.h>
#include <roerrorapi.h>

#include "CObject.h"
#include "CGraphicsBase.h"
#include "CCheckFeatures.h"
#include "CMathCore.h"
#include <d3dcompiler.h>
#include <iostream>
#include "CException.h"
#include <mutex>
#include <algorithm>
#include <cstdint>
#include "CDescriptorHeap.h"


#ifndef CConstantBuffer

	#ifndef CVertexBuffer

		#include "CVertexBuffer.h"

#endif // !CVertexBuffer

	#include "CConstantBuffer.h"

#endif // !CConstantBuffer

// The min/max macros conflict with methods and variables
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

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
	// High resolution displays can require a lot of GPU and battery power to render.
	// High resolution phones, for example, may suffer from poor battery life if
	// games attempt to render at 60 frames per second at full fidelity.
	// The decision to render at full fidelity across all platforms and form factors
	// should be deliberate.
	static const bool SupportHighResolutions = false;

	// The default thresholds that define a "high resolution" display. If the thresholds
	// are exceeded and SupportHighResolutions is false, the dimensions will be scaled
	// by 50%.
	static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
	static const float WidthThreshold = 1920.0f;	// 1080p width.
	static const float HeightThreshold = 1080.0f;	// 1080p height.
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

struct CRCoreAPI SCreativeBaseColors
{
	float CR_BASE_Black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float CR_BASE_White[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float CR_BASE_Red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float CR_BASE_Green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float CR_BASE_Blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
};

struct CRCoreAPI SCreativeDescriptorHeaps {
	ComPtr<ID3D12DescriptorHeap> RTVSamplerHeap;

	//Combo heap contains the CBV, SRV, and UAV elements, just leave them all here.
	ComPtr<ID3D12DescriptorHeap> ComboHeap;

	ComPtr<ID3D12DescriptorHeap> DSVStencilHeap;
	ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap;
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

class CRCoreAPI CDirectXCore : public CObject
{
public:
	static const uint8_t NumberOfBackBuffers = 3;
	bool CanUseWarp = false;
	bool IsInitalized = false;



	const bool FULL_SCREEN = false;
	const bool VSYNC_ENABLED = true;
	bool TEARING_SUPPORTED;
	const float SCREEN_DEPTH = 1000.0f;
	const float SCREEN_NEAR = 0.1f;

	float VideoCardMemory;

	OutputSize m_RenderTargetSize = {};
	OutputSize m_outputSize = {};
	OutputSize m_logicalSize = {};

public:

	void CheckMSAAFeatures();

public:

	CDirectXCore()
	{

#if defined(_DEBUG)
		// If the project is in a debug build, enable debugging via SDK Layers.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
		}
#endif
		TEARING_SUPPORTED = CheckTearingSupport();


		std::thread SystemCheckThread = std::thread([&] 
		{
			Window = NULL;
			Instance = NULL;
			CheckFeatures->SystemCheck();
		});
		
		std::thread FallbackFactoryInitThread = std::thread([&] {
			CreateFallbackFactories();
		});


		HRESULT FactoryResult = CreateDXGIFactory(__uuidof(LatestFactory), &LatestFactory);
		if (FAILED(FactoryResult))
		{
			throw CGraphicsException();
			std::cout << "Creating Factory Failed!" << std::endl;
		}

		FallbackFactoryInitThread.join();
		SystemCheckThread.join();

		CheckFeatures.release();
	}

	~CDirectXCore()
	{
		std::thread FallbackFactoryReleaseThread = std::thread([&] {
			ReleaseFactories();
		});

		std::thread Release = std::thread([&] () mutable 
		{
			CheckFeatures.release();
			GraphicsCard->Release();
			adapter->Release();

			PrimaryCommandQueue->Release();
			SecondaryCommandQueue->Release();
			//TertiaryCommandQueue->Release();

			DescriptorHeaps = nullptr;
			BaseColors = nullptr;
			CreativeDirectXSupportedFeatures = nullptr;

			PrimarySwapChain->Release();
			SecondarySwapChain->Release();
			TeriarySwapChain->Release();

		});


		FallbackFactoryReleaseThread.join();
	}



public:

	std::unique_ptr<SCreativeBaseColors> BaseColors = std::make_unique<SCreativeBaseColors>();
	std::unique_ptr<SDirectXFeatureLevels> CreativeDirectXSupportedFeatures = std::make_unique<SDirectXFeatureLevels>();
	std::unique_ptr<SDirectXDriverTypes> CreativeDirectXDriverTypes = std::make_unique<SDirectXDriverTypes>();
	std::unique_ptr<CCheckFeatures> CheckFeatures = std::make_unique<CCheckFeatures>();
	std::unique_ptr<SCreativeDesciptorHeapsSizes> HeapSizes = std::make_unique<SCreativeDesciptorHeapsSizes>();

public:

	ComPtr<ID3D12Device8> GraphicsCard;
	ComPtr<IDXGIAdapter4> adapter = {};
	DXGI_ADAPTER_DESC3 AdapterInfo;
	D3D12_VIEWPORT g_Viewport0;
	ComPtr<IDXGIOutput> OutputTargetMonitor = {};


public:

	//Four Factories, Latest is main, if it fails then regular, if that fails then fallback. If that fails call exception/
	ComPtr<IDXGIFactory7> LatestFactory;
	ComPtr<IDXGIFactory6> Factory;
	ComPtr<IDXGIFactory5> FallBackFactory;

	void CreateFallbackFactories();
	void ReleaseFactories();


public:

	ComPtr<ID3D12CommandQueue> PrimaryCommandQueue;
	ComPtr<ID3D12CommandQueue> SecondaryCommandQueue;
	ComPtr<ID3D12CommandQueue> TertiaryCommandQueue;

	//Unused until basic implementation.
	ComPtr<ID3D12CommandQueue> GraphicsCommandQueue;
	ComPtr<ID3D12CommandQueue> ComputeCommandQueue;
	ComPtr<ID3D12CommandQueue> CopyCommandQueue;


	HRESULT SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType);
	HRESULT SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, D3D12_COMMAND_QUEUE_PRIORITY CommandQueuePriority);
	HRESULT SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, D3D12_COMMAND_QUEUE_PRIORITY CommandQueuePriority, D3D12_COMMAND_QUEUE_FLAGS Flags, bool UseMultipleGPUs);


public:

	std::unique_ptr<SCreativeDescriptorHeaps> DescriptorHeaps = std::make_unique<SCreativeDescriptorHeaps>();

	HRESULT CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12DescriptorHeap> DescHeap, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator);

	//DO NOT USE unless there are pre-existing containers in this class -- there should be.
	HRESULT CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator);

	const HRESULT UseFlags(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard, ComPtr<ID3D12DescriptorHeap>& DescHeap);
	const HRESULT UseFlags2(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard);

private:
	
	int G_RenderTargetViewDescriptorSize;




public:

	void UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap);
	void UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap, ComPtr<ID3D12Resource> BackBuffer);

	//Choose between the two?
	//std::vector<ComPtr<ID3D12Resource>> BackBuffers;

	static_assert((sizeof(SceneConstantBuffer) % 256) == 0);

	ComPtr<ID3D12Resource> BackBuffers[NumberOfBackBuffers];
	ComPtr<ID3D12Resource> m_constantBuffer;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_depthStencil;

	ComPtr<ID3D12DescriptorHeap> RenderTargetViewDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;


	SceneConstantBuffer m_constantBufferData;
	UINT8* CBVDataBegin;

	int CurrentBackBufferIndex;

	void CreateConstantBuffer(int BufferSize);
	CVertexBuffer* CreateVertexBuffer(void* vertexData, int VertexStride, int bufferSize);

	D3D12_RENDER_TARGET_VIEW_DESC* RTVController;
	D3D12_RENDER_TARGET_BLEND_DESC* RTVBlendController;

	DXGI_FORMAT	m_backBufferFormat;
	DXGI_FORMAT	m_depthBufferFormat;


public:

	//Command allocators are the backing memory used by a command list.
	HRESULT CreateCommandAllocator(ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> CommandAllocator);

	/*ComPtr<ID3D12CommandAllocator> DirectCommandAllocator;
	ComPtr<ID3D12CommandAllocator> BundleCommandAllocator;
	ComPtr<ID3D12CommandAllocator> ComputeCommandAllocator;
	ComPtr<ID3D12CommandAllocator> CopyCommandAllocator;*/

	//Do Not forget about the fact that command allocators and command lists consume memory as well. You've got to be careful!

	int AmountOfCommandAllocators = NumberOfBackBuffers;

	ComPtr<ID3D12CommandAllocator> gDirect_CommandAllocators[NumberOfBackBuffers];
	ComPtr<ID3D12CommandAllocator> gCompute_CommandAllocators[NumberOfBackBuffers];
	ComPtr<ID3D12CommandAllocator> gBundle_CommandAllocators[NumberOfBackBuffers];

	ComPtr<ID3D12GraphicsCommandList5> CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList5> CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type, bool UseMultipleGPU);


	ComPtr<ID3D12GraphicsCommandList5> globalCommandList;

	//ComPtr<ID3D12CommandSignature> CommandSignature;

public:
	//Fences allow for communication between GPUs and CPUs. We will make one for each thread.
	ComPtr<ID3D12Fence1> BaseFence;

	//To do - Bring these back and working with std::arrays
	//HRESULT CreateFence(UINT64 FenceValues[], ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12Fence1> Fence, D3D12_FENCE_FLAGS FenceFlags);

	//HRESULT CreateFence(UINT64 FenceValues[], ComPtr<ID3D12Device8> GraphicsCard, D3D12_FENCE_FLAGS FenceFlags);

	HANDLE CreateOSEvent();
	HANDLE fenceEvent;


	uint64_t Signal(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Fence1> fence, uint64_t& fenceValue);
	
	void WaitForFenceValue(ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent);
	void FlushGraphicsAccelerator(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent);

	UINT64 g_fenceValues[NumberOfBackBuffers];
	UINT64 CurrentBuffer;

	void GPU_STALL();


public:

	void UpdateFrameTicker();
	void RenderFrame();

	
	//Colours for base when rendering
	float R = 0;
	float G = 0;
	float B = 0;
	float A = 1.0;



#/*if defined (_DEBUG)





#endif*/

public:

	ComPtr<IDXGISwapChain4> PrimarySwapChain;
	ComPtr<IDXGISwapChain4> SecondarySwapChain;
	ComPtr<IDXGISwapChain4> TeriarySwapChain;

	HRESULT CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12Device8> GraphicsCard, uint32_t width, uint32_t height, uint32_t bufferCount);

	DXGI_SWAP_CHAIN_DESC swapChainController;

public:

	IDXGIAdapter* DXAdapter;
	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12PipelineState> PipelineState;

public:

	ComPtr<IDXGISwapChain3> DXSwapChain;

	//For drawing graphics on the screen!
	HWND Window;
	HINSTANCE Instance;

	// Cached device properties.
	/*Windows::Foundation::Size m_d3dRenderTargetSize;
	Windows::Foundation::Size m_outputSize;
	Windows::Foundation::Size m_logicalSize;
	Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
	Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;*/
	float m_dpi;

	// This is the DPI that will be reported back to the app, considers whether or not this supports high res screens!.
	float m_effectiveDpi;
	DirectX::XMFLOAT4X4 m_orientationTransform3D;

public:

	bool CheckTearingSupport();

	// Inherited via CObject
	virtual bool Intialize(HWND MainWindow, HINSTANCE hInst) override;
	void CreateWindowSizeDependentResources(HWND MainWindow, HINSTANCE hinst);
	void UpdateRenderTargetSize();
	DXGI_MODE_ROTATION ComputeDisplayRotation();

	void CreateDevice();

public:


	DXGI_SWAP_CHAIN_DESC* SetupSwapChain(HWND MainWindow, UINT nWidth, UINT nHeight, int DoUseAntiAlising);
	DXGI_SWAP_CHAIN_DESC* SetupSwapChain(HWND MainWindow, UINT nWidth, UINT nHeight, int DoUseAntiAlising, int AntiAilisngQuality);

	virtual void Tick(float DeltaTime) override;
	virtual void Tick() override;
	virtual void DeIntialize() override;


	bool CompileShader(LPCWSTR szFilePath, LPCSTR szFunc, LPCSTR szShaderModel, ID3DBlob** buffer);

public:

	void Paused();
	void UnPaused();

	bool LoadContent();
};



