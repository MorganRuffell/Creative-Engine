
#pragma once

#include "pch.h"
#include "CCommandListManager.h"
#include "CRGBAColour.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CGPUBuffer.h"
#include "CTexture.h"
#include "CPixelBuffer.h"
#include "CDynamicDescriptorHeap.h"
#include "LinearAllocator.h"
#include "CDXIndirectArgument.h"
#include "CDirectX12Core.h"
#include <vector>
#include <wrl/client.h>
#include "CBasePipelineStateObject.h"
#include "CBaseSingleton.h"

#include "Display.h"

class CRGBBuffer;
class CGPUDepthBuffer;
class CTexture;
class CGraphicsContext;
class CComputeContext;
class CGraphicsUploadBuffer;
class ReadbackBuffer;

struct DWParam
{
    DWParam( FLOAT f ) : Float(f) {}
    DWParam( UINT u ) : Uint(u) {}
    DWParam( INT i ) : Int(i) {}

    void operator= ( FLOAT f ) { Float = f; }
    void operator= ( UINT u ) { Uint = u; }
    void operator= ( INT i ) { Int = i; }

    union
    {
        FLOAT Float;
        UINT Uint;
        INT Int;
    };
};

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

//Class that manages all the command lists, recall that these are the backing data
//behind all command lists
class CDXContextManager : public CBaseSingleton
{
public:
    CDXContextManager(void) {}

    CCommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE Type, bool ForUI);
    void FreeContext(CCommandContext*);
    void DestroyAllContexts();

public:
    std::vector<std::unique_ptr<CCommandContext> > sm_ContextPool[4];
    std::queue<CCommandContext*> sm_AvailableContexts[4];

    //We allocate resources concurrently - So therefore we lock and unlock
    //a mutex with a lock guard to work concurrently.

    std::mutex sm_ContextAllocationMutex;
};

/// <summary>
/// CCommandContext:
/// 
/// This is a funamentally important class in creative.
/// 
/// DirectX12 functions by decopuling graphics processing from the CPU to the GPU.
/// When working with it, we are to a point acting like driver programmers, it .
/// 
/// This comes in the form of two objects the ID3D12CommandAllocator and the ID3D12CommandList
/// 
///     Command List   -        Stores all of the commands in the command allocator
///     Command Allocator -     The memory that the commands are stored inside of.
/// 
/// CCommandContext derieves from CBaseSingleton, this ensures that this class cannot be copied or moved around
/// there are several command contexts inside of creative and its runtime, but they serve different purposes
/// and they have a contextually relevant purpose in the engine. The best analogy for a programmer used to uneal and Unity
/// is the concept of DontDestroyOnLoad.
/// 
/// imgui will use one of it's derivitives to allocate all of it's resoures.
/// </summary>
class   CCommandContext : CBaseSingleton
{
    //We need access to the context managers private variables to be able to add new ones 
    //and perform CRUD operations
    friend CDXContextManager;

private:

    CCommandContext(D3D12_COMMAND_LIST_TYPE Type);

public:

    void Reset(void);

    ~CCommandContext(void);

    static void DestroyAllContexts(void);

    static CCommandContext& Begin(const std::wstring ID = L"");

    // Flush existing commands to the GPU but keep the context alive
    uint64_t Flush( bool WaitForCompletion = false );

    // Flush existing commands and release the current context
    uint64_t Finish( bool WaitForCompletion = false );

    // Prepare to render by setting a command list and it's backing memory the allocator.
    void Initialize(void);

