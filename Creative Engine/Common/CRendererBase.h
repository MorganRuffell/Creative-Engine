#pragma once
#include "pch.h"
#include "..\Common\DirectXHelper.h"

#include "..\Common\StepTimer.h"
#include "CreativeMacros.h"

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

