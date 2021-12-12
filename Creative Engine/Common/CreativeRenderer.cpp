#pragma once

#include "pch.h"
#include "CreativeRenderer.h"
#include "..\Common\DirectXHelper.h"
#include <ppltasks.h>
#include <synchapi.h>
#include <iostream>

using namespace CRE;

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Storage;

// Indices into the application state map.
Platform::String^ AngleKey = "Angle";
Platform::String^ TrackingKey = "Tracking";

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
CreativeRenderer::CreativeRenderer(const std::shared_ptr<CDirectX::CDirectX12Core>& deviceResources) :
	m_mappedConstantBuffer(nullptr),
	m_deviceResources(deviceResources)
{
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

void CreativeRenderer::InitalizeDeviceResources()
{
	auto GraphicsCard = m_deviceResources->GetD3DDevice();

	// Create a root signature with a single constant buffer.
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;

		//Constant buffer view init.
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
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

	// Load hlsl shaders across two threads.
	auto createVSTask = CDirectX::ReadDataAsync(L"SampleVertexShader.cso").then([&](std::vector<byte>& VertexFileData) {
		VertexShader = VertexFileData;
		});

	auto createPSTask = CDirectX::ReadDataAsync(L"SamplePixelShader.cso").then([&](std::vector<byte>& PixelFileData) {
		PixelShader = PixelFileData;
		});


	// Create the pipeline state once the shaders are loaded.
	auto createPipelineStateTask = (createPSTask && createVSTask).then([this]() {

		// THIS IS WHERE WE FETCH DATA FROM THE VERTEX & PIXEL SHADERS -- WARNING -- IF THIS IS WRONG IT WILL NOT BUILD
		// AS IT CANNOT GO THROUGH THE SHADER INSTRUCTIONS, INCLUDE ALL INPUT ELEMENTS
		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		state.InputLayout = { inputLayout, _countof(inputLayout) };
		state.pRootSignature = RootSignature.Get();
		state.VS = CD3DX12_SHADER_BYTECODE(&VertexShader[0], VertexShader.size());
		state.PS = CD3DX12_SHADER_BYTECODE(&PixelShader[0], PixelShader.size());
		state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		state.NumRenderTargets = 1;
		state.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
		state.DSVFormat = m_deviceResources->GetDepthBufferFormat();
		state.SampleDesc.Count = 1;

		CDirectX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&PipelineState)));


		// Shader data can be deleted once the pipeline state is created.
		VertexShader.clear();
		PixelShader.clear();
		});

	// Create and upload cube geometry resources to the GPU.
	auto createAssetsTask = createPipelineStateTask.then([&]()
	{
		auto GraphicsCard = m_deviceResources->GetD3DDevice();

		HRESULT res = GraphicsCard->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetDirectCommandAllocator(), PipelineState.Get(), IID_PPV_ARGS(&GlobalDirectCommandList));
		if (SUCCEEDED(res))
		{
			HRESULT res2 = GraphicsCard->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_deviceResources->GetComputeCommandAllocator(), PipelineState.Get(), IID_PPV_ARGS(&GlobalComputeCommandList));
			if (FAILED(res2))
			{
				std::cout << "" << std::endl;
			}

			// Load mesh tris. Now we load the components of the cube. Recall that everything must be triangles
			unsigned short cubeIndices[] =
			{
				0, 2, 1, // -x
				1, 2, 3,

				4, 5, 6, // +x
				5, 7, 6,

				0, 1, 5, // -y
				0, 5, 4,

				2, 6, 7, // +y
				2, 7, 3,

				0, 4, 6, // -z
				0, 6, 2,

				1, 3, 7, // +z
				1, 7, 5,
			};

			const UINT indexBufferSize = sizeof(cubeIndices);

			// Cube vertices. Each vertex has a position and a color.
			CVertex cubeVertices[] =
			{
				{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
				{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
				{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
				{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
				{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
				{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
				{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
				{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
			};

			const UINT vertexBufferSize = sizeof(cubeVertices);

			// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
			// The upload resource must not be Destroyed until after the GPU has finished using it.
			ComPtr<ID3D12Resource> vertexBufferUpload;

			CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

			CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

			HRESULT res3 = GraphicsCard->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&VertexBuffer));
			if (SUCCEEDED(res3))
			{
				CDirectX::ThrowIfFailed(GraphicsCard->CreateCommittedResource(
					&uploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&vertexBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&vertexBufferUpload)));

				NAME_D3D12_OBJECT(VertexBuffer);

				//Then the vertex buffer is uploaded to the GPU
				UploadVertexBufferToGPU(cubeVertices, vertexBufferSize, vertexBufferUpload);


				// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
				// The upload resource must not be destroyed until after the GPU has finished using it.
				ComPtr<ID3D12Resource> indexBufferUpload;

				CreateIndexAndUploadIndexBuffers(indexBufferSize, GraphicsCard, defaultHeapProperties, uploadHeapProperties, indexBufferUpload);
				UploadToGPU(cubeIndices, indexBufferSize, indexBufferUpload);
				CreateDescHeapForBCBuffers(GraphicsCard);
				CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(CDirectX::AmountOfFrames * c_alignedConstantBufferSize);
				CDirectX::ThrowIfFailed(GraphicsCard->CreateCommittedResource(
					&uploadHeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&constantBufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&ConstantBuffer)));

				NAME_D3D12_OBJECT(ConstantBuffer);

				// Create constant buffer views to access the upload buffer.
				D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = ConstantBuffer->GetGPUVirtualAddress();
				CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(CBV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
				m_cbvDescriptorSize = GraphicsCard->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

				for (int n = 0; n < CDirectX::AmountOfFrames; n++)
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
				ZeroMemory(m_mappedConstantBuffer, CDirectX::AmountOfFrames * c_alignedConstantBufferSize);
				// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

				// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
				CDirectX::ThrowIfFailed(GlobalDirectCommandList->Close());
				ID3D12CommandList* ppCommandLists[] = { GlobalDirectCommandList.Get() };
				m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				// Create vertex/index buffer views.
				VertexBufferViewGPU.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
				VertexBufferViewGPU.StrideInBytes = sizeof(CVertex);
				VertexBufferViewGPU.SizeInBytes = sizeof(cubeVertices);

				IndexBufferViewGPU.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
				IndexBufferViewGPU.SizeInBytes = sizeof(cubeIndices);
				IndexBufferViewGPU.Format = DXGI_FORMAT_R16_UINT;

				// Wait for the command list to finish executing; the vertex/index buffers need to be uploaded to the GPU before the upload resources go out of scope.
				m_deviceResources->WaitForGpu();

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

void CRE::CreativeRenderer::UploadVertexBufferToGPU(CRE::CVertex  cubeVertices[8], const UINT& vertexBufferSize, Microsoft::WRL::ComPtr<ID3D12Resource>& vertexBufferUpload)
{
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
}

void CRE::CreativeRenderer::CreateDescHeapForBCBuffers(ID3D12Device8* d3dDevice)
{
	// Create a descriptor heap for the constant buffers.
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = CDirectX::AmountOfFrames;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		CDirectX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&CBV_DescriptorHeap)));

		NAME_D3D12_OBJECT(CBV_DescriptorHeap);
	}
}

void CRE::CreativeRenderer::UploadToGPU(unsigned short  cubeIndices[36], const UINT& indexBufferSize, ComPtr<ID3D12Resource>& indexBufferUpload)
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

void CRE::CreativeRenderer::CreateIndexAndUploadIndexBuffers(const UINT& indexBufferSize, ID3D12Device8* d3dDevice, CD3DX12_HEAP_PROPERTIES& defaultHeapProperties, CD3DX12_HEAP_PROPERTIES& uploadHeapProperties, ComPtr<ID3D12Resource>& indexBufferUpload)
{
	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	CDirectX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&IndexBuffer)));

	CDirectX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)));
}

