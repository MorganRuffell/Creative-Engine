

#include "pch.h"
#include "CDOFSettings.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "CCommandContext.h"
#include "BufferManager.h"
#include "CTAAEffects.h"

#include "CompiledShaders/DoFPass1CS.h"
#include "CompiledShaders/DoFTilePassCS.h"
#include "CompiledShaders/DoFTilePassFixupCS.h"
#include "CompiledShaders/DoFPreFilterCS.h"
#include "CompiledShaders/DoFPreFilterFastCS.h"
#include "CompiledShaders/DoFPreFilterFixupCS.h"
#include "CompiledShaders/DoFPass2CS.h"
#include "CompiledShaders/DoFPass2FastCS.h"
#include "CompiledShaders/DoFPass2FixupCS.h"
#include "CompiledShaders/DoFPass2DebugCS.h"
#include "CompiledShaders/DoFMedianFilterCS.h"
#include "CompiledShaders/DoFMedianFilterSepAlphaCS.h"
#include "CompiledShaders/DoFMedianFilterFixupCS.h"
#include "CompiledShaders/DoFCombineCS.h"
#include "CompiledShaders/DoFCombineFastCS.h"
#include "CompiledShaders/DoFDebugRedCS.h"
#include "CompiledShaders/DoFDebugGreenCS.h"
#include "CompiledShaders/DoFDebugBlueCS.h"

using namespace CGraphics;

namespace CDOFSettings
{
	CETBoolean Enable("Graphics/Depth of Field/Enable", false);
	CETBoolean EnablePreFilter("Graphics/Depth of Field/PreFilter", true);
	CETBoolean MedianFilter("Graphics/Depth of Field/Median Filter", true);
	CETBoolean MedianAlpha("Graphics/Depth of Field/Median Alpha", false);
	CETFloat FocalDepth("Graphics/Depth of Field/Focal Center", 0.1f, 0.0f, 1.0f, 0.01f);
	CETFloat FocalRange("Graphics/Depth of Field/Focal Radius", 0.1f, 0.0f, 1.0f, 0.01f);
	CETFloat ForegroundRange("Graphics/Depth of Field/FG Range", 100.0f, 10.0f, 1000.0f, 10.0f);
	CETFloat AntiSparkleWeight("Graphics/Depth of Field/AntiSparkle", 1.0f, 0.0f, 10.0f, 1.0f);
	const char* DebugLabels[] = { "Off", "Foreground", "Background", "FG Alpha", "CoC" };
	CETStaticEnum DebugMode("Graphics/Depth of Field/Debug Mode", 0, _countof(DebugLabels), DebugLabels);
	CETBoolean DebugTiles("Graphics/Depth of Field/Debug Tiles", false);
	CETBoolean ForceSlow("Graphics/Depth of Field/Force Slow Path", false);
	CETBoolean ForceFast("Graphics/Depth of Field/Force Fast Path", false);

	CRootSignature s_RootSignature;

	CComputePSO s_DoFPass1CS(L"DOF: Pass 1 CS");				// Responsible for classifying tiles (1st pass)
	CComputePSO s_DoFTilePassCS(L"DOF: Tile Pass CS");				// Disperses tile info to its neighbors (3x3)
	CComputePSO s_DoFTilePassFixupCS(L"DOF: Tile Pass Fixup CS");		// Searches for straggler tiles to "fixup"

	CComputePSO s_DoFPreFilterCS(L"DOF: Pre-Filter CS");			// Full pre-filter with variable focus
	CComputePSO s_DoFPreFilterFastCS(L"DOF: Pre-Filter Fast CS");		// Pre-filter assuming near-constant focus
	CComputePSO s_DoFPreFilterFixupCS(L"DOF: Pre-Filter Fixup CS");		// Pass through colors for completely in focus tile

