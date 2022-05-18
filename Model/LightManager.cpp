// This code manages all the lighting in creative. It utilizes four different types of light grids.
// these are essentially lighting data at a certain resolution. We try and keep this maximum for making it as pretty as possible.

#include "LightManager.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CCommandContext.h"
#include "CViewportCamera.h"
#include "BufferManager.h"
#include "CTAAEffects.h"

#include "CompiledShaders/FillLightGridCS_24.h"
#include "CompiledShaders/FillLightGridCS_32.h"

using namespace CMath;
using namespace CGraphics;

// Keep the Lighting data the SAME as the HLSL file, it will crash otherwise.
struct LightData
{
    float pos[3];
    float radiusSq;
    float color[3];

    uint32_t type;
    float coneDir[3];
    float coneAngles[2];

    float shadowTextureMatrix[16];
};

enum { kMinLightGridDim = 8 };

namespace Lighting
{
    CETInt LightGridDim("Application/Forward+/Light Grid Dim", 24, kMinLightGridDim, 32, 8 );

    CRootSignature m_FillLightRootSig;

    /// <summary>
    /// Lighting in games I like comes from Pipeline State Objects. These were
    /// the thing we were struggling with last week - But instead of being graphics these are compute ones, these dispatch compute shaders to run
    /// programs on the GPU
    /// </summary>
    /// 
    CComputePSO m_FillLightGridCS_24(L"Fill Light Grid 24 CS");
    CComputePSO m_FillLightGridCS_32(L"Fill Light Grid 32 CS");

    LightData m_LightData[MaxLights];

    CStructuredBuffer m_LightBuffer;
    CByteAddressBuffer m_LightGrid;

    CByteAddressBuffer m_LightGridBitMask;
    uint32_t m_FirstConeLight;
    uint32_t m_FirstConeShadowedLight;

    enum {shadowDim = 512};
    CRGBBuffer m_LightShadowArray;
    CCameraShadowBuffer m_LightShadowTempBuffer;
    CMatrix4 m_LightShadowMatrix[MaxLights];

    void InitializeResources(void);
    void CreateRandomLights(const CVector3 minBound, const CVector3 maxBound);
    void FillLightGrid(CGraphicsContext& gfxContext, const CViewportCamera& camera);
    void Shutdown(void);
}

void Lighting::InitializeResources( void )
{
    m_FillLightRootSig.Reset(3, 0);
    m_FillLightRootSig[0].InitAsConstantBuffer(0);
    m_FillLightRootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
    m_FillLightRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
    m_FillLightRootSig.Finalize(L"FillLightRS");

    m_FillLightGridCS_24.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_24.SetComputeShader(g_pFillLightGridCS_24, sizeof(g_pFillLightGridCS_24));
    m_FillLightGridCS_24.Finalize();

    m_FillLightGridCS_32.SetRootSignature(m_FillLightRootSig);
    m_FillLightGridCS_32.SetComputeShader(g_pFillLightGridCS_32, sizeof(g_pFillLightGridCS_32));
    m_FillLightGridCS_32.Finalize();

    // Assumes max resolution of 3840x2160
    uint32_t lightGridCells = CMath::DivideByMultiple(3840, kMinLightGridDim) * CMath::DivideByMultiple(2160, kMinLightGridDim);
    uint32_t lightGridSizeBytes = lightGridCells * (4 + MaxLights * 4);
    m_LightGrid.Create(L"m_LightGrid", lightGridSizeBytes, 1);

    uint32_t lightGridBitMaskSizeBytes = lightGridCells * 4 * 4;
    m_LightGridBitMask.Create(L"m_LightGridBitMask", lightGridBitMaskSizeBytes, 1);

    m_LightShadowArray.CreateArray(L"m_LightShadowArray", shadowDim, shadowDim, MaxLights, DXGI_FORMAT_R16_UNORM);
    m_LightShadowTempBuffer.Create(L"m_LightShadowTempBuffer", shadowDim, shadowDim);

    m_LightBuffer.Create(L"m_LightBuffer", MaxLights, sizeof(LightData));
}