// Initializes view parameters when the window size changes.
void CreativeRenderer::InitalizePlatformDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
	BasicRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height) };

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system 
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransfromAsFloatMatrix();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
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
		UINT8* destination = m_mappedConstantBuffer + (m_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));
	}
}

void CreativeRenderer::SaveState()
{
	auto state = ApplicationData::Current->LocalSettings->Values;

	if (state->HasKey(AngleKey))
	{
		state->Remove(AngleKey);
	}
	if (state->HasKey(TrackingKey))
	{
		state->Remove(TrackingKey);
	}

	state->Insert(AngleKey, PropertyValue::CreateSingle(AngleDes));
	state->Insert(TrackingKey, PropertyValue::CreateBoolean(IsAppTracking));
}

void CreativeRenderer::LoadState()
{
	auto state = ApplicationData::Current->LocalSettings->Values;
	if (state->HasKey(AngleKey))
	{
		AngleDes = safe_cast<IPropertyValue^>(state->Lookup(AngleKey))->GetSingle();
		state->Remove(AngleKey);
	}
	if (state->HasKey(TrackingKey))
	{
		IsAppTracking = safe_cast<IPropertyValue^>(state->Lookup(TrackingKey))->GetBoolean();
		state->Remove(TrackingKey);
	}
}

// Rotate the 3D cube model a set amount of radians.
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
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
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

	CDirectX::ThrowIfFailed(m_deviceResources->GetDirectCommandAllocator()->Reset());

	// The command list can be reset anytime after ExecuteCommandList() is called - Making it durable for my purposes
	CDirectX::ThrowIfFailed(GlobalDirectCommandList->Reset(m_deviceResources->GetDirectCommandAllocator(), PipelineState.Get()));

	PIXBeginEvent(GlobalDirectCommandList.Get(), 0, L"Draw the Object");
	{
		GlobalDirectCommandList->SetGraphicsRootSignature(RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBV_DescriptorHeap.Get() };
		GlobalDirectCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Bind the current frame's constant buffer to the pipeline.
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(CBV_DescriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_deviceResources->GetCurrentFrameIndex(), m_cbvDescriptorSize);
		GlobalDirectCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

		// Set the viewport and scissor rectangle.
		D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
		GlobalDirectCommandList->RSSetViewports(1, &viewport);
		GlobalDirectCommandList->RSSetScissorRects(1, &BasicRect);

		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GlobalDirectCommandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// Record drawing commands.
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
		GlobalDirectCommandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::Gray, 0, nullptr);
		GlobalDirectCommandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		GlobalDirectCommandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		GlobalDirectCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GlobalDirectCommandList->IASetVertexBuffers(0, 1, &VertexBufferViewGPU);
		GlobalDirectCommandList->IASetIndexBuffer(&IndexBufferViewGPU);
		GlobalDirectCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		// Indicate that the render target will now be used to present when the command list is done executing.
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		GlobalDirectCommandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(GlobalDirectCommandList.Get());

	CDirectX::ThrowIfFailed(GlobalDirectCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { GlobalDirectCommandList.Get() };
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}
