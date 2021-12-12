#include "pch.h"
#include "CreativeGraphicsEngine.h"

#include <ppltasks.h>

using namespace CRE;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new CRE::CreativeGraphicsEngine();
}

CRE::CreativeGraphicsEngine::CreativeGraphicsEngine() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void CRE::CreativeGraphicsEngine::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &CreativeGraphicsEngine::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &CreativeGraphicsEngine::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &CreativeGraphicsEngine::OnResuming);
}

void CRE::CreativeGraphicsEngine::SetWindow(CoreWindow^ window)
{
	window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &CreativeGraphicsEngine::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &CreativeGraphicsEngine::OnVisibilityChanged);

	window->Closed +=
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &CreativeGraphicsEngine::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &CreativeGraphicsEngine::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &CreativeGraphicsEngine::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &CreativeGraphicsEngine::OnDisplayContentsInvalidated);
}

void CRE::CreativeGraphicsEngine::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<CreativeGraphicsEngineMain>(new CreativeGraphicsEngineMain());
	}
}

void CRE::CreativeGraphicsEngine::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			auto commandQueue = GetDeviceResources()->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				m_main->Update();
			}
			PIXEndEvent(commandQueue);

			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (m_main->Render())
				{
					GetDeviceResources()->Present();
				}
			}
			PIXEndEvent(commandQueue);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// class is torn down while the app is in the foreground.
void CRE::CreativeGraphicsEngine::Uninitialize()
{
}

// Application lifecycle event handlers.
void CRE::CreativeGraphicsEngine::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void CRE::CreativeGraphicsEngine::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
		{
			m_main->OnSuspending();
			deferral->Complete();
		});
}

void CRE::CreativeGraphicsEngine::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	m_main->OnResuming();
}

// Window event handlers.

void CRE::CreativeGraphicsEngine::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	m_main->OnWindowSizeChanged();
}

void CRE::CreativeGraphicsEngine::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void CRE::CreativeGraphicsEngine::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}


void CRE::CreativeGraphicsEngine::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	GetDeviceResources()->SetDpi(sender->LogicalDpi);
	m_main->OnWindowSizeChanged();
}

void CRE::CreativeGraphicsEngine::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->OnWindowSizeChanged();
}

void CreativeGraphicsEngine::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	GetDeviceResources()->ValidateDevice();
}

std::shared_ptr<CDirectX::CDirectX12Core> CreativeGraphicsEngine::GetDeviceResources()
{
	if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	{
		m_deviceResources = nullptr;
		m_main->OnDeviceRemoved();

#if defined(_DEBUG)
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	if (m_deviceResources == nullptr)
	{
		m_deviceResources = std::make_shared<CDirectX::CDirectX12Core>();
		m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		m_main->CreateRenderers(m_deviceResources);
	}
	return m_deviceResources;
}