    CGraphicsContext& GetGraphicsContext() {
        ASSERT(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
        return reinterpret_cast<CGraphicsContext&>(*this);
    }

    CComputeContext& GetCComputeContext() {
        return reinterpret_cast<CComputeContext&>(*this);
    }

    ID3D12GraphicsCommandList* GetCommandList() {
        return m_CommandList.Get();
    }

    ID3D12CommandAllocator* GetCommandAllocator()
    {
        return m_CurrentAllocator.Get();
    }
            

    void CopyBuffer( CGPUResource& Dest, CGPUResource& Src );
    void CopyBufferRegion( CGPUResource& Dest, size_t DestOffset, CGPUResource& Src, size_t SrcOffset, size_t NumBytes );
    void CopySubresource(CGPUResource& Dest, UINT DestSubIndex, CGPUResource& Src, UINT SrcSubIndex);
    void CopyCounter(CGPUResource& Dest, size_t DestOffset, CStructuredBuffer& Src);
    void CopyTextureRegion(CGPUResource& Dest, UINT x, UINT y, UINT z, CGPUResource& Source, RECT& rect);
    void ResetCounter(CStructuredBuffer& Buf, uint32_t Value = 0);

    // Creates a readback buffer of sufficient size, copies the texture into it,
    // and returns row pitch in bytes.
    uint32_t ReadbackTexture(ReadbackBuffer& DstBuffer, CPixelBuffer& SrcBuffer);

    SDynAlloc ReserveUploadMemory(size_t SizeInBytes)
    {
        return m_CpuLinearAllocator.Allocate(SizeInBytes);
    }

    CDynamicDescriptorHeap getSRVHeap()
    {
        return m_DynamicViewDescriptorHeap;
    }

    static void InitializeTexture( CGPUResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[] );
    static void InitializeBuffer( CGPUBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
    static void InitializeBuffer( CGPUBuffer& Dest, const CGraphicsUploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0 );
    static void InitializeTextureArraySlice(CGPUResource& Dest, UINT SliceIndex, CGPUResource& Src);

    void WriteBuffer( CGPUResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes );
    void FillBuffer( CGPUResource& Dest, size_t DestOffset, DWParam Value, size_t NumBytes );

    void TransitionResource(CGPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
    void BeginResourceTransition(CGPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
    void InsertUAVBarrier(CGPUResource& Resource, bool FlushImmediate = false);
    void InsertAliasBarrier(CGPUResource& Before, CGPUResource& After, bool FlushImmediate = false);
    inline void FlushResourceBarriers(void);

    void InsertTimeStamp( ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx );
    void ResolveTimeStamps( ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries );
    void PIXBeginEvent(const wchar_t* label);
    void PIXEndEvent(void);
    void PIXSetMarker(const wchar_t* label);

    void SetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr );
    void SetDescriptorHeaps( UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[] );
    void SetPipelineState( const CBasePipelineStateObject& PSO );

    void SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op);

    void SetScissor(const D3D12_RECT& rect);
    void SetViewport(const D3D12_VIEWPORT& viewport);
    void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE& rtv, const D3D12_CPU_DESCRIPTOR_HANDLE& dsv) const
    {
        m_CommandList->OMSetRenderTargets(1, &rtv, true, &dsv);
    }

protected:

    void BindDescriptorHeaps( void );

    CCommandListManager* m_OwningManager;

    ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    ComPtr<ID3D12CommandAllocator> m_CurrentAllocator;

	ID3D12RootSignature* m_CurGraphicsRootSignature;
	ID3D12RootSignature* m_CurComputeRootSignature;
	ID3D12PipelineState* m_CurPipelineState;

    CDynamicDescriptorHeap m_DynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
    CDynamicDescriptorHeap m_DynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

    D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
    UINT m_NumBarriersToFlush;

    std::mutex sm_CommandContextMutex;


    ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    CLinearAllocator m_CpuLinearAllocator;
    CLinearAllocator m_GpuLinearAllocator;

    std::wstring m_ID;
    void SetID(const std::wstring& ID) { m_ID = ID; }

    D3D12_COMMAND_LIST_TYPE m_Type;
};

//Command contexts act as the base for graphics contexts, after all they require all of
//the components of a commandcontext.
class   CGraphicsContext : public CCommandContext
{
public:
    static CGraphicsContext& Begin(const std::wstring& ID = L"")
    {
        return CCommandContext::Begin(ID).GetGraphicsContext();
    }

