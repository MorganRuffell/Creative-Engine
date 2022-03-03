#pragma once

#include <map>
#include "CDirectX12Core.h"
#include <DirectXColors.h>
#include <DirectXMath.h>
#include "CRendererBase.h"
#include "ShaderStructures.h"

namespace CRE
{
	struct CCreativeRendererState
	{
		std::string GetRendererState()
		{
			std::string result;
			return result = RendererState;
		}

		void SetRendererState(int a)
		{
			if (!(a > 5))
			{
				RendererState = RendererStates.at(a);
			}
			else
			{
				RendererState = 5;
			}
		}

		std::map<int, std::string> RendererStates = {
			{0,"Begin"},
			{1,"Init"},
			{2,"Running"},
			{3,"Suspended"},
			{4,"Resuming"},
			{5,"Terminating"}
		};

	private:

		std::string RendererState = "";
	};


	//Contains all the data from all the shaders that are loaded into creative. -- Wait.
	struct CRGraphicsAPI CShaderData
	{
		std::vector<std::vector<byte>> VertexShaders;
		std::vector<std::vector<byte>> PixelShaders;
		std::vector<std::vector<byte>> DomainShaders;
		std::vector<std::vector<byte>> GeometryShaders;

	public:

		std::vector<byte> GeometryShader;
		std::vector<byte> DomainShader;
		std::vector<byte> VertexShader;
		std::vector<byte> PixelShader;
	};


	class CRGraphicsAPI CreativeRenderer : public CRendererBase
	{
	public:
		CreativeRenderer(const std::shared_ptr<CDirectX::CDirectX12Core>& deviceResources);
		~CreativeRenderer();

		//These are the begin plays for a renderer, the first one creates device independent resources, second gets device data to check requirements.
		void InitalizeDeviceResources() override;
		void UploadVertexBufferToGPU(CRE::CVertex  cubeVertices[8], const UINT& vertexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource2>& vertexBufferUpload);
		void CreateDescHeapForBCBuffers(ID3D12Device10* d3dDevice);
		void UploadIndexBufferToGPU(unsigned short  cubeIndices[36], const UINT& indexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource2>& indexBufferUpload);
		void CreateIndexAndUploadIndexBuffers(const UINT& indexBufferSize, ID3D12Device10* d3dDevice, CD3DX12_HEAP_PROPERTIES& defaultHeapProperties, CD3DX12_HEAP_PROPERTIES& uploadHeapProperties, Microsoft::WRL::ComPtr<ID3D12Resource2>& indexBufferUpload);
		void InitalizePlatformDependentResources() override;

		int HandleMultipleGPUs();
		void CreatePipelineDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC state);

    //This section manages the default render target view colour
    public:

        float m_BackgroundRed = 1.0f;
        float m_BackgroundGreen = 1.0f;
        float m_BackgroundBlue = 1.0f;
        float m_BackgroundAlpha = 1.0f;

    private:

        XMVECTORF32 m_RenderTargetViewColour = { m_BackgroundRed, m_BackgroundGreen, m_BackgroundBlue, m_BackgroundAlpha };

	public:

		//Tick, programs will tick every frame. This timer will keep track of time :D
		void Update(CDirectX::StepTimer const& timer) override;
		bool Render() override;

		//This code tracks the state of the renderer - black box for me for now
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return IsAppTracking; }

	private:
		void Rotate(float radians);

	public:
		// Constant buffers must be 256-byte aligned.
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// Cached pointer to device resources.
		std::shared_ptr<CDirectX::CDirectX12Core> DirectXResources;

		D3D12_RECT	BasicRect;

		std::shared_ptr<CShaderData> ShaderData;


		//GPU based structs that allow us to modify data.
		D3D12_VERTEX_BUFFER_VIEW VertexBufferViewGPU;
		D3D12_INDEX_BUFFER_VIEW	IndexBufferViewGPU;



	public:

		//Implementation for pipeline states
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC SecondaryState = {};


	private:


		ComPtr<ID3D12GraphicsCommandList5> GlobalDirectCommandList;
		ComPtr<ID3D12GraphicsCommandList5> GlobalComputeCommandList;

		ComPtr<ID3D12RootSignature>	RootSignature;
		ComPtr<ID3D12PipelineState>	PipelineState;
		ComPtr<ID3D12DescriptorHeap> CBV_DescriptorHeap;

		ComPtr<ID3D12Resource2> VertexBuffer;
		ComPtr<ID3D12Resource2> IndexBuffer;
		ComPtr<ID3D12Resource2> ConstantBuffer;


		ModelViewProjectionConstantBuffer	m_constantBufferData;

		UINT8* m_mappedConstantBuffer;
		UINT m_cbvDescriptorSize;
	};
}

