#pragma optimize ("",off)

#include "CDirectXCore.h"
#include "CreativeGraphicsUtilities.h"

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

ComPtr<ID3D12CommandQueue> CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType)
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueController = {};
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Priority = 1;
	CommandQueueController.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueController.NodeMask = 0;

	GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));


	return CommandQueue;
}

ComPtr<ID3D12CommandQueue> CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, int CommandQueuePriority)
{
	if (CommandQueuePriority > 3)
	{
		throw new std::exception;
	}

	D3D12_COMMAND_QUEUE_DESC CommandQueueController = {};
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Priority = CommandQueuePriority;
	CommandQueueController.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueController.NodeMask = 0;

	GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));


	return CommandQueue;
}

//Used for creating high priority command queues.
ComPtr<ID3D12CommandQueue> CDirectXCore::SetupCommandQueue(ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE CommandListType, int CommandQueuePriority, D3D12_COMMAND_QUEUE_FLAGS Flags, bool UseMultipleGPUs)
{
	if (CommandQueuePriority > 3)
	{
		throw new std::exception;
	}

	D3D12_COMMAND_QUEUE_DESC CommandQueueController = {};
	CommandQueueController.Type = CommandListType;
	CommandQueueController.Priority = CommandQueuePriority;
	CommandQueueController.Flags = Flags;
	CommandQueueController.NodeMask = UseMultipleGPUs;

	HRESULT x = GraphicsCard->CreateCommandQueue(&CommandQueueController, IID_PPV_ARGS(&CommandQueue));
	if (FAILED(x))
	{
		throw new std::exception;

	}

	return CommandQueue;
}

ComPtr<ID3D12DescriptorHeap> CDirectXCore::CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12DescriptorHeap> DescHeap, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator)
{
	if (GraphicsCard == nullptr) { throw new std::exception; }
	if (DescHeap == nullptr) { throw new std::exception; }


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


		HRESULT x = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
		if (FAILED(x))
		{
			throw new std::exception;

		}

		return DescHeap;

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


		HRESULT x = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
		if (FAILED(x))
		{
			throw new CGraphicsException;

		}

		return DescHeap;

	}

}

const ComPtr<ID3D12DescriptorHeap>& CDirectXCore::UseFlags(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard, ComPtr<ID3D12DescriptorHeap>& DescHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC Description = {};

	Description.NumDescriptors = NumberOfDescriptors;
	Description.Type = Type;
	Description.NodeMask = UseSecondaryAccelerator;
	Description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;


	HRESULT x = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&DescHeap));
	if (FAILED(x))
	{
		throw new std::exception;
	}

	return DescHeap;
}

const ComPtr<ID3D12DescriptorHeap>& CDirectXCore::UseFlags2(const uint32_t& NumberOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type, int UseSecondaryAccelerator, ComPtr<ID3D12Device8>& GraphicsCard)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC Description = {};

	Description.NumDescriptors = NumberOfDescriptors;
	Description.Type = Type;
	Description.NodeMask = UseSecondaryAccelerator;
	Description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;


	HRESULT x = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&descriptorHeap));
	if (FAILED(x))
	{
		throw new std::exception;
	}

	return descriptorHeap;
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
ComPtr<ID3D12DescriptorHeap> CDirectXCore::CreateDescriptorHeap(ComPtr<ID3D12Device8> GraphicsCard, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumberOfDescriptors, int UseSecondaryAccelerator)
{

	if (GraphicsCard == nullptr) { throw new std::exception; }

	if (UseSecondaryAccelerator != 0)
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return UseFlags2(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard);
		}

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = NumberOfDescriptors;
		desc.Type = Type;
		desc.NodeMask = UseSecondaryAccelerator;

		GraphicsCard->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));

		return descriptorHeap;
	}
	else
	{
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		{
			return UseFlags2(NumberOfDescriptors, Type, UseSecondaryAccelerator, GraphicsCard);
		}

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;

		D3D12_DESCRIPTOR_HEAP_DESC Description = {};

		Description.NumDescriptors = NumberOfDescriptors;
		Description.Type = Type;
		Description.NodeMask = 1;


		HRESULT x = GraphicsCard->CreateDescriptorHeap(&Description, IID_PPV_ARGS(&descriptorHeap));
		if (FAILED(x))
		{
			throw new std::exception;
		}

		return descriptorHeap;
	}
}