    void ClearUAV( CGPUBuffer& Target );
    void ClearUAV( CRGBBuffer& Target );
    void ClearColor( CRGBBuffer& Target, D3D12_RECT* Rect = nullptr);
    void ClearColor(CRGBBuffer& Target, float Colour[4], D3D12_RECT* Rect = nullptr);
    void ClearDepth( CGPUDepthBuffer& Target );
    void ClearStencil( CGPUDepthBuffer& Target );
    void ClearDepthAndStencil( CGPUDepthBuffer& Target );

    void BeginQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex);
    void EndQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT HeapIndex);
    void ResolveQueryData(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, ID3D12Resource* DestinationBuffer, UINT64 DestinationBufferOffset);


public:

    void SetRootSignature( const CRootSignature& RootSig );

    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV ) { SetRenderTargets(1, &RTV); }
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, D3D12_CPU_DESCRIPTOR_HANDLE DSV ) { SetRenderTargets(1, &RTV, DSV); }
    void SetDepthStencilTarget(D3D12_CPU_DESCRIPTOR_HANDLE DSV ) { SetRenderTargets(0, nullptr, DSV); }

    void SetViewport( const D3D12_VIEWPORT& vp );
    void SetViewport( FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f );

    void SetScissor( const D3D12_RECT& rect );
    void SetScissor( UINT left, UINT top, UINT right, UINT bottom );

    void SetViewportAndScissor( const D3D12_VIEWPORT& vp, const D3D12_RECT& rect );
    void SetViewportAndScissor( UINT x, UINT y, UINT w, UINT h );

public:

    void SetStencilRef( UINT StencilRef );
    void SetBlendFactor( CRGBAColour BlendFactor );
    void SetPrimitiveTopology( D3D12_PRIMITIVE_TOPOLOGY Topology );

    void SetConstantArray( UINT RootIndex, UINT NumConstants, const void* pConstants );
    void SetConstant( UINT RootIndex, UINT Offset, DWParam Val );

    void SetConstants( UINT RootIndex, DWParam X );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W );

    void SetConstantBuffer( UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV );
    void SetDynamicConstantBufferView( UINT RootIndex, size_t BufferSize, const void* BufferData );
    void SetBufferSRV( UINT RootIndex, const CGPUBuffer& SRV, UINT64 Offset = 0);
    void SetBufferUAV( UINT RootIndex, const CGPUBuffer& UAV, UINT64 Offset = 0);
    void SetDescriptorTable( UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle );

    void SetDynamicDescriptor( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle );
    void SetDynamicDescriptors( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] );
    void SetDynamicSampler( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle );
    void SetDynamicSamplers( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] );

    void SetIndexBuffer( const D3D12_INDEX_BUFFER_VIEW& IBView );
    void SetVertexBuffer( UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView );
    void SetVertexBuffers( UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[] );
    void SetDynamicVB( UINT Slot, size_t NumVertices, size_t VertexStride, const void* VBData );
    void SetDynamicIB( size_t IndexCount, const uint16_t* IBData );
    void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);

public:

    //There is a lot of drawing methods -- Regular draw just draws individal vertexes, indexed is for drawing based on an index
    void Draw( UINT VertexCount, UINT VertexStartOffset = 0 );
    void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
    void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
        UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
    void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
        INT BaseVertexLocation, UINT StartInstanceLocation);
    void DrawIndirect( CGPUBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0 );

    //This executes all indirect command contexts
    void ExecuteIndirect(CCommandSignature& CommandSig, CGPUBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
        uint32_t MaxCommands = 1, CGPUBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);


    D3D12_VIEWPORT m_vp;
    D3D12_RECT m_Sci;
    //D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViews[3];
};

/// <summary>
/// Context class that manages compute pipeline
/// </summary>
class  CComputeContext : public CCommandContext
{
public:

    static CComputeContext& Begin(const std::wstring& ID = L"", bool Async = false);

