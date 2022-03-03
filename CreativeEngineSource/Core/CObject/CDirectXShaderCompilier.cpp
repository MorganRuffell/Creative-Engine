#include "CDirectXShaderCompilier.h"
#include <dxcapi.h>
#include <wrl.h>
#include <wrl/client.h>


using Microsoft::WRL::ComPtr;


HRESULT CDirectXShaderCompilier::CompileShaders()
{
    if (SUCCEEDED(CompileVertexShaders()))
    {
        if (SUCCEEDED(CompilePixelShaders()))
        {

        }

        if (SUCCEEDED(CompileGeometryShaders()))
        {

        }

        if (SUCCEEDED(CompileDomainShaders()))
        {

        }

        if (SUCCEEDED(CompileHullShaders()))
        {

        }
    }
    else
    {
        return E_FAIL;
    }
   

    return E_FAIL;
}

HRESULT CDirectXShaderCompilier::CompilePixelShaders()
{
    ID3DBlob* SamplePixelShaderBlob;
    HRESULT SampleRes = LegacyCompileFile(L"SamplePixelShader.hlsl", "main", "ps_5_1", &SamplePixelShaderBlob);

    D3D12_SHADER_BYTECODE SamplePixelShaderByteCode = {};
    SamplePixelShaderByteCode.BytecodeLength = SamplePixelShaderBlob->GetBufferSize();
    SamplePixelShaderByteCode.pShaderBytecode =  SamplePixelShaderBlob->GetBufferPointer();

    ID3DBlob* WireframePixelShaderBlob;
    HRESULT WireframeRes = LegacyCompileFile(L"WireframPS.hlsl", "PSMain", "ps_5_1", &WireframePixelShaderBlob);

    D3D12_SHADER_BYTECODE WireframeVSByteCode = {};
    WireframeVSByteCode.BytecodeLength = WireframePixelShaderBlob->GetBufferSize();
    WireframeVSByteCode.pShaderBytecode = WireframePixelShaderBlob->GetBufferPointer();

}

HRESULT CDirectXShaderCompilier::CompileVertexShaders()
{
    ID3DBlob* SampleVertexShaderBlob;
    HRESULT SampleRes = LegacyCompileFile(L"SampleVertexShader.hlsl", "main", "vs_5_1", &SampleVertexShaderBlob);

    D3D12_SHADER_BYTECODE SampleVertexShadeByteCode = {};
    SampleVertexShadeByteCode.BytecodeLength = SampleVertexShaderBlob->GetBufferSize();
    SampleVertexShadeByteCode.pShaderBytecode = SampleVertexShaderBlob->GetBufferPointer();

    ID3DBlob* WireframeVertexShaderBlob;
    HRESULT WireframeRes = LegacyCompileFile(L"WireframeVS.hlsl", "main", "vs_5_1", &WireframeVertexShaderBlob);

    D3D12_SHADER_BYTECODE WireframeVertexShaderBytecode;
    WireframeVertexShaderBytecode.BytecodeLength = WireframeVertexShaderBlob->GetBufferSize();
    WireframeVertexShaderBytecode.pShaderBytecode = WireframeVertexShaderBlob->GetBufferPointer();
}

HRESULT CDirectXShaderCompilier::CompileGeometryShaders()
{
    ID3DBlob* SampleGeometryShaderBlob;
    HRESULT SampleRes = LegacyCompileFile(L"SameplGeometryShader.hlsl", "main", "gs_5_1", &SampleGeometryShaderBlob);

    D3D12_SHADER_BYTECODE SampleGeometryShaderByteCode = {};
    SampleGeometryShaderByteCode.BytecodeLength = SampleGeometryShaderBlob->GetBufferSize();
    SampleGeometryShaderByteCode.pShaderBytecode = SampleGeometryShaderBlob->GetBufferPointer();

    ID3DBlob* WireframeGeometryShaderBlob;
    HRESULT WireframeRes = LegacyCompileFile(L"WireframeGS.hlsl", "GSMain", "gs_5_1", &WireframeGeometryShaderBlob);

    D3D12_SHADER_BYTECODE WireframeGeometryShaderByteCode;
    WireframeGeometryShaderByteCode.BytecodeLength = WireframeGeometryShaderBlob->GetBufferSize();
    WireframeGeometryShaderByteCode.pShaderBytecode = WireframeGeometryShaderBlob->GetBufferPointer();
}

