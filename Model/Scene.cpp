

#include "Scene.h"
#include "Renderer.h"
#include "ConstantBuffers.h"

using namespace CMath;
using namespace CBaseRenderer;

void CScene::Destroy()
{
    m_BoundingSphere = CSphereCollider(kZero);
    m_DataBuffer.Destroy();
    m_MaterialConstants.Destroy();
    m_NumNodes = 0;
    m_NumMeshes = 0;
    m_MeshData = nullptr;
    m_SceneGraph = nullptr;
}

void CScene::Render(
    CMeshSerializer& sorter,
    const CGPUBuffer& meshConstants,
    const CMScaleAndTranslation sphereTransforms[],
    const CJoint* skeleton ) const
{
    // Pointer to current mesh
    const uint8_t* pMesh = m_MeshData.get();

    const CMFrustum& frustum = sorter.GetViewFrustum();
    const CMAffineTransform& viewMat = (const CMAffineTransform&)sorter.GetViewMatrix();

    for (uint32_t i = 0; i < m_NumMeshes; ++i)
    {
        const CStaticMesh& mesh = *(const CStaticMesh*)pMesh;

        const CMScaleAndTranslation& sphereXform = sphereTransforms[mesh.meshCBV];
        CSphereCollider sphereLS((const XMFLOAT4*)mesh.bounds);
        CSphereCollider sphereWS = sphereXform * sphereLS;
        CSphereCollider sphereVS = CSphereCollider(viewMat * sphereWS.GetCenter(), sphereWS.GetRadius());

        if (frustum.IntersectSphere(sphereVS))
        {
            float distance = -sphereVS.GetCenter().GetZ() - sphereVS.GetRadius();
            sorter.AddMesh(mesh, distance,
                meshConstants.GetGpuVirtualAddress() + sizeof(MeshConstants) * mesh.meshCBV,
                m_MaterialConstants.GetGpuVirtualAddress() + sizeof(MaterialConstants) * mesh.materialCBV,
                m_DataBuffer.GetGpuVirtualAddress(), skeleton);
        }

        pMesh += sizeof(CStaticMesh) + (mesh.numDraws - 1) * sizeof(CStaticMesh::Draw);
    }
}

void CSceneInstance::Render(CMeshSerializer& sorter) const
{
    if (m_Model != nullptr)
    {
        //const Frustum& frustum = sorter.GetWorldFrustum();
        m_Model->Render(sorter, m_MeshConstantsGPU, (const CMScaleAndTranslation*)m_BoundingSphereTransforms.get(),
            m_Skeleton.get());
    }
}

CSceneInstance::CSceneInstance( std::shared_ptr<const CScene> sourceModel )
    : m_Model(sourceModel), m_Locator(kIdentity)
{
    static_assert((_alignof(MeshConstants) & 255) == 0, "CBVs need 256 byte alignment");
    if (sourceModel == nullptr)
    {
        m_MeshConstantsCPU.Destroy();
        m_MeshConstantsGPU.Destroy();
        m_BoundingSphereTransforms = nullptr;
        m_AnimGraph = nullptr;
        m_AnimState.clear();
        m_Skeleton = nullptr;
    }
    else
    {
        m_MeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sourceModel->m_NumNodes * sizeof(MeshConstants));
        m_MeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", sourceModel->m_NumNodes, sizeof(MeshConstants));
        m_BoundingSphereTransforms.reset(new __m128[sourceModel->m_NumNodes]);
        m_Skeleton.reset(new CJoint[sourceModel->m_NumJoints]);

        if (sourceModel->m_NumAnimations > 0)
        {
            m_AnimGraph.reset(new CGraphNode[sourceModel->m_NumNodes]);
            std::memcpy(m_AnimGraph.get(), sourceModel->m_SceneGraph.get(), sourceModel->m_NumNodes * sizeof(CGraphNode));
            m_AnimState.resize(sourceModel->m_NumAnimations);
        }
        else
        {
            m_AnimGraph = nullptr;
            m_AnimState.clear();
        }
    }
}

CSceneInstance::CSceneInstance( const CSceneInstance& modelInstance )
    : CSceneInstance(modelInstance.m_Model)
{
}

CSceneInstance& CSceneInstance::operator=( std::shared_ptr<const CScene> sourceModel )
{
    m_Model = sourceModel;
    m_Locator = CMUniformTransform(kIdentity);
    if (sourceModel == nullptr)
    {
        m_MeshConstantsCPU.Destroy();
        m_MeshConstantsGPU.Destroy();
        m_BoundingSphereTransforms = nullptr;
        m_AnimGraph = nullptr;
        m_AnimState.clear();
        m_Skeleton = nullptr;
    }
    else
    {
        m_MeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sourceModel->m_NumNodes * sizeof(MeshConstants));
        m_MeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", sourceModel->m_NumNodes, sizeof(MeshConstants));
        m_BoundingSphereTransforms.reset(new __m128[sourceModel->m_NumNodes]);
        m_Skeleton.reset(new CJoint[sourceModel->m_NumJoints]);

        if (sourceModel->m_NumAnimations > 0)
        {
            m_AnimGraph.reset(new CGraphNode[sourceModel->m_NumNodes]);
            std::memcpy(m_AnimGraph.get(), sourceModel->m_SceneGraph.get(), sourceModel->m_NumNodes * sizeof(CGraphNode));
            m_AnimState.resize(sourceModel->m_NumAnimations);
        }
        else
        {
            m_AnimGraph = nullptr;
            m_AnimState.clear();
        }
    }
    return *this;
}

