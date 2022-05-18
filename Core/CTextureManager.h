#pragma once

#include "pch.h"
#include "CGPUBaseResource.h"
#include "CUtility.h"
#include "CTexture.h"
#include "GraphicsCommon.h"

// A referenced-counted pointer to a Texture.  See methods below.
class CTextureHandle;

namespace CTextureManager
{
    using CGraphics::EDefaultTexture;
    using CGraphics::kMagenta2D;

    void Initialize( const std::wstring& RootPath );
    void Shutdown(void);

    // Load a texture from a DDS file.  Never returns null references, but if a 
    // texture cannot be found, ref->IsValid() will return false.
    CTextureHandle LoadDDSFromFile( const std::wstring& filePath, EDefaultTexture fallback = kMagenta2D, bool sRGB = false );
    CTextureHandle LoadDDSFromFile( const std::string& filePath, EDefaultTexture fallback = kMagenta2D, bool sRGB = false );
}

// Forward declaration; private implementation
class CManagedTexture;


class CTextureHandle
{
public:

    CTextureHandle( const CTextureHandle& ref );
    CTextureHandle( CManagedTexture* tex = nullptr );
    ~CTextureHandle();

    void operator= (std::nullptr_t);
    void operator= (CTextureHandle& rhs);

    bool IsValid() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

    const CTexture* Get( void ) const;

    const CTexture* operator->( void ) const;

private:
    CManagedTexture* m_ref;
};