	CComputePSO s_DoFPass2CS(L"DOF: Pass 2 CS");				// Perform full CoC convolution pass
	CComputePSO s_DoFPass2FastCS(L"DOF: Pass 2 Fast CS");			// Perform color-only convolution for near-constant focus
	CComputePSO s_DoFPass2FixupCS(L"DOF: Pass 2 Fixup CS");			// Pass through colors again
	CComputePSO s_DoFPass2DebugCS(L"DOF: Pass 2 Debug CS");			// Full pass 2 shader with options for debugging

	CComputePSO s_DoFMedianFilterCS(L"DOF: Median Filter CS");			// 3x3 median filter to reduce fireflies
	CComputePSO s_DoFMedianFilterSepAlphaCS(L"DOF: Median Filter Separate Alpha CS");	// 3x3 median filter to reduce fireflies (separate filter on alpha)
	CComputePSO s_DoFMedianFilterFixupCS(L"DOF: Median Filter Fixup CS");	// Pass through without performing median

	CComputePSO s_DoFCombineCS(L"DOF: Combine CS");				// Combine DoF blurred buffer with focused color buffer
	CComputePSO s_DoFCombineFastCS(L"DOF: Combine Fast CS");			// Upsample DoF blurred buffer
	CComputePSO s_DoFDebugRedCS(L"DOF: Debug Red CS");				// Output red to entire tile for debugging
	CComputePSO s_DoFDebugGreenCS(L"DOF: Debug Green CS");			// Output green to entire tile for debugging
	CComputePSO s_DoFDebugBlueCS(L"DOF: Debug Blue CS");			// Output blue to entire tile for debugging

	CIndirectParamBuffer s_IndirectParameters;
}

void CDOFSettings::Initialize(void)
{
	s_RootSignature.Reset(4, 3);
	s_RootSignature.InitStaticSampler(0, SamplerPointBorderDesc);
	s_RootSignature.InitStaticSampler(1, SamplerPointClampDesc);
	s_RootSignature.InitStaticSampler(2, SamplerLinearClampDesc);
	s_RootSignature[0].InitAsConstantBuffer(0);
	s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6);
	s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 3);
	s_RootSignature[3].InitAsConstants(1, 1);
	s_RootSignature.Finalize(L"Depth of Field");

#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(s_RootSignature); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

	CreatePSO(s_DoFPass1CS, g_pDoFPass1CS);
	CreatePSO(s_DoFTilePassCS, g_pDoFTilePassCS);
	CreatePSO(s_DoFTilePassFixupCS, g_pDoFTilePassFixupCS);
	CreatePSO(s_DoFPreFilterCS, g_pDoFPreFilterCS);
	CreatePSO(s_DoFPreFilterFastCS, g_pDoFPreFilterFastCS);
	CreatePSO(s_DoFPreFilterFixupCS, g_pDoFPreFilterFixupCS);
	CreatePSO(s_DoFPass2CS, g_pDoFPass2CS);
	CreatePSO(s_DoFPass2FastCS, g_pDoFPass2FastCS);
	CreatePSO(s_DoFPass2FixupCS, g_pDoFPass2FixupCS);
	CreatePSO(s_DoFPass2DebugCS, g_pDoFPass2DebugCS);
	CreatePSO(s_DoFMedianFilterCS, g_pDoFMedianFilterCS);
	CreatePSO(s_DoFMedianFilterSepAlphaCS, g_pDoFMedianFilterSepAlphaCS);
	CreatePSO(s_DoFMedianFilterFixupCS, g_pDoFMedianFilterFixupCS);

	CreatePSO(s_DoFCombineCS, g_pDoFCombineCS);
	CreatePSO(s_DoFCombineFastCS, g_pDoFCombineFastCS);
	CreatePSO(s_DoFDebugRedCS, g_pDoFDebugRedCS);
	CreatePSO(s_DoFDebugGreenCS, g_pDoFDebugGreenCS);
	CreatePSO(s_DoFDebugBlueCS, g_pDoFDebugBlueCS);