void CSceneInstance::Update(CGraphicsContext& gfxContext, float deltaTime)
{
    if (m_Model == nullptr)
        return;

    static const size_t kMaxStackDepth = 32;

    size_t stackIdx = 0;
    CMatrix4 matrixStack[kMaxStackDepth];
    CMatrix4 ParentMatrix = CMatrix4((CMAffineTransform)m_Locator);

    CMScaleAndTranslation* boundingSphereTransforms = (CMScaleAndTranslation*)m_BoundingSphereTransforms.get();
    MeshConstants* cb = (MeshConstants*)m_MeshConstantsCPU.Map();

    if (m_AnimGraph)
    {
        UpdateAnimations(deltaTime);

        for (uint32_t i = 0; i < m_Model->m_NumNodes; ++i)
        {
            CGraphNode& node = m_AnimGraph[i];

            // Regenerate the 3x3 matrix if it has scale or rotation
            if (node.staleMatrix)
            {
                node.staleMatrix = false;
                node.xform.Set3x3(CMatrix3(node.rotation) * CMatrix3::MakeScale(node.scale));
            }
        }
    }

    const CGraphNode* sceneGraph = m_AnimGraph ? m_AnimGraph.get() : m_Model->m_SceneGraph.get();

    // Traverse the scene graph in depth first order.  This is the same as linear order
    // for how the nodes are stored in memory.  Uses a matrix stack instead of recursion.
    for (const CGraphNode* Node = sceneGraph; ; ++Node)
    {
        CMatrix4 xform = Node->xform;
        if (!Node->skeletonRoot)
            xform = ParentMatrix * xform;

        // Concatenate the transform with the parent's matrix and update the matrix list
        {
            // Scoped so that I don't forget that I'm pointing to write-combined memory and
            // should not read from it.
            MeshConstants& cbv = cb[Node->matrixIdx];
            cbv.World = xform;
            cbv.WorldIT = InverseTranspose(xform.Get3x3());

            CScalar scaleXSqr = LengthSquare((CVector3)xform.GetX());
            CScalar scaleYSqr = LengthSquare((CVector3)xform.GetY());
            CScalar scaleZSqr = LengthSquare((CVector3)xform.GetZ());
            CScalar sphereScale = Sqrt(Max(Max(scaleXSqr, scaleYSqr), scaleZSqr));
            boundingSphereTransforms[Node->matrixIdx] = CMScaleAndTranslation((CVector3)xform.GetW(), sphereScale);
        }

        // If the next node will be a descendent, replace the parent matrix with our new matrix
        if (Node->hasChildren)
        {
            // ...but if we have siblings, make sure to backup our current parent matrix on the stack
            if (Node->hasSibling)
            {
                ASSERT(stackIdx < kMaxStackDepth, "Overflowed the matrix stack");
                matrixStack[stackIdx++] = ParentMatrix;
            }
            ParentMatrix = xform;
        }
        else if (!Node->hasSibling)
        {
            // There are no more siblings.  If the stack is empty, we are done.  Otherwise, pop
            // a matrix off the stack and continue.
            if (stackIdx == 0)
                break;

            ParentMatrix = matrixStack[--stackIdx];
        }
    }

    // Update skeletal joints
    for (uint32_t i = 0; i < m_Model->m_NumJoints; ++i)
    {
        CJoint& joint = m_Skeleton[i];
        joint.posXform = cb[m_Model->m_JointIndices[i]].World * m_Model->m_JointIBMs[i];
        joint.nrmXform = InverseTranspose(joint.posXform.Get3x3());
    }

    m_MeshConstantsCPU.Unmap();

    gfxContext.TransitionResource(m_MeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
    gfxContext.GetCommandList()->CopyBufferRegion(m_MeshConstantsGPU.GetResource(), 0, m_MeshConstantsCPU.GetResource(), 0, m_MeshConstantsCPU.GetBufferSize());
    gfxContext.TransitionResource(m_MeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void CSceneInstance::Resize( float newRadius )
{
    if (m_Model == nullptr)
        return;

    m_Locator.SetScale(newRadius / m_Model->m_BoundingSphere.GetRadius());
}

CVector3 CSceneInstance::GetCenter() const
{
    if (m_Model == nullptr)
        return CVector3(kOrigin);

    return m_Locator * m_Model->m_BoundingSphere.GetCenter();
}

CScalar CSceneInstance::GetRadius() const
{
    if (m_Model == nullptr)
        return CScalar(kZero);

    return m_Locator.GetScale() * m_Model->m_BoundingSphere.GetRadius();
}

CMath::CSphereCollider CSceneInstance::GetBoundingSphere() const
{
    if (m_Model == nullptr)
        return CSphereCollider(kZero);

    return m_Locator * m_Model->m_BoundingSphere;
}

CMath::OrientedBox CSceneInstance::GetBoundingBox() const
{
    if (m_Model == nullptr)
        return CAxisAlignedBox(CVector3(kZero), CVector3(kZero));

    return m_Locator * m_Model->m_BoundingBox;
}
