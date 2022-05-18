#pragma once

#include "pch.h"
#include "CStaticMesh.h"

/// <summary>
/// 
///		CCompositeMesh is where we have a load of static meshes that 
///		are drawn on screen inside a single object.
/// 
///		A CStaticMesh is literally just one mesh object, a mesh singular mesh object
///		a CCompositeMesh is where there are many non-convex and convex meshes all in one.
/// 
///		My hope for the future of creative, is that it one day is as smooth as something like
///		Unreal, just with a slightly different workflow
/// 
/// </summary>

//Note about rendering, render each Meshes in the array individually.
class CCompositeMesh
{
	friend class CStaticMesh;

public:
	CCompositeMesh();
	~CCompositeMesh();

	void ReserveMeshesArray(_In_ int Size)
	{
		if (Meshes.size() < 1)
		{
			Meshes.reserve(Size);
		}
		else
		{

		}
	}

	void InsertNewStaticMesh(CStaticMesh* Mesh)
	{
		Meshes.push_back(Mesh);
	}

private:

	int m_amountOfStaticMeshes;

	std::vector<CStaticMesh*> Meshes;

};
