#include "pch.h"
#include "Display.h"
#include "CDirectX12Core.h"
#include "BufferManager.h"
#include "CInternalWindowsTime.h"
#include "CCommandContext.h"
#include "RootSignature.h"
#include "ImageScaling.h"
#include "CTAAEffects.h"
#include "CViewportCamera.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


///Includes for ImGUI
#include "../ImGUI/imgui.h"
#include "../ImGUI/imgui_impl_win32.h"
#include "../ImGUI/imgui_impl_dx12.h"
#include "../ImGUI/ImGuizmo.h"
#include "L2DFileDialog.h"


#pragma comment(lib, "dxgi.lib") 

// This macro determines whether to detect if there is an HDR display and enable HDR10 output.
// Currently, with HDR display enabled, the pixel magnfication functionality is broken.
#define CONDITIONALLY_ENABLE_HDR_OUTPUT 1

namespace CECore { extern HWND CreativeBaseWindow; }

#include "CompiledShaders/ScreenQuadPresentVS.h"
#include "CompiledShaders/BufferCopyPS.h"
#include "CompiledShaders/PresentSDRPS.h"
#include "CompiledShaders/CompositeSDRPS.h"
#include "CompiledShaders/ScaleAndCompositeSDRPS.h"
#include "CompiledShaders/ScaleAndCompositeHDRPS.h"
#include "CompiledShaders/MagnifyPixelsPS.h"

#define SWAP_CHAIN_BUFFER_COUNT 3

DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

using namespace CMath;
using namespace ImageScaling;
using namespace CGraphics;

bool show_demo_window = true;

namespace CGraphics
{
	double s_FrameTime = 0.0f;
	int64_t s_FrameStartTick = 0;

	CETBoolean s_EnableVSync("Timing/VSync", true);
	CETBoolean s_LimitTo30Hz("Timing/Limit To 30Hz", false);
	CETBoolean s_DropRandomFrames("Timing/Drop Random Frames", false);
}

namespace CGraphics
{
	CRGBBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];

	IDXGISwapChain1* s_SwapChain1 = nullptr;

	IDXGISwapChain4* s_SwapChain = nullptr;

	void PreparePresentSDR();
	void PreparePresentHDR();
	void CompositeOverlays(CGraphicsContext& Context);

	enum eResolution { k720p, k900p, k1080p, k1440p, k1800p, k2160p };
	enum eEQAAQuality { kEQAA1x1, kEQAA1x8, kEQAA1x16 };

	const uint32_t kNumPredefinedResolutions = 6;

	const char* ResolutionLabels[] = { "1280x720", "1600x900", "1920x1080", "2560x1440", "3200x1800", "3840x2160" };
	CETStaticEnum NativeResolution("Graphics/Display/Native Resolution", k1080p, kNumPredefinedResolutions, ResolutionLabels);
#ifdef _HIGH_PERFORMANCE
	// This can set the window size to common dimensions.  It's also possible for the window to take on other dimensions
	// through resizing or going full-screen.
	CETStaticEnum DisplayResolution("Graphics/Display/Display Resolution", k1080p, kNumPredefinedResolutions, ResolutionLabels);
#endif

	bool g_bEnableHDROutput = false;
	CETFloat g_HDRPaperWhite("Graphics/Display/Paper White (nits)", 200.0f, 100.0f, 500.0f, 50.0f);
	CETFloat g_MaxDisplayLuminance("Graphics/Display/Peak Brightness (nits)", 1000.0f, 500.0f, 10000.0f, 100.0f);
	const char* HDRModeLabels[] = { "HDR", "SDR", "Side-by-Side" };
	CETStaticEnum HDRDebugMode("Graphics/Display/HDR Debug Mode", 0, 3, HDRModeLabels);

	uint32_t g_NativeWidth = 0;
	uint32_t g_NativeHeight = 0;
	uint32_t g_DisplayWidth = 1920;
	uint32_t g_DisplayHeight = 1080;
	CRGBBuffer g_PreDisplayBuffer;

	void ResolutionToUINT(eResolution res, uint32_t& width, uint32_t& height)
	{
		switch (res)
		{
		default:
		case k720p:
			width = 1280;
			height = 720;
			break;
		case k900p:
			width = 1600;
			height = 900;
			break;
		case k1080p:
			width = 1920;
			height = 1080;
			break;
		case k1440p:
			width = 2560;
			height = 1440;
			break;
		case k1800p:
			width = 3200;
			height = 1800;
			break;
		case k2160p:
			width = 3840;
			height = 2160;
			break;
		}
	}
	void SetNativeResolution(void)
	{
		uint32_t NativeWidth, NativeHeight;

		ResolutionToUINT(eResolution((int)NativeResolution), NativeWidth, NativeHeight);

		if (g_NativeWidth == NativeWidth && g_NativeHeight == NativeHeight)
			return;
		DEBUGPRINT("Changing native resolution to %ux%u", NativeWidth, NativeHeight);

		g_NativeWidth = NativeWidth;
		g_NativeHeight = NativeHeight;

		g_CommandManager.IdleGPU();

		InitializeRenderingBuffers(NativeWidth, NativeHeight);
	}
	void SetDisplayResolution(void)
	{
#ifdef _HIGH_PERFORMANCE
		static int SelectedDisplayRes = DisplayResolution;
		if (SelectedDisplayRes == DisplayResolution)
			return;

		SelectedDisplayRes = DisplayResolution;
		ResolutionToUINT((eResolution)SelectedDisplayRes, g_DisplayWidth, g_DisplayHeight);
		DEBUGPRINT("Changing display resolution to %ux%u", g_DisplayWidth, g_DisplayHeight);

		g_CommandManager.IdleGPU();

		Display::Resize(g_DisplayWidth, g_DisplayHeight);

		SetWindowPos(CECore::CreativeBaseWindow, 0, 0, 0, g_DisplayWidth, g_DisplayHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
#endif
	}

	//Root Signature that manages all of the Pipeline state objects
	CRootSignature s_PresentRS;

	//The Graphics Pipelinestate objects developed for the ImGUI implementation
	CGraphicsPSO s_BlendUIPSO(L"Core: BlendUI");
	CGraphicsPSO s_BlendUIHDRPSO(L"Core: BlendUIHDR");

	CGraphicsPSO PresentSDRPS(L"Core: PresentSDR");
	CGraphicsPSO PresentHDRPS(L"Core: PresentHDR");
	CGraphicsPSO CompositeSDRPS(L"Core: CompositeSDR");
	CGraphicsPSO ScaleAndCompositeSDRPS(L"Core: ScaleAndCompositeSDR");
	CGraphicsPSO CompositeHDRPS(L"Core: CompositeHDR");
	CGraphicsPSO ScaleAndCompositeHDRPS(L"Core: ScaleAndCompositeHDR");
	CGraphicsPSO MagnifyPixelsPS(L"Core: MagnifyPixels");

	const char* FilterLabels[] = { "Bilinear", "Sharpening", "Bicubic", "Lanczos" };
	CETStaticEnum UpsampleFilter("Graphics/Display/Scaling Filter", kSharpening, kFilterCount, FilterLabels);

	enum DebugZoomLevel { kDebugZoomOff, kDebugZoom2x, kDebugZoom4x, kDebugZoom8x, kDebugZoom16x, kDebugZoomCount };
	const char* DebugZoomLabels[] = { "Off", "2x Zoom", "4x Zoom", "8x Zoom", "16x Zoom" };
	CETStaticEnum DebugZoom("Graphics/Display/Magnify Pixels", kDebugZoomOff, kDebugZoomCount, DebugZoomLabels);
}