ComPtr<IDXGISwapChain4> CDirectXCore::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> CommandQueue, ComPtr<IDXGISwapChain4> SwapChain, ComPtr<ID3D12Device8> GraphicsCard, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	if (GraphicsCard == nullptr) { throw new std::exception; }
	if (CommandQueue == nullptr) { throw new std::exception; }

	UINT createFactoryFlags = 0;

	DXGI_SWAP_CHAIN_DESC1* swapChainController = {};
	ComPtr<IDXGISwapChain1> swapChain;

	swapChainController->Width = width;
	swapChainController->Height = height;
	swapChainController->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainController->Stereo = FALSE;
	swapChainController->SampleDesc = { 1,0 };
	swapChainController->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainController->BufferCount = bufferCount;
	swapChainController->Scaling = DXGI_SCALING_STRETCH;
	swapChainController->SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainController->AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//LatestFactory->CreateSwapChain(PrimaryCommandQueue.Get(), swapChainController, swapChain.GetAddressOf());
	LatestFactory->CreateSwapChainForHwnd(CommandQueue.Get(), hWnd, swapChainController, nullptr, OutputTargetMonitor.Get(), &swapChain);
	LatestFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_PRINT_SCREEN);
	swapChain.As(&SwapChain);

	if (SwapChain.GetAddressOf() == nullptr)
	{
		throw new std::exception;
	}

	return SwapChain;
}

bool CDirectXCore::Intialize(HWND MainWindow, HINSTANCE hInst)
{
	std::cout << " Beginning DirectX Intitalization" << std::endl;


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

	HRESULT DeviceInit = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
	if (FAILED(DeviceInit))
	{
		HRESULT FallbackDeviceInit = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
		if (FAILED(FallbackDeviceInit))
		{
			throw CGraphicsException();
		}
	}


	auto res = SetupCommandQueue(PrimaryCommandQueue.Get(), GraphicsCard, D3D12_COMMAND_LIST_TYPE_DIRECT, 3, D3D12_COMMAND_QUEUE_FLAG_NONE, true);
	auto res1 = SetupCommandQueue(SecondaryCommandQueue.Get(), GraphicsCard, D3D12_COMMAND_LIST_TYPE_DIRECT, 2, D3D12_COMMAND_QUEUE_FLAG_NONE, true);
	auto res2 = SetupCommandQueue(TertiaryCommandQueue.Get(), GraphicsCard, D3D12_COMMAND_LIST_TYPE_DIRECT, 1, D3D12_COMMAND_QUEUE_FLAG_NONE, false);

	if (res == nullptr)
	{
		std::cout << "Primary Command queue creation failed";
	}
	if (res1 || res2)
	{
		std::cout << "Secondary command queues creation failed";
	}

	if (CheckTearingSupport() == FALSE)
	{
		std::cout << "Does not support tearing";
	}


	CreateSwapChain(MainWindow, PrimaryCommandQueue, PrimarySwapChain, GraphicsCard, CW_USEDEFAULT, CW_USEDEFAULT, 2);
	CreateSwapChain(MainWindow, SecondaryCommandQueue, SecondarySwapChain, GraphicsCard, CW_USEDEFAULT, CW_USEDEFAULT, 2);
	CreateSwapChain(MainWindow, TertiaryCommandQueue, TeriarySwapChain, GraphicsCard, CW_USEDEFAULT, CW_USEDEFAULT, 2);




	// https://www.3dgep.com/learning-directx-12-1/#Create_Window_Instance -- Create a descriptor heap.


	//#if defined (_DEBUG)
	//
	//	ComPtr<ID3D12InfoQueue> DebugInfoQueueOne;
	//
	//	if (SUCCEEDED(GraphicsCard.As(&DebugInfoQueueOne)))
	//	{
	//		DebugInfoQueueOne->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	//		DebugInfoQueueOne->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	//		DebugInfoQueueOne->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
	//	}
	//
	//#endif (_DEBUG)


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

		TestFactory->Release();
	}

	return TEARING_SUPPORTED == TRUE;
}

