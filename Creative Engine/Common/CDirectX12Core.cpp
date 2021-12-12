#include "pch.h"
#include "CDirectX12Core.h"
#include  <forward_list>
#include "DirectXHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;


CDirectX::CDirectX12Core::CDirectX12Core(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat) :
	CurrentBuffer(0),
	m_screenViewport(),
	m_rtvDescriptorSize(0),
	m_fenceEvent(0),
	m_backBufferFormat(backBufferFormat),
	m_depthBufferFormat(depthBufferFormat),
	m_fenceValues{},
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_deviceRemoved(false)
{

	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CheckMSAAFeatures();
	SystemCheck();
}

void CDirectX::CDirectX12Core::CheckMSAAFeatures()
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
				SupportedMSAALevels.insert(std::pair <std::string, bool>("SixteenMSAA", true));
			}
			else
			{
				SupportedMSAALevels.insert(std::pair <std::string, bool>("EightMSAA", true));
			}
		}

		else
		{
			SupportedMSAALevels.insert(std::pair <std::string, bool>("FourMSAA", true));
		}
	}
	else
	{
		throw new CGraphicsException;
	}
}

void CDirectX::CDirectX12Core::SystemCheck()
{
	HRESULT FactoryResult = CreateDXGIFactory1(__uuidof(LatestFactory), &LatestFactory);
	if (FAILED(FactoryResult))
	{

		HRESULT FactoryResultOne = CreateDXGIFactory1(__uuidof(Factory), &Factory);
		if (FAILED(FactoryResultOne))
		{
			throw CGraphicsException();

		}

	}

	HRESULT DeviceInit = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
	if (FAILED(DeviceInit))
	{
		HRESULT FallbackDeviceInit = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
		if (FAILED(FallbackDeviceInit))
		{
			throw CGraphicsException();
		}
	}

	CDirectXSupportedFeatures* SupportedFeatures = new CDirectXSupportedFeatures();

	CheckGPUFeatures(SupportedFeatures);

	delete(SupportedFeatures);
}

