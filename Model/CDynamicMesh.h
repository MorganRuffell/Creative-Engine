#pragma once

#include "pch.h"
#include "CSkinnedMesh.h"
#include "CAnimationCurve.h"
#include "CAnimationSet.h"
#include "CAnimationState.h"

class CDynamicMesh : public CSkinnedMesh
{
public:
	CDynamicMesh()
	{
		
	}
	~CDynamicMesh()
	{

	}

private:

	std::vector<CAnimationCurve>	m_AnimationCurves;
	CAnimationState					AnimationState;
	CAnimationSet					AnimationSet;
};