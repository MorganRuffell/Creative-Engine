#pragma once

#include "pch.h"
#include "CTextureManager.h"
#include "CGPSOHandle.h"

enum EMaterialProperties
{
	None = 0,
	ConstantColor = 1,
	Diffuse = 2,
	SpecularDiffuse = 4,
	DiffuseTexture = 8,
	ReflectionTexture = 16
};

/// <summary>
/// Inactive as of now :: Get this working at some point...
/// </summary>
class CMaterial
{
public:

	EMaterialProperties m_PBRMaterialProperties;

public:

	std::vector<CTextureHandle> m_textures;

public:

	CGraphicsBufferHandle m_constantBuffer;
	CGPSOHandle m_PipelineState;

	void SetProperty(EMaterialProperties inputProperty);

};

typedef std::shared_ptr<CMaterial> MaterialPtr;