void CDirectX::CDirectX12Core::CheckGPUFeatures(CDirectXSupportedFeatures* SupportedFeatures)
{
	if (SupportedFeatures != nullptr)
	{
		std::forward_list<HRESULT> ListOfResults[3];

		std::forward_list<HRESULT>::iterator Iterator0;
		std::forward_list<HRESULT>::iterator Iterator1;
		std::forward_list<HRESULT>::iterator Iterator2;


		HRESULT BasicArch = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &SupportedFeatures->SupportData0, sizeof(SupportedFeatures->SupportData0));
		if (SUCCEEDED(BasicArch))
		{
			Iterator0 = ListOfResults[0].insert_after(ListOfResults[0].before_begin(), BasicArch);
		}

		HRESULT FormatSupport = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &SupportedFeatures->SupportData1, sizeof(SupportedFeatures->SupportData1));
		if (SUCCEEDED(FormatSupport))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, FormatSupport);

		}

		HRESULT BasicFeature = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &SupportedFeatures->SupportData2, sizeof(SupportedFeatures->SupportData2));
		if (SUCCEEDED(FormatSupport))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, BasicFeature);
		}

		HRESULT MultisampleQuality = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &SupportedFeatures->SupportData3, sizeof(SupportedFeatures->SupportData3));
		Iterator0 = ListOfResults[0].insert_after(Iterator0, MultisampleQuality);

		HRESULT FormatInfo = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &SupportedFeatures->SupportData4, sizeof(SupportedFeatures->SupportData4));
		if (SUCCEEDED(FormatSupport))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, FormatInfo);
		}

		HRESULT VirtualAddress = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &SupportedFeatures->SupportData5, sizeof(SupportedFeatures->SupportData5));
		if (SUCCEEDED(VirtualAddress))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, VirtualAddress);
		}

		HRESULT ShaderModel = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SupportedFeatures->SupportData6, sizeof(SupportedFeatures->SupportData6));
		if (SUCCEEDED(ShaderModel))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, ShaderModel);
		}

		HRESULT Options1 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &SupportedFeatures->SupportData7, sizeof(SupportedFeatures->SupportData7));
		if (SUCCEEDED(Options1))
		{
			Iterator0 = ListOfResults[0].insert_after(Iterator0, Options1);
		}


		HRESULT ProtectedResourceSession = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &SupportedFeatures->SupportData8, sizeof(SupportedFeatures->SupportData8));
		if (SUCCEEDED(ProtectedResourceSession))
		{
			Iterator1 = ListOfResults[1].insert_after(ListOfResults[1].before_begin(), ProtectedResourceSession);
		}

		HRESULT RootSignature = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &SupportedFeatures->SupportData9, sizeof(SupportedFeatures->SupportData9));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, ProtectedResourceSession);

		HRESULT ArchOne = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &SupportedFeatures->SupportData10, sizeof(SupportedFeatures->SupportData10));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, ArchOne);

		HRESULT Options2 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &SupportedFeatures->SupportData11, sizeof(SupportedFeatures->SupportData11));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, Options2);

		HRESULT ShaderCache = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &SupportedFeatures->SupportData12, sizeof(SupportedFeatures->SupportData12));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, ShaderCache);

		HRESULT VirtualAddressSupport = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &SupportedFeatures->SupportData13, sizeof(SupportedFeatures->SupportData13));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, VirtualAddressSupport);

		HRESULT Options3 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &SupportedFeatures->SupportData14, sizeof(SupportedFeatures->SupportData14));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, Options3);

		HRESULT ExistingHeaps = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &SupportedFeatures->SupportData15, sizeof(SupportedFeatures->SupportData15));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, ExistingHeaps);

		HRESULT Options4 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &SupportedFeatures->SupportData16, sizeof(SupportedFeatures->SupportData16));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, Options4);

		HRESULT FeatureSerialization = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SERIALIZATION, &SupportedFeatures->SupportData17, sizeof(SupportedFeatures->SupportData17));
		Iterator1 = ListOfResults[1].insert_after(Iterator1, FeatureSerialization);



		HRESULT CrossNode = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE, &SupportedFeatures->SupportData18, sizeof(SupportedFeatures->SupportData18));
		Iterator2 = ListOfResults[2].insert_after(ListOfResults[2].before_begin(), CrossNode);

		HRESULT Raytracing = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &SupportedFeatures->SupportData19, sizeof(SupportedFeatures->SupportData19));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, Raytracing);

		HRESULT Options6 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &SupportedFeatures->SupportData20, sizeof(SupportedFeatures->SupportData20));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, Options6);

		HRESULT QueryMeta = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_QUERY_META_COMMAND, &SupportedFeatures->SupportData21, sizeof(SupportedFeatures->SupportData21));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, QueryMeta);

		HRESULT Options7 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &SupportedFeatures->SupportData22, sizeof(SupportedFeatures->SupportData22));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, Options7);

		HRESULT ProtectedResourceCount = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT, &SupportedFeatures->SupportData23, sizeof(SupportedFeatures->SupportData23));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, ProtectedResourceCount);

		HRESULT ProtectedResourceTypes = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES, &SupportedFeatures->SupportData24, sizeof(SupportedFeatures->SupportData24));
		Iterator2 = ListOfResults[2].insert_after(Iterator2, ProtectedResourceTypes);


		//Dx12FeaturesToCheckThreada.join();

		for (auto i = ListOfResults[0].begin(); i != ListOfResults[0].end(); i++)
		{
			if (FAILED(*i))
			{
				UnsupportedFeatureCount++;
			}

			if (SUCCEEDED(*i))
			{
				SupportedFeatureCount++;
			}
		}

		//Dx12FeaturesToCheckThreadb.join();

		for (auto i = ListOfResults[1].begin(); i != ListOfResults[1].end(); i++)
		{
			if (FAILED(*i))
			{
				UnsupportedFeatureCount++;
			}

			if (SUCCEEDED(*i))
			{
				SupportedFeatureCount++;
			}
		}


		for (auto i = ListOfResults[2].begin(); i != ListOfResults[2].end(); i++)
		{
			if (FAILED(*i))
			{
				UnsupportedFeatureCount++;
			}

			if (SUCCEEDED(*i))
			{
				SupportedFeatureCount++;
			}
		}


	}
	else
	{

	}
}

void CDirectX::CDirectX12Core::CreateDeviceIndependentResources()
{
}

void CDirectX::CDirectX12Core::CreateDeviceResources()
{
#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	std::thread InitThread = std::thread([&] {

		HRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&LatestFactory));
		if (!SUCCEEDED(res))
		{
			HRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&Factory));
			if (!SUCCEEDED(res))
			{
				HRESULT res = CreateDXGIFactory1(IID_PPV_ARGS(&FallbackFactory));
				if (!SUCCEEDED(res))
				{
					LatestFactory.As(&FallbackFactory);
				}
				else
				{
					LatestFactory.As(&Factory);

				}
			}
		}

		GetHardwareAdapter(&adapter);

		CreateGraphicsCard();

		CreateCommandQueues();

		// Create descriptor heaps for render target views and depth stencil views.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = AmountOfFrames;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CDirectX::ThrowIfFailed(GraphicsCard->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
		NAME_D3D12_OBJECT(m_rtvHeap);

		m_rtvDescriptorSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(GraphicsCard->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		NAME_D3D12_OBJECT(m_dsvHeap);

		for (UINT n = 0; n < AmountOfFrames; n++)
		{
			CDirectX::ThrowIfFailed(
				GraphicsCard->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&DirectCommandAllocators[n]))
			);
			CDirectX::ThrowIfFailed(
				GraphicsCard->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&ComputeCommandAllocators[n]))
			);
		}
	});

	InitThread.join();

	// Create sync fences
	CDirectX::ThrowIfFailed(GraphicsCard->CreateFence(m_fenceValues[CurrentBuffer], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&GraphicsFence)));
	m_fenceValues[CurrentBuffer]++;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		CDirectX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}