    void ClearUAV( CGPUBuffer& Target );
    void ClearUAV( CRGBBuffer& Target );

    void SetRootSignature( const CRootSignature& RootSig );

    void SetConstantArray( UINT RootIndex, UINT NumConstants, const void* pConstants );

public:

    void SetConstant( UINT RootIndex, UINT Offset, DWParam Val );
    void SetConstants( UINT RootIndex, DWParam X );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z );
    void SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W );

public:

    void SetConstantBuffer( UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV );
    void SetDynamicConstantBufferView( UINT RootIndex, size_t BufferSize, const void* BufferData );

public:

    void SetDynamicSRV( UINT RootIndex, size_t BufferSize, const void* BufferData ); 
    void SetBufferSRV( UINT RootIndex, const CGPUBuffer& SRV, UINT64 Offset = 0);
    void SetBufferUAV( UINT RootIndex, const CGPUBuffer& UAV, UINT64 Offset = 0);

public:
    void SetDescriptorTable( UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle );

    void SetDynamicDescriptor( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle );
    void SetDynamicDescriptors( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] );
    void SetDynamicSampler( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle );
    void SetDynamicSamplers( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] );

    void Dispatch( size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1 );
    void Dispatch1D( size_t ThreadCountX, size_t GroupSizeX = 64);
    void Dispatch2D( size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX = 8, size_t GroupSizeY = 8);
    void Dispatch3D( size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ );

    // As you know we are running both graphics pipeline shaders and compute pipeline shaders, these have different 'execution'
    // methods.
    void DispatchIndirect( CGPUBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0 );
    void ExecuteIndirect(CCommandSignature& CommandSig, CGPUBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
        uint32_t MaxCommands = 1, CGPUBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);

private:
};

//These are managed inline
inline void CCommandContext::FlushResourceBarriers( void )
{
    std::lock_guard<std::mutex> ResourceBarrierMutex(sm_CommandContextMutex);
    

    if (m_NumBarriersToFlush > 0)
    {
        GetCommandList()->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
        m_NumBarriersToFlush = 0;
    }
}

inline void CGraphicsContext::SetRootSignature( const CRootSignature& RootSig )
{
   
    GetCommandList()->SetGraphicsRootSignature(m_CurGraphicsRootSignature = RootSig.GetSignature());

    m_DynamicViewDescriptorHeap.ParseGraphicsRootSignature(RootSig);
    m_DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(RootSig);
}

inline void CComputeContext::SetRootSignature( const CRootSignature& RootSig )
{
    if (RootSig.GetSignature() == m_CurComputeRootSignature)
        return;

    GetCommandList()->SetComputeRootSignature(m_CurComputeRootSignature = RootSig.GetSignature());

    m_DynamicViewDescriptorHeap.ParseComputeRootSignature(RootSig);
    m_DynamicSamplerDescriptorHeap.ParseComputeRootSignature(RootSig);
}

inline void CCommandContext::SetPipelineState( const CBasePipelineStateObject& _PipelineState )
{
    ID3D12PipelineState* PipelineState = _PipelineState.GetPipelineStateObject();
    if (PipelineState == m_CurPipelineState)
        return;

    GetCommandList()->SetPipelineState(PipelineState);
    m_CurPipelineState = PipelineState;
}

inline void CGraphicsContext::SetViewportAndScissor( UINT x, UINT y, UINT w, UINT h )
{
    SetViewport((float)x, (float)y, (float)w, (float)h);
    SetScissor(x, y, x + w, y + h);
}

inline void CGraphicsContext::SetScissor( UINT left, UINT top, UINT right, UINT bottom )
{
    SetScissor(CD3DX12_RECT(left, top, right, bottom));
}

inline void CGraphicsContext::SetStencilRef( UINT ref )
{
    GetCommandList()->OMSetStencilRef( ref );
}

