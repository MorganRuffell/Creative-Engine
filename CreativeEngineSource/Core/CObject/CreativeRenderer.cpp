#pragma once

#include "ShaderStructures.h"
#include "CGraphicsResource.h"
#include <ppltasks.h>
#include <synchapi.h>
#include <pix3.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include "CCube.h"
#include "CPyramid.h"
#include "CPlane.h"
#include "CCylinder.h"
#include "CDirectX12Core.h"
#include "CreativeRenderer.h"
#include "DirectXHelper.h"


static XMVECTORF32 eye = { 1.0f, 0.7f, 2.5f, 1.0f };
static XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
static XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };


using namespace CRE;
using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;



// Loads vertex and pixel shaders from files and instantiates the cube geometry.
CreativeRenderer::CreativeRenderer(const std::shared_ptr<CDirectX::CDirectX12Core>& deviceResources) :
	m_mappedConstantBuffer(nullptr),
	DirectXResources(deviceResources)
{
	ShaderData = std::make_shared<CShaderData>();

	LoadState();
	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

	InitalizeDeviceResources();
	InitalizePlatformDependentResources();
}

CreativeRenderer::~CreativeRenderer()
{
	ConstantBuffer->Unmap(0, nullptr);
	m_mappedConstantBuffer = nullptr;
}

int CRE::CreativeRenderer::HandleMultipleGPUs()
{
	return 0;
}


