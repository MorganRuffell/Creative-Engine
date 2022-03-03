#pragma once
#include "CObject.h"
#include "CCube.h"
#include "CCylinder.h"
#include "CPyramid.h"
#include "CPlane.h"
#include <memory>
enum CEPrimitives
{
	CPLANE,
	CCYLINDER,
	CPYRAMID,
	CCUBE,
};

class CPrimitives : public CObject
{
	CPrimitives() = default;

public:

	std::unique_ptr<CStaticMesh> SelectPrimitive(CEPrimitives PrimitiveToSelect)
	{
		switch (PrimitiveToSelect)
		{
		case CPLANE:
			return std::unique_ptr<CPlane>();
			break;
		case CCYLINDER:
			return std::unique_ptr<CCylinder>();
			break;
		case CPYRAMID:
			return std::unique_ptr<CPyramid>();
			break;
		case CCUBE:
			return std::unique_ptr<CCube>();
			break;
		default:
			break;
		}
	}

private:

	CEPrimitives PrimiveSelection;

};

