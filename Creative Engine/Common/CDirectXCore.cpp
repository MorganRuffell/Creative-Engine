#pragma optimize ("",off)

#include "CDirectXCore.h"
#include "CreativeGraphicsUtilities.h"
#include "CreativeGraphicsResource.h"
#include <concrt.h>

#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

//https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12 -- will come in handy for the init stage

void CDirectXCore::ReleaseFactories()
{
	if (LatestFactory == nullptr || Factory == nullptr)
	{
		Factory->Release();
		FallBackFactory->Release();
	}
	else
	{
		LatestFactory->Release();
	}
}


bool CDirectXCore::LoadContent()
{
	return true;
}


void CDirectXCore::CheckMSAAFeatures()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels;
	QualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	QualityLevels.NumQualityLevels = 0;
	QualityLevels.SampleCount = 4;
	QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	HRESULT res = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels, sizeof(QualityLevels));
	if (SUCCEEDED(res))
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels8;
		QualityLevels8.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		QualityLevels8.NumQualityLevels = 0;
		QualityLevels8.SampleCount = 8;
		QualityLevels8.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

		HRESULT res2 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels8, sizeof(QualityLevels8));

		if (SUCCEEDED(res))
		{
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels16;
			QualityLevels16.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
			QualityLevels16.NumQualityLevels = 0;
			QualityLevels16.SampleCount = 16;
			QualityLevels16.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

			HRESULT res2 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &QualityLevels16, sizeof(QualityLevels16));

			if (SUCCEEDED(res2))
			{
				std::cout << "Supports 16x MSAA";
			}
			else
			{
				std::cout << "Supports 8x MSAA";

			}
		}

		else
		{
			std::cout << "Supports 4x MSAA";
		}
	}
	else
	{
		throw new CGraphicsException;
	}
}

void CDirectXCore::CreateFallbackFactories()
{
	HRESULT FactoryResultOne = CreateDXGIFactory(__uuidof(Factory), &Factory);
	if (FAILED(FactoryResultOne))
	{
		std::cout << "Creating Factory Failed!" << std::endl;
	}

	HRESULT FactoryResultTwo = CreateDXGIFactory(__uuidof(FallBackFactory), &FallBackFactory);
	if (FAILED(FactoryResultTwo))
	{
		std::cout << "Creating Factory Failed!" << std::endl;
	}
}

HRESULT CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueController = {};
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	CommandQueueController.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueController.NodeMask = 0;

	HRESULT res = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));

	if (SUCCEEDED(res))
	{
		return SUCCEEDED(res);
	}
	else
	{
		return FAILED(res);
	}
}

HRESULT CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, D3D12_COMMAND_QUEUE_PRIORITY CommandQueuePriority)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueController = {};
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT res = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));
	if (SUCCEEDED(res))
	{
		return SUCCEEDED(res);
	}
	else
	{
		return FAILED(res);
	}
}

//Used for creating high priority command queues.
HRESULT CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, D3D12_COMMAND_QUEUE_PRIORITY CommandQueuePriority, D3D12_COMMAND_QUEUE_FLAGS Flags, bool UseMultipleGPUs)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueController;
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Flags = Flags;
	CommandQueueController.NodeMask = 0;
	CommandQueueController.Priority = CommandQueuePriority;

	HRESULT res = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));
	if (SUCCEEDED(res))
	{
		return SUCCEEDED(res);
	}
	else
	{
		return FAILED(res);
	}
}

HRESULT CDirectXCore::CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12DescriptorHeap> DescHeap, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator)
{
	if (GraphicsCard == nullptr) { throw new std::exception; }


	if (UseSecondaryAccelerator != 0)
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return UseFlags(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard, DescHeap);
		}

		D3D12_DESCRIPTOR_HEAP_DESC Description = {};

		Description.NumDescriptors = NumberOfDescriptors;
		Description.Type = Type;
		Description.NodeMask = 1;


		HRESULT res = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
		if (SUCCEEDED(res))
		{
			if (DescHeap->GetDesc().Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			{
				HeapSizes->RTVDescriptorHeapSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}

			return (SUCCEEDED(res));
		}
		else
		{
			return (FAILED(res));
		}


	}
	else
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return UseFlags(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard, DescHeap);
		}

		D3D12_DESCRIPTOR_HEAP_DESC Description = {};

		Description.NumDescriptors = NumberOfDescriptors;
		Description.Type = Type;
		Description.NodeMask = 1;


		HRESULT res = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
		if (SUCCEEDED(res))
		{
			return (SUCCEEDED(res));
		}
		else
		{
			return (FAILED(res));
		}

	}

}

