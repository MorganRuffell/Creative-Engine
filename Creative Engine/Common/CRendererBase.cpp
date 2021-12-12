#include "pch.h"
#include "CRendererBase.h"

bool CRendererBase::Render()
{
    return true;
}

void CRendererBase::SaveState()
{
}

void CRendererBase::LoadState()
{
}

CRendererBase::CRendererBase()
{
    IsLoadingComplete = false;
    RadiansPerSecond = DirectX::XM_PIDIV4;
    AngleDes = 0;
    IsAppTracking = false;
}

CRendererBase::~CRendererBase()
{
}

void CRendererBase::Update(CDirectX::StepTimer const& timer)
{
}

void CRendererBase::InitalizeDeviceResources()
{
}

void CRendererBase::InitalizePlatformDependentResources()
{
}
