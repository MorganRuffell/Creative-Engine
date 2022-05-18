

#pragma once

#include "pch.h"
#include "CGPUBaseResource.h"

/// <summary>
/// Backing class for all textures, similar to the unreal texture2D
/// </summary>

enum class CETextureWrapmode
{
    Repeat,
    Clamp
};

enum class CETextureFilterMode
{
    Nearest,
    Bilinear,
    Trilinear,
    Anisotropic
};

enum class CESupportedTypes
{
    tga,
    dds,
    PIX,
};


class CTexture : public CGPUResource
{
    friend class CCommandContext;

public:

    CTexture() { m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
    CTexture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) : m_hCpuDescriptorHandle(Handle) {}

    // Create a 1-level textures
    void Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData );
    void CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData);

public:

    void CreateTGAFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
    bool CreateDDSFromMemory( const void* memBuffer, size_t fileSize, bool sRGB );
    void CreatePIXImageFromMemory( const void* memBuffer, size_t fileSize );

    void SetTextureWrapMode(CETextureWrapmode& Wrapmode)
    {
        m_WrapMode = Wrapmode;
    }

    CETextureWrapmode GetWrapMode()
    {
        return m_WrapMode;
    }

private:

    CETextureFilterMode     m_FilterMode;
    CETextureWrapmode       m_WrapMode;

public:

    virtual void Destroy() override
    {
        CGPUResource::Destroy();
        m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_hCpuDescriptorHandle; }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetDepth() const { return m_Depth; }

public:

    std::string m_MaterialName;
    std::wstring m_FileName;

private:

    bool    m_HasMultipleLayers;
    bool    m_HasMultipleChannels;

protected:

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
    uint32_t m_Layers;

    D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
};
