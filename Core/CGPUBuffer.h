

#pragma once

#include "pch.h"
#include "CGPUBaseResource.h"

class CCommandContext;
class CGPURamAllocator;
class CGraphicsUploadBuffer;

class CGPUBuffer : public CGPUResource
{
public:
	virtual ~CGPUBuffer() { Destroy(); }

	void Create(_In_ const std::wstring& name, _In_ uint32_t NumElements, _In_ uint32_t ElementSize,
		_Inout_ const void* initialData = nullptr);

	void Create(_In_ const std::wstring& name, _In_ uint32_t NumElements, _In_ uint32_t ElementSize,
		_In_ const CGraphicsUploadBuffer& srcData, _In_ uint32_t srcOffset = 0);

	void Create(_In_ const std::wstring& name, _In_ uint32_t NumElements, _In_ uint32_t ElementSize,
		_In_ CGPURamAllocator& Allocator, _Inout_ const void* initialData = nullptr);

public:

	void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
		const void* initialData = nullptr);

public:

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRV; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView(void) const { return m_GpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(_In_ uint32_t Offset, _In_ uint32_t Size) const;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(_In_ size_t Offset, _In_ uint32_t Size, _In_ uint32_t Stride) const;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(_In_ size_t BaseVertexIndex = 0) const
	{
		size_t Offset = BaseVertexIndex * m_ElementSize;
		return VertexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView(_In_ size_t Offset, _In_ uint32_t Size, _In_ bool b32Bit = false) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(_In_ size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * m_ElementSize;
		return IndexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize == 4);
	}

	size_t GetBufferSize() const { return m_BufferSize; }
	uint32_t GetElementCount() const { return m_ElementCount; }
	uint32_t GetElementSize() const { return m_ElementSize; }

protected:

	CGPUBuffer(void) : m_BufferSize(0), m_ElementCount(0), m_ElementSize(0)
	{
		m_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_UAV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_SRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	D3D12_RESOURCE_DESC DescribeBuffer(void);
	virtual void CreateDerivedViews(void) = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SRV;

	size_t m_BufferSize;
	uint32_t m_ElementCount;
	uint32_t m_ElementSize;
	D3D12_RESOURCE_FLAGS m_ResourceFlags;
};

inline D3D12_VERTEX_BUFFER_VIEW CGPUBuffer::VertexBufferView(_In_ size_t Offset, _In_ uint32_t Size, _In_ uint32_t Stride) const
{
	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = m_GpuVirtualAddress + Offset;
	VBView.SizeInBytes = Size;
	VBView.StrideInBytes = Stride;
	return VBView;
}

inline D3D12_INDEX_BUFFER_VIEW CGPUBuffer::IndexBufferView(_In_ size_t Offset, _In_ uint32_t Size, _In_ bool b32Bit) const
{
	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = m_GpuVirtualAddress + Offset;
	IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	IBView.SizeInBytes = Size;
	return IBView;
}

class CByteAddressBuffer : public CGPUBuffer
{
public:
	virtual void CreateDerivedViews(void) override;
};

class CIndirectParamBuffer : public CByteAddressBuffer
{
public:
	CIndirectParamBuffer(void)
	{
	}
};

class CStructuredBuffer : public CGPUBuffer
{
public:
	virtual void Destroy(void) override
	{
		m_CounterBuffer.Destroy();
		CGPUBuffer::Destroy();
	}

	virtual void CreateDerivedViews(void) override;

	CByteAddressBuffer& GetCounterBuffer(void) { return m_CounterBuffer; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CCommandContext& Context);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CCommandContext& Context);

private:
	CByteAddressBuffer m_CounterBuffer;
};

class CTypeBasedBuffer : public CGPUBuffer
{
public:
	CTypeBasedBuffer(DXGI_FORMAT Format) : m_DataFormat(Format) {}
	virtual void CreateDerivedViews(void) override;

protected:
	DXGI_FORMAT m_DataFormat;
};

