#include "pch.h"
#include "Creative_EngineMain.h"
#include "Common\DirectXHelper.h"
#include <memory>

using namespace CRE;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;


CreativeGraphicsEngineMain::CreativeGraphicsEngineMain()
{

}

bool CreativeGraphicsEngineMain::Render()
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	return m_sceneRenderer->Render();
}

void CreativeGraphicsEngineMain::OnWindowSizeChanged()
{
	m_sceneRenderer->InitalizePlatformDependentResources();
}

// Notifies the app that it is being suspended.
void CreativeGraphicsEngineMain::OnSuspending()
{
	// TODO: Replace this with your app's suspending logic.
	m_sceneRenderer->SaveState();

}

// Notifes the app that it is no longer suspended.
void CreativeGraphicsEngineMain::OnResuming()
{
}

void CreativeGraphicsEngineMain::OnDeviceRemoved()
{
	m_sceneRenderer->SaveState();
	m_sceneRenderer = nullptr;
}