inline void CGraphicsContext::SetBlendFactor( CRGBAColour BlendFactor )
{
    GetCommandList()->OMSetBlendFactor( BlendFactor.GetPtr() );
}

inline void CGraphicsContext::SetPrimitiveTopology( D3D12_PRIMITIVE_TOPOLOGY Topology )
{
    GetCommandList()->IASetPrimitiveTopology(Topology);
}

inline void CComputeContext::SetConstantArray( UINT RootEntry, UINT NumConstants, const void* pConstants )
{
    GetCommandList()->SetComputeRoot32BitConstants( RootEntry, NumConstants, pConstants, 0 );
}

inline void CComputeContext::SetConstant( UINT RootEntry, UINT Offset, DWParam Val )
{
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Val.Uint, Offset );
}

inline void CComputeContext::SetConstants( UINT RootEntry, DWParam X )
{
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, X.Uint, 0 );
}

inline void CComputeContext::SetConstants( UINT RootEntry, DWParam X, DWParam Y )
{
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, X.Uint, 0 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Y.Uint, 1 );
}

inline void CComputeContext::SetConstants( UINT RootEntry, DWParam X, DWParam Y, DWParam Z )
{
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, X.Uint, 0 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Y.Uint, 1 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Z.Uint, 2 );
}

inline void CComputeContext::SetConstants( UINT RootEntry, DWParam X, DWParam Y, DWParam Z, DWParam W )
{
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, X.Uint, 0 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Y.Uint, 1 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, Z.Uint, 2 );
    GetCommandList()->SetComputeRoot32BitConstant( RootEntry, W.Uint, 3 );
}

inline void CGraphicsContext::SetConstantArray( UINT RootIndex, UINT NumConstants, const void* pConstants )
{
    GetCommandList()->SetGraphicsRoot32BitConstants( RootIndex, NumConstants, pConstants, 0 );
}

inline void CGraphicsContext::SetConstant( UINT RootEntry, UINT Offset, DWParam Val )
{
    GetCommandList()->SetGraphicsRoot32BitConstant( RootEntry, Val.Uint, Offset );
}

inline void CGraphicsContext::SetConstants( UINT RootIndex, DWParam X )
{
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, X.Uint, 0 );
}

inline void CGraphicsContext::SetConstants( UINT RootIndex, DWParam X, DWParam Y )
{
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, X.Uint, 0 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, Y.Uint, 1 );
}

inline void CGraphicsContext::SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z )
{
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, X.Uint, 0 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, Y.Uint, 1 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, Z.Uint, 2 );
}

inline void CGraphicsContext::SetConstants( UINT RootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W )
{
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, X.Uint, 0 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, Y.Uint, 1 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, Z.Uint, 2 );
    GetCommandList()->SetGraphicsRoot32BitConstant( RootIndex, W.Uint, 3 );
}

inline void CComputeContext::SetConstantBuffer( UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV )
{
    GetCommandList()->SetComputeRootConstantBufferView(RootIndex, CBV);
}

inline void CGraphicsContext::SetConstantBuffer( UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV )
{
    GetCommandList()->SetGraphicsRootConstantBufferView(RootIndex, CBV);
}

inline void CGraphicsContext::SetDynamicConstantBufferView( UINT RootIndex, size_t BufferSize, const void* BufferData )
{
    ASSERT(BufferData != nullptr && CMath::IsAligned(BufferData, 16));
    SDynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
    //SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
    memcpy(cb.DataPtr, BufferData, BufferSize);
    GetCommandList()->SetGraphicsRootConstantBufferView(RootIndex, cb.GpuAddress);
}

inline void CComputeContext::SetDynamicConstantBufferView( UINT RootIndex, size_t BufferSize, const void* BufferData )
{
    ASSERT(BufferData != nullptr && CMath::IsAligned(BufferData, 16));
    SDynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
    //SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
    memcpy(cb.DataPtr, BufferData, BufferSize);
    GetCommandList()->SetComputeRootConstantBufferView(RootIndex, cb.GpuAddress);
}

