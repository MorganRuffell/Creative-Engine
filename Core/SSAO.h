

#pragma once

namespace CMath { class CViewportCamera;  }
class CGraphicsContext;
class CComputeContext;
class CETBoolean;

namespace SSAO
{
    void Initialize( void );
    void Shutdown( void );
    void Render(CGraphicsContext& Context, const float* ProjMat, float NearClipDist, float FarClipDist );
    void Render(CGraphicsContext& Context, const CMath::CViewportCamera& camera );
    void LinearizeZ(CComputeContext& Context, const CMath::CViewportCamera& camera, uint32_t FrameIndex);

    extern CETBoolean Enable;
    extern CETBoolean DebugDraw;
    extern CETBoolean AsyncCompute;
    extern CETBoolean ComputeLinearZ;
}
