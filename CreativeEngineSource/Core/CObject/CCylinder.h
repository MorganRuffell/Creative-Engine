#pragma once
#include "CStaticMesh.h"
class CRGeometryAPI CCylinder : public CStaticMesh
{
public:
	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	UINT MeshVertexBufferSize() override;

	UINT MeshIndexBufferSize() override;


public:

	CRE::CVertex CylinderVertexes[38] =
	{
		//Base and Top
		{ SFloat3(0.0f, 0.0f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //0
		{ SFloat3(0.0f, 0.0f, 2.90f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.0f, 0.0f, 2.7f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(1.9f, 0.0f, 2.2f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(2.5f, 0.0f, 1.5f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(2.9f, 0.0f, 0.5f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(2.9f, 0.0f, -0.5f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(2.5f, 0.0f, -1.4f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(1.9f, 0.0f, -2.2f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.0f, 0.0f, -2.8f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 0.0f, -3.0f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.0f, 0.0f, -2.7f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.9f, 0.0f, -2.2f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(2.5f, 0.0f, -1.5f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(2.9f, 0.0f, -0.5f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(2.9f, 0.0f, 0.5f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(2.5f, 0.0f, 1.4f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.9f, 0.0f, 2.2f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(1.0f, 0.0f, 2.8f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },

		{ SFloat3(0.0f, 1.0f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //20
		{ SFloat3(0.0f, 1.0f, 2.9f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-1.0f, 1.0f, 2.7f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-1.9f, 1.0f, 2.2f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-2.5f, 1.0f, 1.5f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-2.9f, 1.0f, 0.5f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-2.9f, 1.0f, -0.5f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-2.5f, 1.0f, -1.4f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-1.9f, 1.0f, -2.2f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-1.0f, 1.0f, -2.8f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 1.0f, -3.0f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.0f, 1.0f, -2.7f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.9f, 1.0f, -2.2f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(2.5f, 1.0f, -1.5f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(2.9f, 1.0f, -0.5f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(2.9f, 1.0f, 0.5f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(2.5f, 1.0f, 1.4f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(1.9f, 1.0f, 2.2f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(1.0f, 1.0f, 2.8f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
	};

	//Remember to draw the inverse
	unsigned short CylinderIndicies[120] =
	{
		//Top

		0,1,2,
		2,1,0,

		0,2,3,
		3,2,0,

		0,3,4,
		4,3,0,

		0,4,5,
		5,4,0,

		0,6,5,
		5,6,0,

		0,7,6,
		6,7,0,

		0,8,7,
		7,8,0,

		0,9,8,
		8,9,0,

		0,10,9,
		9,10,0,

		0,11,10,
		10,11,0,

		0,12,11,
		11,12,0,

		0,13,12,
		12,13,0,

		0,14,13,
		13,14,0,

		0,15,14,
		14,15,0,

		0,16,15,
		15,16,0,

		0,17,16,
		16,17,0,

		0,18,17,
		17,18,0,

		0,19,18,
		18,19,0,

		0,20,19,
		19,20,0,





		//Bottom

		20,21,22,
		22,21,20






		// I don't know which numbers to use next ARGH!
	};


};

