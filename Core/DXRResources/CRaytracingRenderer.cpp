#include "CRaytracingRenderer.h"
#include "pch.h"


const wchar_t* CDXR::CRaytracingRenderer::m_hitGroupName = L"MyHitGroup";
const wchar_t* CDXR::CRaytracingRenderer::m_raygenShaderName = L"MyRaygenShader";
const wchar_t* CDXR::CRaytracingRenderer::m_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* CDXR::CRaytracingRenderer::m_missShaderName = L"MyMissShader";

using namespace DirectX;

void CDXR::CRaytracingRenderer::Init(HWND Window, UINT m_width, UINT m_height)
{
    DirectXCore = std::make_unique<CDirectX::CDirectX12Core>(DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        RenderTargetCount,
        D3D_FEATURE_LEVEL_12_0,
        0,
        0);

    DirectXCore->SetWindow(Window, m_width, m_height);
    DirectXCore->CreateDXGIAdapater();

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 Option5 = {};
    HRESULT res = DirectXCore->GetD3DDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Option5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));

    if (SUCCEEDED(res))
    {
        auto RaytracingSupport = Option5.RaytracingTier;
        switch (RaytracingSupport)
        {
        case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
            std::cout << "Raytracing is not supported by your GPU" << std::endl;
            m_DXRSupportExtent = 0;
            break;
        case D3D12_RAYTRACING_TIER_1_0:
            std::cout << "Raytracing Tier 1.0 Initalized" << std::endl;
            m_DXRSupportExtent = 1;
            break;

        case D3D12_RAYTRACING_TIER_1_1:
            std::cout << "Raytracing Tier 1.1 Initalized" << std::endl;
            m_DXRSupportExtent = 2;
            break;
        default:
            break;
        }
    }

    DirectXCore->CreateDeviceResources();
    DirectXCore->CreateWindowSizeDependentResources();

    CreateDXRDeviceResources();
    CreateDXRWindowResources();

}


void CDXR::CRaytracingRenderer::Update(CDirectX::StepTimer const& timer)
{
}

bool CDXR::CRaytracingRenderer::Render()
{
    if (!DirectXCore->IsWindowVisible())
    {
        return false;
    }

    DirectXCore->Prepare();
    DoRaytracing();
    CopyRaytracingOutputToBackbuffer();

    DirectXCore->Present(D3D12_RESOURCE_STATE_PRESENT);
    return true;

}

void CDXR::CRaytracingRenderer::CleanUp()
{
}

void CDXR::CRaytracingRenderer::SaveState()
{
}

void CDXR::CRaytracingRenderer::LoadState()
{
}

void CDXR::CRaytracingRenderer::InitalizeDeviceResources()
{
}

void CDXR::CRaytracingRenderer::InitalizePlatformDependentResources()
{
}

void CDXR::CRaytracingRenderer::RecreateD3D()
{
    // Give GPU a chance to finish its execution in progress.
    try
    {
        DirectXCore->WaitForGpu();
    }
    catch (HrException&)
    {
        // Do nothing, currently attached adapter is unresponsive.
    }

    DirectXCore->HandleDeviceLost();
}

void CDXR::CRaytracingRenderer::DoRaytracing()
{
    auto frameIndex = DirectXCore->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes;
        dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = m_width;
        dispatchDesc->Height = m_height;
        dispatchDesc->Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(dispatchDesc);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables

        m_DXRGraphicsCommandList->SetComputeRootDescriptorTable(Global_RaytracingStructures::VertexBuffersSlot, m_IndexBuffer.gpuDescriptorHandle);
        m_DXRGraphicsCommandList->SetComputeRootDescriptorTable(Global_RaytracingStructures::OutputViewSlot, m_RTXOutputResourceUAVGPUDescriptor);
    };

    m_DXRGraphicsCommandList->SetComputeRootSignature(m_DXRGlobalRootSignature.Get());

    // Copy the updated scene constant buffer to GPU.
    memcpy(&m_mappedConstantData[frameIndex].m_constants, &m_SceneConstantBuffer[frameIndex], sizeof(m_SceneConstantBuffer[frameIndex]));
    auto cbGpuAddress = m_PerFrameConstants->GetGPUVirtualAddress() + frameIndex * sizeof(m_mappedConstantData[0]);
    m_DXRGraphicsCommandList->SetComputeRootConstantBufferView(Global_RaytracingStructures::SceneConstantSlot, cbGpuAddress);

    // Bind the heaps, acceleration structure and dispatch rays.
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    SetCommonPipelineState(m_DXRGraphicsCommandList.Get());
    m_DXRGraphicsCommandList->SetComputeRootShaderResourceView(Global_RaytracingStructures::AccelerationStructureSlot, m_TopLevelAccelerationStrucutre->GetGPUVirtualAddress());


    DispatchRays(m_DXRGraphicsCommandList.Get(), m_DXRStateObject.Get(), &dispatchDesc);


}

