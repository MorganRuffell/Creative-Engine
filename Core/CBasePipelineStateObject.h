#pragma once
#include "pch.h"

using CMath::IsAligned;
using Microsoft::WRL::ComPtr;
using namespace std;

//There is a map containing, each pipeline state object to the size in bytes
//These are static and persist for the entire program lifetime.
static map< size_t, ComPtr<ID3D12PipelineState> > s_GraphicsPSOHashMap;
static map< size_t, ComPtr<ID3D12PipelineState> > s_ComputePSOHashMap;

// In other versions of this engine I've only being using one pipeline state object, this was initalized
// In the renderer, now there is an object that manages the instantiation of these PSOs
class CBasePipelineStateObject
{
public:

	CBasePipelineStateObject(const wchar_t* Name) : m_Name(Name), m_RootSignature(nullptr), m_PSO(nullptr) {}

	static void DestroyAll(void)
	{
		s_GraphicsPSOHashMap.clear();
		s_ComputePSOHashMap.clear();
	}

	void SetRootSignature(const CRootSignature& BindMappings)
	{
		m_RootSignature = &BindMappings;
	}

	const CRootSignature& GetRootSignature(void) const
	{
		assert(m_RootSignature != nullptr);
		return *m_RootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject(void) const { return m_PSO; }

public:

	const wchar_t* m_Name;

	const CRootSignature* m_RootSignature;
	ID3D12PipelineState* m_PSO;
};