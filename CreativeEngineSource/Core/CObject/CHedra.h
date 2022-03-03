#pragma once
#include "CStaticMesh.h"
class CHedra : public CStaticMesh
{
public:
	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	UINT MeshVertexBufferSize() override;

	UINT MeshIndexBufferSize() override;

public:

	CRE::CVertex HedraVertexes[6] =
	{
		{ SFloat3(2.3f, 0.0f, -2.3f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //0
		{ SFloat3(2.3f, 0.0f, 2.3f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-2.3f, 0.0f, 2.3f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(-2.3f, 0.0f, -2.3f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(0.0f, 3.2f, 0.0f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(0.0f, -3.2f, 0.0f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
	};



};

