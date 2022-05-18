#pragma once

#include <cstdint>

class CStructuredBuffer;
class CByteAddressBuffer;
class CRGBBuffer;
class CCameraShadowBuffer;
class CGraphicsContext;
class CETInt;

namespace CMath
{
    class CVector3;
    class CMatrix4;
    class CViewportCamera;
}

namespace Lighting
{
    extern CETInt LightGridDim;

    enum { MaxLights = 128 };

    extern CStructuredBuffer m_LightBuffer;
    extern CByteAddressBuffer m_LightGrid;

    extern CByteAddressBuffer m_LightGridBitMask;
    extern std::uint32_t m_FirstConeLight;
    extern std::uint32_t m_FirstConeShadowedLight;

    extern CRGBBuffer m_LightShadowArray;
    extern CCameraShadowBuffer m_LightShadowTempBuffer;
    extern CMath::CMatrix4 m_LightShadowMatrix[MaxLights];

    void InitializeResources(void);
    void CreateRandomLights(const CMath::CVector3 minBound, const CMath::CVector3 maxBound);
    void FillLightGrid(CGraphicsContext& gfxContext, const CMath::CViewportCamera& camera);
    void Shutdown(void);
}