HRESULT CDirectXShaderCompilier::CompileDomainShaders()
{
    ID3DBlob* SampleDomainShaderBlob;
    HRESULT SampleRes = LegacyCompileFile(L"SampleDomainShader.hlsl", "main", "ds_5_1", &SampleDomainShaderBlob);

    D3D12_SHADER_BYTECODE SampleDomainShaderByteCode;
    SampleDomainShaderByteCode.BytecodeLength = SampleDomainShaderBlob->GetBufferSize();
    SampleDomainShaderByteCode.pShaderBytecode = SampleDomainShaderBlob->GetBufferPointer();
}

HRESULT CDirectXShaderCompilier::CompileHullShaders()
{
    ID3DBlob* SampleHullShaderBlob;
    HRESULT SampleRes = LegacyCompileFile(L"SampleHullShader.hlsl", "main", "hs_5_1", &SampleHullShaderBlob);

    D3D12_SHADER_BYTECODE SampleHullShaderByteCode;
    SampleHullShaderByteCode.BytecodeLength = SampleHullShaderBlob->GetBufferSize();
    SampleHullShaderByteCode.pShaderBytecode = SampleHullShaderBlob->GetBufferPointer();
}

HRESULT CDirectXShaderCompilier::CompileComputeShaders()
{
    ID3DBlob* MipMapsComputeShaderBlob;
    HRESULT ComputeShaderMipRes = CompileComputeShader(L"CComputeShader_Mips.hlsl", "main", "cs_6_0", &MipMapsComputeShaderBlob);

    D3D12_SHADER_BYTECODE MipmapShaderByteCode;
    MipmapShaderByteCode.BytecodeLength = MipMapsComputeShaderBlob->GetBufferSize();
    MipmapShaderByteCode.pShaderBytecode = MipMapsComputeShaderBlob->GetBufferPointer();

}

HRESULT CDirectXShaderCompilier::CompileNearMissShaders()
{
    return E_NOTIMPL;
}

HRESULT CDirectXShaderCompilier::CompileAnyHitShaders()
{
    return E_NOTIMPL;
}

HRESULT CDirectXShaderCompilier::CompileIntersectionShaders()
{
    return E_NOTIMPL;
}

HRESULT CDirectXShaderCompilier::CompileClosestHitShaders()
{
    return E_NOTIMPL;
}

HRESULT CDirectXShaderCompilier::LegacyCompileFile(LPCWSTR SourceFileName, LPCSTR EntryPoint, LPCSTR Profile, ID3DBlob** BinaryObject)
{
    if (!SourceFileName || !EntryPoint || !Profile || !BinaryObject)
        return E_INVALIDARG;

    //We have to remove the reference before we start compiling this from file.
    *BinaryObject = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ID3DBlob* ShaderBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    HRESULT res = D3DCompileFromFile(SourceFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, Profile, flags, 0, &ShaderBlob, &ErrorBlob);

    if (FAILED(res))
    {
        if (ErrorBlob)
        {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }

        if (ShaderBlob)
            ShaderBlob->Release();

        return res;
    }
}

HRESULT CDirectXShaderCompilier::CompileComputeShader(LPCWSTR SourceFileName, LPCSTR EntryPoint, LPCSTR Profile, ID3DBlob** BinaryObject)
{
    if (!SourceFileName || !EntryPoint  || !BinaryObject)
        return E_INVALIDARG;

    //We have to remove the reference before we start compiling this from file.
    *BinaryObject = nullptr;

    ID3DBlob* ShaderBlob = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    HRESULT res = D3DCompileFromFile(SourceFileName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, "cs_5_0", 0, 0, &ShaderBlob, &ErrorBlob);

    if (FAILED(res))
    {
        if (ErrorBlob)
        {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }

        if (ShaderBlob)
            ShaderBlob->Release();

        return res;
    }

    *BinaryObject = ShaderBlob;
}

HRESULT CDirectXShaderCompilier::CompileFile(LPCWSTR SourceFileName, LPCSTR EntryPoint, LPCSTR Profile, ID3DBlob** BinaryObject)
{
    ComPtr<IDxcLibrary> Library;
    ComPtr<IDxcCompiler> ShaderCompilier;
    ComPtr<IDxcBlobEncoding> sourceBlob;

    uint32_t codePage = CP_UTF8;

    HRESULT res = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
    HRESULT res2 = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&ShaderCompilier));

    //Fix Error handling in here at some point
    // look here to finish https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12

    return E_NOTIMPL;
}

