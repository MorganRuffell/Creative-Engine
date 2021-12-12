#pragma once
#include "Common/CVertex.h"
#include "Common/CMathCore.h"


namespace CRE
{
	// Constant buffer used to send Model View Projections to constant buffers
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	class CRGraphicsAPI CVertex 
	{
	public:

		SFloat3 Position;
		SFloat3 color;
		SFloat2 TexCoordinates;

	public:

		SFloat3 Normal; //12 byte offset
		SFloat2 Texture0; //24-byte offset
		SFloat2 Texture1; //32-byte offset
	};


}