void Display::Resize(uint32_t width, uint32_t height)
{
	g_CommandManager.IdleGPU();

	g_DisplayWidth = width;
	g_DisplayHeight = height;

	DEBUGPRINT("Changing display resolution to %ux%u", width, height);

	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", width, height, SwapChainFormat);

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		g_DisplayPlane[i].Destroy();

	ASSERT(s_SwapChain1 != nullptr);
	ASSERT_SUCCEEDED(s_SwapChain1->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, width, height, SwapChainFormat, 0));

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		//Using the latest version, small upgrade but allows for better future proofing!
		ComPtr<ID3D12Resource2> DisplayPlane;
		ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
	}

	g_CurrentBuffer = 0;

	g_CommandManager.IdleGPU();

	ResizeDisplayDependentBuffers(g_NativeWidth, g_NativeHeight);
}

// Initialize the DirectX resources required to run.
void Display::Initialize(HWND window)
{
	ASSERT(s_SwapChain1 == nullptr, "Graphics has already been initialized");

	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	ASSERT_SUCCEEDED(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = g_DisplayWidth;
	swapChainDesc.Height = g_DisplayHeight;
	swapChainDesc.Format = SwapChainFormat;
	swapChainDesc.Stereo = false;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;
	fsSwapChainDesc.RefreshRate.Denominator = 1;
	fsSwapChainDesc.RefreshRate.Numerator = 60;
	fsSwapChainDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	fsSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;

	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForHwnd(
		g_CommandManager.GetCommandQueue(),
		CECore::CreativeBaseWindow,
		&swapChainDesc,
		&fsSwapChainDesc,
		nullptr,
		&s_SwapChain1));

#if CONDITIONALLY_ENABLE_HDR_OUTPUT
	{
		s_SwapChain = (IDXGISwapChain4*)s_SwapChain1;
		ComPtr<IDXGIOutput> output;
		ComPtr<IDXGIOutput6> output6;
		DXGI_OUTPUT_DESC1 outputDesc;

		outputDesc.AttachedToDesktop = TRUE;
		outputDesc.Monitor = Display::GetPrimaryMonitorHandle(window);
		outputDesc.ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

		UINT colorSpaceSupport;

		// Query support for ST.2084 on the display and set the color space accordingly
		if (SUCCEEDED(s_SwapChain->GetContainingOutput(&output)) && SUCCEEDED(output.As(&output6)) &&
			SUCCEEDED(output6->GetDesc1(&outputDesc)) && outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 &&
			SUCCEEDED(s_SwapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colorSpaceSupport)) &&
			(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) &&
			SUCCEEDED(s_SwapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
		{
			g_bEnableHDROutput = true;
		}
	}
#endif 

	//Iterate through each display plan getting the upgraded swapchain that can handle HDR
	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D12Resource2> DisplayPlane;
		ASSERT_SUCCEEDED(s_SwapChain1->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", DisplayPlane.Detach());
	}

	s_PresentRS.Reset(4, 2);

	s_PresentRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	s_PresentRS[1].InitAsConstants(0, 6, D3D12_SHADER_VISIBILITY_ALL);
	s_PresentRS[2].InitAsBufferSRV(2, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);

	s_PresentRS.InitStaticSampler(0, SamplerLinearClampDesc);
	s_PresentRS.InitStaticSampler(1, SamplerPointClampDesc);
	s_PresentRS.Finalize(L"Present");

	// Initialize PSOs
	s_BlendUIPSO.SetRootSignature(s_PresentRS);
	s_BlendUIPSO.SetRasterizerState(RasterizerTwoSided);
	s_BlendUIPSO.SetBlendState(BlendPreMultiplied);
	s_BlendUIPSO.SetDepthStencilState(DepthStateDisabled);
	s_BlendUIPSO.SetSampleMask(0xFFFFFFFF);
	s_BlendUIPSO.SetInputLayout(0, nullptr);
	s_BlendUIPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	s_BlendUIPSO.SetVertexShader(g_pScreenQuadPresentVS, sizeof(g_pScreenQuadPresentVS));
	s_BlendUIPSO.SetPixelShader(g_pBufferCopyPS, sizeof(g_pBufferCopyPS));
	s_BlendUIPSO.SetRenderTargetFormat(SwapChainFormat, DXGI_FORMAT_UNKNOWN);
	s_BlendUIPSO.Finalize();


#define CreatePSO( ObjName, ShaderByteCode ) \
    ObjName = s_BlendUIPSO; \
    ObjName.SetBlendState( BlendDisable ); \
    ObjName.SetPixelShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

	CreatePSO(PresentSDRPS, g_pPresentSDRPS);
	CreatePSO(CompositeSDRPS, g_pCompositeSDRPS);
	CreatePSO(ScaleAndCompositeSDRPS, g_pScaleAndCompositeSDRPS);
	CreatePSO(ScaleAndCompositeHDRPS, g_pScaleAndCompositeHDRPS);
	CreatePSO(MagnifyPixelsPS, g_pMagnifyPixelsPS);

	PresentHDRPS = PresentSDRPS;
	PresentHDRPS.SetPixelShader(g_pPresentSDRPS, sizeof(g_pPresentSDRPS));
	DXGI_FORMAT SwapChainFormats[2] = { DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM };
	PresentHDRPS.SetRenderTargetFormats(2, SwapChainFormats, DXGI_FORMAT_UNKNOWN);
	PresentHDRPS.Finalize();

#undef CreatePSO

	SetNativeResolution();

	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", g_DisplayWidth, g_DisplayHeight, SwapChainFormat);
	ImageScaling::Initialize(g_PreDisplayBuffer.GetFormat());
	InitGUI(window);
}

HMONITOR Display::GetPrimaryMonitorHandle(HWND window)
{
	auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY);

	if (!monitor)
		throw new std::exception;

	return monitor;
}

