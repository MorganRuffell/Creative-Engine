#pragma once
#include <cstdint>

#include "../Core/CGPUBuffer.h"
#include "../Core/VectorMath.h"
#include "../Core/CViewportCamera.h"
#include "../Core/CCommandContext.h"
#include "../Core/CGraphicsUploadBuffer.h"
#include "../Core/CTextureManager.h"
#include "../Core/Math/CBoundingBox.h"
#include "../Core/Math/CSphereCollider.h"

/// <summary>
/// CBaseMesh the parent class for all meshes in creative.
/// 
///     Do not draw this directly! 
///     Only designed for the the very core data about a drawn mesh data, what the renderer sees.
/// </summary>

struct Header
{
	uint32_t meshCount;
	uint32_t materialCount;
	uint32_t vertexDataByteSize;
	uint32_t indexDataByteSize;
	uint32_t vertexDataByteSizeDepth;
	CMath::CAxisAlignedBox boundingBox;
};

struct Attrib
{
	uint16_t offset; // byte offset from the start of the vertex
	uint16_t normalized; // if true, integer formats are interpreted as [-1, 1] or [0, 1]
	uint16_t components; // 1-4
	uint16_t format;
};

class CBaseMesh
{
public:
	enum
	{
		attrib_mask_0 = (1 << 0),
		attrib_mask_1 = (1 << 1),
		attrib_mask_2 = (1 << 2),
		attrib_mask_3 = (1 << 3),
		attrib_mask_4 = (1 << 4),
		attrib_mask_5 = (1 << 5),
		attrib_mask_6 = (1 << 6),
		attrib_mask_7 = (1 << 7),
		attrib_mask_8 = (1 << 8),
		attrib_mask_9 = (1 << 9),
		attrib_mask_10 = (1 << 10),
		attrib_mask_11 = (1 << 11),
		attrib_mask_12 = (1 << 12),
		attrib_mask_13 = (1 << 13),
		attrib_mask_14 = (1 << 14),
		attrib_mask_15 = (1 << 15),

		// friendly name aliases
		attrib_mask_position = attrib_mask_0,
		attrib_mask_texcoord0 = attrib_mask_1,
		attrib_mask_normal = attrib_mask_2,
		attrib_mask_tangent = attrib_mask_3,
		attrib_mask_bitangent = attrib_mask_4,
	};

	enum
	{
		attrib_0 = 0,
		attrib_1 = 1,
		attrib_2 = 2,
		attrib_3 = 3,
		attrib_4 = 4,
		attrib_5 = 5,
		attrib_6 = 6,
		attrib_7 = 7,
		attrib_8 = 8,
		attrib_9 = 9,
		attrib_10 = 10,
		attrib_11 = 11,
		attrib_12 = 12,
		attrib_13 = 13,
		attrib_14 = 14,
		attrib_15 = 15,

		// friendly name aliases
		attrib_position = attrib_0,
		attrib_texcoord0 = attrib_1,
		attrib_normal = attrib_2,
		attrib_tangent = attrib_3,
		attrib_bitangent = attrib_4,

		maxAttribs = 16
	};


public:

	CMath::CAxisAlignedBox boundingBox;

	uint32_t materialIndex;

	uint32_t attribsEnabled;
	uint32_t attribsEnabledDepth;
	uint32_t vertexStride;
	uint32_t vertexStrideDepth;
	Attrib attrib[maxAttribs];
	Attrib attribDepth[maxAttribs];

	uint32_t vertexDataByteOffset;
	uint32_t vertexCount;
	uint32_t indexDataByteOffset;
	uint32_t indexCount;

	uint32_t vertexDataByteOffsetDepth;
	uint32_t vertexCountDepth;

public:

    float    bounds[4];     // A bounding sphere

public:

    uint32_t vbOffset;      // BufferLocation - Buffer.GpuVirtualAddress
    uint32_t vbSize;        // SizeInBytes
    uint32_t vbDepthOffset; // BufferLocation - Buffer.GpuVirtualAddress
    uint32_t vbDepthSize;   // SizeInBytes

public:

    uint32_t ibOffset;      // BufferLocation - Buffer.GpuVirtualAddress
    uint32_t ibSize;        // SizeInBytes
    uint8_t  vbStride;      // StrideInBytes
    uint8_t  ibFormat;      // DXGI_FORMAT

public:

    uint16_t meshCBV;       // Index of mesh constant buffer
};

struct CGraphNode // 96 bytes
{
    CMath::CMatrix4 xform;
    CMath::CQuarternion rotation;
    CMath::XMFLOAT3 scale;

    uint32_t matrixIdx : 28;
    uint32_t hasSibling : 1;
    uint32_t hasChildren : 1;
    uint32_t staleMatrix : 1;
    uint32_t skeletonRoot : 1;
};