inline void CGraphicsContext::SetDynamicVB( UINT Slot, size_t NumVertices, size_t VertexStride, const void* VertexData )
{
    ASSERT(VertexData != nullptr && CMath::IsAligned(VertexData, 16));

    size_t BufferSize = CMath::AlignUp(NumVertices * VertexStride, 16);
    SDynAlloc vb = m_CpuLinearAllocator.Allocate(BufferSize);

    SIMDMemCopy(vb.DataPtr, VertexData, BufferSize >> 4);

    D3D12_VERTEX_BUFFER_VIEW VBView;
    VBView.BufferLocation = vb.GpuAddress;
    VBView.SizeInBytes = (UINT)BufferSize;
    VBView.StrideInBytes = (UINT)VertexStride;

    GetCommandList()->IASetVertexBuffers(Slot, 1, &VBView);
}

inline void CGraphicsContext::SetDynamicIB( size_t IndexCount, const uint16_t* IndexData )
{
    ASSERT(IndexData != nullptr && CMath::IsAligned(IndexData, 16));

    size_t BufferSize = CMath::AlignUp(IndexCount * sizeof(uint16_t), 16);
    SDynAlloc ib = m_CpuLinearAllocator.Allocate(BufferSize);

    SIMDMemCopy(ib.DataPtr, IndexData, BufferSize >> 4);

    D3D12_INDEX_BUFFER_VIEW IBView;
    IBView.BufferLocation = ib.GpuAddress;
    IBView.SizeInBytes = (UINT)(IndexCount * sizeof(uint16_t));
    IBView.Format = DXGI_FORMAT_R16_UINT;

    GetCommandList()->IASetIndexBuffer(&IBView);
}

inline void CGraphicsContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData)
{
    ASSERT(BufferData != nullptr && CMath::IsAligned(BufferData, 16));
    SDynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
    SIMDMemCopy(cb.DataPtr, BufferData, CMath::AlignUp(BufferSize, 16) >> 4);
    GetCommandList()->SetGraphicsRootShaderResourceView(RootIndex, cb.GpuAddress);
}

inline void CComputeContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData)
{
    ASSERT(BufferData != nullptr && CMath::IsAligned(BufferData, 16));
    SDynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
    SIMDMemCopy(cb.DataPtr, BufferData, CMath::AlignUp(BufferSize, 16) >> 4);
    GetCommandList()->SetComputeRootShaderResourceView(RootIndex, cb.GpuAddress);
}

