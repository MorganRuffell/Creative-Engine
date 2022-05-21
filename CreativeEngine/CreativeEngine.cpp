#include "EngineCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "CViewportCamera.h"
#include "CCommandContext.h"
#include "CTAAEffects.h"
#include "CMotionBlurSettings.h"
#include "CDOFSettings.h"
#include "CPostEffects.h"
#include "SSAO.h"
#include "FXAA.h"
#include "OSTime.h"
#include "TextRenderer.h"
#include "EngineInput.h"
#include "SponzaRenderer.h"
#include "glTF.h"
#include "Renderer.h"
#include "Scene.h"
#include "ModelLoader.h"
#include "ShadowCamera.h"

#define SCENE_RENDERER

using namespace CECore;
using namespace CMath;
using namespace CGraphics;
using namespace std;

using CBaseRenderer::CMeshSerializer;

class CreativeEngine : public CECore::CreativeEngineApplication
{
public:

    CreativeEngine(void) {}

    virtual void Startup(HWND window) override;
    virtual void Cleanup(void) override;

    virtual void Update(float deltaT) override;
    virtual void RenderScene() override;

private:

    CViewportCamera m_Camera;
    unique_ptr<CViewportCameraController> m_CameraController;
    CDirectX12Core* m_GraphicsCore;

    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    CSceneInstance m_ModelInst;
    CShadowCamera m_SunShadowCamera;
};

CREATE_APPLICATION(CreativeEngine)

