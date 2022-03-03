#include "CGraphicsResource.h"

HRESULT __stdcall CGraphicsResource::QueryInterface(REFIID riid, void** ppvObject)
{
    return E_NOTIMPL;
}

ULONG __stdcall CGraphicsResource::AddRef(void)
{
    return 0;
}

ULONG __stdcall CGraphicsResource::Release(void)
{
    return 0;
}

HRESULT __stdcall CGraphicsResource::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::SetName(LPCWSTR Name)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::GetDevice(REFIID riid, void** ppvDevice)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData)
{
    return E_NOTIMPL;
}

void __stdcall CGraphicsResource::Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange)
{
}

D3D12_RESOURCE_DESC __stdcall CGraphicsResource::GetDesc(void)
{
    return D3D12_RESOURCE_DESC();
}

D3D12_GPU_VIRTUAL_ADDRESS __stdcall CGraphicsResource::GetGPUVirtualAddress(void)
{
    return D3D12_GPU_VIRTUAL_ADDRESS();
}

HRESULT __stdcall CGraphicsResource::WriteToSubresource(UINT DstSubresource, const D3D12_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::ReadFromSubresource(void* pDstData, UINT DstRowPitch, UINT DstDepthPitch, UINT SrcSubresource, const D3D12_BOX* pSrcBox)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::GetHeapProperties(D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS* pHeapFlags)
{
    return E_NOTIMPL;
}

HRESULT __stdcall CGraphicsResource::GetProtectedResourceSession(REFIID riid, void** ppProtectedSession)
{
    return E_NOTIMPL;
}

D3D12_RESOURCE_DESC1 __stdcall CGraphicsResource::GetDesc1(void)
{
    return D3D12_RESOURCE_DESC1();
}
