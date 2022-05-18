//Creative Engine - Ruffell Interactive 2022 - Morgan Ruffell

#pragma once
#include "CRendererBase.h"
#include <memory>
#include "ShaderStructures.h"
#include "CDirectX12Core.h"
#include "d3dx12.h"
#include <DirectXMath.h>
#include "DXRResources/CShaderTable.h"


namespace CDXR
{
#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

    struct SceneConstantBuffer
    {
        DirectX::XMMATRIX projectionToWorld;
        DirectX::XMVECTOR cameraPosition;
        DirectX::XMVECTOR lightPosition;
        DirectX::XMVECTOR lightAmbientColor;
        DirectX::XMVECTOR lightDiffuseColor;
    };

    struct CubeConstantBuffer
    {
        DirectX::XMFLOAT4 albedo;
    };

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
    };

    struct CDXRBuffer
    {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    };

    struct Global_RaytracingStructures
    {
        enum Value {
            OutputViewSlot = 0,
            AccelerationStructureSlot,
            SceneConstantSlot,
            VertexBuffersSlot,
            Count
        };
    };


    struct Local_RaytracingStructures
    {
        enum Value
        {
            ViewportConst = 0,
            Count
        };

    };

    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer     m_constants;
        uint8_t                 m_alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };

    struct RayGenerationConstantBuffer
    {
        Viewport viewport;
        Viewport stencil;
    };

    static constexpr UINT RenderTargetCount = 3;

    interface IDeviceNotify
    {
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;
    };

    class CRaytracingRenderer : public CRendererBase
    {
    public:

        CRaytracingRenderer(_In_ UINT Width, _In_ UINT Height, std::string Name)
        {
            //This is the index of the unordered access buffer that we are going to use to store
            //the rays.

            m_RTXOutputResourceUAVDescHeapIndex = UINT_MAX;
            UpdateForSizeChange(Width, Height);
        }

        CRaytracingRenderer(_In_opt_ CDirectX::CDirectX12Core* GraphicsCore, _In_ UINT Width, _In_ UINT Height, std::string Name)
        {
            //This is the index of the unordered access buffer that we are going to use to store
            //the rays.
            m_RTXOutputResourceUAVDescHeapIndex = UINT_MAX;

            UpdateForSizeChange(Width, Height);
        }

        void Init(HWND Window, _In_ UINT m_width, _In_ UINT m_height);


        void Update(CDirectX::StepTimer const& timer) override;
        bool Render() override;
        void CleanUp();

    protected:

        void SaveState() override;
        void LoadState() override;


    protected:

        void InitalizeDeviceResources() override;
        void InitalizePlatformDependentResources() override;

    public:

        IDXGISwapChain4* GetRaytracingSwapchain()
        {
            return DirectXCore->GetSwapChain();
        }

        /// <summary>
        /// Manages the creation of rays as well as recreating Direct3D
        /// </summary>
        void RecreateD3D();

        void DoRaytracing();

    public:

        /// <summary>
        /// Releases Resources
        /// </summary>
        void ReleaseDeviceDependentResources();

        void ReleaseWindowSizeDependentResources();

    public:
        /// <summary>
        /// Creating required resources
        /// </summary>
        void CreateDXRDeviceResources();

        void CreateDXRWindowResources();

        void CreateRaytracingOutputResources();

        void InitDXRInterfaces();

        void CreateRootSignatures();

        void CreateLocalRootSignatureSubobjects(_In_  CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);

        int CreateShaderResourceViewBuffer(ID3D12Device8* GraphicsCard, CDXRBuffer
            * ShaderResourceViewBuffer, UINT NumberOfElements, UINT ElementSize);

        void InitDXRPSO();

        void CreateDescriptorHeap();

        void InitDXROutputResource();

    public:

        void SerializeDXRRootSignature(_In_ D3D12_ROOT_SIGNATURE_DESC& desc, _In_ ID3D12RootSignature* rootSig);

    public:

        void BuildGeometry();

        void InitDXRAccelerationStructures();

        void BuildPlaneGeometry();

        void BuildShaderTables();

        void UpdateForSizeChange(_In_ UINT clientWidth, _In_ UINT clientHeight);

        void CopyRaytracingOutputToBackbuffer();

        void CalculateFrameStats();

        UINT AllocateDescriptor(_In_ D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);

    private:

        void AllocateUploadBuffer(ID3D12Device8* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr)
        {
            auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);
            HRESULT UploadRes = (pDevice->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(ppResource)));

            if (!SUCCEEDED(UploadRes))
            {
                throw new std::exception;
            }
            else
            {
                if (resourceName)
                {
                    (*ppResource)->SetName(resourceName);
                }
                void* pMappedData;
                (*ppResource)->Map(0, nullptr, &pMappedData);
                memcpy(pMappedData, pData, datasize);
                (*ppResource)->Unmap(0, nullptr);
            }

        }

    private:

        //Scene Data
        SceneConstantBuffer                                m_SceneConstantBuffer[RenderTargetCount];
        CubeConstantBuffer                                 m_CubeConstantBuffer;




    private:

        // DirectX Raytracing (DXR) attributes
        ComPtr<ID3D12Device8>                              m_DXRDevice;
        ComPtr<ID3D12GraphicsCommandList5>                 m_DXRGraphicsCommandList;
        ComPtr<ID3D12StateObject>                          m_DXRStateObject;
        UINT                                               m_DXRSupportExtent;

        // Root signatures
        ComPtr<ID3D12RootSignature>                        m_DXRGlobalRootSignature;
        ComPtr<ID3D12RootSignature>                        m_DXRLocalRootSignature;

        // Descriptors
        ComPtr<ID3D12DescriptorHeap>                       m_descriptorHeap;
        UINT                                               m_descriptorsAllocated;
        UINT                                               m_descriptorSize;

    private:

        AlignedSceneConstantBuffer*                        m_mappedConstantData;
        ComPtr<ID3D12Resource>                             m_PerFrameConstants;

    private:

        /// <summary>
        /// Ray Generation Buffer - This buffer contains information about light generation
        /// </summary>

        Local_RaytracingStructures                          m_RayGenerationBuffer;
        RayGenerationConstantBuffer                         m_RayGenerationCB;

        /// <summary>
        /// Geometry
        /// </summary>
        typedef UINT16 Index;


        CDXRBuffer                                          m_IndexBuffer;
        CDXRBuffer                                          m_VertexBuffer;

    private:

        /// <summary>
        /// Raytracing Acceleration Components
        /// </summary>
        ComPtr<ID3D12Resource>                              m_LocalAcclerationStructure;

        ComPtr<ID3D12Resource>                              m_BottomLevelAccelerationStructure;
        ComPtr<ID3D12Resource>                              m_TopLevelAccelerationStrucutre;

        // Raytracing output
        ComPtr<ID3D12Resource>                              m_RTXOutput;
        D3D12_GPU_DESCRIPTOR_HANDLE                         m_RTXOutputResourceUAVGPUDescriptor;
        UINT                                                m_RTXOutputResourceUAVDescHeapIndex;

    private:

        /// <summary>
        /// Raytracing Shader Handlers 
        /// </summary>
        static const wchar_t* m_hitGroupName;
        static const wchar_t* m_raygenShaderName;
        static const wchar_t* m_closestHitShaderName;
        static const wchar_t* m_missShaderName;

        /// <summary>
        /// Resources to hold the shaders that are dispatched
        /// to manage the raytracing
        /// </summary>
        ComPtr<ID3D12Resource>                              m_missShaderTable;
        ComPtr<ID3D12Resource>                              m_hitGroupShaderTable;
        ComPtr<ID3D12Resource>                              m_rayGenShaderTable;

    private:
        /// <summary>
        /// DXR Specific Structures
        ///
        /// These are some of main components that create Raytracing Acceleration Structures
        /// </summary>
        ///

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC                              m_RaytracingAccelerationStructureDescription;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS                            m_RaytracingAccelerationStructureInput;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_TOOLS_VISUALIZATION_HEADER        m_RaytracingAccelerationStructureTVH;

        const D3D12_RESOURCE_STATES                                                     m_initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;


        /// <summary>
        /// These are for managing the shader payload and the baycentrics of the shader
        /// Again Raytracing Gems... 10/10 -- Great Book!
        ///
        /// Config Payload Size -- This is the maximum size in bytes for the ray payload
        /// Config Attributes Size -- This is the maximum size in bytes for the ray attributes
        /// Recursion Depth --  This is the maximum TraceRay() recursion depth, keep this looww!
        /// </summary>

        UINT                                                                            m_DXRConfig_PayloadSize = 2;
        UINT                                                                            m_DXRConfig_AttributeSize = 4;
        UINT                                                                            m_DXRRecursionDepth = 1;



    public:

        std::unique_ptr<CDirectX::CDirectX12Core>                                       DirectXCore;
        UINT                                                                            AdapterIDOverrider = UINT_MAX;

    };
}

