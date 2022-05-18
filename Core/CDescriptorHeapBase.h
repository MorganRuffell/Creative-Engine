#pragma once
#include "CDescriptorHandle.h"

class CDescriptorHeapBase
{
protected:

	uint32_t m_DescriptorSize;

	CDescriptorHandle m_FirstHandle;
};