const HRESULT CDirectXCore::UseFlags(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard, ComPtr<ID3D12DescriptorHeap>& DescHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC Description = {};

	Description.NumDescriptors = NumberOfDescriptors;
	Description.Type = Type;
	Description.NodeMask = UseSecondaryAccelerator;
	Description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;


	HRESULT res = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
	if (SUCCEEDED(res))
	{
		return(SUCCEEDED(res));
	}
	else
	{
		return(FAILED(res));
	}
}

const HRESULT CDirectXCore::UseFlags2(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC Description = {};

	Description.NumDescriptors = NumberOfDescriptors;
	Description.Type = Type;
	Description.NodeMask = UseSecondaryAccelerator;
	Description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;


	HRESULT res = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&descriptorHeap));
	if (SUCCEEDED(res))
	{
		return(SUCCEEDED(res));
	}
	else
	{
		return(FAILED(res));
	}
}

void CDirectXCore::UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap)
{
	auto RenderTargetViewSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < NumberOfBackBuffers; i++)
	{
		ComPtr<ID3D12Resource> BackBuffer;

		HRESULT x = SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffer));
		if (FAILED(x))
		{

		}

		GraphicsCard->CreateRenderTargetView(BackBuffer.Get(), nullptr, RenderTargetViewHandle);

		BackBuffers[i] = BackBuffer;

	}

}

void CDirectXCore::UpdateRenderTargetViews(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12DescriptorHeap> DescriptorHeap, ComPtr<ID3D12Resource> BackBuffer)
{
	auto RenderTargetViewSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < NumberOfBackBuffers; i++)
	{
		HRESULT x = SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffer));
		if (FAILED(x))
		{

		}

		GraphicsCard->CreateRenderTargetView(BackBuffer.Get(), nullptr, RenderTargetViewHandle);

		BackBuffers[i] = BackBuffer;

	}

	//Create render target views --https://www.3dgep.com/learning-directx-12-1/#Create_Window_Instance
}



//Used for initalizing a new descriptor heap
HRESULT CDirectXCore::CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator)
{

	if (GraphicsCard == nullptr) { throw new std::exception; }

	if (UseSecondaryAccelerator != 0)
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			UseFlags2(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard);
		}

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = NumberOfDescriptors;
		desc.Type = Type;
		desc.NodeMask = UseSecondaryAccelerator;

		HRESULT res = GraphicsCard->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));

		if (SUCCEEDED(res))
		{
			return(SUCCEEDED(res));
		}
		else
		{
			return(FAILED(res));
		}
	}
	else
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			UseFlags2(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard);
		}

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC Description = {};

		Description.NumDescriptors = NumberOfDescriptors;
		Description.Type = Type;
		Description.NodeMask = 1;


		HRESULT res = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&descriptorHeap));
		if (SUCCEEDED(res))
		{
			return(SUCCEEDED(res));
		}
		else
		{
			return(FAILED(res));
		}

	}
}

HRESULT CDirectXCore::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12Device8> GraphicsCard, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	if (GraphicsCard == nullptr) { throw new std::exception; }
	if (CommandQueue == nullptr) { throw new std::exception; }

	UINT createFactoryFlags = 0;

	DXGI_SWAP_CHAIN_DESC1 swapChainController = {};
	ComPtr<IDXGISwapChain1> swapChain;

	swapChainController.Width = width;
	swapChainController.Height = height;
	swapChainController.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController.Stereo = FALSE;
	swapChainController.SampleDesc = { 1,0 };
	swapChainController.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController.BufferCount = bufferCount;
	swapChainController.Scaling = DXGI_SCALING_STRETCH;
	swapChainController.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainController.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//LatestFactory->CreateSwapChain(PrimaryCommandQueue.Get(), swapChainController, swapChain.GetAddressOf());
	HRESULT res = LatestFactory->CreateSwapChainForHwnd(CommandQueue.Get(), hWnd, &swapChainController, nullptr, nullptr, swapChain.GetAddressOf());
	if (SUCCEEDED(res))
	{
		HRESULT res2 = LatestFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_PRINT_SCREEN);
		if (SUCCEEDED(res2))
		{
			swapChain.As(&SwapChain);

			if (SwapChain.GetAddressOf() == nullptr)
			{
				throw new std::exception;
			}
			else
			{
				return SUCCEEDED(res2);
			}
		}
		else
		{
			return FAILED(res2);

		}

		return FAILED(res);
	}

}

