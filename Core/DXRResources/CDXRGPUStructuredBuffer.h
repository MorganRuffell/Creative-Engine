#pragma once
#include "CDXRGPUUploadBuffer.h"
template<class T>
class CDXRGPUStructuredBuffer : public CDXRGPUUploadBuffer
{
public:

    static_assert(sizeof(T) % 16 == 0, L"Follow NVidias Advice align them on 16byte boundary.");

    CDXRGPUStructuredBuffer()
    {
        m_MappedBuffers = nullptr;
        m_NumberOfInstances = 0;
    }

    /// <summary>
    /// Throughout this project we've been using Device8.
    /// Do not change this unless you want a bad time!
    /// </summary>
    /// <typeparam name="T"></typeparam>
    void Init(ID3D12Device* GraphicsCard, UINT NumberOfElements, UINT NumberOfInstances = 1, LPCWSTR ResourceName = nullptr);

    /// <summary>
    /// Copy the vector the gpu, ready for the command list to fire it away!
    /// </summary>
    void CopyStagingToGPU(UINT instanceIndex = 0);

public:

    T& operator[](UINT elementIndex)
    {
        return m_BufferStaging[elementIndex];
    }

    size_t NumElementsPerInstance()
    {
        return m_BufferStaging.size();
    }

    UINT NumInstances()
    {
        return m_BufferStaging.size();
    }
    size_t InstanceSize()
    {
        return NumElementsPerInstance() * sizeof(T);
    }

    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_Resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize();
    }

private:

    T m_MappedBuffers;
    std::vector<T> m_BufferStaging;
    UINT m_NumberOfInstances;

};

template<class T>
inline void CDXRGPUStructuredBuffer<T>::Init(ID3D12Device* GraphicsCard, UINT NumberOfElements, UINT NumberOfInstances, LPCWSTR ResourceName)
{
    assert(GraphicsCard != nullptr);

    m_BufferStaging.resize(NumberOfElements);
    UINT BufferSize = (NumberOfElements * NumberOfInstances) * sizeof(T);
    Allocate(GraphicsCard, BufferSize, ResourceName);
    m_MappedBuffers = reinterpret_cast<T*>(MapCPUWriteOnly());
}

template<class T>
inline void CDXRGPUStructuredBuffer<T>::CopyStagingToGPU(UINT instanceIndex)
{
    memcpy(m_MappedBuffers + instanceIndex, &m_BufferStaging[0], InstanceSize());


}