void CDXR::CRaytracingRenderer::CreateDXRDeviceResources()
{
    InitDXRInterfaces();
    CreateRootSignatures();
    InitDXRPSO();
    CreateDescriptorHeap();
    

    BuildGeometry();
    InitDXRAccelerationStructures();
    BuildShaderTables();
    InitDXROutputResource();

}

void CDXR::CRaytracingRenderer::CreateDXRWindowResources()
{
    CreateRaytracingOutputResources();
    BuildShaderTables();

}

void CDXR::CRaytracingRenderer::CreateRaytracingOutputResources()
{
    auto BackBufferFormat = DirectXCore->GetBackBufferFormat();

    //You've got additional controls for texture layout -- But this must be a UAV
    auto UnorderedAccessViewDesc = CD3DX12_RESOURCE_DESC::Tex2D(BackBufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT res = DirectXCore->GetD3DDevice()->CreateCommittedResource(&DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, &UnorderedAccessViewDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_RTXOutput));

    D3D12_CPU_DESCRIPTOR_HANDLE DXR_UAVDescriptorHandle;
    m_RTXOutputResourceUAVDescHeapIndex = AllocateDescriptor(&DXR_UAVDescriptorHandle, m_RTXOutputResourceUAVDescHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC DXR_UAVDescriptor;
    DXR_UAVDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

    DirectXCore->GetD3DDevice()->CreateUnorderedAccessView(m_RTXOutput.Get(), nullptr, &DXR_UAVDescriptor, DXR_UAVDescriptorHandle);

    m_RTXOutputResourceUAVGPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_RTXOutputResourceUAVDescHeapIndex, m_descriptorSize);

}

void CDXR::CRaytracingRenderer::ReleaseDeviceDependentResources()
{
}

void CDXR::CRaytracingRenderer::ReleaseWindowSizeDependentResources()
{
    m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();
    m_RTXOutput.Reset();
}

void CDXR::CRaytracingRenderer::InitDXRInterfaces()
{
    m_DXRDevice = DirectXCore->GetD3DDevice();
    m_DXRGraphicsCommandList = DirectXCore->GetGraphicsCommandList();
}

void CDXR::CRaytracingRenderer::SerializeDXRRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ID3D12RootSignature* rootSig)
{
}

void CDXR::CRaytracingRenderer::CreateRootSignatures()
{
    /// <summary>
    /// Global Root Signature Paramaters - These are shared globally across all rays dispatched
    /// </summary>


    CD3DX12_DESCRIPTOR_RANGE DXRUAVDescriptor;
    DXRUAVDescriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

    CD3DX12_ROOT_PARAMETER DXRRootParamaters[Global_RaytracingStructures::Count];
    DXRRootParamaters[Global_RaytracingStructures::OutputViewSlot].InitAsDescriptorTable(1, &DXRUAVDescriptor);
    DXRRootParamaters[Global_RaytracingStructures::AccelerationStructureSlot].InitAsShaderResourceView(0);

    CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(DXRRootParamaters), DXRRootParamaters);
    SerializeDXRRootSignature(globalRootSignatureDesc, m_DXRGlobalRootSignature.Get());

    /// <summary>
    /// Local Root Signature Paramaters - These allow a raytracing shader to have special paramaters
    /// </summary>

    CD3DX12_ROOT_PARAMETER rootParameters[Local_RaytracingStructures::Count];
    rootParameters[Local_RaytracingStructures::ViewportConst].InitAsConstants(SizeOfInUint32(m_RayGenerationCB), 0, 0);


    CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
    localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    SerializeDXRRootSignature(localRootSignatureDesc, m_DXRLocalRootSignature.Get());
}

void CDXR::CRaytracingRenderer::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
}

