#pragma once
#include <string>
#include "CMathCore.h"
#include "CBaseTexture.h"

class CBasicMaterial
{
public:
	std::string Name;

	//index into cnst buffer corresponding to material
	int MatCBIndex = -1;
	int DiffuseHeapIndex = -1;
	int NumberFrames;

	float R = 1.0f;
	float G = 1.0f;
	float B = 1.0f;
	float A = 1.0f;

	//UV Space
	float u = 0.0f;
	float v = 0.0f;
	float w = 0.0f;




	//Color Space
	SFloat4 RGBA{ R, G, B, A };
	SFloat3 UVW{ u,v,w };



protected:

	DirectX::XMFLOAT4 DiffuseAlbedo = { R, G, B, A };
	float Roughness = 0.25f;
	DirectX::XMFLOAT3 MaterialTransform = { u,v,w };

};