void Display::Shutdown(void)
{
	s_SwapChain1->SetFullscreenState(FALSE, nullptr);
	s_SwapChain1->Release();

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
		g_DisplayPlane[i].Destroy();

	g_PreDisplayBuffer.Destroy();
}

void Display::InitGUI(const HWND& hwnd)
{
	const std::lock_guard<std::recursive_mutex> imguilock(g_imguiMutex);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& inputOutput = ImGui::GetIO(); (void)inputOutput;
	inputOutput.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	inputOutput.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

    inputOutput.Fonts->AddFontFromFileTTF("..\\Resources\\fonts\\Roboto-Regular.ttf", 14);    

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 4;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (g_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_SRVDescHeap)) != S_OK)
		throw new std::exception;


	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(g_Device, 3, DXGI_FORMAT_R10G10B10A2_UNORM, g_SRVDescHeap, g_SRVDescHeap->GetCPUDescriptorHandleForHeapStart(), g_SRVDescHeap->GetGPUDescriptorHandleForHeapStart());


	InitCreativeIcon();

}

//UI reacts, but it needs to be upgraded further -- 19/04
void Display::UpdateUI()
{
	const std::lock_guard<std::recursive_mutex> imguilock(g_imguiMutex);

    CGraphicsContext& GPUContext = CGraphicsContext::Begin(L"UIGraphicsContext");

	auto NewCommandList = GPUContext.GetCommandList();
    auto NewCommandAllocator = GPUContext.GetCommandAllocator();

	g_CommandManager.CreateNewCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, &NewCommandList, &NewCommandAllocator);

	UINT l_backbufferindex = s_SwapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = g_DisplayPlane[l_backbufferindex].GetResource();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    NewCommandList->Close();
	NewCommandList->Reset(NewCommandAllocator, nullptr);

	NewCommandList->ResourceBarrier(1, &barrier);


	ImVec4 clear_colour = ImVec4(0.0f, 0.0f, 0.00f, 0.00f);
	const float clear_color_with_alpha[4] = { clear_colour.x * clear_colour.w, clear_colour.y * clear_colour.w, clear_colour.z * clear_colour.w, clear_colour.w };

	NewCommandList->OMSetRenderTargets(1, &g_DisplayPlane[l_backbufferindex].GetRTV(), FALSE, NULL);
	NewCommandList->SetDescriptorHeaps(1, &g_SRVDescHeap);


	Display::DrawGUI();
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), NewCommandList);


	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	NewCommandList->ResourceBarrier(1, &barrier);

    uint64_t FenceValue =  g_CommandManager.GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT).ExecuteCommandList(NewCommandList);
    g_CommandManager.GetGraphicsQueue().DiscardAllocator(FenceValue, NewCommandAllocator);

	GPUContext.TransitionResource(g_DisplayPlane[l_backbufferindex], D3D12_RESOURCE_STATE_PRESENT, true);
    GPUContext.Finish();

	s_SwapChain->Present(1, 0);
}

