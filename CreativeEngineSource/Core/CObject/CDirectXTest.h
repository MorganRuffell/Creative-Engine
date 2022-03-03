#pragma once

#include "dxgi1_6.h"
#include "CreativeMacros.h"
#include "CMathCore.h"
#include "CException.h"
#include "CTestBase.h"
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <concurrent_vector.h>
#include <vector>
#include <forward_list>


using Microsoft::WRL::ComPtr;

namespace CDirectX
{
	struct CRCoreAPI CDirectXSupportedFeatures
	{
		D3D12_FEATURE_DATA_ARCHITECTURE SupportData0 = {};
		D3D12_FEATURE_DATA_FORMAT_SUPPORT SupportData1 = {};
		D3D12_FEATURE_DATA_FEATURE_LEVELS SupportData2 = {};
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS SupportData3 = {};
		D3D12_FEATURE_DATA_FORMAT_INFO SupportData4 = {};
		D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT SupportData5 = {};
		D3D12_FEATURE_DATA_SHADER_MODEL SupportData6 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 SupportData7 = {};
		D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_SUPPORT SupportData8 = {};
		D3D12_FEATURE_DATA_ROOT_SIGNATURE SupportData9 = {};
		D3D12_FEATURE_DATA_ARCHITECTURE1 SupportData10 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS2 SupportData11 = {};
		D3D12_FEATURE_DATA_SHADER_CACHE SupportData12 = {};
		D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY SupportData13 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS3 SupportData14 = {};
		D3D12_FEATURE_DATA_EXISTING_HEAPS SupportData15 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS4 SupportData16 = {};
		D3D12_FEATURE_DATA_SERIALIZATION SupportData17 = {};
		D3D12_FEATURE_DATA_CROSS_NODE SupportData18 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 SupportData19 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 SupportData20 = {};
		D3D12_FEATURE_DATA_QUERY_META_COMMAND SupportData21 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 SupportData22 = {};
		D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPE_COUNT SupportData23 = {};
		D3D12_FEATURE_DATA_PROTECTED_RESOURCE_SESSION_TYPES SupportData24 = {};
	};

	struct CRCoreAPI CTestResults
	{
		/// The amount of GPUs in a system
		int AmountOfAccelerators;


		//These are Concurrent collections which are part of the Parallel Pattern Library
		Concurrency::concurrent_vector<std::string> FeatureNames;

		std::string MaximumShaderModel;
		std::unordered_map<std::string, bool> SupportedMSAALevels;

		bool DoesSupportWaveShaderOperations;
		bool TransitionsPossibleThroughConstantBufferViews;

		int WaveLaneCountMinimum;
		int WaveLaneCountMaximum;
		int TotalLaneCount;

		bool DoesUseTileBasedRenderer;
		bool DoesSupportUnifiedMemoryAllocation;
		bool DoesDriverAndHardwareSupportUnifiedMemoryAllocation;
		bool IsMemoryManagementUnitIsolated;

		bool DepthBoundsTestSupported;
		int ShaderSamplePositionsSupportLevel;

		std::unordered_map<std::string, bool> SupportsImmediateWritingToBuffersFromCommandQueues; //commandQueues because we're focusing on memory, command lists are just for CPUs
		std::string InstancingTier; //How Many times a shader can be drawn in a render target
		bool CastingFromOneFormatToAnother;

		bool SupportsPersistantDiagnosticMemoryHeaps;
		int _SharedResourceCompatibilityTier;
		bool MSAAAlignedTexturesSupported;
		bool Native16BitShaderOpsSupported;

		bool IsHeapSerializationSupported;
		int SharingTier;

		bool RequiresSRV3Support;

		std::string SupportForRenderPasses;
		int SupportForRenderPass_CODE; //An integer representing level of Render Pass support

		std::string SupportForRaytracing;
		int RaytracingSupport_CODE; //An integer representing level of Raytracing support

		bool MatrixPixelSizesSupported;
		bool PerPrimitiveAcrossViewports;
		bool BackgroundProcessingSupported;
		int VRSSupportTier;

		float SamplerFeedbackTier;
		bool MeshShaderSupported;
		D3D_FEATURE_LEVEL MaximumFeatureLevel;
	};

	class CRCoreAPI CDirectXTest : public CTestBase
	{
	public:

		std::shared_mutex FeatureMutex;
		std::shared_mutex FriendlyNameMutex;

		std::condition_variable FeatureCondition;
		std::condition_variable FriendlyNameCondition;

		CTestResults* TestResult;

		CDirectXTest()
		{
			CDirectXSupportedFeatures* SupportedFeatures = new CDirectXSupportedFeatures;

			TestResult = new CTestResults();

			SystemCheck();
			CheckMSAAFeatures();

			CheckGPUFeatures(SupportedFeatures);

			delete(SupportedFeatures);
		}


	private:

		void CheckMSAAFeatures();
		void SystemCheck();
		void CheckGPUFeatures(CDirectXSupportedFeatures* SupportedFeatures);

		void GetMaximumFeatureLevel(CDirectXSupportedFeatures* SupportedFeatures, std::string MaximumLevel);
		

		void DetermineResourceSharingLevel(CDirectXSupportedFeatures* SupportedFeatures)
		{
			auto SharingLevel = SupportedFeatures->SupportData18.SharingTier;

			switch (SharingLevel)
			{
			case D3D12_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED:
				TestResult->SharingTier = 0;
				break;
			case D3D12_CROSS_NODE_SHARING_TIER_1_EMULATED:
				TestResult->SharingTier = 1;
				break;
			case D3D12_CROSS_NODE_SHARING_TIER_1:
				TestResult->SharingTier = 2;
				break;
			case D3D12_CROSS_NODE_SHARING_TIER_2:
				TestResult->SharingTier = 3;
				break;
			case D3D12_CROSS_NODE_SHARING_TIER_3:
				TestResult->SharingTier = 4;
				break;
			default:
				break;
			}
		}

		std::string InstancingTiers(CDirectXSupportedFeatures* SupportedFeatures, std::string FriendlyName);
		void GraphicsBufferWriteData(CDirectXSupportedFeatures* SupportedFeatures);
		

		void AddResultToList(HRESULT Result, std::forward_list<HRESULT> ListOfResults, std::forward_list<HRESULT>::iterator iterator);
		void AddFriendlyNameToList(std::string Result);


	public:

		// Direct3D objects.
		ComPtr<ID3D12Device8> GraphicsCard;
		ComPtr<ID3D12Device6> BackupGraphicsCard;

		ComPtr<IDXGIFactory7> LatestFactory;
		ComPtr<IDXGIFactory6> Factory;
	

	private:

		bool WarpSupport;


		int UnsupportedFeatureCount;
		int SupportedFeatureCount;
		D3D12_FEATURE Features;
	};



}