bool CDirectX::CDirectX12Core::CreateGraphicsCard()
{
	HRESULT resGraphicsCard = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&GraphicsCard));
	if (FAILED(resGraphicsCard))
	{
		HRESULT resGraphicsCard = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
		if (FAILED(resGraphicsCard))
		{
			HRESULT resGraphicsCard = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&GraphicsCard));
			if (FAILED(resGraphicsCard))
			{
				if (CreateBackupGraphicsCard() == false)
				{
					throw new CException;
				}
				else
				{
					return true;
				}
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool CDirectX::CDirectX12Core::CreateBackupGraphicsCard()
{
	HRESULT GraphicsRes = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&BackupGraphicsCard));
	if (!SUCCEEDED(GraphicsRes))
	{
		HRESULT GraphicsRes = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&BackupGraphicsCard));
		if (FAILED(GraphicsRes))
		{
			HRESULT GraphicsRes = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&BackupGraphicsCard));
			if (FAILED(GraphicsRes))
			{
				return false;
			}
			else
			{
				GraphicsCard.As(&BackupGraphicsCard);
				return true;
			}
		}
		else
		{
			GraphicsCard.As(&BackupGraphicsCard);
			return true;
		}
	}
	else
	{
		GraphicsCard.As(&BackupGraphicsCard);
		return true;
	}
}

void CDirectX::CDirectX12Core::CreateCommandQueues()
{
	// Create the command queues.
	D3D12_COMMAND_QUEUE_DESC PriqueueDesc = {};
	PriqueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	PriqueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	PriqueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	CDirectX::ThrowIfFailed(GraphicsCard->CreateCommandQueue(&PriqueueDesc, IID_PPV_ARGS(&PrimaryCommandQueue)));
	NAME_D3D12_OBJECT(PrimaryCommandQueue);
	CommandQueueArray.push_back(PrimaryCommandQueue);


	D3D12_COMMAND_QUEUE_DESC SecQueueDesc = {};
	SecQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	SecQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	SecQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	CDirectX::ThrowIfFailed(GraphicsCard->CreateCommandQueue(&SecQueueDesc, IID_PPV_ARGS(&SecondaryCommandQueue)));
	NAME_D3D12_OBJECT(SecondaryCommandQueue);
	CommandQueueArray.push_back(SecondaryCommandQueue);

	D3D12_COMMAND_QUEUE_DESC TertQueueDesc = {};
	TertQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	TertQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	TertQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	CDirectX::ThrowIfFailed(GraphicsCard->CreateCommandQueue(&TertQueueDesc, IID_PPV_ARGS(&TertiaryCommandQueue)));
	NAME_D3D12_OBJECT(SecondaryCommandQueue);
	CommandQueueArray.push_back(SecondaryCommandQueue);
}

// These resources need to be recreated every time the window size is changed.
void CDirectX::CDirectX12Core::CreateWindowSizeDependentResources()
{
	WaitForGpu();

	// Clear the previous window size specific content and update the fence values.
	for (UINT n = 0; n < AmountOfFrames; n++)
	{
		m_renderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[CurrentBuffer];
	}

	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	UINT backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	UINT backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	//If swapchains are empty make a new one!
	if (SwapChain != nullptr)
	{
		HRESULT hr = SwapChain->ResizeBuffers(AmountOfFrames, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			m_deviceRemoved = true;
			return;
		}
		else
		{
			CDirectX::ThrowIfFailed(hr);
		}
	}
	else
	{
		DXGI_SCALING scaling = CDisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = AmountOfFrames;					// Use triple-buffering we defined in the header file
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;

		HRESULT res = LatestFactory->CreateSwapChainForCoreWindow(PrimaryCommandQueue.Get(), reinterpret_cast<IUnknown*>(m_window.Get()), &swapChainDesc, nullptr, &swapChain);
		if (SUCCEEDED(res))
		{
			swapChain.As(&SwapChain);
		}
		else
		{
			CDirectX::ThrowIfFailed(
				LatestFactory->CreateSwapChainForCoreWindow(
					PrimaryCommandQueue.Get(),								// Swap chains need a reference to the command queue in DirectX 12.
					reinterpret_cast<IUnknown*>(m_window.Get()),
					&swapChainDesc,
					nullptr,
					&swapChain
				));

			CDirectX::ThrowIfFailed(swapChain.As(&SwapChain));
		}

	}

	GetDisplayRotationForRotatedScreens(displayRotation);

	CurrentBuffer = SwapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < AmountOfFrames; n++)
	{
		CDirectX::ThrowIfFailed(SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		GraphicsCard->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvDescriptor);
		rtvDescriptor.Offset(m_rtvDescriptorSize);

		WCHAR name[25];
		if (swprintf_s(name, L"m_renderTargets[%u]", n) > 0)
		{
			CDirectX::SetName(m_renderTargets[n].Get(), name);
		}
	}

	D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1);
	depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_depthBufferFormat, 1.0f, 0);

	ThrowIfFailed(GraphicsCard->CreateCommittedResource(
		&depthHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));

	NAME_D3D12_OBJECT(m_depthStencil);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = m_depthBufferFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	GraphicsCard->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());


	m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };
}

