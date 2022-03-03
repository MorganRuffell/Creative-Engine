#pragma once

#include "dxgi1_6.h"
#include "CreativeMacros.h"
#include "CMathCore.h"
#include "CObject.h"
#include <d3dcompiler.h>
#include "CException.h"
#include <mutex>
#include <algorithm>
#include <cstdint>
#include <set>
#include <map>
#include <wrl.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <string.h>

using Microsoft::WRL::ComPtr;

namespace CGraphics
{
	struct CRGraphicsAPI SCoreState
	{
		std::string GetCoreState() const
		{
			std::string result;
			return result = CoreState;
		}

		void SetCoreState(int a)
		{
			if (!(a > 5))
			{
				CoreState = CoreStates.at(a);
			}
			else
			{
				CoreState = 5;
			}
		}

	private:

		const std::map<int, std::string> CoreStates = {
			{0,"Begin"},
			{1,"Init"},
			{2,"CreatingIndependents"},
			{3,"CreatingDependents"},
			{4,"Cleanup"},
			{5,"Terminating"}
		};


		std::string CoreState = "";
	};


	enum CGraphicsWARPStatus
	{
		NotInitalized,
		Initalized,
	};


	class CRGraphicsAPI CGraphicsCore : public CObject
	{
	public:

		CGraphicsWARPStatus WarpStatus;
        

	public:
		virtual void SetAmountOfRenderTargets();
		virtual void SetDpi(float dpi, HWND window);
		virtual void ValidateDevice();
		virtual void Present();
		virtual void WaitForGpu();

		WCHAR GraphicsCardName;


	public:

		ComPtr<ID3D12Device10> GraphicsCard;
		ComPtr<ID3D12Device6> BackupGraphicsCard;
		ComPtr<ID3D12Device> BasicGraphicsCard;

		ComPtr<IDXGIFactory7> LatestFactory;
		ComPtr<IDXGIFactory6> Factory;
		ComPtr<IDXGIFactory4> FallbackFactory;


        //We Should combine these, save us a problem
		ComPtr<IDXGIAdapter4> LatestAdapter;
		ComPtr<IDXGIAdapter2> ModernAdapater;
		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIAdapter4> __WarpAdapter;
		ComPtr<IDXGIAdapter2> ModernWarpAdapter;
		ComPtr<IDXGIAdapter1> WarpAdapter;

		ComPtr<IDXGISwapChain3>	SwapChain;
		ComPtr<IDXGISwapChain2>	FallbackSwapChain;
		ComPtr<IDXGISwapChain1>	OldSwapChain;

        //Controls for the monitor -- Gonna stick this to primary!
        ComPtr<IDXGIOutput> OutputMonitor;


		double MaximumAmountOfVRAM;
		double CurrentUsageOfVRAM;

	protected:

		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	};

}

