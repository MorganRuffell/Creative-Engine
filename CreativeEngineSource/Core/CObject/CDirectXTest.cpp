#pragma once

#include <concrt.h>
#include <string>
#include "CDirectXTest.h"


using namespace Concurrency;

void CDirectX::CDirectXTest::CheckMSAAFeatures()
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
				TestResult->SupportedMSAALevels.insert(std::pair <std::string, bool>("SixteenMSAA", true));
			}
			else
			{
				TestResult->SupportedMSAALevels.insert(std::pair <std::string, bool>("EightMSAA", true));
			}
		}

		else
		{
			TestResult->SupportedMSAALevels.insert(std::pair <std::string, bool>("FourMSAA", true));
		}
	}
	else
	{
		throw new CGraphicsException;
	}
}

void CDirectX::CDirectXTest::SystemCheck()
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
}

void CDirectX::CDirectXTest::CheckGPUFeatures(CDirectXSupportedFeatures* SupportedFeatures)
{
	std::forward_list<HRESULT> ListOfResults;
	std::forward_list<HRESULT>::iterator Iterator;
	Iterator = ListOfResults.before_begin();

	TestResult->AmountOfAccelerators = GraphicsCard->GetNodeCount();


	HRESULT BasicArchitecture = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &SupportedFeatures->SupportData0, sizeof(SupportedFeatures->SupportData0));
	if (SUCCEEDED(BasicArchitecture))
	{
		std::string SupportsBasicArchitecture = "Supports Basic Architecture";

		AcceleratorIndex = SupportedFeatures->SupportData0.NodeIndex;
		TestResult->DoesSupportUnifiedMemoryAllocation = SupportedFeatures->SupportData0.UMA;
		TestResult->DoesDriverAndHardwareSupportUnifiedMemoryAllocation = SupportedFeatures->SupportData0.CacheCoherentUMA;

		ListOfResults.push_front(BasicArchitecture);
		AddFriendlyNameToList(SupportsBasicArchitecture);
	}



	std::thread ThreadA = std::thread([&]() {

		HRESULT FormatSupport = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &SupportedFeatures->SupportData1, sizeof(SupportedFeatures->SupportData1));
		if (SUCCEEDED(FormatSupport))
		{
			std::string SupportsBasicFormats = "Supports Basic Formats";

			AddFriendlyNameToList(SupportsBasicFormats);
			AddResultToList(FormatSupport, ListOfResults, Iterator);
		}

		HRESULT BasicFeature = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &SupportedFeatures->SupportData2, sizeof(SupportedFeatures->SupportData2));
		if (SUCCEEDED(BasicFeature))
		{
			std::string SupportsBasicFeatureLevels = "DirectX Level: ";

			GetMaximumFeatureLevel(SupportedFeatures, SupportsBasicFeatureLevels);

			AddFriendlyNameToList(SupportsBasicFeatureLevels);
			AddResultToList(BasicFeature, ListOfResults, Iterator);
		}

		HRESULT MultisampleQuality = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &SupportedFeatures->SupportData3, sizeof(SupportedFeatures->SupportData3));
		if (SUCCEEDED(MultisampleQuality))
		{
			std::string SupportsBasicFormats = "Supports Multisample quality levels";
			AddFriendlyNameToList(SupportsBasicFormats);
			AddResultToList(MultisampleQuality, ListOfResults, Iterator);
		}

		HRESULT VirtualAddress = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &SupportedFeatures->SupportData5, sizeof(SupportedFeatures->SupportData5));
		if (SUCCEEDED(VirtualAddress))
		{
			std::string VirtualAddressSupport = "Supports Virtual Address writing";
			AddFriendlyNameToList(VirtualAddressSupport);
			AddResultToList(VirtualAddress, ListOfResults, Iterator);
		}

		HRESULT ShaderModel = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SupportedFeatures->SupportData6, sizeof(SupportedFeatures->SupportData6));
		if (SUCCEEDED(ShaderModel))
		{
			std::string ShaderModelSupport = "Supports up to Shader Model ";
			std::string FriendlyName;

			switch (SupportedFeatures->SupportData6.HighestShaderModel)
			{
			case D3D_SHADER_MODEL_5_1:
				FriendlyName = "Shader Model 5.1";
				break;

			case D3D_SHADER_MODEL_6_0:
				FriendlyName = "Shader Model 6.0";
				break;

			case D3D_SHADER_MODEL_6_1:
				FriendlyName = "Shader Model 6.1";
				break;

			case D3D_SHADER_MODEL_6_2:
				FriendlyName = "Shader Model 6.2";
				break;

			case D3D_SHADER_MODEL_6_3:
				FriendlyName = "Shader Model 6.3";
				break;

			case D3D_SHADER_MODEL_6_4:
				FriendlyName = "Shader Model 6.4";
				break;

			case D3D_SHADER_MODEL_6_5:
				FriendlyName = "Shader Model 6.5";
				break;

			case D3D_SHADER_MODEL_6_6:
				FriendlyName = "Shader Model 6.6";
				break;

			default:
				break;
			}

			ShaderModelSupport += FriendlyName;
			TestResult->MaximumShaderModel = FriendlyName;

			AddFriendlyNameToList(ShaderModelSupport);
			AddResultToList(ShaderModel, ListOfResults, Iterator);
		}

		HRESULT Options1 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &SupportedFeatures->SupportData7, sizeof(SupportedFeatures->SupportData7));
		if (SUCCEEDED(Options1))
		{
			if (SupportedFeatures->SupportData7.WaveOps == TRUE)
			{
				std::string WaveOperationSupport = "Supports HLSL 6.0 Wave operations";

				TestResult->DoesSupportWaveShaderOperations = SupportedFeatures->SupportData7.WaveOps;
				TestResult->TransitionsPossibleThroughConstantBufferViews = SupportedFeatures->SupportData7.ExpandedComputeResourceStates;
				TestResult->WaveLaneCountMinimum = SupportedFeatures->SupportData7.WaveLaneCountMin;
				TestResult->WaveLaneCountMaximum = SupportedFeatures->SupportData7.WaveLaneCountMax;
				TestResult->TotalLaneCount = SupportedFeatures->SupportData7.TotalLaneCount;

				AddFriendlyNameToList(WaveOperationSupport);
				AddResultToList(Options1, ListOfResults, Iterator);
			}
			else
			{

			}
		}


		HRESULT ProtectedResourceSession = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &SupportedFeatures->SupportData8, sizeof(SupportedFeatures->SupportData8));
		if (SUCCEEDED(ProtectedResourceSession))
		{
			std::string ProtectedResourceSessionName = "Supports Protected Resource Sessions";
			AddFriendlyNameToList(ProtectedResourceSessionName);
			AddResultToList(ProtectedResourceSession, ListOfResults, Iterator);
		}

	});

	std::thread ThreadB = std::thread([&]() {

		HRESULT ArchOne = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &SupportedFeatures->SupportData10, sizeof(SupportedFeatures->SupportData10));
		if (SUCCEEDED(ArchOne))
		{
			std::string ArchTwo = "Architecture Stage Two Passed!";

			TestResult->DoesUseTileBasedRenderer = SupportedFeatures->SupportData10.TileBasedRenderer;
			TestResult->IsMemoryManagementUnitIsolated = SupportedFeatures->SupportData10.IsolatedMMU;

			AddFriendlyNameToList(ArchTwo);
			AddResultToList(ArchOne, ListOfResults, Iterator);

		}

		HRESULT Options2 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &SupportedFeatures->SupportData11, sizeof(SupportedFeatures->SupportData11));
		if (SUCCEEDED(Options2))
		{
			if (SupportedFeatures->SupportData11.DepthBoundsTestSupported == TRUE)
			{
				TestResult->DepthBoundsTestSupported = SupportedFeatures->SupportData11.DepthBoundsTestSupported;
				std::string Options2Stage = "Supports Depth bounding and ";
				std::string ProgrammableShaders;

				auto ShaderSamplePositions = SupportedFeatures->SupportData11.ProgrammableSamplePositionsTier;

				switch (ShaderSamplePositions)
				{
				case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_NOT_SUPPORTED:
					TestResult->ShaderSamplePositionsSupportLevel = 0;
					ProgrammableShaders = "Does not support sample positions shaders";
					break;
				case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_1:
					TestResult->ShaderSamplePositionsSupportLevel = 1;
					ProgrammableShaders = "Support for sample position shaders is TIER ONE";

					break;
				case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_2:
					TestResult->ShaderSamplePositionsSupportLevel = 2;
					ProgrammableShaders = "Support for sample position shaders is TIER TWO";
					break;
				default:
					break;
				}

				Options2Stage += ProgrammableShaders;

				AddFriendlyNameToList(Options2Stage);
				AddResultToList(Options2, ListOfResults, Iterator);
			}
		}

		HRESULT ShaderCache = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &SupportedFeatures->SupportData12, sizeof(SupportedFeatures->SupportData12));
		if (SUCCEEDED(ShaderCache))
		{
			std::string FriendlyName = "Shader Cache features supported: ";

			auto ShaderCacheData = SupportedFeatures->SupportData12.SupportFlags;

			std::string info;

			switch (ShaderCacheData)
			{
			case D3D12_SHADER_CACHE_SUPPORT_NONE:
				info += "Shader Cache is not supported, shaders will run slower because of memory location";
				break;
			case D3D12_SHADER_CACHE_SUPPORT_SINGLE_PSO:
				info += "Shader Cache supports Pipeline state data and Compute pipeline data!";
				break;
			case D3D12_SHADER_CACHE_SUPPORT_LIBRARY:
				info += "Supports Pipeline Library interface!";
				break;
			case D3D12_SHADER_CACHE_SUPPORT_AUTOMATIC_INPROC_CACHE:
				info += "Stored shaders are compiled and stored in MEMORY";
				break;
			case D3D12_SHADER_CACHE_SUPPORT_AUTOMATIC_DISK_CACHE:
				info += "Stored shaders are compiled and stored in DISK";
				break;
			default:
				break;
			}

			FriendlyName += info;

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(ShaderCache, ListOfResults, Iterator);
		}


		HRESULT VirtualAddressSupport = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_COMMAND_QUEUE_PRIORITY, &SupportedFeatures->SupportData13, sizeof(SupportedFeatures->SupportData13));
		if (SUCCEEDED(VirtualAddressSupport))
		{
			std::string FriendlyName = "";

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(VirtualAddressSupport, ListOfResults, Iterator);
		}

		HRESULT Options3 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &SupportedFeatures->SupportData14, sizeof(SupportedFeatures->SupportData14));
		if (SUCCEEDED(Options3))
		{
			std::string FriendlyName = "Supports Baycentrics, an umbrella term.";

			GraphicsBufferWriteData(SupportedFeatures);

			FriendlyName = InstancingTiers(SupportedFeatures, FriendlyName);
			TestResult->CastingFromOneFormatToAnother = SupportedFeatures->SupportData14.CastingFullyTypedFormatSupported;

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(Options3, ListOfResults, Iterator);
		}

		HRESULT ExistingHeaps = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &SupportedFeatures->SupportData15, sizeof(SupportedFeatures->SupportData15));
		if (SUCCEEDED(ExistingHeaps))
		{
			std::string FriendlyName = "Supports Existing Heaps, a memory heap useful in diagnostics.";
			TestResult->SupportsPersistantDiagnosticMemoryHeaps = true;
			AddFriendlyNameToList(FriendlyName);
			AddResultToList(ExistingHeaps, ListOfResults, Iterator);
		}

		HRESULT Options4 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &SupportedFeatures->SupportData16, sizeof(SupportedFeatures->SupportData16));
		if (SUCCEEDED(Options4))
		{
			std::string FriendlyName = "";

			auto SharedResCompatibilityTier = SupportedFeatures->SupportData16.SharedResourceCompatibilityTier;
			switch (SharedResCompatibilityTier)
			{
			case D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_0:
				FriendlyName = "Shared Resources are not supported";
				TestResult->_SharedResourceCompatibilityTier = 0;
				break;
			case D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1:
				FriendlyName = "Shared Resources are supported to tier 1";
				TestResult->_SharedResourceCompatibilityTier = 1;
				break;
			case D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_2:
				FriendlyName = "Shared Resources are supported to tier 2";
				TestResult->_SharedResourceCompatibilityTier = 2;
				break;
			default:
				break;
			}

			TestResult->Native16BitShaderOpsSupported = SupportedFeatures->SupportData16.Native16BitShaderOpsSupported;
			TestResult->MSAAAlignedTexturesSupported = SupportedFeatures->SupportData16.MSAA64KBAlignedTextureSupported;

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(Options4, ListOfResults, Iterator);
		}

		HRESULT FeatureSerialization = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SERIALIZATION, &SupportedFeatures->SupportData17, sizeof(SupportedFeatures->SupportData17));
		if (FeatureSerialization)
		{
			std::string FriendlyName;

			auto HeapSerialization = SupportedFeatures->SupportData17.HeapSerializationTier;
			switch (HeapSerialization)
			{
			case D3D12_HEAP_SERIALIZATION_TIER_0:
				FriendlyName = "Heap Serialization is not supported";
				TestResult->IsHeapSerializationSupported = false;
				break;
			case D3D12_HEAP_SERIALIZATION_TIER_10:
				FriendlyName = "Heap Serialization is supported, data can be serialised through heaps.";
				TestResult->IsHeapSerializationSupported = true;
				break;
			default:
				break;
			}

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(FeatureSerialization, ListOfResults, Iterator);
		}
	});

	std::thread ThreadC = std::thread([&]() {

		HRESULT CrossNode = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE, &SupportedFeatures->SupportData18, sizeof(SupportedFeatures->SupportData18));
		if (SUCCEEDED(CrossNode))
		{
			std::string FriendlyName;

			if (SupportedFeatures->SupportData18.AtomicShaderInstructions)
			{
				FriendlyName = "Shaders can work across multiple graphics cards!";

				DetermineResourceSharingLevel(SupportedFeatures);
			}
			else
			{
				FriendlyName = "Shaders are isolated to a single accelerator!";
				DetermineResourceSharingLevel(SupportedFeatures);
			}

			AddFriendlyNameToList(FriendlyName);
			AddResultToList(CrossNode, ListOfResults, Iterator);
		}

		HRESULT RenderPasses = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &SupportedFeatures->SupportData19, sizeof(SupportedFeatures->SupportData19));
		if (SUCCEEDED(RenderPasses))
		{
			std::string FriendlyName;

			TestResult->RequiresSRV3Support = SupportedFeatures->SupportData19.SRVOnlyTiledResourceTier3;

			auto RenderPassSupport = SupportedFeatures->SupportData19.RenderPassesTier;
			switch (RenderPassSupport)
			{
			case D3D12_RENDER_PASS_TIER_0:
				TestResult->SupportForRenderPasses = "Support for render passes isn't implemented";
				TestResult->SupportForRenderPass_CODE = 0;
				break;
			case D3D12_RENDER_PASS_TIER_1:
				TestResult->SupportForRenderPasses = "Support for render passes is implemented in the user display driver, RT/DB writes may be accelerated. UAVs are NOT supported";
				TestResult->SupportForRenderPass_CODE = 1;
				break;
			case D3D12_RENDER_PASS_TIER_2:
				TestResult->SupportForRenderPasses = "Render Passes are supported by the display driver, and UAV writes ARE supported, RT/DB writes may be accelerated";
				TestResult->SupportForRenderPass_CODE = 2;
				break;
			default:
				break;
			}

			auto RaytracingSupport = SupportedFeatures->SupportData19.RaytracingTier; //Favourite thing rn!
			switch (RaytracingSupport)
			{
			case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
				TestResult->SupportForRaytracing = "DXR is not supported on this device!";
				break;
			case D3D12_RAYTRACING_TIER_1_0:
				TestResult->SupportForRaytracing = "DXR is supported on this device! To Tier 1.0";
				break;
			case D3D12_RAYTRACING_TIER_1_1:
				TestResult->SupportForRaytracing = "DXR is supported on this device! To Tier 1.1";
				break;
			default:
				break;
			}

			AddFriendlyNameToList(FriendlyName);
			AddFriendlyNameToList(TestResult->SupportForRenderPasses);
			AddFriendlyNameToList(TestResult->SupportForRaytracing);

			AddResultToList(RenderPasses, ListOfResults, Iterator);
		}

		HRESULT VariableRateShading = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &SupportedFeatures->SupportData20, sizeof(SupportedFeatures->SupportData20));
		{
			std::string FriendlyName;

			TestResult->MatrixPixelSizesSupported = SupportedFeatures->SupportData20.AdditionalShadingRatesSupported;
			TestResult->PerPrimitiveAcrossViewports = SupportedFeatures->SupportData20.PerPrimitiveShadingRateSupportedWithViewportIndexing;
			TestResult->BackgroundProcessingSupported = SupportedFeatures->SupportData20.BackgroundProcessingSupported;

			auto VRSSupport = SupportedFeatures->SupportData20.VariableShadingRateTier;

			switch (VRSSupport)
			{
			case D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED:
				TestResult->VRSSupportTier = 0;
				break;
			case D3D12_VARIABLE_SHADING_RATE_TIER_1:
				TestResult->VRSSupportTier = 1;
				break;
			case D3D12_VARIABLE_SHADING_RATE_TIER_2:
				TestResult->VRSSupportTier = 2;
				break;
			default:
				break;
			}

			FriendlyName = "VRS is supported to Tier " + std::to_string(TestResult->VRSSupportTier);


			AddFriendlyNameToList(FriendlyName);
			AddResultToList(VariableRateShading, ListOfResults, Iterator);
		}

		HRESULT QueryMeta = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_QUERY_META_COMMAND, &SupportedFeatures->SupportData21, sizeof(SupportedFeatures->SupportData21));
		if (SUCCEEDED(QueryMeta))
		{
			AddResultToList(QueryMeta, ListOfResults, Iterator);
		}

		HRESULT Options7 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &SupportedFeatures->SupportData22, sizeof(SupportedFeatures->SupportData22));
		if (SUCCEEDED(Options7))
		{
			std::string FriendlyName;

			auto x = SupportedFeatures->SupportData22.MeshShaderTier;

			switch (x)
			{
			case D3D12_MESH_SHADER_TIER_NOT_SUPPORTED:
				TestResult->MeshShaderSupported = false;
				break;
			case D3D12_MESH_SHADER_TIER_1:
				TestResult->MeshShaderSupported = true;
				break;
			default:
				break;
			}

			auto y = SupportedFeatures->SupportData22.SamplerFeedbackTier;

			switch (y)
			{
			case D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED:
				TestResult->SamplerFeedbackTier = 0.0f;
				break;
			case D3D12_SAMPLER_FEEDBACK_TIER_0_9:
				TestResult->SamplerFeedbackTier = 0.9f;
				break;
			case D3D12_SAMPLER_FEEDBACK_TIER_1_0:
				TestResult->SamplerFeedbackTier = 1.0f;
				break;
			default:
				break;
			}

			if (TestResult->MeshShaderSupported)
			{
				FriendlyName = "Mesh Shader is supported";

				if (TestResult->SamplerFeedbackTier > 0.0f)
				{
					FriendlyName += " Sampler Feedback and amplification shaders are supported to " + std::to_string(TestResult->SamplerFeedbackTier);
				}
				else
				{
					FriendlyName += " Sampler Feedback and amplification shaders are not supported";
				}
			}
			else
			{
				FriendlyName = "Mesh Shader is not supported";

				if (TestResult->SamplerFeedbackTier > 0.0f)
				{
					FriendlyName += " Sampler Feedback and amplification shaders are supported to " + std::to_string(TestResult->SamplerFeedbackTier);
				}
				else
				{
					FriendlyName += " Sampler Feedback and amplification shaders are not supported";
				}
			}


			AddResultToList(Options7, ListOfResults, Iterator);
			AddFriendlyNameToList(FriendlyName);

		}

		HRESULT ProtectedResourceCount = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPE_COUNT, &SupportedFeatures->SupportData23, sizeof(SupportedFeatures->SupportData23));
		if (SUCCEEDED(ProtectedResourceCount))
		{
			AddResultToList(ProtectedResourceCount, ListOfResults, Iterator);
		}

		HRESULT ProtectedResourceTypes = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_TYPES, &SupportedFeatures->SupportData24, sizeof(SupportedFeatures->SupportData24));
		if (SUCCEEDED(ProtectedResourceTypes))
		{
			AddResultToList(ProtectedResourceTypes, ListOfResults, Iterator);
		}

	});

	// Re consolidate results and kill threads.

	ThreadA.join();
	ThreadB.join();
	ThreadC.join();

}

