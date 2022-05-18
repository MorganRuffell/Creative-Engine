
#include "pch.h"
#include "CTextureManager.h"
#include "DDSTextureLoader.h"
#include "CTexture.h"
#include "CUtility.h"
#include "FileUtility.h"
#include "GraphicsCommon.h"
#include "CCommandContext.h"
#include <map>
#include <thread>

using namespace std;
using namespace CGraphics;
using CUtility::CSmartByteVector;

//
// A ManagedTexture allows for multiple threads to request a Texture load of the same
// file.  It also contains a reference count of the Texture so that it can be freed
// when it is no longer referenced.
//
// Raw ManagedTexture pointers are not exposed to anyone other than engine devs.  
//
class CManagedTexture : public CTexture
{
    friend class CTextureHandle;

public:
    CManagedTexture( const wstring& FileName );

    void WaitForLoad(void) const;
    void CreateFromMemory(CSmartByteVector memory, EDefaultTexture fallback, bool sRGB);

private:

    bool IsValid(void) const { return m_IsValid; }
    void Unload();

    std::wstring m_MapKey;		// For deleting from the map later
    bool m_IsValid;
    bool m_IsLoading;
    size_t m_ReferenceCount;
};

namespace CTextureManager
{
    wstring s_RootPath = L"";
    map<wstring, std::unique_ptr<CManagedTexture>> s_TextureCache;

    void Initialize( const wstring& TextureLibRoot )
    {
        s_RootPath = TextureLibRoot;
    }

    void Shutdown( void )
    {
        s_TextureCache.clear();
    }

    mutex s_Mutex;

    CManagedTexture* FindOrLoadTexture( const wstring& fileName, EDefaultTexture fallback, bool forceSRGB )
    {
        CManagedTexture* tex = nullptr;

        {
            lock_guard<mutex> Guard(s_Mutex);

            wstring key = fileName;
            if (forceSRGB)
                key += L"_sRGB";

            // Search for an existing managed texture
            auto iter = s_TextureCache.find(key);
            if (iter != s_TextureCache.end())
            {
                // If a texture was already created make sure it has finished loading before
                // returning a point to it.
                tex = iter->second.get();
                tex->WaitForLoad();
                return tex;
            }
            else
            {
                // If it's not found, create a new managed texture and start loading it
                tex = new CManagedTexture(key);
                s_TextureCache[key].reset(tex);
            }
        }

        CUtility::CSmartByteVector ba = CUtility::ReadFileSync( s_RootPath + fileName );
        tex->CreateFromMemory(ba, fallback, forceSRGB);

        // This was the first time it was requested, so indicate that the caller must read the file
        return tex;
    }

    void DestroyTexture(const wstring& key)
    {
        lock_guard<mutex> Guard(s_Mutex);

        auto iter = s_TextureCache.find(key);
        if (iter != s_TextureCache.end())
            s_TextureCache.erase(iter);
    }

} 

CManagedTexture::CManagedTexture( const wstring& FileName )
    : m_MapKey(FileName), m_IsValid(false), m_IsLoading(true), m_ReferenceCount(0)
{
    m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}

void CManagedTexture::CreateFromMemory(CSmartByteVector ba, EDefaultTexture fallback, bool forceSRGB)
{
    if (ba->size() == 0)
    {
        m_hCpuDescriptorHandle = GetDefaultTexture(fallback);
    }
    else
    {
        // We probably have a texture to load, so let's allocate a new descriptor
        m_hCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        if ( SUCCEEDED( CreateDDSTextureFromMemory( g_Device, (const uint8_t*)ba->data(), ba->size(),
            0, forceSRGB, m_pResource.GetAddressOf(), m_hCpuDescriptorHandle) ) )
        {
            m_IsValid = true;
            D3D12_RESOURCE_DESC desc = GetResource()->GetDesc();
            m_Width = (uint32_t)desc.Width;
            m_Height = desc.Height;
            m_Depth = desc.DepthOrArraySize;
        }
        else
        {
            g_Device->CopyDescriptorsSimple(1, m_hCpuDescriptorHandle, GetDefaultTexture(fallback),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
    }

    m_IsLoading = false;
}

void CManagedTexture::WaitForLoad( void ) const
{
    while ((volatile bool&)m_IsLoading)
        this_thread::yield();
}

void CManagedTexture::Unload()
{
    CTextureManager::DestroyTexture(m_MapKey);
}

CTextureHandle::CTextureHandle( const CTextureHandle& ref ) : m_ref(ref.m_ref)
{
    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

CTextureHandle::CTextureHandle( CManagedTexture* tex ) : m_ref(tex)
{
    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

CTextureHandle::~CTextureHandle()
{
    if (m_ref != nullptr && --m_ref->m_ReferenceCount == 0)
        m_ref->Unload();
}

void CTextureHandle::operator= (std::nullptr_t)
{
    if (m_ref != nullptr)
        --m_ref->m_ReferenceCount;

    m_ref = nullptr;
}

void CTextureHandle::operator= (CTextureHandle& rhs)
{
    if (m_ref != nullptr)
        --m_ref->m_ReferenceCount;

    m_ref = rhs.m_ref;

    if (m_ref != nullptr)
        ++m_ref->m_ReferenceCount;
}

bool CTextureHandle::IsValid() const
{
    return m_ref && m_ref->IsValid();
}

const CTexture* CTextureHandle::Get( void ) const
{
    return m_ref;
}

const CTexture* CTextureHandle::operator->( void ) const
{
    ASSERT(m_ref != nullptr);
    return m_ref;
}

D3D12_CPU_DESCRIPTOR_HANDLE CTextureHandle::GetSRV() const
{
    if (m_ref != nullptr)
        return m_ref->GetSRV();
    else
        return GetDefaultTexture(kMagenta2D);
}


CTextureHandle CTextureManager::LoadDDSFromFile( const wstring& filePath, EDefaultTexture fallback, bool forceSRGB )
{
    return FindOrLoadTexture(filePath, fallback, forceSRGB);
}

CTextureHandle CTextureManager::LoadDDSFromFile( const string& filePath, EDefaultTexture fallback, bool forceSRGB )
{
    return LoadDDSFromFile(CUtility::UTF8ToWideString(filePath), fallback, forceSRGB);
}
