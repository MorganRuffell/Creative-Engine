#pragma once
#include "CDirectX12Core.h"
#include "CRendererBase.h"
#include "ShaderStructures.h"

class CGraphicsResource
{

public:

    D3D12_GPU_VIRTUAL_ADDRESS __stdcall GetGPUVirtualAddress(void);

    HRESULT __stdcall WriteToSubresource(UINT DstSubresource, const D3D12_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch);

    HRESULT __stdcall ReadFromSubresource(void* pDstData, UINT DstRowPitch, UINT DstDepthPitch, UINT SrcSubresource, const D3D12_BOX* pSrcBox);

    HRESULT __stdcall GetHeapProperties(D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS* pHeapFlags);

    HRESULT __stdcall GetProtectedResourceSession(REFIID riid, void** ppProtectedSession);

    D3D12_RESOURCE_DESC1 __stdcall GetDesc1(void);

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject);

    ULONG __stdcall AddRef(void);

    ULONG __stdcall Release(void);

public:

    HRESULT __stdcall GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData);

    HRESULT __stdcall SetPrivateData(REFGUID guid, UINT DataSize, const void* pData);

    HRESULT __stdcall SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);

    HRESULT __stdcall SetName(LPCWSTR Name);

public:

    HRESULT __stdcall GetDevice(REFIID riid, void** ppvDevice);

    HRESULT __stdcall Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData);

    void __stdcall Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange);

    D3D12_RESOURCE_DESC __stdcall GetDesc(void);
};