int CDXR::CRaytracingRenderer::CreateShaderResourceViewBuffer(ID3D12Device8* GraphicsCard, CDXRBuffer* ShaderResourceViewBuffer, UINT NumberOfElements, UINT ElementSize)
{
    assert(GraphicsCard != nullptr);
    assert(ShaderResourceViewBuffer != nullptr);

    //Shader Resource View
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.Buffer.NumElements = NumberOfElements;

    if (ElementSize == 0)
    {
        SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        SRVDesc.Buffer.StructureByteStride = 0;
    }
    else
    {
        SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
        SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        SRVDesc.Buffer.StructureByteStride = ElementSize;
    }

    int DescriptorIndex = AllocateDescriptor(&ShaderResourceViewBuffer->cpuDescriptorHandle);
    GraphicsCard->CreateShaderResourceView(ShaderResourceViewBuffer->resource.Get(), &SRVDesc, ShaderResourceViewBuffer->cpuDescriptorHandle);

    return DescriptorIndex;
}

/// <summary>
/// Creates a special Pipeline state object called DXRSO - Raytracing State Object
/// This is really special and really powerful for our purposes as it allows us access to all the shaders
/// reachable through the DispatchRays() -- Think of them like compute shaders!
///
/// There are SEVEN objects that combine to make a DXRSO -- So far you've only seen two / three
///
/// I really reccomend reading 'Raytracing Gems 1 & 2' to understand what I'm doing!
/// 
/// </summary>
void CDXR::CRaytracingRenderer::InitDXRPSO()
{
    CD3DX12_STATE_OBJECT_DESC DXRPipelineDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    auto Library = DXRPipelineDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    //Check the #includes for the HLSL -- Let's make a PROPER Lit lighting system
    
    D3D12_SHADER_BYTECODE DXRShaderLibrary = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    Library->SetDXILLibrary(&DXRShaderLibrary);
    {
        Library->DefineExport(m_raygenShaderName);
        Library->DefineExport(m_closestHitShaderName);
        Library->DefineExport(m_missShaderName);
    }

    auto hitGroup = DXRPipelineDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(L"Closest Import bois");
    hitGroup->SetHitGroupExport(L"Export Bois");
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    hitGroup->SetIntersectionShaderImport(L"Intersectional Bois");

    auto shaderConfig = DXRPipelineDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = 4 * sizeof(float);   // float4 color
    UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    CreateLocalRootSignatureSubobjects(&DXRPipelineDesc);

    auto globalRootSignature = DXRPipelineDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_DXRGlobalRootSignature.Get());

    auto pipelineConfig = DXRPipelineDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    pipelineConfig->Config(m_DXRRecursionDepth);

#if _DEBUG
    std::cout << "Um... You're in luck the secret initalized properly!" << std::endl;
#endif

    HRESULT DXRRes = m_DXRDevice->CreateStateObject(DXRPipelineDesc, IID_PPV_ARGS(&m_DXRStateObject));
    if (SUCCEEDED(DXRRes))
    {
        std::cout << "Raytracing State Object Initalized" << std::endl;
    }
    else
    {
        std::cout << "Raytracing State Object Failed to Initalize" << std::endl;
    }
}

void CDXR::CRaytracingRenderer::CreateDescriptorHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC DXRDescriptorHeap = {};
    DXRDescriptorHeap.NumDescriptors = 1;
    DXRDescriptorHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    DXRDescriptorHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    DXRDescriptorHeap.NodeMask = 0;

    m_descriptorSize = m_DXRDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

/// <summary>
/// Recall that all data that is used in raytracing pipelines must be stored in UAVs.
/// UAVs are unordered access views -- These can be accessed anywhere at anytime during runtime.
/// </summary>
void CDXR::CRaytracingRenderer::InitDXROutputResource()
{

    auto backbufferFormat = DirectXCore->GetBackBufferFormat();


    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(m_DXRDevice->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_RTXOutput)));
    NAME_D3D12_OBJECT(m_RTXOutput);

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
    m_RTXOutputResourceUAVDescHeapIndex = AllocateDescriptor(&uavDescriptorHandle, m_RTXOutputResourceUAVDescHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

    m_DXRDevice->CreateUnorderedAccessView(m_RTXOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
    m_RTXOutputResourceUAVGPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_RTXOutputResourceUAVDescHeapIndex, m_descriptorSize);

}

