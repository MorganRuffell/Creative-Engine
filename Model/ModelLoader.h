

#pragma once

#include "CStaticMesh.h"
#include "CDynamicMesh.h"
#include "CAnimationCurve.h"
#include "CAnimationSet.h"
#include "CAnimationState.h"
#include "ConstantBuffers.h"
#include "Scene.h"

#include "../Core/Math/CSphereCollider.h"
#include "../Core/Math/CBoundingBox.h"

#include <cstdint>
#include <vector>

namespace glTF { class Asset; struct CBaseMesh; }

#define CURRENT_MINI_FILE_VERSION 13

namespace CBaseRenderer
{
    using namespace CMath;

    // Unaligned mirror of MaterialConstants
    struct MaterialConstantData
    {
        float baseColorFactor[4]; // default=[1,1,1,1]
        float emissiveFactor[3]; // default=[0,0,0]
        float normalTextureScale; // default=1
        float metallicFactor; // default=1
        float roughnessFactor; // default=1
        uint32_t flags;
    };

    // Used at load time to construct descriptor tables
    struct MaterialTextureData
    {
        uint16_t stringIdx[kNumTextures];
        uint32_t addressModes;
    };

    // All of the information that needs to be written to a .mini data file
    struct ModelData
    {
        CSphereCollider m_BoundingSphere;
        CAxisAlignedBox m_BoundingBox;

        std::vector<cs_byte> m_GeometryData;
        std::vector<cs_byte> m_AnimationKeyFrameData;

        std::vector<CAnimationCurve> m_AnimationCurves;
        std::vector<CAnimationSet> m_Animations;

        std::vector<uint16_t> m_JointIndices;
        std::vector<CMatrix4> m_JointIBMs;

        std::vector<MaterialTextureData> m_MaterialTextures;
        std::vector<MaterialConstantData> m_MaterialConstants;

        std::vector<CStaticMesh*> m_Meshes;
        std::vector<CSkinnedMesh*> m_SkinnedMeshes;

        std::vector<CGraphNode> m_SceneGraph;
        std::vector<std::string> m_TextureNames;
        std::vector<uint8_t> m_TextureOptions;
    };

    struct CFileHeader
    {
        char     id[4];   // "MINI"
        uint32_t version; // CURRENT_MINI_FILE_VERSION
        uint32_t numNodes;
        uint32_t numMeshes;
        uint32_t numMaterials;
        uint32_t meshDataSize;
        uint32_t numTextures;
        uint32_t stringTableSize;
        uint32_t geometrySize;
        uint32_t keyFrameDataSize;      // Animation data
        uint32_t numAnimationCurves;
        uint32_t numAnimations;
        uint32_t numJoints;     // All joints for all skins
        float    boundingSphere[4];
        float    minPos[3];
        float    maxPos[3];
    };

    void CompileMesh(
        _Inout_ std::vector<CStaticMesh*>& meshList,
        _Inout_ std::vector<cs_byte>& bufferMemory,
        _In_ glTF::CBaseMesh& srcMesh,
        _In_ uint32_t matrixIdx,
        _Inout_ const CMatrix4& localToObject,
        _Inout_ CMath::CSphereCollider& boundingSphere,
        _Inout_ CMath::CAxisAlignedBox& boundingBox
    );

    bool BuildModel(_Inout_ ModelData& model, _Inout_ const glTF::Asset& asset, _In_ int sceneIdx = -1 );
    bool SaveModel(_Inout_ const std::wstring& filePath, _Inout_ const ModelData& model );
    
    std::shared_ptr<CScene> LoadModel(_Inout_ const std::wstring& filePath, _In_ bool forceRebuild = false );
}