void CDirectX::CDirectXTest::GetMaximumFeatureLevel(CDirectXSupportedFeatures* SupportedFeatures, std::string MaximumLevel)
{
	D3D_FEATURE_LEVEL m_MaximumLevel = SupportedFeatures->SupportData2.MaxSupportedFeatureLevel;

	switch (m_MaximumLevel)
	{
	case D3D_FEATURE_LEVEL_1_0_CORE:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;
		MaximumLevel += "DirectX 9.0";
		break;
	case D3D_FEATURE_LEVEL_9_1:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_9_1;
		MaximumLevel += "DirectX 9.1";
		break;
	case D3D_FEATURE_LEVEL_9_2:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_9_2;
		MaximumLevel += "DirectX 9.2";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_9_3;
		MaximumLevel += "DirectX 9.3";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_10_0;
		MaximumLevel += "DirectX 10.0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_10_1;
		MaximumLevel += "DirectX 10.1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		MaximumLevel += "DirectX 11.0";
		break;
	case D3D_FEATURE_LEVEL_11_1:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_11_1;
		MaximumLevel += "DirectX 11.1";
		break;
	case D3D_FEATURE_LEVEL_12_0:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_12_0;
		MaximumLevel += "DirectX 12.0";
		break;
	case D3D_FEATURE_LEVEL_12_1:
		TestResult->MaximumFeatureLevel = D3D_FEATURE_LEVEL_12_1;
		MaximumLevel += "DirectX 12.1";
		break;
	default:
		break;
	}
}

