#pragma once
#include "CVertex.h"
#include "CMathCore.h"


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
	class CRGraphicsAPI CVertex : public CVertexBase
	{
	public:

		CVertex(float x, float y, float z)
		{
			Position.x = x;
			Position.y = y;
			Position.z = z;
		}

		CVertex(SFloat3 location)
		{
			Position.x = location.x;
			Position.y = location.y;
			Position.z = location.z;
		}

		//Multiple components for beginning and Colour it now supports alpha channels
		CVertex(SFloat3 position, SFloat4 Colour)
		{
			Position.x = position.x;
			Position.y = position.y;
			Position.z = position.z;

			color.x = Colour.x;
			color.y = Colour.y;
			color.z = Colour.z;
			color.w = Colour.w;
		}

		CVertex(SFloat3 position, SFloat3 Colour)
		{
			Position.x = position.x;
			Position.y = position.y;
			Position.z = position.z;

			color.x = Colour.x;
			color.y = Colour.y;
			color.z = Colour.z;
			color.w = 1.0f;
		}

		CVertex(SFloat4 LocationAndNormal)
		{
			Position.x = LocationAndNormal.x;
			Position.y = LocationAndNormal.y;
			Position.z = LocationAndNormal.z;
		}

	public:

		~CVertex()
		{

		}

	};


}
