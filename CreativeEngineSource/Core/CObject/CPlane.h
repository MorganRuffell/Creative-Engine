#pragma once
#include "CStaticMesh.h"
class CRGeometryAPI CPlane : public CStaticMesh
{
public:
	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	UINT MeshVertexBufferSize() override;

	UINT MeshIndexBufferSize() override;

public:

	CRE::CVertex PlaneVertexes[4] =
	{
		{ SFloat3(-0.7f, -0.0f, -0.7f), SFloat4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(0.7f, -0.0f,  -0.7f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.7f,  0.0f, 0.7f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-0.7f,  0.0f,  0.7f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
	};

	//Remember to draw the inverse
	unsigned short PlaneIndicies[36] =
	{
		0,1,2,
		2,1,0,

		3,2,0,
		0,2,3
	};


};

