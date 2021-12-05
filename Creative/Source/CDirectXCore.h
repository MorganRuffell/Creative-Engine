 #pragma once

#include "CObject.h"
#include "CGraphicsBase.h"
#include "CCheckFeatures.h"
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
	ComPtr<ID3D12DescriptorHeap> DepthStencilViewHeap;

	//Combo heap contains the CBV, SRV, and UAV elements, just leave them all here.
	ComPtr<ID3D12DescriptorHeap> ComboHeap;

	ComPtr<ID3D12DescriptorHeap> DSVStencilHeap;
	ComPtr<ID3D12DescriptorHeap> SamplerDescriptorHeap;
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

public:

	void CheckMSAAFeatures();

public:

	CDirectXCore()
	{
		std::thread SystemCheckThread = std::thread([&] 
		{
			TEARING_SUPPORTED = CheckTearingSupport();
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
			OutputTargetMonitor->Release();

			PrimaryCommandQueue->Release();
			SecondaryCommandQueue->Release();
			TertiaryCommandQueue->Release();

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

public:

	ComPtr<ID3D12Device8> GraphicsCard;
	ComPtr<IDXGIAdapter4> adapter = {};
	DXGI_ADAPTER_DESC3 AdapterInfo;
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


	ComPtr<ID3D12CommandQueue> SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType);
	ComPtr<ID3D12CommandQueue> SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, int CommandQueuePriority);
	ComPtr<ID3D12CommandQueue> SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, int CommandQueuePriority, D3D12_COMMAND_QUEUE_FLAGS Flags, bool UseMultipleGPUs);


public:

	std::unique_ptr<SCreativeDescriptorHeaps> DescriptorHeaps = std::make_unique<SCreativeDescriptorHeaps>();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12DescriptorHeap> DescHeap, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator);

	//DO NOT USE unless there are pre-existing containers in this class -- there should be.
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator);

	const ComPtr<ID3D12DescriptorHeap>& UseFlags(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard, ComPtr<ID3D12DescriptorHeap>& DescHeap);
	const ComPtr<ID3D12DescriptorHeap>& UseFlags2(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard);

private:
	
	int G_RenderTargetViewDescriptorSize;




public:

	void UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap);
	void UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap, ComPtr<ID3D12Resource> BackBuffer);

	//Choose between the two?
	//std::vector<ComPtr<ID3D12Resource>> BackBuffers;

	ComPtr<ID3D12Resource> BackBuffers[NumberOfBackBuffers];
	int CurrentBackBufferIndex;

	CConstantBuffer* CreateConstantBuffer(int BufferSize);
	CVertexBuffer* CreateVertexBuffer(void* vertexData, int VertexStride, int bufferSize);

	D3D12_RENDER_TARGET_VIEW_DESC* RTVController;
	D3D12_RENDER_TARGET_BLEND_DESC* RTVBlendController;

public:

	//Command allocators are the backing memory used by a command list.
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> CommandAllocator);

	/*ComPtr<ID3D12CommandAllocator> DirectCommandAllocator;
	ComPtr<ID3D12CommandAllocator> BundleCommandAllocator;
	ComPtr<ID3D12CommandAllocator> ComputeCommandAllocator;
	ComPtr<ID3D12CommandAllocator> CopyCommandAllocator;*/

	//Do Not forget about the fact that command allocators and command lists consume memory as well. You've got to be careful!

	int AmountOfCommandAllocators;
	int MaximumAmountOfCommandAllocators = 20;
	ComPtr<ID3D12CommandAllocator>g_CommandAllocators[4];


	ComPtr<ID3D12GraphicsCommandList5> CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList5> CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type, bool UseMultipleGPU);

	ComPtr<ID3D12GraphicsCommandList5> globalCommandList;

	//ComPtr<ID3D12CommandSignature> CommandSignature;

public:
	//Fences allow for communication between GPUs and CPUs. We will make one for each thread.
	ComPtr<ID3D12Fence1> BaseFence;

	ComPtr<ID3D12Fence1> CreateFence(ComPtr<ID3D12Device8> GraphicsCard, D3D12_FENCE_FLAGS FenceFlags);
	ComPtr<ID3D12Fence1> CreateFence(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12Fence1> Fence, D3D12_FENCE_FLAGS FenceFlags);


	HANDLE CreateOSEvent();

	uint64_t Signal(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Fence1> fence, uint64_t& fenceValue);
	
	void WaitForFenceValue(ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent);
	void FlushGraphicsAccelerator(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent);

public:

	void UpdateFrameTicker();
	void RenderFrame();

	ComPtr<ID3D12DescriptorHeap> RenderTargetViewDescriptorHeap;
	
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

	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12Device8> GraphicsCard, uint32_t width, uint32_t height, uint32_t bufferCount);

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


public:

	bool CheckTearingSupport();

	// Inherited via CObject
	virtual bool Intialize(HWND MainWindow, HINSTANCE hInst) override;

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



