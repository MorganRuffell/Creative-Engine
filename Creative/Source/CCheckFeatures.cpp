#include "CCheckFeatures.h"


void CCheckFeatures::SystemCheck()
{
	HRESULT FactoryResult = CreateDXGIFactory(__uuidof(LatestFactory), &LatestFactory);
	if (FAILED(FactoryResult))
	{
		throw CGraphicsException();
		std::cout << "Creating System CheckFactory Failed!" << std::endl;

		HRESULT FactoryResultOne = CreateDXGIFactory(__uuidof(Factory), &Factory);
		if (FAILED(FactoryResultOne))
		{
			HRESULT FactoryResultTwo = CreateDXGIFactory(__uuidof(FallBackFactory), &FallBackFactory);
			if (FAILED(FactoryResultTwo))
			{
				std::cout << "Creating Factory Failed!" << std::endl;
			}

			std::cout << "Creating Factory Failed!" << std::endl;
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

void CCheckFeatures::CheckGPUFeatures(CDirectXSupportedFeatures* SupportedFeatures)
{
	if (SupportedFeatures != nullptr)
	{
		std::forward_list<HRESULT> ListOfResults[3];

		std::forward_list<HRESULT>::iterator Iterator0;
		std::forward_list<HRESULT>::iterator Iterator1;
		std::forward_list<HRESULT>::iterator Iterator2;


		std::thread Dx12FeaturesToCheckThreada = std::thread([&] {
			HRESULT BasicArch = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &SupportedFeatures->SupportData0, sizeof(SupportedFeatures->SupportData0));
			Iterator0 = ListOfResults[0].insert_after(ListOfResults[0].before_begin(), BasicArch);

			HRESULT BasicFeature = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &SupportedFeatures->SupportData1, sizeof(SupportedFeatures->SupportData1));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, BasicArch);

			HRESULT FormatSupport = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &SupportedFeatures->SupportData2, sizeof(SupportedFeatures->SupportData2));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, FormatSupport);

			HRESULT MultisampleQuality = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &SupportedFeatures->SupportData3, sizeof(SupportedFeatures->SupportData3));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, MultisampleQuality);

			HRESULT FormatInfo = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &SupportedFeatures->SupportData4, sizeof(SupportedFeatures->SupportData4));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, FormatInfo);

			HRESULT VirtualAddress = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &SupportedFeatures->SupportData5, sizeof(SupportedFeatures->SupportData5));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, VirtualAddress);

			HRESULT ShaderModel = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &SupportedFeatures->SupportData6, sizeof(SupportedFeatures->SupportData6));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, ShaderModel);

			HRESULT Options1 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &SupportedFeatures->SupportData7, sizeof(SupportedFeatures->SupportData7));
			Iterator0 = ListOfResults[0].insert_after(Iterator0, Options1);
			});

		std::thread Dx12FeaturesToCheckThreadb = std::thread([&] {

			HRESULT ProtectedResourceSession = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT, &SupportedFeatures->SupportData8, sizeof(SupportedFeatures->SupportData8));
			Iterator1 = ListOfResults[1].insert_after(ListOfResults[1].before_begin(), ProtectedResourceSession);

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

			});

		std::thread Dx12FeaturesToCheckThreadc = std::thread([&] {

			HRESULT CrossNode = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE, &SupportedFeatures->SupportData18, sizeof(SupportedFeatures->SupportData18));
			Iterator2 = ListOfResults[2].insert_after(ListOfResults[1].before_begin(), CrossNode);

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

			HRESULT Options8 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS8, &SupportedFeatures->SupportData25, sizeof(SupportedFeatures->SupportData25));
			Iterator2 = ListOfResults[2].insert_after(Iterator2, Options8);

			HRESULT Options9 = GraphicsCard->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9, &SupportedFeatures->SupportData26, sizeof(SupportedFeatures->SupportData26));
			Iterator2 = ListOfResults[2].insert_after(Iterator2, Options9);
			});

		Dx12FeaturesToCheckThreada.join();

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

		Dx12FeaturesToCheckThreadb.join();

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

		Dx12FeaturesToCheckThreadc.join();

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

