#pragma once

#include "CDirectXCore.h"
#include "CMathCore.h"
#include "CVertex.h"


struct CRGraphicsAPI CStaticMeshVertex : public CVertex
{
	SFloat2 GetTextureCoordinates()
	{
		return TexCoordinates;
	}

	SFloat3 GetPosition()
	{
		return Position;
	}
};


struct CRGraphicsAPI CShaders
{
	
	ComPtr<ID3D11InputLayout> SM_InputLayout;
	ComPtr<ID3D11Buffer> SM_GraphicsBuffer;

	ComPtr<ID3D11HullShader> SM_HullShader;
	ComPtr<ID3D11DomainShader> SM_DomainShader;
	ComPtr<ID3D11ComputeShader> SM_ComputeShader;


	ComPtr<ID3D11ShaderResourceView> SM_ShaderResourceView;
	ComPtr<ID3D11SamplerState> SM_SamplerState;

	ComPtr<ID3D11ClassInstance> SM_ClassInstance;

};


class CRGraphicsAPI CStaticMesh : public CDirectXCore
{
public:
	CStaticMesh();
	virtual ~CStaticMesh();

public:

	virtual bool SM_LoadData();
	virtual bool SM_UnloadData();

	virtual void SM_Update();
	virtual void SM_Render();

protected:

	std::unique_ptr<CShaders> SM_Shaders;
	ID3D11Resource* SM_Resource;
	ID3D11VertexShader* SM_VertexShader;
	ID3D11PixelShader* SM_PixelShader;

};

