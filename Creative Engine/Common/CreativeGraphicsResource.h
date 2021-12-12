#pragma once

#include <ppltasks.h>	// For create_task
#include "CGraphicsBase.h"
namespace DX
{
	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	// Assign a name to the object to aid with debugging.
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
#if defined(_DEBUG)
	
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif
}