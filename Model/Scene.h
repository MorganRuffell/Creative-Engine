

#pragma once

#include "../Core/CGPUBuffer.h"
#include "../Core/VectorMath.h"
#include "../Core/CViewportCamera.h"
#include "../Core/CCommandContext.h"
#include "../Core/CGraphicsUploadBuffer.h"
#include "../Core/CTextureManager.h"
#include "../Core/Math/CBoundingBox.h"
#include "../Core/Math/CSphereCollider.h"
#include "CAnimationCurve.h"
#include "CAnimationSet.h"
#include "CAnimationState.h"
#include "CStaticMesh.h"
#include "CJoint.h"
#include <cstdint>

namespace CBaseRenderer
{
    class CMeshSerializer;
}

//
// To request a PSO index, provide flags that describe the kind of PSO
// you need.  If one has not yet been created, it will be created.
//
namespace PSOFlags
{
    enum : uint16_t
    { 
        kHasPosition    = 0x001,  // Required
        kHasNormal      = 0x002,  // Required
        kHasTangent     = 0x004,
        kHasUV0         = 0x008,  // Required (for now)
        kHasUV1         = 0x010,
        kAlphaBlend     = 0x020,
        kAlphaTest      = 0x040,
        kTwoSided       = 0x080,
        kHasSkin        = 0x100,  // Implies having indices and weights
    };
}

class CScene
{
public:

    ~CScene() { Destroy(); }

    void Render(CBaseRenderer::CMeshSerializer& sorter,
        const CGPUBuffer& meshConstants,
        const CMath::CMScaleAndTranslation sphereTransforms[],
        const CJoint* skeleton) const;

    CMath::CSphereCollider m_BoundingSphere; // Object-space bounding sphere
    CMath::CAxisAlignedBox m_BoundingBox;

public:

    CByteAddressBuffer m_DataBuffer;
    CByteAddressBuffer m_MaterialConstants;

public:

    uint32_t m_NumNodes;
    uint32_t m_NumMeshes;
    uint32_t m_NumAnimations;
    uint32_t m_NumJoints;

public:

    uint32_t m_NumberOfDynamicMeshes;
    uint32_t m_NumberOfStaticMeshes;
    uint32_t m_NumberOfSkinnedMeshes;

public:
    std::unique_ptr<uint8_t[]> m_MeshData;
    std::unique_ptr<CGraphNode[]> m_SceneGraph;
    std::vector<CTextureHandle> textures;
    std::unique_ptr<uint8_t[]> m_KeyFrameData;

public:
    std::unique_ptr<CAnimationCurve[]>      m_CurveData;
    std::unique_ptr<CAnimationSet[]>        m_Animations;
    std::unique_ptr<uint16_t[]>             m_JointIndices;
    std::unique_ptr<CMath::CMatrix4[]>      m_JointIBMs;

protected:
    void Destroy();
};

class CSceneInstance
{
public:
    CSceneInstance() {}
    ~CSceneInstance() {
        m_MeshConstantsCPU.Destroy();
        m_MeshConstantsGPU.Destroy();
    }
    CSceneInstance( std::shared_ptr<const CScene> sourceModel );
    CSceneInstance( const CSceneInstance& modelInstance );

    CSceneInstance& operator=( std::shared_ptr<const CScene> sourceModel );

    bool IsNull(void) const { return m_Model == nullptr; }

    void Update(CGraphicsContext& gfxContext, float deltaTime);
    void Render(CBaseRenderer::CMeshSerializer& sorter) const;

    void Resize(float newRadius);

    CMath::CVector3 GetCenter() const;
    CMath::CScalar GetRadius() const;
    CMath::CSphereCollider GetBoundingSphere() const;
    CMath::OrientedBox GetBoundingBox() const;

    size_t GetNumAnimations(void) const { return m_AnimState.size(); }
    void PlayAnimation(uint32_t animIdx, bool loop);
    void PauseAnimation(uint32_t animIdx);
    void ResetAnimation(uint32_t animIdx);
    void StopAnimation(uint32_t animIdx);
    void UpdateAnimations(float deltaTime);
    void LoopAllAnimations(void);

private:
    std::shared_ptr<const CScene> m_Model;
    CGraphicsUploadBuffer m_MeshConstantsCPU;
    CByteAddressBuffer m_MeshConstantsGPU;
    std::unique_ptr<__m128[]> m_BoundingSphereTransforms;
    CMath::CMUniformTransform m_Locator;

    std::unique_ptr<CGraphNode[]> m_AnimGraph;   // A copy of the scene graph when instancing animation
    std::vector<CAnimationState> m_AnimState;    // Per-animation (not per-curve)
    std::unique_ptr<CJoint[]> m_Skeleton;
};
