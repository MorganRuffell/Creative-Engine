#pragma once
#include "CDXRGPUUploadBuffer.h"

/// <summary>
/// One of the issues that you'll run into a lot with DXR is buffer alignment
/// this generic class solves a lot of these issues
/// <usage> 
///     CDXRGPUConstantBufffer<T> constantBuffer;
///     constantBuffer.Create();
///     constantBuffer.stagong.var = ...;
///     constantBuffer.ReplicateStagingToGPU();
/// 
/// </usage>
/// </summary>
template<class T>
class CDXRGPUConstantBuffer : public CDXRGPUUploadBuffer
{
public:

    CDXRGPUConstantBuffer();

    HRESULT Initalize(ID3D12Device8* Device, UINT numInstances = 1, LPCWSTR resourceName = nullptr);

    HRESULT ReplicateStagingToGPU(UINT instanceIndex = 0);

    HRESULT ClearBuffer();

public:

    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }

private:

    uint8_t*            m_MappedConstantData;
    UINT                m_AlignedConstantData;
    UINT                m_NumberOfInstances;
    bool                m_BufferAllocationStatus;
};

template<class T>
inline CDXRGPUConstantBuffer<T>::CDXRGPUConstantBuffer()
{
    m_MappedConstantData = 0;
    m_NumberOfInstances = 0;
    m_MappedConstantData = nullptr;
}

template<class T>
inline HRESULT CDXRGPUConstantBuffer<T>::Initalize(ID3D12Device8* Device, UINT numInstances, LPCWSTR resourceName)
{
    assert(Device != nullptr);

    if (numInstances < 1)
    {
        std::cout << "Number of instances are less than one!" << std::endl;
        throw new std::exception;
    }

    m_NumberOfInstances = numInstances;


    UINT AlignedSize = (sizeof(T) + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
    UINT BufferSize = numInstances * AlignedSize;

    Allocate(Device, BufferSize, resourceName);
    m_MappedConstantData = MapCPUWriteOnly();

    m_BufferAllocationStatus = true;
    return S_OK;
}

template<class T>
inline HRESULT CDXRGPUConstantBuffer<T>::ReplicateStagingToGPU(UINT instanceIndex)
{
    if (m_MappedConstantData != nullptr)
    {
        memcpy(m_MappedConstantData + instanceIndex * m_AlignedConstantData, &staging, sizeof(T));
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

template<class T>
inline HRESULT CDXRGPUConstantBuffer<T>::ClearBuffer()
{
    if (free(m_MappedConstantData))
    {
        m_BufferAllocationStatus = false;
        return S_OK;
    }
    else
    {
        std::cout << "Failed to Clear DXR Constant Buffer" << std::endl;
        return E_FAIL;
    }
    
}