void CRE::CreativeRenderer::CreatePipelineDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC state)
{
	// THIS IS WHERE WE FETCH DATA FROM THE VERTEX & PIXEL SHADERS -- WARNING -- IF THIS IS WRONG IT WILL NOT BUILD
		// AS IT CANNOT GO THROUGH THE SHADER INSTRUCTIONS, INCLUDE ALL INPUT ELEMENTS
	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	state.InputLayout = { inputLayout, _countof(inputLayout) };
	state.pRootSignature = RootSignature.Get();

	std::vector<byte> vs = ShaderData->VertexShader;
	std::vector<byte> ps = ShaderData->PixelShader;

	//This is where we load in the different shaders, these have to be compiled to bytecode.
	state.VS = CD3DX12_SHADER_BYTECODE(&ShaderData->VertexShader[0], ShaderData->VertexShader.size());
	state.PS = CD3DX12_SHADER_BYTECODE(&ShaderData->PixelShader[0], ShaderData->PixelShader.size());

	/*state.HS
	state.GS
	state.DS*/

	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = DirectXResources->GetBackBufferFormat();
	state.DSVFormat = DirectXResources->GetDepthBufferFormat();
	state.SampleDesc.Count = 1;
	state.NodeMask = HandleMultipleGPUs();

	if (DirectXResources->WarpSupport == true)
	{
		state.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	}
	else
	{
		state.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	state.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
}


void CreativeRenderer::InitalizeDeviceResources()
{
	auto GraphicsCard = DirectXResources->GetD3DDevice();

	// Create a root signature with a single constant buffer.
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;
		parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Constant buffer view init.
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | // Change this
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT | 
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; //We lock to all the other shaders, just need the input assembler. (IA stage)

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);
		
		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;

		CDirectX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));

		HRESULT res = GraphicsCard->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&RootSignature));
		if (FAILED(res))
		{

		}
		else
		{
			CDirectX::ThrowIfFailed(GraphicsCard->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
			NAME_D3D12_OBJECT(RootSignature);
		}

	}

	// Load hlsl shaders across threads -- We're going to have to move away from this model.
	auto createVSTask = CDirectX::ReadDataAsync(L"SampleVertexShader.cso").then([&](std::vector<byte>& VertexFileData) {
		ShaderData->VertexShader = VertexFileData;
		ShaderData->VertexShaders.push_back(VertexFileData);
	});

	auto createPSTask = CDirectX::ReadDataAsync(L"SamplePixelShader.cso").then([&](std::vector<byte>& PixelFileData) {
		ShaderData->PixelShader = PixelFileData;
		ShaderData->PixelShaders.push_back(PixelFileData);
	});

	auto createGSTask = CDirectX::ReadDataAsync(L"SampleGeometryShader.cso").then([&](std::vector<byte>& GeometryFileData) {
		ShaderData->GeometryShader = GeometryFileData;
		ShaderData->GeometryShaders.push_back(GeometryFileData);
	});

	auto createDSTask = CDirectX::ReadDataAsync(L"SampleDomainShader.cso").then([&](std::vector<byte>& DomainFileData) {
		ShaderData->DomainShader = DomainFileData;
		ShaderData->DomainShaders.push_back(DomainFileData);
	});


	// Create the pipeline state once the shaders are loaded.
	auto createPipelineStateTask = (createPSTask && createVSTask && createGSTask && createDSTask).then([this]() {

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		CreatePipelineDesc(state);

		CDirectX::ThrowIfFailed(DirectXResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&PipelineState)));

		// Shader data can be deleted once the pipeline state is created.
		ShaderData->VertexShader.clear();
		ShaderData->PixelShader.clear();
		ShaderData->DomainShader.clear();
		ShaderData->GeometryShader.clear();


		for (auto VertexIterator = ShaderData->VertexShaders.begin(); VertexIterator != ShaderData->VertexShaders.end(); VertexIterator++)
		{
            VertexIterator->clear();
		}

        for (auto PixelIterator = ShaderData->PixelShaders.begin(); PixelIterator != ShaderData->PixelShaders.end(); PixelIterator++)
        {
            PixelIterator->clear();
        }

        for (auto GeometryIterator = ShaderData->GeometryShaders.begin(); GeometryIterator != ShaderData->GeometryShaders.end(); GeometryIterator++)
        {
            GeometryIterator->clear();
        }

        for (auto DomainIterator = ShaderData->DomainShaders.begin(); DomainIterator != ShaderData->DomainShaders.end(); DomainIterator++)
        {
            DomainIterator->clear();
        }

	});

	// Create and upload cube geometry resources to the GPU.
	auto createAssetsTask = createPipelineStateTask.then([&]()
	{
		auto GraphicsCard = DirectXResources->GetD3DDevice();

		HRESULT res = GraphicsCard->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, DirectXResources->GetDirectCommandAllocator(), PipelineState.Get(), IID_PPV_ARGS(&GlobalDirectCommandList));
		if (SUCCEEDED(res))
		{
			HRESULT res2 = GraphicsCard->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, DirectXResources->GetComputeCommandAllocator(), PipelineState.Get(), IID_PPV_ARGS(&GlobalComputeCommandList));
			if (FAILED(res2))
			{
				std::cout << "" << std::endl;
			} 

			std::unique_ptr<CCylinder> StaticMesh = std::make_unique<CCylinder>();

			const UINT vertexBufferSize = StaticMesh->MeshVertexBufferSize();
			const UINT indexBufferSize = StaticMesh->MeshIndexBufferSize();


			CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

			CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

			HRESULT res3 = GraphicsCard->CreateCommittedResource1(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, nullptr, IID_PPV_ARGS(&VertexBuffer));
			if (SUCCEEDED(res3))
			{
				CDirectX::ThrowIfFailed(GraphicsCard->CreateCommittedResource1(
					&uploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&vertexBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					nullptr,
					IID_PPV_ARGS(&StaticMesh->UploadBuffer)));

				NAME_D3D12_OBJECT(VertexBuffer);

				//Then the vertex buffer is uploaded to the GPU
				UploadVertexBufferToGPU(StaticMesh->CylinderVertexes, vertexBufferSize, StaticMesh->UploadBuffer);


				// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
				// The upload resource must not be destroyed until after the GPU has finished using it.
				ComPtr<ID3D12Resource2> indexBufferUpload;

				CreateIndexAndUploadIndexBuffers(indexBufferSize, GraphicsCard, defaultHeapProperties, uploadHeapProperties, indexBufferUpload);
				UploadIndexBufferToGPU(StaticMesh->CylinderIndicies, indexBufferSize, indexBufferUpload);
				CreateDescHeapForBCBuffers(GraphicsCard);

				CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CDirectX::AmountOfRenderTargets * c_alignedConstantBufferSize);

				CDirectX::ThrowIfFailed(GraphicsCard->CreateCommittedResource1(
					&uploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&constantBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					nullptr,
					IID_PPV_ARGS(&ConstantBuffer)));

				NAME_D3D12_OBJECT(ConstantBuffer);


				// Create constant buffer views to access the upload buffer.
				D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = ConstantBuffer->GetGPUVirtualAddress();
				CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(CBV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
				m_cbvDescriptorSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				for (int n = 0; n < CDirectX::AmountOfRenderTargets; n++)
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
					desc.BufferLocation = cbvGpuAddress;
					desc.SizeInBytes = c_alignedConstantBufferSize;
					GraphicsCard->CreateConstantBufferView(&desc, cbvCpuHandle);

					cbvGpuAddress += desc.SizeInBytes;
					cbvCpuHandle.Offset(m_cbvDescriptorSize);
				}

				// Map the constant buffers.
				CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
				CDirectX::ThrowIfFailed(ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
				ZeroMemory(m_mappedConstantBuffer, CDirectX::AmountOfRenderTargets * c_alignedConstantBufferSize);
				// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

				// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
				CDirectX::ThrowIfFailed(GlobalDirectCommandList->Close());
				ID3D12CommandList* ppCommandLists[] = { GlobalDirectCommandList.Get() };

				DirectXResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				// Create vertex/index buffer views.
				VertexBufferViewGPU.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
				VertexBufferViewGPU.StrideInBytes = sizeof(CVertex);
				VertexBufferViewGPU.SizeInBytes = StaticMesh->MeshVertexBufferSize();

				IndexBufferViewGPU.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
				IndexBufferViewGPU.SizeInBytes = StaticMesh->MeshIndexBufferSize();
				IndexBufferViewGPU.Format = DXGI_FORMAT_R16_UINT;

				// Wait for the command list to finish executing; the vertex/index buffers need to be uploaded to the GPU before the upload resources go out of scope.
				DirectXResources->WaitForGpu();
			}

		}
		else
		{
			throw new CException;
		}

	});

	createAssetsTask.then([this]() {
		IsLoadingComplete = true;
	});
}

