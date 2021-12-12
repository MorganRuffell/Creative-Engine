#pragma once

#include "..\Common\CDirectX12Core.h"
#include "CRendererBase.h"
#include "..\Content\ShaderStructures.h"

namespace CRE
{
	class CRGraphicsAPI CreativeRenderer : public CRendererBase
	{
	public:
		CreativeRenderer(const std::shared_ptr<CDirectX::CDirectX12Core>& deviceResources);
		~CreativeRenderer();

		//These are the begin plays for a renderer, the first one creates device independent resources, second gets device data to check requirements.
		void InitalizeDeviceResources() override;
		void UploadVertexBufferToGPU(CRE::CVertex  cubeVertices[8], const UINT& vertexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource>& vertexBufferUpload);
		void CreateDescHeapForBCBuffers(ID3D12Device8* d3dDevice);
		void UploadToGPU(unsigned short  cubeIndices[36], const UINT& indexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource>& indexBufferUpload);
		void CreateIndexAndUploadIndexBuffers(const UINT& indexBufferSize, ID3D12Device8* d3dDevice, CD3DX12_HEAP_PROPERTIES& defaultHeapProperties, CD3DX12_HEAP_PROPERTIES& uploadHeapProperties, Microsoft::WRL::ComPtr<ID3D12Resource>& indexBufferUpload);
		void InitalizePlatformDependentResources() override;

	public:

		//Tick, programs will tick every frame. This timer will keep track of time :D
		void Update(CDirectX::StepTimer const& timer) override;
		bool Render() override;

		//This code tracks the state of the renderer - black box for me for now
		void SaveState() override;
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return IsAppTracking; }

	private:
		void LoadState() override;
		void Rotate(float radians);

	private:
		// Constant buffers must be 256-byte aligned.
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// Cached pointer to device resources.
		std::shared_ptr<CDirectX::CDirectX12Core> m_deviceResources;

		// Direct3D resources for cube geometry and shader resources
		D3D12_RECT	BasicRect;

		//Shader data must be stored as bytes
		std::vector<byte> VertexShader;
		std::vector<byte> PixelShader;

		//GPU based structs that allow us to modify data.
		D3D12_VERTEX_BUFFER_VIEW VertexBufferViewGPU;
		D3D12_INDEX_BUFFER_VIEW	IndexBufferViewGPU;

		ComPtr<ID3D12GraphicsCommandList5> GlobalDirectCommandList;
		ComPtr<ID3D12GraphicsCommandList5> GlobalComputeCommandList;

		ComPtr<ID3D12RootSignature>	RootSignature;
		ComPtr<ID3D12PipelineState>	PipelineState;
		ComPtr<ID3D12DescriptorHeap> CBV_DescriptorHeap;

		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		ComPtr<ID3D12Resource> ConstantBuffer;


		ModelViewProjectionConstantBuffer	m_constantBufferData;

		UINT8* m_mappedConstantBuffer;
		UINT m_cbvDescriptorSize;
	};
}