CConstantBuffer* CDirectXCore::CreateConstantBuffer(int BufferSize)
{
	//To Do: Finish the implementation of constant buffers -- Come back and get this ready for multiple GPUs

	ComPtr<ID3D12Resource> ConstantBufferResource = NULL;
	uint32_t alignedSize;


	D3D12_RESOURCE_DESC ConstantBufferData;
	ConstantBufferData.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ConstantBufferData.Alignment = 0;
	ConstantBufferData.Width = alignedSize;
	ConstantBufferData.Height = 1;
	ConstantBufferData.DepthOrArraySize = 1;
	ConstantBufferData.MipLevels = 1;
	ConstantBufferData.Format = DXGI_FORMAT_UNKNOWN;
	ConstantBufferData.SampleDesc.Count = 1;
	ConstantBufferData.SampleDesc.Quality = 0;
	ConstantBufferData.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ConstantBufferData.Flags = D3D12_RESOURCE_FLAG_NONE;


	D3D12_HEAP_PROPERTIES HeapProperties;
	HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 0;
	HeapProperties.VisibleNodeMask = 0;



	HRESULT ResourceCreation = GraphicsCard->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ConstantBufferData, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&ConstantBufferResource));

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc = {};
	constantBufferViewDesc.BufferLocation = ConstantBufferResource->GetGPUVirtualAddress();
	constantBufferViewDesc.SizeInBytes = alignedSize;

	CResourceDescriptorDelegate constantBufferHeapHandle = mHeapManager->GetNewSRVDescriptorHeapHandle();
	GraphicsCard->CreateConstantBufferView(&constantBufferViewDesc, constantBufferHeapHandle.GetCPUHandle());

	//rewrite -- the signature is wrong.
	CConstantBuffer* constantBuffer = new CConstantBuffer(ConstantBufferResource, D3D12_RESOURCE_STATE_GENERIC_READ, alignedSize, &constantBufferHeapHandle);
	constantBuffer->SetState(true);



	//https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/UWP/D3D12PipelineStateCache/src/SimpleCamera.cpp -- for camera


}

CVertexBuffer* CDirectXCore::CreateVertexBuffer(void* vertexData, int VertexStride, int bufferSize)
{
	ComPtr<ID3D12Resource> VertexBufferResource = NULL;

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

	GraphicsCard->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&VertexBufferResource));

	CVertexBuffer* vertexBuffer = new CVertexBuffer(VertexBufferResource, GraphicsCard, D3D12_RESOURCE_STATE_COPY_DEST, VertexStride, bufferSize);

	return vertexBuffer;
}

ComPtr<ID3D12CommandAllocator> CDirectXCore::CreateCommandAllocator(ComPtr<ID3D12Device8> GraphicsCard, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> CommandAllocator)
{
	//Remember these represent the backing memory behind command lists, when multithreading you will need more than one of these.
	//Just be careful about making too many. Set a hard limit and stick to it!

	HRESULT x = GraphicsCard->CreateCommandAllocator(type, IID_PPV_ARGS(&CommandAllocator));
	if (FAILED(x))
	{
		std::cout << "Failed to initalize command allocator" << std::endl;
	}
	else if (SUCCEEDED(x))
	{
		if (AmountOfCommandAllocators == MaximumAmountOfCommandAllocators || AmountOfCommandAllocators == MaximumAmountOfCommandAllocators - 1)
		{
			throw new CGraphicsException();
		}
		else
		{
			std::cout << "Creating new Command Allocator, Command allocator list is at " << AmountOfCommandAllocators << "Out of " << MaximumAmountOfCommandAllocators << std::endl;
			AmountOfCommandAllocators++;
			return CommandAllocator;
		}

	}

	AmountOfCommandAllocators++;
	return CommandAllocator;
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

ComPtr<ID3D12Fence1> CDirectXCore::CreateFence(ComPtr<ID3D12Device8> GraphicsCard, D3D12_FENCE_FLAGS FenceFlags)
{
	ComPtr<ID3D12Fence1> Fence;

	HRESULT res = GraphicsCard->CreateFence(0, FenceFlags, IID_PPV_ARGS(&Fence));
	if (FAILED(res))
	{
		throw new CGraphicsException;
	}

	return Fence;
}

ComPtr<ID3D12Fence1> CDirectXCore::CreateFence(ComPtr<ID3D12Device8> GraphicsCard, ComPtr<ID3D12Fence1> Fence, D3D12_FENCE_FLAGS FenceFlags)
{
	HRESULT res = GraphicsCard->CreateFence(0, FenceFlags, IID_PPV_ARGS(&Fence));
	if (FAILED(res))
	{
		throw new CGraphicsException;
	}


	return Fence;
}

HANDLE CDirectXCore::CreateOSEvent()
{
	HANDLE fenceEvent;

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
	auto CommandAllocator = g_CommandAllocators[CurrentBackBufferIndex];
	auto backBuffer = BackBuffers[CurrentBackBufferIndex];

	CommandAllocator->Reset();
	globalCommandList->Reset(CommandAllocator.Get(), nullptr);


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
			g_FrameFenceValues[g_CurrentBackBufferIndex] = Signal(g_CommandQueue, g_Fence, g_FenceValue);

		}
		if (FAILED(res))
		{

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