void CRE::CreativeRenderer::UploadVertexBufferToGPU(CRE::CVertex  cubeVertices[8], const UINT& vertexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource2>& vertexBufferUpload)
{
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(GlobalDirectCommandList.Get(), VertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

	CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	GlobalDirectCommandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
}

void CRE::CreativeRenderer::CreateDescHeapForBCBuffers(ID3D12Device10* d3dDevice)
{
	// Create a descriptor heap for the constant buffers.
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = CDirectX::AmountOfRenderTargets;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CDirectX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&CBV_DescriptorHeap)));

	NAME_D3D12_OBJECT(CBV_DescriptorHeap);
}

void CRE::CreativeRenderer::UploadIndexBufferToGPU(unsigned short  cubeIndices[36], const UINT& indexBufferSize, ComPtr<ID3D12Resource2>& indexBufferUpload)
{
	// Upload the index buffer to the GPU.
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(GlobalDirectCommandList.Get(), IndexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	GlobalDirectCommandList->ResourceBarrier(1, &indexBufferResourceBarrier);
}

void CRE::CreativeRenderer::CreateIndexAndUploadIndexBuffers(const UINT& indexBufferSize, ID3D12Device10* d3dDevice, CD3DX12_HEAP_PROPERTIES& defaultHeapProperties, CD3DX12_HEAP_PROPERTIES& uploadHeapProperties, ComPtr<ID3D12Resource2>& indexBufferUpload)
{
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

	D3D12_DEPTH_STENCIL_VALUE DValue;
	DValue.Depth = 1.0f;
	DValue.Stencil = 0;

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	ClearValue.DepthStencil = DValue;

	HRESULT Indexres = d3dDevice->CreateCommittedResource1(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, &ClearValue, nullptr, IID_PPV_ARGS(&IndexBuffer));
	if (!SUCCEEDED(Indexres))
	{
		HRESULT Indexres = d3dDevice->CreateCommittedResource1(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, nullptr, IID_PPV_ARGS(&IndexBuffer));

		if (!SUCCEEDED(Indexres))
		{
			CDirectX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
				&defaultHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&indexBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&IndexBuffer)));
		}
	}

	HRESULT IndexUploadRes = d3dDevice->CreateCommittedResource1(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &ClearValue, nullptr, IID_PPV_ARGS(&indexBufferUpload));
	if (!SUCCEEDED(IndexUploadRes))
	{
		CDirectX::ThrowIfFailed(d3dDevice->CreateCommittedResource1(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			nullptr,
			IID_PPV_ARGS(&indexBufferUpload)));
	}
}

