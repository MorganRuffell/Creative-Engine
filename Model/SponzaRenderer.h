
#pragma once
#include "pch.h"
#include <d3d12.h>

class CGraphicsContext;
class CShadowCamera;
class CMesh;
class ExponentialCETFloat;

using namespace CGraphics;

namespace CMath
{
    class CViewportCamera;
    class CVector3;
}

namespace CrytekSponza
{
    void Startup( CMath::CViewportCamera& camera, CGraphicsContext& gfxContext);
    void Cleanup( void );

    void RenderScene(
        CGraphicsContext& gfxContext,
        const CMath::CViewportCamera& camera,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissor,
        bool skipDiffusePass,
        bool skipShadowMap);

    const CMesh& GetModel();

    extern CMath::CVector3          m_SunDirection;
    extern CShadowCamera            m_SunShadow;
    extern ExponentialCETFloat                   m_AmbientIntensity;
    extern ExponentialCETFloat                   m_SunLightIntensity;
}
