#include "pch.h"
#include "CMaterial.h"

void CMaterial::SetProperty(EMaterialProperties inputProperty)
{
	m_PBRMaterialProperties = static_cast<EMaterialProperties>(inputProperty | m_PBRMaterialProperties);
}