inline void CGraphicsContext::SetBufferSRV( UINT RootIndex, const CGPUBuffer& SRV, UINT64 Offset)
{
    ASSERT((SRV.m_UsageState & (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) != 0);
    GetCommandList()->SetGraphicsRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
}

inline void CComputeContext::SetBufferSRV( UINT RootIndex, const CGPUBuffer& SRV, UINT64 Offset)
{
    ASSERT((SRV.m_UsageState & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) != 0);
    GetCommandList()->SetComputeRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
}

inline void CGraphicsContext::SetBufferUAV( UINT RootIndex, const CGPUBuffer& UAV, UINT64 Offset)
{
    ASSERT((UAV.m_UsageState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
    GetCommandList()->SetGraphicsRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
}

inline void CComputeContext::SetBufferUAV( UINT RootIndex, const CGPUBuffer& UAV, UINT64 Offset)
{
    ASSERT((UAV.m_UsageState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
    GetCommandList()->SetComputeRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
}

inline void CComputeContext::Dispatch( size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ )
{
    FlushResourceBarriers();
    m_DynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(GetCommandList());
    m_DynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(GetCommandList());
    GetCommandList()->Dispatch((UINT)GroupCountX, (UINT)GroupCountY, (UINT)GroupCountZ);
}

inline void CComputeContext::Dispatch1D( size_t ThreadCountX, size_t GroupSizeX )
{
    Dispatch( CMath::DivideByMultiple(ThreadCountX, GroupSizeX), 1, 1 );
}

inline void CComputeContext::Dispatch2D( size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX, size_t GroupSizeY )
{
    Dispatch(
        CMath::DivideByMultiple(ThreadCountX, GroupSizeX),
        CMath::DivideByMultiple(ThreadCountY, GroupSizeY), 1);
}

inline void CComputeContext::Dispatch3D( size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ )
{
    Dispatch(
        CMath::DivideByMultiple(ThreadCountX, GroupSizeX),
        CMath::DivideByMultiple(ThreadCountY, GroupSizeY),
        CMath::DivideByMultiple(ThreadCountZ, GroupSizeZ));
}

inline void CCommandContext::SetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr )
{
    if (m_CurrentDescriptorHeaps[Type] != HeapPtr)
    {
        m_CurrentDescriptorHeaps[Type] = HeapPtr;
        BindDescriptorHeaps();
    }
}

inline void CCommandContext::SetDescriptorHeaps( UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[] )
{
    bool AnyChanged = false;

    for (UINT i = 0; i < HeapCount; ++i)
    {
        if (m_CurrentDescriptorHeaps[Type[i]] != HeapPtrs[i])
        {
            m_CurrentDescriptorHeaps[Type[i]] = HeapPtrs[i];
            AnyChanged = true;
        }
    }

    if (AnyChanged)
        BindDescriptorHeaps();
}

inline void CCommandContext::SetPredication(ID3D12Resource* Buffer, UINT64 BufferOffset, D3D12_PREDICATION_OP Op)
{
    GetCommandList()->SetPredication(Buffer, BufferOffset, Op);
}

inline void CCommandContext::SetScissor(const D3D12_RECT& rect)
{
    m_CommandList->RSSetScissorRects(1, &rect);
}

inline void CCommandContext::SetViewport(const D3D12_VIEWPORT& viewport)
{
    m_CommandList->RSSetViewports(1, &viewport);
}

inline void CGraphicsContext::SetDynamicDescriptor( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

inline void CComputeContext::SetDynamicDescriptor( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

inline void CGraphicsContext::SetDynamicDescriptors( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
    m_DynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

inline void CComputeContext::SetDynamicDescriptors( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
    m_DynamicViewDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
}

inline void CGraphicsContext::SetDynamicSampler( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
}

inline void CGraphicsContext::SetDynamicSamplers( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
    m_DynamicSamplerDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

inline void CComputeContext::SetDynamicSampler( UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle )
{
    SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
}

inline void CComputeContext::SetDynamicSamplers( UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[] )
{
    m_DynamicSamplerDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
}

inline void CGraphicsContext::SetDescriptorTable( UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle )
{
    GetCommandList()->SetGraphicsRootDescriptorTable( RootIndex, FirstHandle );
}

inline void CComputeContext::SetDescriptorTable( UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle )
{
    GetCommandList()->SetComputeRootDescriptorTable( RootIndex, FirstHandle );
}

inline void CGraphicsContext::SetIndexBuffer( const D3D12_INDEX_BUFFER_VIEW& IBView )
{
    GetCommandList()->IASetIndexBuffer(&IBView);
}

inline void CGraphicsContext::SetVertexBuffer( UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView )
{
    SetVertexBuffers(Slot, 1, &VBView);
}

inline void CGraphicsContext::SetVertexBuffers( UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[] )
{
    GetCommandList()->IASetVertexBuffers(StartSlot, Count, VBViews);
}

inline void CGraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset)
{
    DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}

inline void CGraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
    DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
}

inline void CGraphicsContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
    UINT StartVertexLocation, UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    GetCommandList()->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

inline void CGraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
    INT BaseVertexLocation, UINT StartInstanceLocation)
{
    FlushResourceBarriers();
    m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    GetCommandList()->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

inline void CGraphicsContext::ExecuteIndirect(CCommandSignature& CommandSig,
    CGPUBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset,
    uint32_t MaxCommands, CGPUBuffer* CommandCounterBuffer, uint64_t CounterOffset)
{
    FlushResourceBarriers();
    m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(GetCommandList());
    GetCommandList()->ExecuteIndirect(CommandSig.GetSignature(), MaxCommands,
        ArgumentBuffer.GetResource(), ArgumentStartOffset,
        CommandCounterBuffer == nullptr ? nullptr : CommandCounterBuffer->GetResource(), CounterOffset);
}


inline void CGraphicsContext::DrawIndirect(CGPUBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset)
{
    ExecuteIndirect(CGraphics::DrawIndirectCommandSignature, ArgumentBuffer, ArgumentBufferOffset);
}

inline void CComputeContext::ExecuteIndirect(CCommandSignature& CommandSig,
    CGPUBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset,
    uint32_t MaxCommands, CGPUBuffer* CommandCounterBuffer, uint64_t CounterOffset)
{
    FlushResourceBarriers();
    m_DynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(GetCommandList());
    m_DynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(GetCommandList());
    GetCommandList()->ExecuteIndirect(CommandSig.GetSignature(), MaxCommands,
        ArgumentBuffer.GetResource(), ArgumentStartOffset,
        CommandCounterBuffer == nullptr ? nullptr : CommandCounterBuffer->GetResource(), CounterOffset);
}

inline void CComputeContext::DispatchIndirect( CGPUBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset )
{
    ExecuteIndirect(CGraphics::DispatchIndirectCommandSignature, ArgumentBuffer, ArgumentBufferOffset);
}

inline void CCommandContext::CopyBuffer( CGPUResource& Dest, CGPUResource& Src )
{
    TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();
    GetCommandList()->CopyResource(Dest.GetResource(), Src.GetResource());
}

inline void CCommandContext::CopyBufferRegion( CGPUResource& Dest, size_t DestOffset, CGPUResource& Src, size_t SrcOffset, size_t NumBytes )
{
    TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
    //TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();
    GetCommandList()->CopyBufferRegion( Dest.GetResource(), DestOffset, Src.GetResource(), SrcOffset, NumBytes);
}

inline void CCommandContext::CopyCounter(CGPUResource& Dest, size_t DestOffset, CStructuredBuffer& Src)
{
    TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(Src.GetCounterBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();
    GetCommandList()->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetCounterBuffer().GetResource(), 0, 4);
}

inline void CCommandContext::CopyTextureRegion(CGPUResource& Dest, UINT x, UINT y, UINT z, CGPUResource& Source, RECT& Rect)
{
    TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(Source, D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();

    D3D12_TEXTURE_COPY_LOCATION destLoc = CD3DX12_TEXTURE_COPY_LOCATION(Dest.GetResource(), 0);
    D3D12_TEXTURE_COPY_LOCATION srcLoc = CD3DX12_TEXTURE_COPY_LOCATION(Source.GetResource(), 0);

    D3D12_BOX box = {};
    box.back = 1;
    box.left = Rect.left;
    box.right = Rect.right;
    box.top = Rect.top;
    box.bottom = Rect.bottom;

    GetCommandList()->CopyTextureRegion(&destLoc, x, y, z, &srcLoc, &box);
}

inline void CCommandContext::ResetCounter(CStructuredBuffer& Buf, uint32_t Value )
{
    FillBuffer(Buf.GetCounterBuffer(), 0, Value, sizeof(uint32_t));
    TransitionResource(Buf.GetCounterBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

inline void CCommandContext::InsertTimeStamp(ID3D12QueryHeap* pQueryHeap, uint32_t QueryIdx)
{
    GetCommandList()->EndQuery(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
}

inline void CCommandContext::ResolveTimeStamps(ID3D12Resource* pReadbackHeap, ID3D12QueryHeap* pQueryHeap, uint32_t NumQueries)
{
    GetCommandList()->ResolveQueryData(pQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, pReadbackHeap, 0);
}