std::string CDirectX::CDirectXTest::InstancingTiers(CDirectXSupportedFeatures* SupportedFeatures, std::string FriendlyName)
{
	auto InstancingTiers = SupportedFeatures->SupportData14.ViewInstancingTier;
	bool InstancingSupported = false;
	switch (InstancingTiers)
	{
	case D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED:
		TestResult->InstancingTier = "Instancing Tier not supported";
		break;
	case D3D12_VIEW_INSTANCING_TIER_1:
		TestResult->InstancingTier = "Instancing Tier Supported up to Tier One";
		InstancingSupported = true;
		break;
	case D3D12_VIEW_INSTANCING_TIER_2:
		TestResult->InstancingTier = "Instancing Tier Supported up to Tier Two";
		InstancingSupported = true;
		break;
	case D3D12_VIEW_INSTANCING_TIER_3:
		TestResult->InstancingTier = "Instancing Tier Supported up to Tier Three";
		InstancingSupported = true;
		break;
	default:
		break;
	}

	if (InstancingSupported)
	{
		FriendlyName += "Instancing across shaders is also supported";
	}		return FriendlyName;
}

void CDirectX::CDirectXTest::GraphicsBufferWriteData(CDirectXSupportedFeatures* SupportedFeatures)
{
	auto WillWriteToGraphicsBufferImmediately = SupportedFeatures->SupportData14.WriteBufferImmediateSupportFlags;
	std::pair<std::string, bool>* Pair = new std::pair<std::string, bool>();

	switch (WillWriteToGraphicsBufferImmediately)
	{
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_BUNDLE:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_BUNDLE";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_COMPUTE:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_COMPUTE";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_COPY:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_COPY";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_DECODE:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_DECODE";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_PROCESS:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_PROCESS";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	case D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_ENCODE:
		Pair->first = "D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_ENCODE";
		Pair->second = true;
		TestResult->SupportsImmediateWritingToBuffersFromCommandQueues.insert(*Pair);
		break;
	default:
		break;
	}

	delete(Pair);
}

//There is an issue with the mutexes, I think we should do this manually. They are attempting to release resources they do not own.
void CDirectX::CDirectXTest::AddResultToList(HRESULT Result, std::forward_list<HRESULT> ListOfResults, std::forward_list<HRESULT>::iterator Iterator)
{
	/*if (FeatureMutex.try_lock_shared() != true)
	{
	}*/

	std::lock_guard<std::shared_mutex> Lock(FeatureMutex);

	ListOfResults.insert_after(Iterator, Result);
}

void CDirectX::CDirectXTest::AddFriendlyNameToList(std::string Result)
{
	std::lock_guard<std::shared_mutex> Lock(FriendlyNameMutex);
	TestResult->FeatureNames.push_back(Result);
}