//This creates random lights between two bound expressed as vectors
//These are alright not being passed by reference. But I'm trying to maintain const correctness
void Lighting::CreateRandomLights( const CVector3 minBound, const CVector3 maxBound )
{
    CVector3 posScale = maxBound - minBound;
    CVector3 posBias = minBound;

    // These are lambdas that calculate it for us, I'm really abusing the auto keyword. But it works
    // for my purposes.
    srand(12645);
    auto randUint = []() -> uint32_t
    {
        return rand(); // [0, RAND_MAX]
    };
    auto randFloat = [randUint]() -> float
    {
        return randUint() * (1.0f / RAND_MAX); 
    };
    auto randVecUniform = [randFloat]() -> CVector3
    {
        return CVector3(randFloat(), randFloat(), randFloat());
    };
    auto randGaussian = [randFloat]() -> float
    {
        // polar box-muller -- This is a mathematical form for lighting.
        static bool gaussianPair = true;
        static float y2;

        if (gaussianPair)
        {
            gaussianPair = false;

            float x1, x2, w;
            do
            {
                x1 = 2 * randFloat() - 1;
                x2 = 2 * randFloat() - 1;
                w = x1 * x1 + x2 * x2;
            } while (w >= 1);

            w = sqrtf(-2 * logf(w) / w);
            y2 = x2 * w;
            return x1 * w;
        }
        else
        {
            gaussianPair = true;
            return y2;
        }
    };
    auto randVecGaussian = [randGaussian]() -> CVector3
    {
        return Normalize(CVector3(randGaussian(), randGaussian(), randGaussian()));
    };

    //We are going to be using Pi a lot throughout this. We only need it to a few decimal places.
    const float pi = 3.14159265359f;

    for (uint32_t n = 0; n < MaxLights; n++)
    {
        CVector3 pos = randVecUniform() * posScale + posBias;

        float lightRadius = randFloat() * 800.0f + 200.0f;

        CVector3 color = randVecUniform();
        float colorScale = randFloat() * .3f + .3f;
        color = color * colorScale;

        uint32_t type;

        // force types to match 32-bit boundaries for the BIT_MASK_SORTED case
        if (n < 32 * 1)
            type = 0;
        else if (n < 32 * 3)
            type = 1;
        else
            type = 2;

        CVector3 coneDir = randVecGaussian();
        float coneInner = (randFloat() * .2f + .025f) * pi;
        float coneOuter = coneInner + randFloat() * .1f * pi;

        if (type == 1 || type == 2)
        {
            // emphasize cone style lights
            color = color * 5.0f;
        }

        //This uses the CViewPortCamera class, this is an upgraded camera built on top of the base camera.
        CMath::CViewportCamera shadowCamera;

        shadowCamera.SetEyeAtUp(pos, pos + coneDir, CVector3(0, 1, 0));
        shadowCamera.SetPerspectiveMatrix(coneOuter * 2, 1.0f, lightRadius * .05f, lightRadius * 1.0f);
        shadowCamera.Update();
        m_LightShadowMatrix[n] = shadowCamera.GetViewProjMatrix();

        //Recall from the lectures on computer graphics - EVERYTHING transform related is a matrix. 
        CMatrix4 shadowTextureMatrix = CMatrix4(CMAffineTransform(CMatrix3::MakeScale( 0.5f, -0.5f, 1.0f ), CVector3(0.5f, 0.5f, 0.0f))) * m_LightShadowMatrix[n];

        m_LightData[n].pos[0] = pos.GetX();
        m_LightData[n].pos[1] = pos.GetY();
        m_LightData[n].pos[2] = pos.GetZ();
        m_LightData[n].radiusSq = lightRadius * lightRadius;
        m_LightData[n].color[0] = color.GetX();
        m_LightData[n].color[1] = color.GetY();
        m_LightData[n].color[2] = color.GetZ();
        m_LightData[n].type = type;
        m_LightData[n].coneDir[0] = coneDir.GetX();
        m_LightData[n].coneDir[1] = coneDir.GetY();
        m_LightData[n].coneDir[2] = coneDir.GetZ();
        m_LightData[n].coneAngles[0] = 1.0f / (cosf(coneInner) - cosf(coneOuter));
        m_LightData[n].coneAngles[1] = cosf(coneOuter);

        //We use memcpy to copy the data into the lighting data.
        std::memcpy(m_LightData[n].shadowTextureMatrix, &shadowTextureMatrix, sizeof(shadowTextureMatrix));
    }
  
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 1)
        {
            m_FirstConeLight = n;
            break;
        }
    }
    for (uint32_t n = 0; n < MaxLights; n++)
    {
        if (m_LightData[n].type == 2)
        {
            m_FirstConeShadowedLight = n;
            break;
        }
    }

    //Finally we initalize the buffer for this. Very important it will not be read by the compute pipeline state otherwise.
    CCommandContext::InitializeBuffer(m_LightBuffer, m_LightData, MaxLights * sizeof(LightData));
}