bool CDirectXCore::Intialize(HWND MainWindow, HINSTANCE hInst)
{
	std::cout << " Beginning DirectX Intitalization" << std::endl;

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

	//We are using the factory that we created on the seperate thread in the constructor to create a virtual instance of our graphics card.
	//For now we are going to assume default.

	HRESULT AdapterInit = LatestFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
	if (FAILED(AdapterInit))
	{
		HRESULT AdapterInitTwo = Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
		if (FAILED(AdapterInitTwo))
		{
			throw CGraphicsException();
		}
	}

	CreateDevice();


	D3D12_COMMAND_QUEUE_DESC CommandQueueController;
	CommandQueueController.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueController.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueController.NodeMask = 0;
	CommandQueueController.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	HRESULT res = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&PrimaryCommandQueue));
	if (SUCCEEDED(res))
	{
		std::cout << "";
	}
	else
	{
		std::cout << "";
	}

	D3D12_COMMAND_QUEUE_DESC CommandQueueControllerb;
	CommandQueueControllerb.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	CommandQueueControllerb.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueControllerb.NodeMask = 0;
	CommandQueueControllerb.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	HRESULT res2 = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&SecondaryCommandQueue));
	if (SUCCEEDED(res2))
	{
		std::cout << "";
	}
	else
	{
		std::cout << "";
	}


	if (CheckTearingSupport() == FALSE)
	{
		std::cout << "Does not support tearing";
	}

	//D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

	HRESULT res6 = CreateDescriptorHeap(GraphicsCard, DescriptorHeaps->RTVSamplerHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NumberOfBackBuffers, 0);
	HRESULT res7 = CreateDescriptorHeap(GraphicsCard, DescriptorHeaps->ComboHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumberOfBackBuffers, 0);
	HRESULT res8 = CreateDescriptorHeap(GraphicsCard, DescriptorHeaps->DSVStencilHeap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, NumberOfBackBuffers, 0);
	HRESULT res9 = CreateDescriptorHeap(GraphicsCard, DescriptorHeaps->SamplerDescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, NumberOfBackBuffers, 0);


	std::jthread BackBufferSetup = std::jthread([&](std::stop_token Stoken2) {
		for (auto i = 0; i < NumberOfBackBuffers; i++)
		{
			CreateCommandAllocator(GraphicsCard, D3D12_COMMAND_LIST_TYPE_DIRECT, gDirect_CommandAllocators[i]);
			CreateCommandAllocator(GraphicsCard, D3D12_COMMAND_LIST_TYPE_COMPUTE, gCompute_CommandAllocators[i]);
			CreateCommandAllocator(GraphicsCard, D3D12_COMMAND_LIST_TYPE_BUNDLE, gBundle_CommandAllocators[i]);

			if (Stoken2.stop_requested())
			{
				std::cout << "Stopping thread. Be aware Only call this if ABSOLUTELY NEEDED";
			}

		}

		if (Stoken2.stop_requested())
		{
			std::cout << "Stopping thread. Be aware Only call this if ABSOLUTELY NEEDED";
		}
		});


	HRESULT res10 = GraphicsCard->CreateFence(g_fenceValues[CurrentBuffer], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&BaseFence));
	g_fenceValues[CurrentBuffer]++;

	CreateOSEvent();

	if (fenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}


	if (BackBufferSetup.request_stop())
	{
		BackBufferSetup.join();
	}

	Window = MainWindow;
	Instance = hInst;

	RECT Background;
	GetClientRect(MainWindow, &Background);
	UINT nWidth = Background.right - Background.left;
	UINT nHeight = Background.bottom - Background.top;

	// To do: Check the volatility of this code!

	CreativeDirectXSupportedFeatures->FeatureLevel;
	CreativeDirectXSupportedFeatures->NumberOfFeatureLevels;
	CreativeDirectXDriverTypes->driverTypes;
	CreativeDirectXDriverTypes->numDriverTypes;

	return true;

}

