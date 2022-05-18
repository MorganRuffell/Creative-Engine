#pragma once
#include <pch.h>

/// <summary>
/// Base GPU Resource specialised for buffers
/// 
/// This is NOT to be used with contexts -- These have thier own base class.
/// 
/// This acts like a wrapper around ID3D12Resource2, it allows us to have all of the data relating to
/// the specific resource constrained into one class.
/// 
/// </summary>
class CGPUResource
{
    friend class CCommandContext;
    friend class CGraphicsContext;
    friend class CComputeContext;

public:
    CGPUResource() : 
        m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
        m_UsageState(D3D12_RESOURCE_STATE_COMMON),
        m_TransitioningState((D3D12_RESOURCE_STATES)-1)
    {
    }

    CGPUResource(ID3D12Resource2* pResource, D3D12_RESOURCE_STATES CurrentState) :
        m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
        m_pResource(pResource),
        m_UsageState(CurrentState),
        m_TransitioningState((D3D12_RESOURCE_STATES)-1)
    {
    }

    ~CGPUResource() { Destroy(); }

public:

    virtual void Destroy()
    {
        m_pResource = nullptr;
        m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        ++m_VersionID;
    }


public:


    ID3D12Resource2** GetAddressOf() { return m_pResource.GetAddressOf(); }

    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

    uint32_t GetVersionID() const { return m_VersionID; }

public:

    // We overload the -> operator to allow us to access the raw pointer behind the comptr. 
    // The comptr is useful, but in the case of this we want to be able to directly access the raw pointer.
    ID3D12Resource2* operator->() { return m_pResource.Get(); }
    const ID3D12Resource2* operator->() const { return m_pResource.Get(); }

    ID3D12Resource2* GetResource() { return m_pResource.Get(); }
    const ID3D12Resource2* GetResource() const { return m_pResource.Get(); }
    

protected:

    ComPtr<ID3D12Resource2> m_pResource;

    D3D12_RESOURCE_STATES m_UsageState;
    D3D12_RESOURCE_STATES m_TransitioningState;

    D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

    // Used to identify when a resource changes so descriptors can be copied etc.
    uint32_t m_VersionID = 0;
};