void Display::DrawGUI()
{
	bool ShowDebug = true;
	ImVec2 DetailsXYSize(600.0f, 1500.0f);
	ImVec2 DetailsXYPosition(2050, 0);

	ImVec2 FileExplorerSize(2050.0, 500.0f);
	ImVec2 FileExplorerPosition(0, 1000.0f);

	ImGui::StyleColorsDark();



	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(11.0f, 11.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(7.0f, 10.0f));

	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 18.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(14.0f, 18.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(22.0f, 18.0f));

		ImGui::Image((ImTextureID)UILogoGPU_CBVSRVUAV_HANDLE.ptr, ImVec2((float)IconWidth, (float)IconHeight));

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();


		if (ImGui::BeginMenu("File"))
		{	

			if (ImGui::MenuItem("New", "Ctrl+N")) {
				ImGui::Begin("Example");
				ImGui::End();
			}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {}
			if (ImGui::BeginMenu("Open Recent"))
			{
				if (ImGui::BeginMenu("More.."))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {}
			if (ImGui::MenuItem("Save As..")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Open Visual Studio 2022"))
			{
				//We can use CreateProcesstoStartVS https://stackoverflow.com/questions/42531/how-do-i-call-createprocess-in-c-to-launch-a-windows-executable
				//check to see if VS is running if it is swap the users focus!
				ImGui::EndMenu();
			}
			ImGui::Separator();

			if (ImGui::BeginMenu("Compile Project"))
			{
				if (ImGui::MenuItem("Test Compile", "Ctrl + Shift + B")) {
					ImGui::EndMenu();

				}

				if (ImGui::MenuItem("ReCompile", "")) {
					ImGui::EndMenu();

				}

				ImGui::Separator();

				if (ImGui::MenuItem("Final Compile", "Ctrl + B + F")) {

				}

				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Compile Settings"))
			{
				if (ImGui::BeginMenu("Graphics Quality", true)) {

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));

					ImGui::RadioButton("Raytraced Creative", false);
					ImGui::RadioButton("Fine Creative", true);
					ImGui::RadioButton("Creative", false);
					ImGui::RadioButton("Med", false);
					ImGui::RadioButton("Low", false);

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();


					ImGui::EndMenu();

				}

				if (ImGui::BeginMenu("Rendering API", true)) {

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));


					ImGui::RadioButton("DirectX12 - Raytracing", false);
					ImGui::RadioButton("DirectX12 ", true);
					ImGui::RadioButton("DirectX11", false);
					ImGui::RadioButton("Vulkan", false);

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();


					ImGui::EndMenu();

				}

				if (ImGui::BeginMenu("Platform", false)) {
					ImGui::RadioButton("Windows", true);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Lighting Quality", true)) {

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));


					ImGui::RadioButton("Aurora Lighting", true);
					ImGui::RadioButton("Lustre Lighting", false);
					ImGui::RadioButton("Ultra Lighting", false);
					ImGui::RadioButton("High Lighting", false);
					ImGui::RadioButton("Med", false);
					ImGui::RadioButton("Low", false);

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();


					ImGui::EndMenu();

				}

				ImGui::Separator();

				if (ImGui::BeginMenu("Build Classes", true)) {

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));


					ImGui::RadioButton("Build C++ Classes", false);
					ImGui::RadioButton("Build Rust Structs", false);

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();


					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Build Graphics Pipelines", true)) {

					ImGui::Text("DirectX 12 Only!");

					ImGui::Separator();

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));


					ImGui::RadioButton("Conduit Quality", true);
					ImGui::RadioButton("High Quality", false);
					ImGui::RadioButton("Medium Quality", false);
					ImGui::RadioButton("Standard Pipeline Quality", false);

					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();


					ImGui::EndMenu();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Test Compile", "Ctrl + Shift + B")) {
					ImGui::EndMenu();

				}

				ImGui::EndMenu();

			}

			ImGui::Separator();
			if (ImGui::BeginMenu("Import"))
			{
				if (ImGui::BeginMenu("Texture"))
				{
					if (ImGui::BeginMenu(".PNG"))
					{
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu(".TARGA"))
					{
						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("3D Object"))
				{
					if (ImGui::BeginMenu(".FBX"))
					{
						if (ImGui::BeginMenu("Static Mesh"))
						{
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Composite Mesh"))
						{
							ImGui::EndMenu();

						}

						if (ImGui::BeginMenu("Dynamic Mesh"))
						{
							ImGui::EndMenu();

						}

						ImGui::EndMenu();

					}

					if (ImGui::BeginMenu(".Obj", false))
					{
						if (ImGui::BeginMenu("Static Mesh"))
						{
							ImGui::EndMenu();

						}

						if (ImGui::BeginMenu("Composite Mesh"))
						{
							ImGui::EndMenu();

						}

						if (ImGui::BeginMenu("Dynamic Mesh"))
						{
							ImGui::EndMenu();

						}

						ImGui::EndMenu();

					}

					ImGui::EndMenu();

				}
				if (ImGui::BeginMenu("Sound", false))
				{
					if (ImGui::BeginMenu(".MP4"))
					{
						ImGui::EndMenu();
					}

					ImGui::EndMenu();

				}


				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Create New", true)) {

				if (ImGui::BeginMenu("Code Structures", true)) {

					if (ImGui::BeginMenu("Rust"))
					{

						if (ImGui::MenuItem("Rust Structure")) {
							ImGui::EndMenu();
						}

						if (ImGui::MenuItem("Rust Trait")) {
							ImGui::EndMenu();
						}

						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("C++"))
					{
						if (ImGui::MenuItem("C++ Class")) {
							ImGui::EndMenu();
						}

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}


			ImGui::Separator();

			if (ImGui::MenuItem("Exit", "Alt+F4")) {

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();

			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Preferences"))
		{
			if (ImGui::BeginMenu("Engine Settings")) {

				if (ImGui::BeginMenu("Graphics"))
				{
					if (ImGui::BeginMenu("Renderer"))
					{
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));

						if (ImGui::RadioButton("DirectX 12 - Raytracing", false)) {}

						if (ImGui::RadioButton("DirectX 12", true)) {}

						if (ImGui::RadioButton("DirectX 11", false)) {}

						if (ImGui::RadioButton("OpenGL", false)) {}

                        if (ImGui::RadioButton("Vulkan", false)) {}


						ImGui::PopStyleVar();
						ImGui::PopStyleVar();
						ImGui::PopStyleVar();


						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Features"))
					{
						if (ImGui::RadioButton("SSAO (Screen Space Ambient Occlusion)", false)) {}
						if (ImGui::RadioButton("VSync", true)) {}
						if (ImGui::RadioButton("Raytracing", false)) {}
						if (ImGui::RadioButton("TAA (Temporaral Anti Aliasing)", true)) {}

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Appearance"))
				{
					if (ImGui::BeginMenu("Theme"))
					{
						if (ImGui::MenuItem("Dark Theme", false)) { ImGui::StyleColorsDark; }
						if (ImGui::MenuItem("Classic Theme", false)) { ImGui::StyleColorsClassic; }
						if (ImGui::MenuItem("Light Theme", false)) { ImGui::StyleColorsLight; }


						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Style Editor"))
					{
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));

						ImGui::ShowStyleEditor();

						ImGui::PopStyleVar();
						ImGui::PopStyleVar();
						ImGui::PopStyleVar();

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Misc"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Project Settings")) {}

			ImGui::Separator();



			if (ImGui::BeginMenu("Graphics Quality", true)) {

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));

				ImGui::RadioButton("Raytraced Creative", false);
				ImGui::RadioButton("Fine Creative", true);
				ImGui::RadioButton("Creative", false);
				ImGui::RadioButton("Med", false);
				ImGui::RadioButton("Low", false);

				ImGui::PopStyleVar();
				ImGui::PopStyleVar();
				ImGui::PopStyleVar();

				ImGui::EndMenu();

			}

			if (ImGui::BeginMenu("Lighting Quality", true)) {

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0, 4.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 4.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 4.0f));

				ImGui::RadioButton("Aurora", true);
				ImGui::RadioButton("Lustre", false);
				ImGui::RadioButton("Ultra", false);
				ImGui::RadioButton("High", false);
				ImGui::RadioButton("Med", false);
				ImGui::RadioButton("Low", false);

				ImGui::PopStyleVar();
				ImGui::PopStyleVar();
				ImGui::PopStyleVar();

				ImGui::EndMenu();

			}



			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{


			if (ImGui::MenuItem("Engine Console", "CTRL + E")) {

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Layout"))
			{

				ImGui::EndMenu();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Cake")) {
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("General Help", "F1")) {

				ImGui::EndMenu();

			}
			if (ImGui::MenuItem("About Creative Engine"))
			{
				ImGui::EndMenu();

			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}



	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();


	ImGui::Begin("Details", &ShowDebug, ImGuiWindowFlags_NoDecoration);
	{
		ImGui::SetWindowSize(DetailsXYSize, ImGuiCond_Always);
		ImGui::SetWindowPos(DetailsXYPosition, ImGuiCond_Always);

		ImGui::BeginChild("Scene Outliner", { 500.0f, 210.0 }, true, ImGuiWindowFlags_None);
		{

			ImGui::SetWindowFontScale(1.2f);
			ImGui::TextWrapped("Scene Outliner");
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Separator();

			bool Active = true;
			bool InActive = false;

			if (ImGui::BeginTable("Outliner", 4))
			{
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 25.0f);

				ImGui::TableNextColumn(); ImGui::Text("Active");
				ImGui::TableNextColumn(); ImGui::Text("Label");
				ImGui::TableNextColumn(); ImGui::Text("SM6 Trace");
				ImGui::TableNextColumn(); ImGui::Text("Type");

				ImGui::TableNextRow(0, 10.0f);

				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface1");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface2");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface3");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface4");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface5");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);



				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface6");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface7");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Polysurface8");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains1");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains2");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains3");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CLight");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains1");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 20.0f);

				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains2");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CStaticMesh");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("Curtains3");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CLight");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("DynamicLights1");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CLight");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("DynamicLights2");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CLight");

				ImGui::TableNextRow(0, 10.0f);


				ImGui::TableNextColumn(); ImGui::Checkbox("", &Active);
				ImGui::TableNextColumn(); ImGui::Text("DynamicLights3");
				ImGui::TableNextColumn(); ImGui::Text("");
				ImGui::TableNextColumn(); ImGui::Text("CLight");

				ImGui::EndTable();
			}


			ImGui::EndChild();
		}


		ImGui::BeginChild("Inside Details", { 500, 1190 }, true, ImGuiWindowFlags_NoMove);
		{

			float BasePosition[3] = { 0.0f, 0.0f, 0.0f };

			ImGui::SetWindowFontScale(1.2f);
			ImGui::TextWrapped("Details");
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Separator();


			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 6.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Position		  ");

				ImGui::SameLine();
				ImGui::StyleColorsX();
				ImGui::DragFloat("X", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();


				ImGui::SameLine();
				ImGui::StyleColorsY();
				ImGui::DragFloat("Y", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::SameLine();
				ImGui::StyleColorsZ();
				ImGui::DragFloat("Z", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::Text("Rotation		  ");
				ImGui::SameLine();
				ImGui::StyleColorsX();
				ImGui::DragFloat("X", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::SameLine();
				ImGui::StyleColorsY();
				ImGui::DragFloat("Y", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::SameLine();
				ImGui::StyleColorsZ();
				ImGui::DragFloat("Z", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();


				ImGui::Text("Scale			   ");
				ImGui::SameLine();
				ImGui::StyleColorsX();
				ImGui::DragFloat("X", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::SameLine();
				ImGui::StyleColorsY();
				ImGui::DragFloat("Y", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::SameLine();
				ImGui::StyleColorsZ();
				ImGui::DragFloat("Z", BasePosition, 0.1, -360, 360);
				ImGui::StyleColorsDark();

				ImGui::PopStyleVar();
				ImGui::PopStyleVar();

			}


			ImGui::Spacing();


			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(60.0f, 1.0f));
			ImGui::Spacing();
			if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PopStyleVar();
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(60.0f, 1.1f));

				if (ImGui::BeginTable("Lemonade", 4))
				{
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);

					ImGui::TableNextColumn(); ImGui::Text("Mesh Name");
					ImGui::TableNextColumn(); ImGui::Text("Data Type");
					ImGui::TableNextColumn(); ImGui::Text("HasMaterial");
					ImGui::TableNextColumn(); ImGui::Text("Poly Count");


					ImGui::TableNextRow(0, 21.0f);

					ImGui::TableNextColumn(); ImGui::Text("Curtain");
					ImGui::TableNextColumn(); ImGui::Text(".obj | Static");

					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");
					ImGui::TableNextColumn(); ImGui::TextWrapped("3000");

					ImGui::TableNextRow(0, 21.0f);

					ImGui::TableNextColumn(); ImGui::Text("CurtainPole");
					ImGui::TableNextColumn(); ImGui::Text(".obj | Static");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");
					ImGui::TableNextColumn(); ImGui::TextWrapped("90");

					ImGui::TableNextRow(0, 21.0f);


					ImGui::TableNextColumn(); ImGui::Text("Drape");
					ImGui::TableNextColumn(); ImGui::Text(".obj | Composite");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");
					ImGui::TableNextColumn(); ImGui::TextWrapped("1500");

					ImGui::TableNextRow(0, 21.0f);

					ImGui::TableNextColumn(); ImGui::Text("Drape");
					ImGui::TableNextColumn(); ImGui::Text(".obj | Composite");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");
					ImGui::TableNextColumn(); ImGui::TextWrapped("1500");

					ImGui::EndTable();
				}


			}
			ImGui::PopStyleVar();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100.0f, 3.0f));

			if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PopStyleVar();

				float Coloura[4] = { 1.0f, 0.1f, 0.1f,1.0f };
				float Colourb[4] = { 0.0f, 1.0f, 1.0f,1.0f };
				float Colour[4] = { 1.0f, 1.0f, 1.0f,1.0f };

				ImGui::PushItemWidth(190.0f);

				ImGui::ColorPicker4("M_Curtain Albedo", Colourb, ImGuiColorEditFlags_PickerHueWheel);
				ImGui::ColorPicker4("M_Drapes Albedo", Coloura, ImGuiColorEditFlags_PickerHueWheel);
				ImGui::ColorPicker4("m_CurtianPole Albedo", Colour, ImGuiColorEditFlags_PickerHueWheel);


				ImGui::PopItemWidth();

				if (ImGui::BeginTable("Greengrass", 3))
				{
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 21.0);
					ImGui::TableNextColumn(); ImGui::Text("Material Name");
					ImGui::TableNextColumn(); ImGui::Text("Creative Domain");
					ImGui::TableNextColumn(); ImGui::Text("UseNormals");


					ImGui::TableNextRow(0, 18.0f);
					ImGui::TableNextColumn(); ImGui::Text("M_Curtain");
					ImGui::TableNextColumn(); ImGui::Text("PBR_LitRegular");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");

					ImGui::TableNextRow(0, 18.0f);
					ImGui::TableNextColumn(); ImGui::Text("M_CurtainPoleend");
					ImGui::TableNextColumn(); ImGui::Text("PBR_LitMetallic");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");


					ImGui::TableNextRow(0, 18.0f);
					ImGui::TableNextColumn(); ImGui::Text("M_Pole");
					ImGui::TableNextColumn(); ImGui::Text("PBR_LitMetallic");
					ImGui::TableNextColumn(); ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "YES");

					ImGui::TableNextRow(0, 18.0f);
					ImGui::TableNextColumn(); ImGui::Text("M_Drapes");
					ImGui::TableNextColumn(); ImGui::Text("PBR_LitMetallic");
					ImGui::TableNextColumn(); ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "NO");


					ImGui::EndTable();

				}

			}


			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100.0f, 8.0f));


			if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PopStyleVar();
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100.0f, 3.0f));

				float PlaceholderPhysicsValue = 5.12;
				bool SimulatePhysics = true;


				if (ImGui::BeginTable("CloudControl", 2))
				{
					ImGui::TableNextRow(0, 20.0f);


					ImGui::TableNextColumn(); ImGui::Text("Simulate Physics");
					ImGui::TableNextColumn(); ImGui::Checkbox("", &SimulatePhysics);

					ImGui::TableNextRow(0, 20.0f);

					ImGui::TableNextColumn(); ImGui::Checkbox("Mass in (Kg)", &SimulatePhysics);
					ImGui::TableNextColumn(); ImGui::DragFloat("", &PlaceholderPhysicsValue, 0.1f, -100, 100);

					ImGui::TableNextRow(0, 20.0f);

					ImGui::TableNextColumn(); ImGui::Text("Linear Dampening");
					ImGui::TableNextColumn(); ImGui::DragFloat("", &PlaceholderPhysicsValue, 0.1f, -100, 100);

					ImGui::TableNextRow(0, 20.0f);

					ImGui::TableNextColumn(); ImGui::Text("Angular Dampening");
					ImGui::TableNextColumn(); ImGui::DragFloat("", &PlaceholderPhysicsValue, 0.1f, -100, 100);

					ImGui::TableNextRow(0, 20.0f);

					ImGui::TableNextColumn(); ImGui::Text("Enable Newton(C) Physics");
					ImGui::TableNextColumn(); ImGui::Checkbox("", &SimulatePhysics);

					ImGui::TableNextRow(0, 20.0f);

					ImGui::TableNextColumn(); ImGui::Text("Inertia Tensor Scale");

					float COMO[3] = { 0.0f,0.0f,0.0f };
					ImGui::TableNextColumn(); ImGui::DragFloat3("", COMO, 0.01, -360, 360);



					ImGui::EndTable();
				}





			}

			ImGui::PopStyleVar();

			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.4f);

			ImGui::TextWrapped("Engine Info");
			ImGui::SetWindowFontScale(1.0f);


			ImGui::TextWrapped("Creative Engine V0.1");
			ImGui::TextWrapped("Rendering API: DirectX12 Ultimate");
			ImGui::TextWrapped("UI System: Dear ImGUI");
			ImGui::TextWrapped("Developer: Morgan Ruffell");
			ImGui::Spacing();

			std::string DisplayTimeBetweenFrames = std::to_string(s_FrameTime);
			const char* CharRepresentationA = DisplayTimeBetweenFrames.c_str();
			ImGui::TextWrapped("Time Between Frames:");
			ImGui::Text(CharRepresentationA);

			std::string FrameRate = std::to_string(GetFrameRate());
			const char* FrameRateRepresentation = FrameRate.c_str();
			ImGui::TextWrapped("FPS:");
			ImGui::Text(FrameRateRepresentation);





			ImGui::EndChild();
		}

		

	}

	ImGui::Begin("File Browser", false, ImGuiWindowFlags_NoDecoration || ImGuiWindowFlags_NoBackground);
	{
		ImGui::SetWindowSize(FileExplorerSize, ImGuiCond_Always);
		ImGui::SetWindowPos(FileExplorerPosition, ImGuiCond_Always);

		ImGui::SameLine();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7.0f, 16.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(7.0f, 16.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7.0f, 16.0f));


		ImGui::SetWindowFontScale(1.0f);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();



		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 6.0f));


		ImGui::SameLine();
		ImGui::SetWindowFontScale(1.2f);
		ImGui::Button("Import New", { 100.0f, 30.0f });


		ImGui::SameLine();
		ImGui::Button("Save All", { 90.0f, 30.0f });

		ImGui::SameLine();
		ImGui::Button("Refresh", { 89.0f, 30.0f });

		ImGui::SetWindowFontScale(1.0f);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		ImGui::BeginChild("Oranges", { 2030.0f, 352.0f }, true, ImGuiWindowFlags_None);
		{
			ImGui::SetWindowFontScale(1.2f);

			if (ImGui::BeginMenu("Content Browser", true))
			{
				ImGui::SetWindowFontScale(1.0f);

				if (ImGui::MenuItem("Import New "))
				{
				}
				if (ImGui::MenuItem("Save All"))
				{

				}
				if (ImGui::MenuItem("Export"))
				{

				}


				ImGui::EndMenu();
			}

			ImGui::Separator();

            static char* file_dialog_buffer = nullptr;
            static char path1[500] = "..\\Project";

            ImGui::SetNextItemWidth(380);
            ImGui::InputText("##path1", path1, sizeof(path1));
            ImGui::SameLine();
            file_dialog_buffer = path1;
            FileDialog::file_dialog_open = true;
            FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;
            
            if (FileDialog::file_dialog_open) {
                FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, file_dialog_buffer, sizeof(file_dialog_buffer), FileDialog::file_dialog_open_type);
            }


			ImGui::EndChild();
		}

		ImGui::TextWrapped("Creative Engine V0.1 - (C) 2021 Ruffell Interactive ltd ");

		ImGui::SameLine();

		ImVec4 Green = {0.0f, 1.0f, 0.1, 1.0f};
		ImGui::TextColored(Green, "DirectX 12 Ultimate - Agility SDK October 2021");

		ImGui::End();

		ImGui::End();

	}
}

