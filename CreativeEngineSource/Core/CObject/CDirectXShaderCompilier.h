#pragma once
#include <d3dcompiler.h>
#include <vector>
#include <thread>
#include <mutex>
#include <d3d12.h>

struct CShaderData
{
    //Standard Pipeline Shaders
    std::vector<std::vector<byte>> VertexShaders;
    std::vector<std::vector<byte>> PixelShaders;
    std::vector<std::vector<byte>> DomainShaders;
    std::vector<std::vector<byte>> GeometryShaders;
    std::vector<std::vector<byte>> HullShaders;

    //Compute Shaders
    std::vector<std::vector<byte>> ComputeShaders;

    //Data Shaders
    std::vector<std::vector<byte>> DataShaders;

    //Raytracing Shaders
    std::vector<std::vector<byte>> AnyHitShaders;
    std::vector<std::vector<byte>> MissShaders;
    std::vector<std::vector<byte>> ClosestHitShaders;
    std::vector<std::vector<byte>> IntersectionShaders;
};


class CDirectXShaderCompilier
{
public:

    std::string SupportedShaderProfiles[30] = {
        "cs_4_0",
        "gs_4_0",
        "ps_4_0",
        "vs_4_0",

        "cs_4_1",
        "gs_4_1",
        "ps_4_1",
        "vs_4_1",
        "lib_4_0",
        "lib_4_1",

        "ps_5_0",
        "vs_5_0",
        "ds_5_0",
        "gs_5_0",
        "hs_5_0",
        "cs_5_0",

        "ps_5_1",
        "vs_5_1",
        "ds_5_1",
        "gs_5_1",
        "hs_5_1",
        "cs_5_1",

        "cs_6_0",
        "ds_6_0",
        "gs_6_0",
        "hs_6_0",
        "ps_6_0",
        "vs_6_0",
        "lib_6_0"
    };


    CDirectXShaderCompilier()
    {
        std::jthread ClearVertexArray = std::jthread([&] {

            for (auto VertexIterator = ShaderData->VertexShaders.begin(); VertexIterator != ShaderData->VertexShaders.end(); VertexIterator++)
            {
                VertexIterator->clear();
            }

        });

        std::jthread ClearPixelArray = std::jthread([&] {

            for (auto PixelIterator = ShaderData->PixelShaders.begin(); PixelIterator != ShaderData->PixelShaders.end(); PixelIterator++)
            {
                PixelIterator->clear();
            }

        });

        std::jthread ClearGeometryArray = std::jthread([&] {

            for (auto GeometryIterator = ShaderData->GeometryShaders.begin(); GeometryIterator != ShaderData->GeometryShaders.end(); GeometryIterator++)
            {
                GeometryIterator->clear();
            }
        });

        for (auto ComputeIterator = ShaderData->ComputeShaders.begin(); ComputeIterator != ShaderData->ComputeShaders.end(); ComputeIterator++)
        {
            ComputeIterator->clear();
        }

        ClearVertexArray.join();
        ClearPixelArray.join();
        ClearGeometryArray.join();

        CompileShaders();
    }

    ~CDirectXShaderCompilier()
    {
        // Something to clean up memory later? -- Deallocating all of the vectors with binaries.
    }

protected:

    /// <summary>
    /// Global Function to compile all shaders
    /// </summary>
    /// <returns></returns>
    HRESULT CompileShaders();

private:


    /// <summary>
    /// Used for compiling all pipeline shaders except for the compute shaders
    /// </summary>
    /// <returns></returns>
    HRESULT CompilePixelShaders();
    HRESULT CompileVertexShaders();
    HRESULT CompileDomainShaders();
    HRESULT CompileGeometryShaders();
    HRESULT CompileHullShaders();


    HRESULT CompileComputeShaders();

    /// <summary>
    /// Used for compiling all raytracing shaders, not in use yet
    /// </summary>
    /// <returns></returns>

    HRESULT CompileNearMissShaders();
    HRESULT CompileAnyHitShaders();
    HRESULT CompileIntersectionShaders();
    HRESULT CompileClosestHitShaders();

protected:

    /// <summary>
    /// This function is used to compile an individual shader -- This uses the legacy API, The other utilizes the library functions inside C++
    /// </summary>
    /// <param name="SourceFileName"> This is the source file name </param>
    /// <param name="EntryPoint"> This is the entry point </param>
    /// <param name="Profile"> This is the profile that is required, with our shaders as we are using #includes its imperative that we use 'D3D_COMPILE_STANDARD_FILE_INCLUDE '</param>
    /// <param name="BinaryObject"> An ID3DBlob which is the binary object</param>
    /// <returns></returns>
    HRESULT LegacyCompileFile(_In_ LPCWSTR SourceFileName, _In_ LPCSTR EntryPoint, _In_ LPCSTR Profile, _Outptr_ ID3DBlob** BinaryObject);


    /// <summary>
    /// This function is specialized to compile compute shaders
    /// </summary>
    /// <param name="srcFile"> This is the source file name </param>
    /// <param name="entryPoint"> This is the entry point </param>
    /// <param name="device"> This is the device that we are using to compile the compute shader </param>
    /// <param name="blob"> This is a binary large object that we are storing the Shader inside of </param>
    /// <returns></returns>
    HRESULT CompileComputeShader(_In_ LPCWSTR SourceFileName, _In_ LPCSTR EntryPoint, _In_ LPCSTR Profile, _Outptr_ ID3DBlob** BinaryObject);

protected:


    HRESULT CompileFile(_In_ LPCWSTR SourceFileName, _In_ LPCSTR EntryPoint, _In_ LPCSTR Profile, _Outptr_ ID3DBlob** BinaryObject);



public:

    //Accessors -- These are used by external classes when we want to know how many shaders have compiled.
    int GetTotalAmountOfShaders()
    {
        return m_TotalAmountOfShaders;
    }
    
    int GetTotalAmountOfVertexShaders()
    {
        return m_AmountOfVertexShaders;
    }

    int GetTotalAmountOfPixelShaders()
    {
        return m_AmountOfPixelShaders;
    }

    int GetTotalAmountOfGeometryShaders()
    {
        return m_AmountOfGeometryShaders;
    }

private:

    int m_TotalAmountOfShaders;

    int m_AmountOfVertexShaders;

    int m_AmountOfPixelShaders;

    int m_AmountOfDomainShaders;

    int m_AmountOfGeometryShaders;

    int m_AmountOfHullShaders;

    int m_AmountOfComputeShaders;

    //Add in values for the raytracing shaders... We'll do this later...
    //Get to MVP

protected:

    CShaderData* ShaderData;
};

