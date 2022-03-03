#include "CStaticMesh.h"
#include "CreativeMacros.h"

class CDiamond : public CStaticMesh
{
public:

	CDiamond();
	~CDiamond();

public:

	unsigned short GetIndCount() override;
	unsigned int GetVertexCount() override;


	UINT MeshVertexBufferSize() override;

	UINT MeshIndexBufferSize() override;

    CRE::CVertex DiamondVertexes[6] =
    {
        { SFloat3(0.0f, 1.0f, 0.0f), SFloat4(1.0f, 0.0f, 0.0f, 1.0f) }, //0

        { SFloat3(-1.0f, 0.0f, 1.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
        { SFloat3(-1.0f, 0.0f, -1.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
        { SFloat3(1.0f, 0.0f, -1.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },
        { SFloat3(1.0f, 0.0f, 1.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },

        { SFloat3(0.0f, 0.0f, 0.0f), SFloat4(1.0f, 0.0f, 1.0f, 1.0f) },

    };


    unsigned short DiamondIndicies[33] =
    {
        0,1,2,
        2,1,0,

        2,0,3,
        3,0,2,

        3,0,4,
        5,1,2,
        2,1,5,

        2,5,3,
        3,5,2,

        3,5,4,
        4,5,3,

    };


};