static bool Display::LoadUITextureFromFile(const char* filename, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, ID3D12Resource** out_tex_resource, int* out_width, int* out_height)
{
	assert(g_Device != nullptr);

	int img_width, img_height = 0;
	unsigned char* imageData = stbi_load(filename, &img_width, &img_height, NULL, 4);

	assert(imageData != nullptr);

	// Create texture resource
	D3D12_HEAP_PROPERTIES props;
	memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
	props.Type = D3D12_HEAP_TYPE_DEFAULT;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = img_width;
	desc.Height = img_height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* UITexture = NULL;
	ID3D12Resource* UITextureUploadBuffer = NULL;

	HRESULT res = g_Device->CreateCommittedResource1(&props, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, NULL, IID_PPV_ARGS(&UITexture));

	UINT uploadPitch = (img_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
	UINT uploadSize = img_height * uploadPitch;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = uploadSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	props.Type = D3D12_HEAP_TYPE_UPLOAD;
	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	HRESULT UploadRes = g_Device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&UITextureUploadBuffer));

	void* bIsMapped = NULL;

	D3D12_RANGE range = { 0, uploadSize };
	UploadRes = UITextureUploadBuffer->Map(0, &range, &bIsMapped);

	for (int y = 0; y < img_height; y++)
		memcpy((void*)((uintptr_t)bIsMapped + y * uploadPitch), imageData + y * img_width * 4, img_width * 4);

	UITextureUploadBuffer->Unmap(0, &range);

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = UITextureUploadBuffer;
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srcLocation.PlacedFootprint.Footprint.Width = img_width;
	srcLocation.PlacedFootprint.Footprint.Height = img_height;
	srcLocation.PlacedFootprint.Footprint.Depth = 1;
	srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = UITexture;
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = UITexture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	ID3D12Fence* t_fence = NULL;
	HRESULT fence_Result = g_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&t_fence));

	HANDLE t_FenceEventHandle = CreateEvent(0, 0, 0, 0);

	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.NodeMask = 1;


	//Rather than interfacing with the systems inside creative, just to initalize a simple UI Texture, we are going to use 
	//The raw versions of these without the creative support structures inside the context hierarchy. 
	//On important things like UI, and rendering scenes we DO NOT DO THIS.

	ID3D12CommandQueue* t_CommandQueue;
	ID3D12CommandAllocator* t_CommandAllocator;
	ID3D12GraphicsCommandList* t_GraphicsCommandList;

	g_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&t_CommandQueue));
	g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&t_CommandAllocator));
	g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, t_CommandAllocator, NULL, IID_PPV_ARGS(&t_GraphicsCommandList));

	t_GraphicsCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
	t_GraphicsCommandList->ResourceBarrier(1, &barrier);

	t_GraphicsCommandList->Close();

	t_CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&t_GraphicsCommandList);
	t_CommandQueue->Signal(t_fence, 1);


	t_fence->SetEventOnCompletion(1, t_FenceEventHandle);
	WaitForSingleObject(t_FenceEventHandle, INFINITE);

	t_GraphicsCommandList->Release();
	t_CommandAllocator->Release();
	t_CommandQueue->Release();
	CloseHandle(t_FenceEventHandle);

	t_fence->Release();
	UITextureUploadBuffer->Release();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	g_Device->CreateShaderResourceView(UITexture, &srvDesc, srv_cpu_handle);
	out_tex_resource = &UITexture;
	*out_width = img_width;
	*out_height = img_height;
	stbi_image_free(imageData);

	return true;
}

