#pragma once
#include "CStaticMesh.h"

class CCircle : public CStaticMesh
{
public:

	CCircle();
	~CCircle();

public:

	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	void DrawIndicies(CRE::CVertex* Vertexes);

	CRE::CVertex CircleVertexes[19] =
	{
		{ SFloat3(0.0f,		0.0f,	0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //0

		{ SFloat3(-1.16f,	0.0f,	0.0f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) }, //V1
		{ SFloat3(-1.09f,	0.0f,	0.39f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.89f,	0.0f,	0.75f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.58f,	0.0f,	1.0086f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.20f,	0.0f,	1.147f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) }, 
		{ SFloat3(0.20f,	0.0f,	1.147f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.58f,	0.0f,	1.0086f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.89f,	0.0f,	0.75f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(1.09f,	0.0f,	0.39f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(1.16f,	0.0f,	0.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },

		{ SFloat3(1.09f,	0.0f,	-0.39f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) }, //V11
		{ SFloat3(0.89f,	0.0f,	-0.75f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.58f,	0.0f,	-0.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.20f,	0.0f,	-1.0086), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.20f,	0.0f,	-1.147f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.58f,	0.0f,	-1.0086f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.89f,	0.0f,	-0.75f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(1.09f,	0.0f,	-0.39f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
	};

	unsigned short CircleIndicies[96] =
	{
		0,1,2,
		2,1,0,

		0,2,3,
		3,2,0,

		0,3,4,
		4,0,3,

		0,4,5,
		5,4,0,

		0,5,6,
		6,5,0,

		0,6,7,
		7,6,0,

		0,7,8,
		8,7,0,

		0,8,9,
		9,8,0,

		0,9,10,
		10,9,0,

		0,10,11,
		11,10,0,

		0,11,12,
		12,11,0,

		0,12,13,
		13,12,0,

		0,13,14,
		14,13,0,
		
		0,14,15,
		15,14,0,

		0,15,16,
		16,15,0,

		0,16,17,
		17,16,0,

	};



};