#undef CreatePSO

	__declspec(align(16)) const uint32_t initArgs[9] = { 0, 1, 1, 0, 1, 1, 0, 1, 1 };
	s_IndirectParameters.Create(L"DoF Indirect Parameters", 3, sizeof(D3D12_DISPATCH_ARGUMENTS), initArgs);
}

void CDOFSettings::Shutdown(void)
{
	s_IndirectParameters.Destroy();
}

void CDOFSettings::Render(CCommandContext& BaseContext, float /*NearClipDist*/, float FarClipDist)
{

	if (!g_bTypedUAVLoadSupport_R11G11B10_FLOAT)
	{
		WARN_ONCE_IF(!g_bTypedUAVLoadSupport_R11G11B10_FLOAT, "Unable to perform final pass of DoF without support for R11G11B10F UAV loads");
		Enable = false;
	}

	CComputeContext& Context = BaseContext.GetCComputeContext();
	Context.SetRootSignature(s_RootSignature);

	CRGBBuffer& LinearDepth = g_LinearDepth[CTAAEffects::GetFrameIndexMod2()];

	uint32_t BufferWidth = (uint32_t)LinearDepth.GetWidth();
	uint32_t BufferHeight = (uint32_t)LinearDepth.GetHeight();
	uint32_t TiledWidth = (uint32_t)g_DoFTileClass[0].GetWidth();
	uint32_t TiledHeight = (uint32_t)g_DoFTileClass[0].GetHeight();

	__declspec(align(16)) struct DoFConstantBuffer
	{
		float FocalCenter, FocalSpread;
		float FocalMinZ, FocalMaxZ;
		float RcpBufferWidth, RcpBufferHeight;
		uint32_t BufferWidth, BufferHeight;
		int32_t HalfWidth, HalfHeight;
		uint32_t TiledWidth, TiledHeight;
		float RcpTiledWidth, RcpTiledHeight;
		uint32_t DebugState, DisablePreFilter;
		float FGRange, RcpFGRange, AntiSparkleFilterStrength;
	};
	DoFConstantBuffer cbuffer =
	{
		(float)FocalDepth, 1.0f / (float)FocalRange,
		(float)FocalDepth - (float)FocalRange, (float)FocalDepth + (float)FocalRange,
		1.0f / BufferWidth, 1.0f / BufferHeight,
		BufferWidth, BufferHeight,
		(int32_t)CMath::DivideByMultiple(BufferWidth, 2), (int32_t)CMath::DivideByMultiple(BufferHeight, 2),
		TiledWidth, TiledHeight,
		1.0f / TiledWidth, 1.0f / TiledHeight,
		(uint32_t)DebugMode, EnablePreFilter ? 0u : 1u,
		ForegroundRange / FarClipDist, FarClipDist / ForegroundRange, (float)AntiSparkleWeight
	};
	Context.SetDynamicConstantBufferView(0, sizeof(cbuffer), &cbuffer);

	{

		// Initial pass to discover max CoC and closest depth in 16x16 tiles
		Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DoFTileClass[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.SetPipelineState(s_DoFPass1CS);
		Context.SetDynamicDescriptor(1, 0, LinearDepth.GetSRV());
		Context.SetDynamicDescriptor(2, 0, g_DoFTileClass[0].GetUAV());
		Context.Dispatch2D(BufferWidth, BufferHeight, 16, 16);

		Context.ResetCounter(g_DoFWorkQueue);
		Context.ResetCounter(g_DoFFastQueue);
		Context.ResetCounter(g_DoFFixupQueue);

		// 3x3 filter to spread max CoC and closest depth to neighboring tiles
		Context.TransitionResource(g_DoFTileClass[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DoFTileClass[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_DoFWorkQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_DoFFastQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.SetPipelineState(s_DoFTilePassCS);
		Context.SetDynamicDescriptor(1, 0, g_DoFTileClass[0].GetSRV());
		Context.SetDynamicDescriptor(2, 0, g_DoFTileClass[1].GetUAV());
		Context.SetDynamicDescriptor(2, 1, g_DoFWorkQueue.GetUAV());
		Context.SetDynamicDescriptor(2, 2, g_DoFFastQueue.GetUAV());
		Context.Dispatch2D(TiledWidth, TiledHeight);

		Context.TransitionResource(g_DoFTileClass[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DoFFixupQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.SetPipelineState(s_DoFTilePassFixupCS);
		Context.SetDynamicDescriptor(1, 0, g_DoFTileClass[1].GetSRV());
		Context.SetDynamicDescriptor(2, 0, g_DoFFixupQueue.GetUAV());
		Context.Dispatch2D(TiledWidth, TiledHeight);

		Context.TransitionResource(g_DoFWorkQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.CopyCounter(s_IndirectParameters, 0, g_DoFWorkQueue);

		Context.TransitionResource(g_DoFFastQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.CopyCounter(s_IndirectParameters, 12, g_DoFFastQueue);

		Context.TransitionResource(g_DoFFixupQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.CopyCounter(s_IndirectParameters, 24, g_DoFFixupQueue);

		Context.TransitionResource(s_IndirectParameters, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	}

	{

		if (ForceFast && !DebugMode)
			Context.SetPipelineState(s_DoFPreFilterFastCS);
		else
			Context.SetPipelineState(s_DoFPreFilterCS);
		Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DoFPresortBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_DoFPrefilter, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.SetDynamicDescriptor(1, 0, LinearDepth.GetSRV());
		Context.SetDynamicDescriptor(1, 1, g_DoFTileClass[1].GetSRV());
		Context.SetDynamicDescriptor(1, 2, g_SceneColorBuffer.GetSRV());
		Context.SetDynamicDescriptor(1, 3, g_DoFWorkQueue.GetSRV());
		Context.SetDynamicDescriptor(2, 0, g_DoFPresortBuffer.GetUAV());
		Context.SetDynamicDescriptor(2, 1, g_DoFPrefilter.GetUAV());
		Context.DispatchIndirect(s_IndirectParameters, 0);

		if (!ForceSlow && !DebugMode)
			Context.SetPipelineState(s_DoFPreFilterFastCS);
		Context.SetDynamicDescriptor(1, 3, g_DoFFastQueue.GetSRV());
		Context.DispatchIndirect(s_IndirectParameters, 12);

		Context.SetPipelineState(s_DoFPreFilterFixupCS);
		Context.SetDynamicDescriptor(1, 3, g_DoFFixupQueue.GetSRV());
		Context.DispatchIndirect(s_IndirectParameters, 24);
	}

	{

		Context.TransitionResource(g_DoFPrefilter, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DoFBlurColor[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.TransitionResource(g_DoFBlurAlpha[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		if (ForceFast && !DebugMode)
			Context.SetPipelineState(s_DoFPass2FastCS);
		else
			Context.SetPipelineState(DebugMode > 0 ? s_DoFPass2DebugCS : s_DoFPass2CS);
		Context.SetDynamicDescriptor(1, 0, g_DoFPrefilter.GetSRV());
		Context.SetDynamicDescriptor(1, 1, g_DoFPresortBuffer.GetSRV());
		Context.SetDynamicDescriptor(1, 2, g_DoFTileClass[1].GetSRV());
		Context.SetDynamicDescriptor(1, 3, g_DoFWorkQueue.GetSRV());
		Context.SetDynamicDescriptor(2, 0, g_DoFBlurColor[0].GetUAV());
		Context.SetDynamicDescriptor(2, 1, g_DoFBlurAlpha[0].GetUAV());
		Context.DispatchIndirect(s_IndirectParameters, 0);

		if (!ForceSlow && !DebugMode)
			Context.SetPipelineState(s_DoFPass2FastCS);
		Context.SetDynamicDescriptor(1, 3, g_DoFFastQueue.GetSRV());
		Context.DispatchIndirect(s_IndirectParameters, 12);

		Context.SetPipelineState(s_DoFPass2FixupCS);
		Context.SetDynamicDescriptor(1, 3, g_DoFFixupQueue.GetSRV());
		Context.DispatchIndirect(s_IndirectParameters, 24);
	}

	{
		Context.TransitionResource(g_DoFBlurColor[0], D3D12_RESOURCE_STATE_GENERIC_READ);
		Context.TransitionResource(g_DoFBlurAlpha[0], D3D12_RESOURCE_STATE_GENERIC_READ);

		if (MedianFilter)
		{
			Context.TransitionResource(g_DoFBlurColor[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			Context.TransitionResource(g_DoFBlurAlpha[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			Context.SetPipelineState(MedianAlpha ? s_DoFMedianFilterSepAlphaCS : s_DoFMedianFilterCS);
			Context.SetDynamicDescriptor(1, 0, g_DoFBlurColor[0].GetSRV());
			Context.SetDynamicDescriptor(1, 1, g_DoFBlurAlpha[0].GetSRV());
			Context.SetDynamicDescriptor(1, 2, g_DoFWorkQueue.GetSRV());
			Context.SetDynamicDescriptor(2, 0, g_DoFBlurColor[1].GetUAV());
			Context.SetDynamicDescriptor(2, 1, g_DoFBlurAlpha[1].GetUAV());
			Context.DispatchIndirect(s_IndirectParameters, 0);

			Context.SetDynamicDescriptor(1, 2, g_DoFFastQueue.GetSRV());
			Context.DispatchIndirect(s_IndirectParameters, 12);

			Context.SetPipelineState(s_DoFMedianFilterFixupCS);
			Context.SetDynamicDescriptor(1, 2, g_DoFFixupQueue.GetSRV());
			Context.DispatchIndirect(s_IndirectParameters, 24);

			Context.TransitionResource(g_DoFBlurColor[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			Context.TransitionResource(g_DoFBlurAlpha[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}

	{
		Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (DebugTiles)
		{
			Context.SetPipelineState(s_DoFDebugRedCS);
			Context.SetDynamicDescriptor(1, 5, g_DoFWorkQueue.GetSRV());
			Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
			Context.DispatchIndirect(s_IndirectParameters, 0);

			Context.SetPipelineState(s_DoFDebugGreenCS);
			Context.SetDynamicDescriptor(1, 5, g_DoFFastQueue.GetSRV());
			Context.DispatchIndirect(s_IndirectParameters, 12);

			Context.SetPipelineState(s_DoFDebugBlueCS);
			Context.SetDynamicDescriptor(1, 5, g_DoFFixupQueue.GetSRV());
			Context.DispatchIndirect(s_IndirectParameters, 24);
		}
		else
		{
			Context.SetPipelineState(s_DoFCombineCS);
			Context.SetDynamicDescriptor(1, 0, g_DoFBlurColor[MedianFilter ? 1 : 0].GetSRV());
			Context.SetDynamicDescriptor(1, 1, g_DoFBlurAlpha[MedianFilter ? 1 : 0].GetSRV());
			Context.SetDynamicDescriptor(1, 2, g_DoFTileClass[1].GetSRV());
			Context.SetDynamicDescriptor(1, 3, LinearDepth.GetSRV());
			Context.SetDynamicDescriptor(1, 4, g_DoFWorkQueue.GetSRV());
			Context.SetDynamicDescriptor(2, 0, g_SceneColorBuffer.GetUAV());
			Context.DispatchIndirect(s_IndirectParameters, 0);

			Context.SetPipelineState(s_DoFCombineFastCS);
			Context.SetDynamicDescriptor(1, 4, g_DoFFastQueue.GetSRV());
			Context.DispatchIndirect(s_IndirectParameters, 12);
		}

		Context.InsertUAVBarrier(g_SceneColorBuffer);
	}
}
