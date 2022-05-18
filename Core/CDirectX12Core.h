
#pragma once
#include "pch.h"
#include "PipelineState.h"
#include "CDirectXTest.h"
#include "CDescriptorHeap.h"
#include "RootSignature.h"
#include "GraphicsCommon.h"

class CCommandListManager;
class CDXContextManager;

namespace CGraphics
{

    class CDirectX12Core
    {
	public:
		CDirectX12Core()
		{
			//CDirectX::CDirectXTest* Test = new CDirectX::CDirectXTest();
		}

    public:

		~CDirectX12Core()
		{
			m_DXGIFactory->Release();
			m_Adapter->Release();
			m_DXDevice->Release();
		}

		IDXGIAdapter4* GetAdapter() const
		{
			return m_Adapter.Get();
		}

		ID3D12Device8* GetD3DDevice() const
		{
			return m_DXDevice.Get();
		}


		IDXGIFactory7* GetDXGIFactory() const
		{
			return m_DXGIFactory.Get();
		}

		IDXGISwapChain4* GetSwapChain() const
		{
			return m_SwapChain.Get();
		}


	private:
		/// <summary>
		/// Direct3D Objects -- Initalize once, use everywhere.
		/// </summary>

		ComPtr<IDXGIAdapter1>                               m_InitAdapter;
		ComPtr<IDXGIAdapter4>                               m_Adapter;

		DXGI_ADAPTER_DESC3                                  m_CurrentAdapterData;
		DXGI_ADAPTER_DESC1                                  m_LegacyCurrentAdapterData;

	public:
		ComPtr<ID3D12Device8>                               m_DXDevice;

	public:
		/// <summary>
		/// Swap chain and DXGI objects -- The ones underneath are for init -- must be released once called.
		/// </summary>
		ComPtr<IDXGIFactory7>                               m_DXGIFactory;
		ComPtr<IDXGISwapChain4>                             m_SwapChain;
		ComPtr<ID3D12Resource>                              m_DepthStencil;


		ComPtr<IDXGIFactory1>                               m_InitalizerFactory;
		ComPtr<IDXGISwapChain1>                             m_InitalizerSwapChain;

	public:

		ComPtr<ID3D12DescriptorHeap>						m_ImGUIHeap;
		D3D12_DESCRIPTOR_HEAP_DESC							GUIRenderViewDesc;

		CCommandContext*										m_ImGUICommandContext;

    };
    

	static CDirectX12Core* Core;


#ifndef RELEASE
    extern const GUID WKPDID_D3DDebugObjectName;
#endif

    using namespace Microsoft::WRL;

    void Initialize(bool RequireDXRSupport, HWND window);
	//void Render(D3D12_RECT ScissorRect, D3D12_VIEWPORT ViewportToUpdate, GraphicsContext& GraphicsData, CDirectX12Core& core);
    void Shutdown(void);
	//void DrawGUI();

    bool IsDeviceNvidia(ID3D12Device8* pDevice);
    bool IsDeviceAMD(ID3D12Device8* pDevice);
    bool IsDeviceIntel(ID3D12Device8* pDevice);


    extern ID3D12Device8* g_Device;
    extern CCommandListManager g_CommandManager;
    extern CDXContextManager g_ContextManager;


    extern CDirectX::CDirectXTest g_DirectXSupportedFeatures;

    extern D3D_FEATURE_LEVEL g_D3DFeatureLevel;
    extern bool g_bTypedUAVLoadSupport_R11G11B10_FLOAT;
    extern bool g_bTypedUAVLoadSupport_R16G16B16A16_FLOAT;

    extern CDescriptorAllocator g_DescriptorAllocator[];
    inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor( D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1 )
    {
        return g_DescriptorAllocator[Type].Allocate(Count);
    }
}




