#pragma once

#include "pch.h"
#include "CDynamicMesh.h"
#include "../Core/CGPUBuffer.h"
#include "../Core/VectorMath.h"
#include "../Core/CViewportCamera.h"
#include "../Core/CCommandContext.h"
#include "../Core/CGraphicsUploadBuffer.h"
#include "../Core/CTextureManager.h"
#include <cstdint>
#include <vector>


#include <d3d12.h>

class CGraphicsPSO;
class CRootSignature;
class CDescriptorHeap;
class CShadowCamera;
class CCameraShadowBuffer;
struct GlobalConstants;
class CStaticMesh;
struct CJoint;

namespace CBaseRenderer
{
    extern CETBoolean SeparateZPass;

    using namespace CMath;

    /// <summary>
    /// We are getting the Graphics Pipeline state objects from another class or file.
    /// </summary>
    extern std::vector<CGraphicsPSO> sm_PSOs;

    extern CRootSignature m_RootSig;

    extern CDescriptorHeap s_TextureHeap;
    extern CDescriptorHeap s_SamplerHeap;
    extern CDescriptorHandle m_CommonTextures;

    enum RootBindings
    {
        kMeshConstants,
        kMaterialConstants,
        kMaterialSRVs,
        kMaterialSamplers,
        kCommonSRVs,
        kCommonCBV,
        kSkinMatrices,

        kNumRootBindings
    };

    void Initialize(void);
    void Shutdown(void);

    uint8_t GetPSO(uint16_t psoFlags);

    void SetIBLTextures(CTextureHandle diffuseIBL, CTextureHandle specularIBL);
    void SetIBLBias(float LODBias);

    void UpdateGlobalDescriptors(void);

    void DrawSkybox( CGraphicsContext& gfxContext, const CViewportCamera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor );

    class CMeshSerializer
    {
    public:
		enum BatchType { kDefault, kShadows };
        enum DrawPass { kZPass, kOpaque, kTransparent, kNumPasses };

        CMeshSerializer(BatchType type)
		{
			m_BatchType = type;
			m_Camera = nullptr;
			m_Viewport = {};
			m_Scissor = {};
			m_NumRTVs = 0;
			m_DSV = nullptr;
			m_SortObjects.clear();
			m_SortKeys.clear();
			std::memset(m_PassCounts, 0, sizeof(m_PassCounts));
			m_CurrentPass = kZPass;
			m_CurrentDraw = 0;
		}

		void SetCamera( const CBaseCamera& camera ) { m_Camera = &camera; }
		void SetViewport( const D3D12_VIEWPORT& viewport ) { m_Viewport = viewport; }
		void SetScissor( const D3D12_RECT& scissor ) { m_Scissor = scissor; }
		void AddRenderTarget( CRGBBuffer& RTV )
		{ 
			ASSERT(m_NumRTVs < 8);
			m_RTV[m_NumRTVs++] = &RTV;
		}
		void SetDepthStencilTarget( CGPUDepthBuffer& DSV ) { m_DSV = &DSV; }

        const CMFrustum& GetWorldFrustum() const { return m_Camera->GetWorldSpaceFrustum(); }
        const CMFrustum& GetViewFrustum() const { return m_Camera->GetViewSpaceFrustum(); }
        const CMatrix4& GetViewMatrix() const { return m_Camera->GetViewMatrix(); }

        void AddMesh( const CStaticMesh& mesh, float distance,
            D3D12_GPU_VIRTUAL_ADDRESS meshCBV,
            D3D12_GPU_VIRTUAL_ADDRESS materialCBV,
            D3D12_GPU_VIRTUAL_ADDRESS bufferPtr,
            const CJoint* skeleton = nullptr);

        void Sort();

        void RenderMeshes(DrawPass pass, CGraphicsContext& context, GlobalConstants& globals);
        
    private:

        struct SortKey
        {
            union
            {
                uint64_t value;
                struct
                {
                    uint64_t objectIdx : 16;
                    uint64_t psoIdx : 12;
                    uint64_t key : 32;
                    uint64_t passID : 4;
                };
            };
        };

        struct SortObject
        {
            const CStaticMesh* mesh;
            const CSkinnedMesh* skinnedMeshes;
            const CJoint* skeleton;
            D3D12_GPU_VIRTUAL_ADDRESS meshCBV;
            D3D12_GPU_VIRTUAL_ADDRESS materialCBV;
            D3D12_GPU_VIRTUAL_ADDRESS bufferPtr;
        };

        std::vector<SortObject> m_SortObjects;
        std::vector<uint64_t> m_SortKeys;

		BatchType m_BatchType;
        uint32_t m_PassCounts[kNumPasses];
        DrawPass m_CurrentPass;
        uint32_t m_CurrentDraw;

		const CBaseCamera* m_Camera;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_Scissor;
		uint32_t m_NumRTVs;
		CRGBBuffer* m_RTV[8];
		CGPUDepthBuffer* m_DSV;
	};

} 