void CDirectXCore::CreateWindowSizeDependentResources(HWND MainWindow, HINSTANCE hinst)
{
	GPU_STALL();

	for (int i = 0; i < NumberOfBackBuffers; i++)
	{
		BackBuffers[i] = nullptr;
		g_fenceValues[i] = g_fenceValues[CurrentBuffer];
	}

	UpdateRenderTargetSize();

	//This will stick at landscape for now, until I can get things working in other applications
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_RenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_RenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	UINT backBufferWidth = lround(m_RenderTargetSize.Width);
	UINT backBufferHeight = lround(m_RenderTargetSize.Height);

	if (PrimarySwapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT res = PrimarySwapChain->ResizeBuffers(NumberOfBackBuffers, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (res == DXGI_ERROR_DEVICE_REMOVED || res == DXGI_ERROR_DEVICE_RESET)
		{
			throw new CGraphicsException;
			// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
		}
		else
		{
			if (FAILED(res))
			{
				throw new CGraphicsException;
			}
		}
	}
	else
	{
		ComPtr<IDXGISwapChain1> FallbackSwapChain;

		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SCALING scaling = CDisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;						// Match the size of the window.
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = NumberOfBackBuffers;					// Use triple-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		//LatestFactory->CreateSwapChain(PrimaryCommandQueue.Get(), swapChainController, swapChain.GetAddressOf());
		HRESULT res = LatestFactory->CreateSwapChainForHwnd(PrimaryCommandQueue.Get(), MainWindow, &swapChainDesc, nullptr, nullptr, FallbackSwapChain.GetAddressOf());
		if (SUCCEEDED(res))
		{
			HRESULT res2 = LatestFactory->MakeWindowAssociation(MainWindow, DXGI_MWA_NO_PRINT_SCREEN);
			if (SUCCEEDED(res2))
			{
				FallbackSwapChain.As(&PrimarySwapChain);

				if (PrimarySwapChain.GetAddressOf() == nullptr)
				{
					throw new std::exception;
				}
				else
				{
				}
			}
			else
			{

			}

		}


	}

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_UNSPECIFIED:
		m_orientationTransform3D = CScreenRotation::Rotation0;
		break;
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform3D = CScreenRotation::Rotation270;
		break;
	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform3D = CScreenRotation::Rotation270;
		break;
	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform3D = CScreenRotation::Rotation270;
		break;
	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform3D = CScreenRotation::Rotation270;
		break;
	default:
		break;
	}

	CurrentBuffer = PrimarySwapChain->GetCurrentBackBufferIndex();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(RenderTargetViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < NumberOfBackBuffers; i++)
	{
		HRESULT res = PrimarySwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffers[i]));
		if (SUCCEEDED(res))
		{
			GraphicsCard->CreateRenderTargetView(BackBuffers[i].Get(), nullptr, rtvDescriptor);
			rtvDescriptor.Offset(G_RenderTargetViewDescriptorSize);
		}
	}

	D3D12_HEAP_PROPERTIES DepthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC DepthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1);
	DepthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	CD3DX12_CLEAR_VALUE depthOptimisedClearValue(m_depthBufferFormat, 1.0f, 0);

	HRESULT res = GraphicsCard->CreateCommittedResource(&DepthHeapProperties, D3D12_HEAP_FLAG_NONE, &DepthResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimisedClearValue, IID_PPV_ARGS(&m_depthStencil));
	if (SUCCEEDED(res))
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		GraphicsCard->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	g_Viewport0 = { 0.0f, 0.0f, m_RenderTargetSize.Width, m_RenderTargetSize.Height, 0.0f, 1.0f };
}

void CDirectXCore::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	if (!CDisplayMetrics::SupportHighResolutions && m_dpi > CDisplayMetrics::DpiThreshold)
	{
		float width = DX::ConvertDipsToPixels(m_RenderTargetSize.Width, m_dpi);
		float height = DX::ConvertDipsToPixels(m_RenderTargetSize.Height, m_dpi);

		if (max(width, height) > CDisplayMetrics::WidthThreshold && min(width, height) > CDisplayMetrics::HeightThreshold)
		{
			m_effectiveDpi /= 2.0f;
		}
	}

	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);

}

