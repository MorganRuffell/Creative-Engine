#pragma once
#include "DirectXHelper.h"
#include "StepTimer.h"
#include "CreativeMacros.h"
#include <DirectXMath.h>


using namespace DirectX;


class CRGraphicsAPI CRendererBase
{
public:

	CRendererBase();
	~CRendererBase();

	bool IsLoadingComplete;
	float RadiansPerSecond;
	float AngleDes;
	bool IsAppTracking;




protected:

	virtual void Update(CDirectX::StepTimer const& timer);
	virtual bool Render();

protected:

	virtual void SaveState();
	virtual void LoadState();


protected:

	virtual void InitalizeDeviceResources();
	virtual void InitalizePlatformDependentResources();
};