void Display::InitCreativeIcon()
{
	static_assert(sizeof(ImTextureID) >= sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "D3D12 CPU Descriptor handle is too large to fit and ImTextureID");

	ID3D12Resource* UITexture = NULL;

	UINT handle_Increment = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	int DescriptorIndex = 1;

	D3D12_CPU_DESCRIPTOR_HANDLE UILogoCPU_CBVSRVUAV_HANDLE = g_SRVDescHeap->GetCPUDescriptorHandleForHeapStart();
	UILogoCPU_CBVSRVUAV_HANDLE.ptr += (handle_Increment * DescriptorIndex);

	UILogoGPU_CBVSRVUAV_HANDLE = g_SRVDescHeap->GetGPUDescriptorHandleForHeapStart();
	UILogoGPU_CBVSRVUAV_HANDLE.ptr += (handle_Increment * DescriptorIndex);

	bool retrived = LoadUITextureFromFile("..\\Resources\\engineResources\\Creative Icon InEngine 512.png", UILogoCPU_CBVSRVUAV_HANDLE, &UITexture, &IconWidth, &IconHeight);
	IconWidth /= 14;
	IconHeight /= 14;
}

void CGraphics::PreparePresentHDR(void)
{
	CGraphicsContext& Context = CGraphicsContext::Begin(L"Present");

	bool NeedsScaling = g_NativeWidth != g_DisplayWidth || g_NativeHeight != g_DisplayHeight;

	Context.SetRootSignature(s_PresentRS);
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSRV());

	CRGBBuffer& Dest = DebugZoom == kDebugZoomOff ? g_DisplayPlane[g_CurrentBuffer] : g_PreDisplayBuffer;

	// On Windows, prefer scaling and compositing in one step via pixel shader
	Context.SetRootSignature(s_PresentRS);
	Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (DebugZoom == kDebugZoomOff)
	{
		Context.SetDynamicDescriptor(0, 1, g_OverlayBuffer.GetSRV());
		Context.SetPipelineState(NeedsScaling ? ScaleAndCompositeHDRPS : CompositeHDRPS);
	}
	else
	{
		Context.SetDynamicDescriptor(0, 1, GetDefaultTexture(kBlackTransparent2D));
		Context.SetPipelineState(NeedsScaling ? ScaleAndCompositeHDRPS : PresentHDRPS);
	}
	Context.SetConstants(1, (float)g_HDRPaperWhite / 10000.0f, (float)g_MaxDisplayLuminance,
		0.7071f / g_NativeWidth, 0.7071f / g_NativeHeight);
	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
	Context.SetRenderTarget(Dest.GetRTV());
	Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
	Context.Draw(3);

	// Magnify without stretching
	if (DebugZoom != kDebugZoomOff)
	{
		Context.SetPipelineState(MagnifyPixelsPS);
		Context.TransitionResource(g_PreDisplayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(g_DisplayPlane[g_CurrentBuffer].GetRTV());
		Context.SetDynamicDescriptor(0, 0, g_PreDisplayBuffer.GetSRV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		Context.SetConstants(1, 1.0f / ((int)DebugZoom + 1.0f));
		Context.Draw(3);

		CompositeOverlays(Context);
	}

	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);


	// Close the final context to be executed before frame present.
	Context.Finish();
}

