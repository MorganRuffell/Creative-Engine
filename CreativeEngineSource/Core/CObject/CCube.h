#pragma once

#include "CStaticMesh.h"

class CRGeometryAPI CCube : public CStaticMesh
{
public:

	unsigned short GetIndCount() override {
		return sizeof(cubeIndices) / sizeof (unsigned short);
	}

	UINT MeshVertexBufferSize() override {
		return sizeof(cubeVertices);
	}

	UINT MeshIndexBufferSize() override
	{
		return sizeof(cubeIndices);
	}

	unsigned int GetVertexCount() override {
		return sizeof(cubeVertices) / sizeof(unsigned int);
	}


public:

	CRE::CVertex cubeVertices[8] =
	{
		{ SFloat3(-0.5f, -0.5f, -0.5f), SFloat4(0.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(-0.5f, -0.5f,  0.5f), SFloat4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(-0.5f,  0.5f, -0.5f), SFloat4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(-0.5f,  0.5f,  0.5f), SFloat4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ SFloat3(0.5f, -0.5f, -0.5f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ SFloat3(0.5f, -0.5f,  0.5f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ SFloat3(0.5f,  0.5f, -0.5f), SFloat4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ SFloat3(0.5f,  0.5f,  0.5f), SFloat4(1.0f, 1.0f, 1.0f, 1.0f) },
	};

	unsigned short cubeIndices[36] =
	{
		0, 2, 1, // -x
		1, 2, 3,

		4, 5, 6, // +x
		5, 7, 6,

		0, 1, 5, // -y
		0, 5, 4,

		2, 6, 7, // +y
		2, 7, 3,

		0, 4, 6, // -z
		0, 6, 2,

		1, 3, 7, // +z
		1, 7, 5,
	};
};

