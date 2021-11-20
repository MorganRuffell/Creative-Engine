#include "CStaticMesh.h"
#include "CException.h"

CStaticMesh::CStaticMesh()
{

}

CStaticMesh::~CStaticMesh()
{
}

bool CStaticMesh::SM_LoadData()
{
	ComPtr<ID3D10Blob> VertexShaderBuffer = NULL;
	ComPtr<ID3D10Blob> PixelShaderBuffer = NULL;
	ComPtr<ID3D10Blob> HullShaderBuffer = NULL;



	//Fill the rest of this in.
	HRESULT VertexShaderValid = GraphicsCard->CreateVertexShader(VertexShaderBuffer->GetBufferPointer(), VertexShaderBuffer->GetBufferSize(), 0, &SM_VertexShader); // You need to write the shader ;)
	HRESULT PixelShaderValid = GraphicsCard->CreatePixelShader(PixelShaderBuffer->GetBufferPointer(), PixelShaderBuffer->GetBufferSize(), 0, &SM_PixelShader);
	HRESULT HullShaderValid = GraphicsCard->CreateHullShader(HullShaderBuffer->GetBufferPointer(), HullShaderBuffer->GetBufferSize(), 0, &SM_Shaders->SM_HullShader);



	if (FAILED(VertexShaderValid) || FAILED(PixelShaderValid) || FAILED(HullShaderValid))
		return false;

	D3D11_INPUT_ELEMENT_DESC shaderInputLayout[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TextureCoordinate", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT LayoutElementsCount = ARRAYSIZE(shaderInputLayout);

	auto Result = GraphicsCard->CreateInputLayout(shaderInputLayout, LayoutElementsCount, VertexShaderBuffer->GetBufferPointer(), VertexShaderBuffer->GetBufferSize(), &SM_Shaders->SM_InputLayout);


	VertexShaderBuffer->Release();
	PixelShaderBuffer->Release();

	if (VertexShaderBuffer != nullptr || PixelShaderBuffer != nullptr)
	{
	}


	CStaticMeshVertex CVertexes[] =
	{
		{ SFloat3(0.4f,  0.5f, 1.0f) , SFloat2(1.0f, 1.0f) },
		{ SFloat3(0.4f, -0.5f, 1.0f) , SFloat2(1.0f, 0.0f) },
		{ SFloat3(-0.4f, -0.5f, 1.0f) , SFloat2(0.0f, 0.0f) },

		{ SFloat3(-0.4f, -0.5f, 1.0f) , SFloat2(0.0f, 0.0f) },
		{ SFloat3(-0.4f,  0.5f, 1.0f) , SFloat2(0.0f, 1.0f) },
		{ SFloat3(0.4f,  0.5f, 1.0f) , SFloat2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC VertexDESC;
	ZeroMemory(&VertexDESC, sizeof(VertexDESC));
	VertexDESC.Usage = D3D11_USAGE_DEFAULT; //This binds all grahics rendering to the GPU
	VertexDESC.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexDESC.ByteWidth = sizeof(CStaticMeshVertex);

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = CVertexes;

	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceView;
	ZeroMemory(&ShaderResourceView, sizeof(ShaderResourceView));

	auto ShaderResourceView = GraphicsCard->CreateShaderResourceView(SM_Resource, &ShaderResourceView, &SM_Shaders->SM_ShaderResourceView);

	HRESULT Result = GraphicsCard->CreateBuffer(&VertexDESC,&resourceData, &SM_Shaders->SM_GraphicsBuffer);
	if (FAILED(Result))
		return false;

	//Object that allows us to sample a texture - You have to set attributes manually.
	D3D11_SAMPLER_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	textureDesc.MaxLOD = D3D11_FLOAT32_MAX;
	auto SamplerState = GraphicsCard->CreateSamplerState(&textureDesc, &SM_Shaders->SM_SamplerState);

	return true;
}

bool CStaticMesh::SM_UnloadData()
{
	return false;
}

void CStaticMesh::SM_Update()
{
}

void CStaticMesh::SM_Render()
{
	UINT stride = sizeof(CStaticMeshVertex);
	UINT offset = 0;

	try
	{
		if (GraphicsCardController == NULL || GraphicsCard == NULL) { throw CGraphicsException(); }
		if (stride == NULL) { throw CGraphicsException(); }

		GraphicsCardController->ClearRenderTargetView(RenderTargetView.Get(), BaseColors->CR_BASE_Black);
		GraphicsCardController->IASetInputLayout(SM_Shaders->SM_InputLayout.Get());
		GraphicsCardController->IASetVertexBuffers(0, 1, &SM_Shaders->SM_GraphicsBuffer, &stride, &offset);
		GraphicsCardController->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		GraphicsCardController->VSSetShader(SM_VertexShader, 0, 0);
		GraphicsCardController->PSSetShader(SM_PixelShader, 0, 0);
		GraphicsCardController->PSSetShaderResources(0,1,&SM_Shaders->SM_ShaderResourceView);
		GraphicsCardController->PSSetSamplers(0, 1, &SM_Shaders->SM_SamplerState);

		GraphicsCardController->Draw(6, 0);	
		DXSwapChain->Present(0, 0);

		//Setup other shader types?
	}
	catch (CGraphicsException)
	{

	}
}