void CGraphics::CompositeOverlays(CGraphicsContext& Context)
{
	// Now blend (or write) the UI overlay
	Context.SetRootSignature(s_PresentRS);
	Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Context.SetDynamicDescriptor(0, 0, g_OverlayBuffer.GetSRV());
	Context.SetPipelineState(g_bEnableHDROutput ? s_BlendUIHDRPSO : s_BlendUIPSO);
	Context.SetConstants(1, (float)g_HDRPaperWhite / 10000.0f, (float)g_MaxDisplayLuminance);
	Context.Draw(3);
}

void CGraphics::PreparePresentSDR(void)
{
	CGraphicsContext& Context = CGraphicsContext::Begin(L"Present");

	Context.SetRootSignature(s_PresentRS);
	Context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// We're going to be reading these buffers to write to the swap chain buffer(s)
	Context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	Context.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSRV());

	bool NeedsScaling = g_NativeWidth != g_DisplayWidth || g_NativeHeight != g_DisplayHeight;

	// On Windows, prefer scaling and compositing in one step via pixel shader
	if (DebugZoom == kDebugZoomOff && (UpsampleFilter == kSharpening || !NeedsScaling))
	{
		Context.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		Context.SetDynamicDescriptor(0, 1, g_OverlayBuffer.GetSRV());
		Context.SetPipelineState(NeedsScaling ? ScaleAndCompositeSDRPS : CompositeSDRPS);
		Context.SetConstants(1, 0.7071f / g_NativeWidth, 0.7071f / g_NativeHeight);
		Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
		Context.SetRenderTarget(g_DisplayPlane[g_CurrentBuffer].GetRTV());
		Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
		Context.Draw(3);
	}
	else
	{
		CRGBBuffer& Dest = DebugZoom == kDebugZoomOff ? g_DisplayPlane[g_CurrentBuffer] : g_PreDisplayBuffer;

		// Scale or Copy
		if (NeedsScaling)
		{
			ImageScaling::Upscale(Context, Dest, g_SceneColorBuffer, EScalingType((int)UpsampleFilter));
		}
		else
		{
			Context.SetPipelineState(PresentSDRPS);
			Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_RENDER_TARGET);
			Context.SetRenderTarget(Dest.GetRTV());
			Context.SetViewportAndScissor(0, 0, g_NativeWidth, g_NativeHeight);
			Context.Draw(3);
		}

		// Magnify without stretching
		if (DebugZoom != kDebugZoomOff)
		{
			Context.SetPipelineState(MagnifyPixelsPS);
			Context.TransitionResource(g_PreDisplayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
			Context.SetRenderTarget(g_DisplayPlane[g_CurrentBuffer].GetRTV());
			Context.SetDynamicDescriptor(0, 0, g_PreDisplayBuffer.GetSRV());
			Context.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
			Context.SetConstants(1, 1.0f / ((int)DebugZoom + 1.0f));
			Context.Draw(3);
		}

		CompositeOverlays(Context);
	}


	Context.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);

	// Close the final context to be executed before frame present.
	Context.Finish();
}

