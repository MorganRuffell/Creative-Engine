#pragma once
#include "Common\StepTimer.h"
#include "Common\CDirectX12Core.h"
#include "Common\CreativeRenderer.h"

namespace CRE
{
	class CreativeGraphicsEngineMain
	{
	public:
		CreativeGraphicsEngineMain();
		void CreateRenderers(const std::shared_ptr<CDirectX::CDirectX12Core>& deviceResources)
		{
			m_sceneRenderer = std::make_unique<CreativeRenderer>(deviceResources);

			OnWindowSizeChanged();
		}

		void Update()
		{
			// Update scene objects.
			m_timer.Tick([&]()
				{
					m_sceneRenderer->Update(m_timer);
				});
		}

		bool Render();
		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		std::unique_ptr<CreativeRenderer> m_sceneRenderer;

		CDirectX::StepTimer m_timer;
	};
}