DXGI_MODE_ROTATION CDirectXCore::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	/*switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}*/


	return DXGI_MODE_ROTATION_IDENTITY;


}

void CDirectXCore::CreateDevice()
{
	HRESULT DeviceInit = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
	if (FAILED(DeviceInit))
	{
		HRESULT FallbackDeviceInit = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
		if (FAILED(FallbackDeviceInit))
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			HRESULT res = LatestFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			if (SUCCEEDED(res))
			{
				HRESULT res2 = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&GraphicsCard));
				if (FAILED(res2))
				{
					throw CGraphicsException();
				}
				else
				{

				}
			}

			if (FAILED(res))
			{
				throw CGraphicsException();
			}

		}
	}
}

bool CDirectXCore::CheckTearingSupport()
{
	bool TEARING_SUPPORTED = FALSE;

	ComPtr<IDXGIFactory4> BaseFactory;

	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&BaseFactory))))
	{
		ComPtr<IDXGIFactory7> TestFactory;

		if (SUCCEEDED(BaseFactory.As(&TestFactory)))
		{
			if (FAILED(TestFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &TEARING_SUPPORTED, sizeof(TEARING_SUPPORTED))))
			{
				TEARING_SUPPORTED = FALSE;
			}
		}

	}

	return TEARING_SUPPORTED == TRUE;
}

void CDirectXCore::CreateConstantBuffer(int BufferSize)
{
	//To Do: Finish the implementation of constant buffers -- Come back and get this ready for multiple GPUs

	const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.

	D3D12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC ResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

	D3D12_HEAP_PROPERTIES* HeapPropertiesPtr = &HeapProperties;
	D3D12_RESOURCE_DESC* ResourceDescriptionPtr = &ResourceDescription;

	HRESULT res = GraphicsCard->CreateCommittedResource(
		HeapPropertiesPtr,
		D3D12_HEAP_FLAG_NONE,
		ResourceDescriptionPtr,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer));

	if (SUCCEEDED(res))
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		GraphicsCard->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&CBVDataBegin));
		memcpy(CBVDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	}

}

CVertexBuffer* CDirectXCore::CreateVertexBuffer(void* vertexData, int VertexStride, int bufferSize)
{

	D3D12_RESOURCE_DESC vertexBufferDesc;
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.Alignment = 0;
	vertexBufferDesc.Width = bufferSize;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.MipLevels = 1;
	vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.SampleDesc.Quality = 0;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES defaultHeapProperties;
	defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeapProperties.CreationNodeMask = 0;
	defaultHeapProperties.VisibleNodeMask = 0;

	GraphicsCard->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&m_vertexBuffer));

	CVertexBuffer* vertexBuffer = new CVertexBuffer(m_vertexBuffer, GraphicsCard, D3D12_RESOURCE_STATE_COPY_DEST, VertexStride, bufferSize);

	return vertexBuffer;
}

HRESULT CDirectXCore::CreateCommandAllocator(ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> CommandAllocator)
{
	//Remember these represent the backing memory behind command lists, when multithreading you will need more than one of these.
	//Just be careful about making too many. Set a hard limit and stick to it!

	HRESULT res = GraphicsCard->CreateCommandAllocator(type, IID_PPV_ARGS(&CommandAllocator));
	if (FAILED(res))
	{
		return(FAILED(res));
	}
	else if (SUCCEEDED(res))
	{
		AmountOfCommandAllocators++;
		return (SUCCEEDED(res));
	}

	AmountOfCommandAllocators++;
	return (SUCCEEDED(res));
}

ComPtr<ID3D12GraphicsCommandList5> CDirectXCore::CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	HRESULT res = GraphicsCard->CreateCommandList(0, type, CmdAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));
	if (FAILED(res))
	{
		throw new CGraphicsException;
	}
	if (SUCCEEDED(res))
	{
		HRESULT res2 = CommandList->Close();
		if (FAILED(res2))
		{
			throw new CGraphicsException;
		}

		return CommandList;

	}

	return CommandList;

}

