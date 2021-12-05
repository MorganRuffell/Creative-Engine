#pragma once
#include "CGraphicsBase.h"

using Microsoft::WRL::ComPtr;



class CGPUResource
{
public:
	CGPUResource(ComPtr<ID3D12Resource> Resource, D3D12_RESOURCE_STATES usageState);
	virtual ~CGPUResource();

public:
	ComPtr<ID3D12Resource> GetResource() 
	{ 
		return LocalResource; 
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() 
	{
		return LocalGPUAddress; 
	}

	D3D12_RESOURCE_STATES GetUsageState() 
	{ 
		return LocalUsageState;  
	}

	void SetUsageState(D3D12_RESOURCE_STATES usageState) 
	{
		LocalUsageState = usageState; 
	}

	void SetState(bool NewState)
	{
		State = NewState;
	}

	bool GetState() 
	{ 
		return State; 
	}

protected:
	ComPtr<ID3D12Resource> LocalResource;
	D3D12_GPU_VIRTUAL_ADDRESS LocalGPUAddress;
	D3D12_RESOURCE_STATES LocalUsageState;
	bool State;

};

