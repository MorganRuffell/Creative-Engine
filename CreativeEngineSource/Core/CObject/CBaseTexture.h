#pragma once

#include "CMathCore.h"
#include "CGraphicsCore.h"
#include "CreativeMacros.h"
#include <string>

using Microsoft::WRL::ComPtr;


enum class CESupportedFileTypes
{
	tga, //Targa
	dds, //
	png,
};

class CRMaterialsAPI CBaseTexture
{
	CESupportedFileTypes _STypes;

public:
	CBaseTexture();
	CBaseTexture(D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_FLAGS flag);
	CBaseTexture(D3D12_RESOURCE_DESC1 desc, D3D12_RESOURCE_FLAGS flag);


	~CBaseTexture();

public:

	std::string MaterialName;
	std::wstring FileName;



public:

	ComPtr<ID3D12Resource2> Resource = nullptr;
	ComPtr<ID3D12Resource2> UploadHeap = nullptr;

};



