#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "CDirectX12Core.h"
#include  <forward_list>
#include "DirectXHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;


CDirectX::CDirectX12Core::CDirectX12Core(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, HWND Window)
    :   CurrentBuffer(0),
	    m_screenViewport(),
	    m_rtvDescriptorSize(0),
	    m_fenceEvent(0),
	    m_backBufferFormat(backBufferFormat),
	    m_depthBufferFormat(depthBufferFormat),
	    m_fenceValues{},
	    m_dpi(-1.0f),
	    m_effectiveDpi(-1.0f),
	    m_deviceRemoved(false)
{
	CreateDeviceResources(Window);

	if (GraphicsTest == nullptr)
	{
		GraphicsTest = new CDirectX::CDirectXTest();
	}

	GetVRAMInfo();
}


void CDirectX::CDirectX12Core::CreateDeviceResources(HWND window)
{
#if defined(_debug)
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

		if (GetHardwareAdapter(&LatestAdapter) == false)
		{
			GetSoftwareAdapter(&LatestAdapter);
		}
		
		CreateGraphicsCard();
		CreateCommandQueues();


		// Create descriptor heaps for render target views and depth stencil views.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = AmountOfRenderTargets;
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

		for (UINT n = 0; n < AmountOfRenderTargets; n++)
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
					HRESULT resGraphicsCard = D3D12CreateDevice(LatestAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&GraphicsCard));
					if (FAILED(resGraphicsCard))
					{
						HRESULT resGraphicsCard = D3D12CreateDevice(LatestAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&GraphicsCard));
						if (FAILED(resGraphicsCard))
						{
							HRESULT resGraphicsCard = D3D12CreateDevice(LatestAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&GraphicsCard));
							if (FAILED(resGraphicsCard))
							{
								throw new CException;
							}
						}

						WarpStatus = CGraphics::CGraphicsWARPStatus::Initalized;

					}

				}
				else
				{
					WarpStatus = CGraphics::CGraphicsWARPStatus::NotInitalized;
					return true;
				}
			}
		}
		else
		{
			WarpStatus = CGraphics::CGraphicsWARPStatus::NotInitalized;
			return true;
		}
	}
	else
	{
		WarpStatus = CGraphics::CGraphicsWARPStatus::NotInitalized;
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
	D3D12_COMMAND_QUEUE_DESC PrimaryQueueDescriptor = {};
	PrimaryQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	PrimaryQueueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	PrimaryQueueDescriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

	CDirectX::ThrowIfFailed(GraphicsCard->CreateCommandQueue(&PrimaryQueueDescriptor, IID_PPV_ARGS(&PrimaryCommandQueue)));
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

bool CDirectX::CDirectX12Core::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC HeapDesc, ComPtr<ID3D12DescriptorHeap> Heap)
{
    return false;
}

// These resources need to be recreated every time the window size is changed.
void CDirectX::CDirectX12Core::CreateWindowSizeDependentResources(HWND window)
{
	WaitForGpu();

	// Clear the previous window size specific content and update the fence values.
	for (UINT n = 0; n < AmountOfRenderTargets; n++)
	{
		m_renderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[CurrentBuffer];
	}

	UpdateRenderTargetSize();

    m_backBufferWidth = 0;
    m_backBufferHeight = 0;

    RECT rectangle;
    if (GetWindowRect(window, &rectangle))
    {
        m_backBufferWidth = rectangle.right - rectangle.left;
        m_backBufferHeight = rectangle.bottom - rectangle.top;
    }

	//If swapchains are empty make a new one!
	if (SwapChain != nullptr)
	{
		HRESULT hr = SwapChain->ResizeBuffers(AmountOfRenderTargets, m_backBufferWidth, m_backBufferHeight, m_backBufferFormat, 0);

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
        DXGI_OUTPUT_DESC OutputDesc;
        OutputDesc.AttachedToDesktop = TRUE;

        HMONITOR Monitor = GetPrimaryMonitorHandle(window);
        OutputDesc.Monitor = Monitor;

        if (LatestAdapter != nullptr)
        {
            HRESULT res = LatestAdapter->EnumOutputs(0, &OutputMonitor);
            if (FAILED(res))
            {
                throw new CException;
            }
        }

		DXGI_SCALING scaling = CDisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = m_backBufferWidth;
		swapChainDesc.Height = m_backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = AmountOfRenderTargets;					// Use triple-buffering we defined in the header file
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;


		HRESULT res = LatestFactory->CreateSwapChainForHwnd(PrimaryCommandQueue.Get(), window, &swapChainDesc, nullptr, OutputMonitor.Get(), &swapChain);
		if (SUCCEEDED(res))
		{
			swapChain.As(&SwapChain);
		}
		else
		{
			CDirectX::ThrowIfFailed(
				LatestFactory->CreateSwapChainForHwnd(PrimaryCommandQueue.Get(), window, &swapChainDesc, nullptr, OutputMonitor.Get(), &swapChain));
			CDirectX::ThrowIfFailed(swapChain.As(&SwapChain));
		}

	}

	CurrentBuffer = SwapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < AmountOfRenderTargets; n++)
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

	D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, m_backBufferWidth, m_backBufferHeight, 1, 1);
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


	m_screenViewport = { 0.0f, 0.0f, (float) m_backBufferWidth, (float) m_backBufferHeight, 0.0f, 1.0f };
}


void CDirectX::CDirectX12Core::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;
}


void CDirectX::CDirectX12Core::SetDpi(float dpi, HWND window)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;
		CreateWindowSizeDependentResources(window);
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

bool CDirectX::CDirectX12Core::GetHardwareAdapter(IDXGIAdapter4** ppAdapter)
{
	*ppAdapter = nullptr;

	HRESULT res = LatestFactory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(LatestAdapter.GetAddressOf()));

	if (SUCCEEDED(res))
	{
		DXGI_ADAPTER_DESC3 desc;
		desc.Flags = DXGI_ADAPTER_FLAG3_SUPPORT_MONITORED_FENCES;
		GraphicsCardName = *desc.Description;

		LatestAdapter->GetDesc3(&desc);

		*ppAdapter = LatestAdapter.Get();

		WarpStatus = CGraphics::NotInitalized;

		return true;
	}
	else
	{
		return false;
	}
		
}

bool CDirectX::CDirectX12Core::GetSoftwareAdapter(IDXGIAdapter4** ppAdapter)
{
	*ppAdapter = nullptr;

	HRESULT res = LatestFactory->EnumWarpAdapter(IID_PPV_ARGS(&LatestAdapter));

	if (SUCCEEDED(res))
	{
		DXGI_ADAPTER_DESC3 desc;
		desc.Flags = DXGI_ADAPTER_FLAG3_SUPPORT_MONITORED_FENCES;

		LatestAdapter->GetDesc3(&desc);

		*ppAdapter = LatestAdapter.Detach();
	
		WarpStatus = CGraphics::Initalized;

		return true;
	}
	else
	{
		return false;
	}
}

HMONITOR CDirectX::CDirectX12Core::GetPrimaryMonitorHandle(HWND window)
{
    auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);

    if (monitor != nullptr)
    {
        return monitor;
    }
}


