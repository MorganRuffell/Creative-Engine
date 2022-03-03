#pragma once
#include "CStaticMesh.h"
class CRGeometryAPI CPyramid : public CStaticMesh
{
public:
	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	UINT MeshVertexBufferSize() override;

	UINT MeshIndexBufferSize() override;


public:

	CRE::CVertex PyramidVertexes[5] =
	{
		{ SFloat3(-0.7f, -0.0f, -0.0f), SFloat4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(0.0f, -0.0f,  0.7f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.0f,  0.0f, -0.7f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(0.7f,  0.0f,  0.0f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 1.2f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
	};

	//Remember to draw the inverse
	unsigned short PyramidIndicies[36] =
	{
		0,1,2,
		2,1,0,

		1,3,2,
		2,3,1,

		4,1,0,
		0,1,4,

		0,4,2,
		2,4,0,

		3,4,1,
		1,4,3,

		4,2,3,
		3,2,4,
	};

};