// Initializes view parameters when the window size changes.
void CreativeRenderer::InitalizePlatformDependentResources()
{
    if (DirectXResources->OutputMonitor != nullptr)
    {
        float outputSize = (DirectXResources->m_backBufferWidth * DirectXResources->m_backBufferHeight);
        float fovAngleY = 70.0f * XM_PI / 180.0f;
        float aspectRatio = (DirectXResources->m_backBufferWidth / DirectXResources->m_backBufferHeight);

        D3D12_VIEWPORT viewport = DirectXResources->GetScreenViewport();
        BasicRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height) };

        // This sample makes use of a right-handed coordinate system 
        XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
            fovAngleY,
            aspectRatio,
            0.01f,
            100.0f
        );

        XMFLOAT4X4 orientation = DirectXResources->GetOrientationTransfromAsFloatMatrix();
        XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

        XMStoreFloat4x4(
            &m_constantBufferData.projection,
            XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
        );


        //// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
        //static XMVECTORF32 eye = { 1.0f, 0.7f, 2.5f, 1.0f };
        //static XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
        //static XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

        XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
    }
    else
    {
        if (DirectXResources->LatestAdapter != nullptr)
        {
            HRESULT res = DirectXResources->LatestAdapter->EnumOutputs(0, &DirectXResources->OutputMonitor);
            if (FAILED(res))
            {
                throw new CException;
            }
            else
            {
                InitalizePlatformDependentResources();
            }
        }
    }
	
}


// Called once per frame, rotates the cube and calculates the model and view matrices.
void CreativeRenderer::Update(CDirectX::StepTimer const& timer)
{
	if (IsLoadingComplete)
	{
		if (!IsAppTracking)
		{
			// Rotate the cube a small amount.
			AngleDes += static_cast<float>(timer.GetElapsedSeconds()) * RadiansPerSecond / 5.0f;

			Rotate(-AngleDes);
		}

		// Update the constant buffer resource.
		UINT8* destination = m_mappedConstantBuffer + (DirectXResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));

		//? What if we took user input and modified?
	}
}

// Rotate the 3D model a set amount of radians.
void CreativeRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader.
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void CreativeRenderer::StartTracking()
{
	IsAppTracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void CreativeRenderer::TrackingUpdate(float positionX)
{
	if (IsAppTracking)
	{
		float radians = XM_2PI * 2.0f * positionX / DirectXResources->m_backBufferWidth;
		Rotate(radians);
	}
}

void CreativeRenderer::StopTracking()
{
	IsAppTracking = false;
}

// Renders one frame using the vertex and pixel shaders.
bool CreativeRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!IsLoadingComplete)
	{
		return false;
	}

	CDirectX::ThrowIfFailed(DirectXResources->GetDirectCommandAllocator()->Reset());

	// The command list can be reset anytime after ExecuteCommandList() is called - Making it durable for my purposes
	CDirectX::ThrowIfFailed(GlobalDirectCommandList->Reset(DirectXResources->GetDirectCommandAllocator(), PipelineState.Get()));

	PIXBeginEvent(GlobalDirectCommandList.Get(), 0, L"Draw the Object");
	{
		GlobalDirectCommandList->SetGraphicsRootSignature(RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBV_DescriptorHeap.Get() };
		GlobalDirectCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Bind the current frame's constant buffer to the pipeline.
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(CBV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), DirectXResources->GetCurrentFrameIndex(), m_cbvDescriptorSize);
		GlobalDirectCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

		// Set the viewport and scissor rectangle.
		D3D12_VIEWPORT viewport = DirectXResources->GetScreenViewport();
		GlobalDirectCommandList->RSSetViewports(1, &viewport);
		GlobalDirectCommandList->RSSetScissorRects(1, &BasicRect);

		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(DirectXResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GlobalDirectCommandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// Record drawing commands.
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = DirectXResources->GetRenderTargetView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DirectXResources->GetDepthStencilView();
		GlobalDirectCommandList->ClearRenderTargetView(renderTargetView, m_RenderTargetViewColour, 0, nullptr);
		GlobalDirectCommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		GlobalDirectCommandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		GlobalDirectCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GlobalDirectCommandList->IASetVertexBuffers(0, 1, &VertexBufferViewGPU);
		GlobalDirectCommandList->IASetIndexBuffer(&IndexBufferViewGPU);
		GlobalDirectCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		// Indicate that the render target will now be used to present when the command list is done executing.
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(DirectXResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		GlobalDirectCommandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(GlobalDirectCommandList.Get());

	CDirectX::ThrowIfFailed(GlobalDirectCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { GlobalDirectCommandList.Get() };
	DirectXResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}