void Lighting::Shutdown(void)
{
    m_LightBuffer.Destroy();
    m_LightGrid.Destroy();
    m_LightGridBitMask.Destroy();
    m_LightShadowArray.Destroy();
    m_LightShadowTempBuffer.Destroy();
}

void Lighting::FillLightGrid(CGraphicsContext& gfxContext, const CViewportCamera& camera)
{
    CComputeContext& Context = gfxContext.GetCComputeContext();

    Context.SetRootSignature(m_FillLightRootSig);

    switch ((int)LightGridDim)
    {
    case 24: Context.SetPipelineState(m_FillLightGridCS_24); break;
    case 32: Context.SetPipelineState(m_FillLightGridCS_32); break;
    default: ASSERT(false); break;
    }

    CRGBBuffer& LinearDepth = g_LinearDepth[ CTAAEffects::GetFrameIndexMod2() ];

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    Context.SetDynamicDescriptor(1, 0, m_LightBuffer.GetSRV());
    Context.SetDynamicDescriptor(1, 1, LinearDepth.GetSRV());
    Context.SetDynamicDescriptor(2, 0, m_LightGrid.GetUAV());
    Context.SetDynamicDescriptor(2, 1, m_LightGridBitMask.GetUAV());

    // we are always assuming 1920x1080 resolution as this is what my monitor this will render on will be.
    uint32_t tileCountX = CMath::DivideByMultiple(g_SceneColorBuffer.GetWidth(), LightGridDim);
    uint32_t tileCountY = CMath::DivideByMultiple(g_SceneColorBuffer.GetHeight(), LightGridDim);

    float FarClipDist = camera.GetFarClip();
    float NearClipDist = camera.GetNearClip();

    const float RcpZMagic = NearClipDist / (FarClipDist - NearClipDist);

    //The are the compute shader constants for the lighting.
    struct CSConstants
    {
        uint32_t ViewportWidth, ViewportHeight;
        float InvTileDim;
        float RcpZMagic;
        uint32_t TileCount;
        CMatrix4 ViewProjMatrix;
    } csConstants;

    //Please read previous comment for texture width/height
    csConstants.ViewportWidth = g_SceneColorBuffer.GetWidth();
    csConstants.ViewportHeight = g_SceneColorBuffer.GetHeight();
    csConstants.InvTileDim = 1.0f / LightGridDim;
    csConstants.RcpZMagic = RcpZMagic;
    csConstants.TileCount = tileCountX;
    csConstants.ViewProjMatrix = camera.GetViewProjMatrix();
    Context.SetDynamicConstantBufferView(0, sizeof(CSConstants), &csConstants);

    Context.Dispatch(tileCountX, tileCountY, 2);

    Context.TransitionResource(m_LightBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGrid, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Context.TransitionResource(m_LightGridBitMask, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