ComPtr<ID3D12GraphicsCommandList5> CDirectXCore::CreateCommandList(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12GraphicsCommandList5> CommandList, ComPtr<ID3D12CommandAllocator> CmdAllocator, D3D12_COMMAND_LIST_TYPE type, bool UseMultipleGPU)
{
	HRESULT res = GraphicsCard->CreateCommandList(!UseMultipleGPU, type, CmdAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));
	if (FAILED(res))
	{
		throw new CGraphicsException;
	}
	if (SUCCEEDED(res))
	{
		HRESULT res2 = CommandList->Close();
		if (FAILED(res2))
		{
			throw new CGraphicsException;
		}

		return CommandList;

	}

	return CommandList;
}

//HRESULT CDirectXCore::CreateFence(UINT64 FenceValues[3], ComPtr<ID3D12Device8> GraphicsCard, D3D12_FENCE_FLAGS FenceFlags)
//{
//	ComPtr<ID3D12Fence1> Fence;
//
//	HRESULT res = GraphicsCard->CreateFence(fenceValues[CurrentBuffer], FenceFlags, IID_PPV_ARGS(&Fence));
//	if (SUCCEEDED(res))
//	{
//		return(SUCCEEDED(res));
//	}
//	else
//	{
//		return(FAILED(res));
//	}
//}
//HRESULT CDirectXCore::CreateFence(UINT64 FenceValues[], ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12Fence1> Fence, D3D12_FENCE_FLAGS FenceFlags)
//{
//	HRESULT res = GraphicsCard->CreateFence(fenceValues[CurrentBuffer], FenceFlags, IID_PPV_ARGS(&Fence));
//	if (SUCCEEDED(res))
//	{
//		return(SUCCEEDED(res));
//	}
//	else
//	{
//		return(FAILED(res));
//	}
//}

HANDLE CDirectXCore::CreateOSEvent()
{
	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	return fenceEvent;
}

uint64_t CDirectXCore::Signal(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Fence1> fence, uint64_t& fenceValue)
{
	uint64_t fenceValueForSignal = ++fenceValue;

	HRESULT res = CommandQueue->Signal(fence.Get(), fenceValueForSignal);
	if (FAILED(res))
	{
		throw new CGraphicsException;
	}

	return fenceValueForSignal;
}

void CDirectXCore::WaitForFenceValue(ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent)
{
	std::chrono::milliseconds duration = std::chrono::milliseconds::duration();

	if (Fence == nullptr)
	{
		throw new CGraphicsException;
	}

	if (Fence->GetCompletedValue() < fenceValue)
	{
		HRESULT x = Fence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(x))
		{
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
		}
	}
}

void CDirectXCore::FlushGraphicsAccelerator(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence1> Fence, uint64_t fenceValue, HANDLE fenceEvent)
{
	int FenceValueForSignal = Signal(commandQueue, Fence, fenceValue);
	WaitForFenceValue(Fence, FenceValueForSignal, fenceEvent);
}

void CDirectXCore::GPU_STALL()
{
	HRESULT res = PrimaryCommandQueue->Signal(BaseFence.Get(), g_fenceValues[CurrentBackBufferIndex]);
	if (FAILED(res))
	{
		HRESULT res2 = SecondaryCommandQueue->Signal(BaseFence.Get(), g_fenceValues[CurrentBackBufferIndex]);
		if (FAILED(res2))
		{

		}
	}

	BaseFence->SetEventOnCompletion(g_fenceValues[CurrentBuffer], fenceEvent);
	WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);

	g_fenceValues[CurrentBuffer]++;

}

