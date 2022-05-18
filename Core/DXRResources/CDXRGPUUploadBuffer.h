#pragma once
#include <wrl/client.h>
#include "../d3dx12.h"
#include <iostream>
#include <assert.h>

using Microsoft::WRL::ComPtr;

/// <summary>
/// DirectX12 Needs to have all of it's visual resources created on the GPU
/// This resource is the back end of it all - Before we were just using raw
/// ID3D12 Resources. 
/// </summary>
class CDXRGPUUploadBuffer
{
public:
    ComPtr<ID3D12Resource2> GetResource() {
        return m_Resource;
    }

    UINT GetGPUUploadBufferSize()
    {
        return m_BufferSize;
    }

protected:


    CDXRGPUUploadBuffer() = default;
    ~CDXRGPUUploadBuffer()
    {
        if (m_Resource.Get())
        {
            m_Resource->Unmap(0, nullptr);
            m_Resource->Release();
        }
    }

    void Allocate(ID3D12Device8* GraphicsCard, UINT bufferSize, LPCWSTR resourceName = nullptr)
    {
        if (bufferSize < 1)
        {
            throw new std::exception;
        }
        else
        {
            D3D12_HEAP_PROPERTIES UploadBufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

            auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            HRESULT res = GraphicsCard->CreateCommittedResource(
                &UploadBufferProperties,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_Resource));

            if (FAILED(res))
            {
                std::cout << "GPU Upload buffer was not initalized correctly" << std::endl;
                throw new std::exception;
            }

            m_Resource->SetName(resourceName);

        }
    }

    uint8_t* MapCPUWriteOnly()
    {
        uint8_t* DataToMap;

        // We do not unmap this until Application::quit is called.
        CD3DX12_RANGE readRange(0, 0);

        HRESULT res = m_Resource->Map(0, &readRange, reinterpret_cast<void**>(&DataToMap));
        if (!SUCCEEDED(res))
        {
            std::cout << "CPU Map write of GPU Upload buffer failed";
            throw new std::exception;
        }
        else
        {
            return DataToMap;
        }
    }

protected:

    UINT m_BufferSize;
	ComPtr<ID3D12Resource2> m_Resource;

};

