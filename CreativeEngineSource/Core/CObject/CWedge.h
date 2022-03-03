#pragma once
#include "CStaticMesh.h"
class CRGeometryAPI CWedge : public CStaticMesh
{
public:
	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;

	UINT MeshVertexBufferSize() override;
	UINT MeshIndexBufferSize() override;

public:

	CRE::CVertex WedgeVertexes[8] =
	{
		{ SFloat3(1.0f, 0.0f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //0
		{ SFloat3(-1.0f, 0.0f, 0.0f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(0.0f, 0.0f, 1.0f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 0.0f, -1.0f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },

		{ SFloat3(1.0f, 1.0f, 0.0f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) }, //4
		{ SFloat3(-1.0f, 1.0f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(0.0f, 0.3f, 1.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 0.3f, -1.0f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },

	};

	//Remember to draw the inverse
	unsigned short WedgeIndicies[36] =
	{
		//Test to see which side these are drawing on!
		0,1,2,
		2,1,0,

		3,1,0,
		0,1,3,

		3,7,2,
		2,7,3,

		4,5,6,
		6,5,4,

		6,4,7,
		7,4,6,

		1,5,3,
		3,5,1,

		//Incomplete keep drawing


	};
	 
};