ExponentialCETFloat g_SunLightIntensity("Viewer/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
CETFloat g_SunOrientation("Viewer/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f);
CETFloat g_SunInclination("Viewer/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f);

void ChangeIBLSet(CreativeEngineType::ActionType);
void ChangeIBLBias(CreativeEngineType::ActionType);

CETDynmEnum g_IBLSet("Viewer/Lighting/Environment", ChangeIBLSet);
std::vector<std::pair<CTextureHandle, CTextureHandle>> g_IBLTextures;
CETFloat g_IBLBias("Viewer/Lighting/Gloss Reduction", 2.0f, 0.0f, 10.0f, 1.0f, ChangeIBLBias);

void ChangeIBLSet(CreativeEngineType::ActionType)
{
    int setIdx = g_IBLSet - 1;
    if (setIdx < 0)
    {
        CBaseRenderer::SetIBLTextures(nullptr, nullptr);
    }
    else
    {
        auto texturePair = g_IBLTextures[setIdx];
        CBaseRenderer::SetIBLTextures(texturePair.first, texturePair.second);
    }
}

void ChangeIBLBias(CreativeEngineType::ActionType)
{
    CBaseRenderer::SetIBLBias(g_IBLBias);
}

#include <direct.h> // for _getcwd() to check data root path
#include <ImGUI/imgui.h>

void LoadIBLTextures()
{
    char CWD[256];
    _getcwd(CWD, 256);

    CUtility::Printf("Loading IBL environment maps\n");

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(L"Textures/*_diffuseIBL.dds", &ffd);

    g_IBLSet.AddEnum(L"None");

    if (hFind != INVALID_HANDLE_VALUE) do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::wstring diffuseFile = ffd.cFileName;
        std::wstring baseFile = diffuseFile;
        baseFile.resize(baseFile.rfind(L"_diffuseIBL.dds"));
        std::wstring specularFile = baseFile + L"_specularIBL.dds";

        CTextureHandle diffuseTex = CTextureManager::LoadDDSFromFile(L"Textures/" + diffuseFile);
        if (diffuseTex.IsValid())
        {
            CTextureHandle specularTex = CTextureManager::LoadDDSFromFile(L"Textures/" + specularFile);
            if (specularTex.IsValid())
            {
                g_IBLSet.AddEnum(baseFile);
                g_IBLTextures.push_back(std::make_pair(diffuseTex, specularTex));
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    CUtility::Printf("Found %u IBL environment map sets\n", g_IBLTextures.size());

    if (g_IBLTextures.size() > 0)
        g_IBLSet.Increment();
}

void CreativeEngine::Startup(HWND window)
{
    CMotionBlurSettings::Enable = true;
    CTAAEffects::EnableTAA = true;
    FXAA::Enable = false;
    CGlobalPostEffects::EnableHDR = true;
    CGlobalPostEffects::EnableAdaptation = true;
    SSAO::Enable = true;

    CBaseRenderer::Initialize();

    LoadIBLTextures();

    std::wstring gltfFileName;

    CGraphicsContext& gfxContext = CGraphicsContext::Begin(L"INIT");
    m_GraphicsCore = new CDirectX12Core();

    m_Camera.SetMainWindow(window);

    bool forceRebuild = true;
    uint32_t rebuildValue;
    if (CommandLineArgs::GetInteger(L"rebuild", rebuildValue))
        forceRebuild = rebuildValue != 0;

    if (CommandLineArgs::GetString(L"model", gltfFileName) == false)
    {
#ifdef SCENE_RENDERER
        CrytekSponza::Startup(m_Camera, gfxContext);
#else
        m_ModelInst = CBaseRenderer::LoadModel(L"Sponza/PBR/sponza2.gltf", forceRebuild);
        m_ModelInst.Resize(100.0f * m_ModelInst.GetRadius());

        OrientedBox obb = m_ModelInst.GetBoundingBox();

        float modelRadius = Length(obb.GetDimensions()) * 0.5f;

        const CVector3 eye = obb.GetCenter() + CVector3(modelRadius * 0.5f, 0.0f, 0.0f);

        m_Camera.SetEyeAtUp(eye, CVector3(kZero), CVector3(kYUnitVector));
#endif
    }
    else
    {
        m_ModelInst = CBaseRenderer::LoadModel(gltfFileName, forceRebuild);
        m_ModelInst.LoopAllAnimations();
        m_ModelInst.Resize(10.0f);

        CMotionBlurSettings::Enable = false;
    }

    m_Camera.SetZRange(1.0f, 10000.0f);
    if (gltfFileName.size() == 0)
        m_CameraController.reset(new CAdvancedViewportCamera(m_Camera, CVector3(kYUnitVector)));
    else
        m_CameraController.reset(new CRotationalCamera(m_Camera, m_ModelInst.GetBoundingSphere(), CVector3(kYUnitVector)));
}

void CreativeEngine::Cleanup(void)
{
    m_ModelInst = nullptr;

    g_IBLTextures.clear();

#ifdef SCENE_RENDERER
    CrytekSponza::Cleanup();
#endif

    CBaseRenderer::Shutdown();
}

namespace CGraphics
{
    extern CETStaticEnum DebugZoom;
}

void CreativeEngine::Update(float deltaT)
{
    CScopeTimingBlock _prof(L"Update State");

    m_CameraController->Update(deltaT);

    CGraphicsContext& gfxContext = CGraphicsContext::Begin(L"Scene Update");

    m_ModelInst.Update(gfxContext, deltaT);


    CTAAEffects::GetJitterOffset(m_MainViewport.TopLeftX, m_MainViewport.TopLeftY);

    gfxContext.Finish();

    m_MainViewport.Width = (float)g_SceneColorBuffer.GetWidth();
    m_MainViewport.Height = (float)g_SceneColorBuffer.GetHeight();
    m_MainViewport.MinDepth = 0.0f;
    m_MainViewport.MaxDepth = 1.0f;

    m_MainScissor.left = 0;
    m_MainScissor.top = 0;
    m_MainScissor.right = (LONG)g_SceneColorBuffer.GetWidth();
    m_MainScissor.bottom = (LONG)g_SceneColorBuffer.GetHeight();

}

void CreativeEngine::RenderScene()
{

    CGraphicsContext& gfxContext = CGraphicsContext::Begin(L"Scene Render");

    uint32_t FrameIndex = CTAAEffects::GetFrameIndexMod2();
    const D3D12_VIEWPORT& viewport = m_MainViewport;
    const D3D12_RECT& scissor = m_MainScissor;


    if (m_ModelInst.IsNull())
    {
#ifdef SCENE_RENDERER
        CrytekSponza::RenderScene(gfxContext, m_Camera, viewport, scissor, false, false);
#endif
    }
    else
    {
        // Update global constants
        float costheta = cosf(g_SunOrientation);
        float sintheta = sinf(g_SunOrientation);
        float cosphi = cosf(g_SunInclination * 3.14159f * 0.5f);
        float sinphi = sinf(g_SunInclination * 3.14159f * 0.5f);

        CVector3 SunDirection = Normalize(CVector3(costheta * cosphi, sinphi, sintheta * cosphi));
        CVector3 ShadowBounds = CVector3(m_ModelInst.GetRadius());
        //m_SunShadowCamera.UpdateMatrix(-SunDirection, m_ModelInst.GetCenter(), ShadowBounds,
        m_SunShadowCamera.UpdateMatrix(-SunDirection, CVector3(0, -500.0f, 0), CVector3(5000, 3000, 3000),
            (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

        GlobalConstants globals;
        globals.ViewProjMatrix = m_Camera.GetViewProjMatrix();
        globals.SunShadowMatrix = m_SunShadowCamera.GetShadowMatrix();
        globals.CameraPos = m_Camera.GetPosition();
        globals.SunDirection = SunDirection;
        globals.SunIntensity = CVector3(CScalar(g_SunLightIntensity));

        // Begin rendering depth
        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
        gfxContext.ClearDepth(g_SceneDepthBuffer);

        CMeshSerializer sorter(CMeshSerializer::kDefault);
        sorter.SetCamera(m_Camera);
        sorter.SetViewport(viewport);
        sorter.SetScissor(scissor);
        sorter.SetDepthStencilTarget(g_SceneDepthBuffer);
        sorter.AddRenderTarget(g_SceneColorBuffer);

        m_ModelInst.Render(sorter);

        sorter.Sort();

        {
            CScopeTimingBlock _prof(L"Depth Pre-Pass", gfxContext);
            sorter.RenderMeshes(CMeshSerializer::kZPass, gfxContext, globals);
        }

        SSAO::Render(gfxContext, m_Camera);

        if (!SSAO::DebugDraw)
        {
            CScopeTimingBlock _outerprof(L"Main Render", gfxContext);

            {
                CScopeTimingBlock _prof(L"Sun Shadow Map", gfxContext);

                CMeshSerializer shadowSorter(CMeshSerializer::kShadows);
                shadowSorter.SetCamera(m_SunShadowCamera);
                shadowSorter.SetDepthStencilTarget(g_ShadowBuffer);

                m_ModelInst.Render(shadowSorter);

                shadowSorter.Sort();
                shadowSorter.RenderMeshes(CMeshSerializer::kZPass, gfxContext, globals);
            }

            gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
            gfxContext.ClearColor(g_SceneColorBuffer);

            {
                CScopeTimingBlock _prof(L"Render Color", gfxContext);

                gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                gfxContext.SetViewportAndScissor(viewport, scissor);

                sorter.RenderMeshes(CMeshSerializer::kOpaque, gfxContext, globals);
            }

            CBaseRenderer::DrawSkybox(gfxContext, m_Camera, viewport, scissor);

            sorter.RenderMeshes(CMeshSerializer::kTransparent, gfxContext, globals);
        }
    }

    CMotionBlurSettings::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);

    CTAAEffects::ResolveImage(gfxContext);

    // Until I work out how to couple these two, it's "either-or".
    if (CDOFSettings::Enable)
        CDOFSettings::Render(gfxContext, m_Camera.GetNearClip(), m_Camera.GetFarClip());
    else
        CMotionBlurSettings::RenderObjectBlur(gfxContext, g_VelocityBuffer);


    gfxContext.Finish();
}