void Display::Present(void)
{
	PreparePresentSDR();

	g_CurrentBuffer = (g_CurrentBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	int64_t CurrentTick = CInternalWindowsTime::GetCurrentTick();

	if (s_EnableVSync)
	{
		// With VSync enabled, the time step between frames becomes a multiple of 16.666 ms.  We need
		// to add logic to vary between 1 and 2 (or 3 fields).  This delta time also determines how
		// long the previous frame should be displayed (i.e. the present interval.)
		s_FrameTime = (s_LimitTo30Hz ? 2.0f : 1.0f) / 60.0f;
		if (s_DropRandomFrames)
		{
			if (std::rand() % 50 == 0)
				s_FrameTime += (1.0f / 60.0f);
		}
	}
	else
	{
		// When running free, keep the most recent total frame time as the time step for
		// the next frame simulation.  This is not super-accurate, but assuming a frame
		// time varies smoothly, it should be close enough.
		s_FrameTime = (float)CInternalWindowsTime::TimeBetweenTicks(s_FrameStartTick, CurrentTick);
	}

	s_FrameStartTick = CurrentTick;

	++s_FrameIndex;

	CTAAEffects::Update((uint32_t)s_FrameIndex);

	SetNativeResolution();
	SetDisplayResolution();


	Display::UpdateUI();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
}

uint64_t CGraphics::GetFrameCount(void)
{
	return s_FrameIndex;
}

float CGraphics::GetFrameTime(void)
{
	return s_FrameTime;
}

float CGraphics::GetFrameRate(void)
{
	return s_FrameTime == 0.0f ? 0.0f : 1.0f / s_FrameTime;
}