void CDirectXCore::UpdateFrameTicker()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;

	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count() * 1e-9;

	if (elapsedSeconds > 1.0)
	{
		char buffer[500];
		auto framesPerSecond = frameCounter / elapsedSeconds;
		sprintf_s(buffer, 500, "Frames Per Second: %f\n", framesPerSecond);
		OutputDebugString((LPCWSTR)buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

}

void CDirectXCore::RenderFrame()
{
	auto DirectCommandAllocator = gDirect_CommandAllocators[CurrentBackBufferIndex];
	auto backBuffer = BackBuffers[CurrentBackBufferIndex];

	DirectCommandAllocator->Reset();
	globalCommandList->Reset(DirectCommandAllocator.Get(), nullptr);


	//Clear the rendering target

	{
		D3D12_RESOURCE_BARRIER ResourceBarrier;

		// Setup a resource barrier.
		// https://developer.nvidia.com/dx12-dos-and-donts -- Things you must remember when writing code with DX12

		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		globalCommandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		FLOAT EmptyColour[] = { R, G, B, A };

		CD3DX12_CPU_DESCRIPTOR_HANDLE RTV(RenderTargetViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBackBufferIndex, G_RenderTargetViewDescriptorSize);

		globalCommandList->ClearRenderTargetView(RTV, EmptyColour, 0, nullptr);

	}

	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		globalCommandList->ResourceBarrier(1, &barrier);

		HRESULT res = globalCommandList->Close();
		if (SUCCEEDED(res))
		{
			ID3D12CommandList* const CommandLists[] = {
				globalCommandList.Get()
			};

			PrimaryCommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

			UINT syncInterval = VSYNC_ENABLED ? 1 : 0;
			UINT presentFlags = TEARING_SUPPORTED && !VSYNC_ENABLED ? DXGI_PRESENT_ALLOW_TEARING : 0;
			PrimarySwapChain->Present(syncInterval, presentFlags);

			//Now onto fencing, go grab some lunch and come back.
			g_fenceValues[CurrentBackBufferIndex] = Signal(PrimaryCommandQueue, BaseFence, g_fenceValues[CurrentBuffer]);

		}
		if (FAILED(res))
		{
			//To do: Finish all of this
		}


	}


}

DXGI_SWAP_CHAIN_DESC* CDirectXCore::SetupSwapChain(HWND MainWindow, UINT nWidth, UINT nHeight, int DoUseAntiAlising)
{
	swapChainController.BufferCount = 2;
	swapChainController.BufferDesc.Width = nWidth;
	swapChainController.BufferDesc.Height = nHeight;
	swapChainController.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController.BufferDesc.RefreshRate.Numerator = 60;
	swapChainController.BufferDesc.RefreshRate.Denominator = 1;
	swapChainController.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController.OutputWindow = (HWND)MainWindow;
	swapChainController.SampleDesc.Count = DoUseAntiAlising;
	swapChainController.SampleDesc.Quality = 0;
	swapChainController.Windowed = true;

	return &swapChainController;
}

DXGI_SWAP_CHAIN_DESC* CDirectXCore::SetupSwapChain(HWND MainWindow, UINT nWidth, UINT nHeight, int DoUseAntiAlising, int AntiAilisngQuality)
{
	swapChainController.BufferCount = 2;
	swapChainController.BufferDesc.Width = nWidth;
	swapChainController.BufferDesc.Height = nHeight;
	swapChainController.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController.BufferDesc.RefreshRate.Numerator = 60;
	swapChainController.BufferDesc.RefreshRate.Denominator = 1;
	swapChainController.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController.OutputWindow = (HWND)MainWindow;
	swapChainController.SampleDesc.Count = DoUseAntiAlising;
	swapChainController.SampleDesc.Quality = AntiAilisngQuality;
	swapChainController.Windowed = true;

	return &swapChainController;
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

bool CDirectXCore::CompileShader(LPCWSTR szFilePath, LPCSTR szFunc, LPCSTR szShaderModel, ID3DBlob** buffer)
{
	DWORD CompilierFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG


#endif // DEBUG

	HRESULT HResultShaderCompileSuccess;
	ComPtr<ID3DBlob> ErrorBuffer = 0;

	HResultShaderCompileSuccess = D3DCompileFromFile(szFilePath, 0, 0, szFunc, szShaderModel, CompilierFlags, 0, 0, buffer);


	// Check for errors
	if (FAILED(HResultShaderCompileSuccess)) {
		if (ErrorBuffer != NULL) {
			::OutputDebugStringA((char*)ErrorBuffer->GetBufferPointer());
			ErrorBuffer->Release();
		}
		return false;
	}

	// Cleanup
	if (ErrorBuffer != NULL)
		ErrorBuffer->Release();


	return false;
}

void CDirectXCore::Paused()
{
}

void CDirectXCore::UnPaused()
{
}

#pragma optimize ("",on)