void CDirectX::CDirectX12Core::GetDisplayRotationForRotatedScreens(DXGI_MODE_ROTATION displayRotation)
{
	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		DisplayOrientation = CScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		DisplayOrientation = CScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		DisplayOrientation = CScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		DisplayOrientation = CScreenRotation::Rotation90;
		break;

	default:
		throw ref new FailureException();
	}

	CDirectX::ThrowIfFailed(
		SwapChain->SetRotation(displayRotation)
	);
}

void CDirectX::CDirectX12Core::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	// Calculate the necessary render target size in pixels.
	m_outputSize.Width = CDirectX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = CDirectX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

void CDirectX::CDirectX12Core::SetWindow(CoreWindow^ window)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_dpi = currentDisplayInformation->LogicalDpi;

	CreateWindowSizeDependentResources();
}

void CDirectX::CDirectX12Core::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

void CDirectX::CDirectX12Core::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		m_logicalSize = Windows::Foundation::Size(m_window->Bounds.Width, m_window->Bounds.Height);

		CreateWindowSizeDependentResources();
	}
}

void CDirectX::CDirectX12Core::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

void CDirectX::CDirectX12Core::ValidateDevice()
{
	DXGI_ADAPTER_DESC previousDesc;
	{
		ComPtr<IDXGIAdapter1> previousDefaultAdapter;
		CDirectX::ThrowIfFailed(LatestFactory->EnumAdapters1(0, &previousDefaultAdapter));

		CDirectX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	DXGI_ADAPTER_DESC currentDesc;
	{
		ComPtr<IDXGIFactory4> currentDxgiFactory;
		CDirectX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		ComPtr<IDXGIAdapter1> currentDefaultAdapter;
		CDirectX::ThrowIfFailed(currentDxgiFactory->EnumAdapters1(0, &currentDefaultAdapter));

		CDirectX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(GraphicsCard->GetDeviceRemovedReason()))
	{
		m_deviceRemoved = true;
	}
}

void CDirectX::CDirectX12Core::Present()
{
	HRESULT hr = SwapChain->Present(1, 0);

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		CDirectX::ThrowIfFailed(hr);

		MoveToNextFrame();
	}
}

void CDirectX::CDirectX12Core::WaitForGpu()
{
	CDirectX::ThrowIfFailed(PrimaryCommandQueue->Signal(GraphicsFence.Get(), m_fenceValues[CurrentBuffer]));

	CDirectX::ThrowIfFailed(GraphicsFence->SetEventOnCompletion(m_fenceValues[CurrentBuffer], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	m_fenceValues[CurrentBuffer]++;
}

void CDirectX::CDirectX12Core::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_fenceValues[CurrentBuffer];
	CDirectX::ThrowIfFailed(PrimaryCommandQueue->Signal(GraphicsFence.Get(), currentFenceValue));

	CurrentBuffer = SwapChain->GetCurrentBackBufferIndex();

	if (GraphicsFence->GetCompletedValue() < m_fenceValues[CurrentBuffer])
	{
		CDirectX::ThrowIfFailed(GraphicsFence->SetEventOnCompletion(m_fenceValues[CurrentBuffer], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValues[CurrentBuffer] = currentFenceValue + 1;
}

DXGI_MODE_ROTATION CDirectX::CDirectX12Core::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	switch (m_nativeOrientation)
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
	}
	return rotation;
}

void CDirectX::CDirectX12Core::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != LatestFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

bool CDirectX::CDirectX12Core::Intialize(HWND MainWindow, HINSTANCE hInst)
{
	return false;
}

void CDirectX::CDirectX12Core::Tick(float DeltaTime)
{
}

void CDirectX::CDirectX12Core::Tick()
{
}

void CDirectX::CDirectX12Core::DeIntialize()
{
}
