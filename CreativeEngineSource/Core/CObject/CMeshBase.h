#pragma once
#include "CMathCore.h"
#include "ShaderStructures.h"
#include <wrl.h>
#include <wrl/client.h>

using namespace DirectX;
using namespace Microsoft::WRL;

//Base class for all meshes, anything that possesses a vertex!
class CRCoreAPI CMeshBase
{
public:

	CMeshBase();
	~CMeshBase();

	virtual unsigned short GetIndCount() = 0;
	virtual unsigned int GetVertexCount() = 0;

	virtual CRE::CVertex* GetVertexes() = 0;
	virtual unsigned short* GetIndicies() = 0;

public:

	SFloat3 OriginPoint = {0.0f, 0.0f, 0.0f};

};

