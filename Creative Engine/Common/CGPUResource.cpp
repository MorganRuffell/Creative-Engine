#include "CGPUResource.h"
#include "pch.h"

CGPUResource::CGPUResource(ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES usageState)
{
	LocalResource = Resource;
	LocalUsageState = usageState;
	LocalGPUAddress = 0;
	State = true;
}

CGPUResource::~CGPUResource()
{
	LocalResource->Release();
	LocalResource = nullptr;
}