void CDXR::CRaytracingRenderer::BuildGeometry()
{
    auto device = DirectXCore->GetD3DDevice();

    // Cube indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };

    // Cube vertices positions and corresponding triangle normals.
    Vertex vertices[] =
    {
        { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },

        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f) },

        { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },

        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) },

        { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_IndexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_VertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor table.
    // Vertex buffer descriptor must follow index buffer descriptor in the descriptor heap.
    UINT descriptorIndexIB = CreateShaderResourceViewBuffer(device, &m_IndexBuffer, sizeof(indices) / 4, 0);
    UINT descriptorIndexVB = CreateShaderResourceViewBuffer(device, &m_VertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index!");
}

void CDXR::CRaytracingRenderer::InitDXRAccelerationStructures()
{
    m_DXRGraphicsCommandList->Reset(DirectXCore->GetCommandAllocator(), nullptr);

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.IndexBuffer = m_IndexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_IndexBuffer.resource->GetDesc().Width) / sizeof(Index);
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.Transform3x4 = 1; //Used to be 0 -- I'm autistic, wanna see what it does!
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_VertexBuffer.resource->GetDesc().Width) / sizeof(CRE::CVertex);
    geometryDesc.Triangles.VertexBuffer.StartAddress = m_VertexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(CRE::CVertex);

    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS RAS_BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS DXRAS_TopLevelInputs = {};
    DXRAS_TopLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    DXRAS_TopLevelInputs.Flags = RAS_BuildFlags;
    DXRAS_TopLevelInputs.NumDescs = 1;
    DXRAS_TopLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    m_DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&m_RaytracingAccelerationStructureInput, &topLevelPrebuildInfo);
    HRESULT topRes = (topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS DXRAS_BottomLevelInputs = DXRAS_TopLevelInputs;
    DXRAS_BottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    DXRAS_BottomLevelInputs.pGeometryDescs = &geometryDesc;


    m_DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&DXRAS_BottomLevelInputs, &bottomLevelPrebuildInfo);
    HRESULT bottomRes = (bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    ComPtr<ID3D12Resource> ScratchDisk;
    AllocateUAVBuffer(m_DXRDevice.Get(),max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &ScratchDisk,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

   
    // Create an instance desc for the bottom-level acceleration structure.
    ComPtr<ID3D12Resource> instanceDescs;
    D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
    instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
    instanceDesc.InstanceMask = 1;
    instanceDesc.AccelerationStructure = m_BottomLevelAccelerationStructure->GetGPUVirtualAddress();

    AllocateUploadBuffer(m_DXRDevice.Get(), &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");

    // Bottom Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    {
        bottomLevelBuildDesc.Inputs = DXRAS_BottomLevelInputs;
        bottomLevelBuildDesc.ScratchAccelerationStructureData = ScratchDisk->GetGPUVirtualAddress();
        bottomLevelBuildDesc.DestAccelerationStructureData = m_BottomLevelAccelerationStructure->GetGPUVirtualAddress();
    }

    // Top Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    {
        m_RaytracingAccelerationStructureInput.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
        topLevelBuildDesc.Inputs = m_RaytracingAccelerationStructureInput;
        topLevelBuildDesc.DestAccelerationStructureData = m_TopLevelAccelerationStrucutre->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = ScratchDisk->GetGPUVirtualAddress();
    }

    auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
    {
        raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
        m_DXRGraphicsCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_BottomLevelAccelerationStructure.Get()));
        raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    };

    // Always yeet your acceleeration structures inside of the default heap, using these cool AllocateUAVBuffer -- They like it in there
    // It's nice, cosy, and the system syncronizes reads/writes to acceleration structures using UAV barriers

    AllocateUAVBuffer(m_DXRDevice.Get(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_BottomLevelAccelerationStructure, m_initialResourceState, L"BottomLevelAccelerationStructure");
    AllocateUAVBuffer(m_DXRDevice.Get(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_TopLevelAccelerationStrucutre, m_initialResourceState, L"TopLevelAccelerationStructure");

    // Build acceleration structure.
    BuildAccelerationStructure(m_DXRGraphicsCommandList.Get());

    DirectXCore->ExecuteCommandList();

    DirectXCore->WaitForGpu();
}

void  CDXR::CRaytracingRenderer::BuildPlaneGeometry()
{
    auto device = DirectXCore->GetD3DDevice();
    // Plane indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

    };

    // Cube vertices positions and corresponding triangle normals.
    Vertex vertices[] =
    {
        { DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_IndexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_VertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = CreateShaderResourceViewBuffer(m_DXRDevice.Get(), &m_IndexBuffer, sizeof(indices) / 4, 0);

    //Use ARRAYSIZE on second to last paramater if this doesnt work
    UINT descriptorIndexVB = CreateShaderResourceViewBuffer(m_DXRDevice.Get(), &m_VertexBuffer, sizeof(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
}

void CDXR::CRaytracingRenderer::BuildShaderTables()
{
    if (m_DXRDevice == nullptr)
        throw new std::exception;

    void* ID_RayGenerationShader;
    void* ID_MissShader;
    void* ID_HitGroup;
    void* ID_ClosestHitShader;


    auto InitShaderIDs = [&](auto* DXRStateObjectProps) {
        ID_RayGenerationShader = DXRStateObjectProps->GetShaderIdentifier(m_raygenShaderName);
        ID_MissShader = DXRStateObjectProps->GetShaderIdentifier(m_missShaderName);
        ID_HitGroup = DXRStateObjectProps->GetShaderIdentifier(m_hitGroupName);
        ID_ClosestHitShader = DXRStateObjectProps->GetShaderIdentifier(m_closestHitShaderName);
    };

    UINT shaderIdentifierSize;
    {
        //These are combined the state object declared in the
        //Header file.
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties;

        ThrowIfFailed(m_DXRStateObject.As(&stateObjectProperties));
        InitShaderIDs(stateObjectProperties.Get());

        //Once We've collected all the identifers we store this directly in
        //bytes
        shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    /// Credit to Chuck Walbourn for explaing some of this -- Thanks Dude!
    // Ray gen shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        CShaderTable rayGenShaderTable(m_DXRDevice.Get(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        rayGenShaderTable.push_back(CShaderRow(ID_RayGenerationShader, shaderIdentifierSize));
        m_rayGenShaderTable = rayGenShaderTable.GetResource();
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        CShaderTable missShaderTable(m_DXRDevice.Get(), numShaderRecords, shaderRecordSize, L"MissShaderTable");
        missShaderTable.push_back(CShaderRow(ID_MissShader, shaderIdentifierSize));
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table
    {
        struct RootArguments {
            CubeConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = m_CubeConstantBuffer;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        CShaderTable hitGroupShaderTable(m_DXRDevice.Get(), numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        hitGroupShaderTable.push_back(CShaderRow(ID_HitGroup, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }

}

void CDXR::CRaytracingRenderer::UpdateForSizeChange(UINT clientWidth, UINT clientHeight)
{
    m_width = clientWidth;
    m_height = clientHeight;
}

void CDXR::CRaytracingRenderer::CopyRaytracingOutputToBackbuffer()
{
    auto commandList = DirectXCore->GetGraphicsCommandList();
    auto renderTarget = DirectXCore->GetRenderTarget();

    //We have to allocate the render target -- what we are renderering and the results of our raytrace to the graphics command list so that
    //it can tell the GPU what to render for this frame
    D3D12_RESOURCE_BARRIER PreListBarriers[2];

    PreListBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    PreListBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_RTXOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(ARRAYSIZE(PreListBarriers), PreListBarriers);

    commandList->CopyResource(renderTarget, m_RTXOutput.Get());

    //Then we cleanup
    D3D12_RESOURCE_BARRIER PostListBarriers[2];

    PostListBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    PostListBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_RTXOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandList->ResourceBarrier(ARRAYSIZE(PostListBarriers), PostListBarriers);
}

void CDXR::CRaytracingRenderer::CalculateFrameStats()
{
}

UINT CDXR::CRaytracingRenderer::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto DescriptorHeapCPUBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    auto DescriptorHeapGPUBase = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();

    if (descriptorIndexToUse >- m_descriptorHeap->GetDesc().NumDescriptors)
    {
        //We increment this so that there is a degree of clearance within our descriptor heaps
        descriptorIndexToUse = m_descriptorsAllocated++;
    }

    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapCPUBase, descriptorIndexToUse, m_descriptorSize);
    auto GPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(DescriptorHeapGPUBase, descriptorIndexToUse, m_descriptorSize);
    

    return descriptorIndexToUse;
}

