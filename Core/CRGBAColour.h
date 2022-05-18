
#pragma once

#include <DirectXMath.h>

using namespace DirectX;

/// <summary>
/// Colour is a huge part of rendering, CMath contains CRGBAColour, this is a class that takes the values 
/// of FXMVector and specializes them for rendering out four channel colour.
/// </summary>
class CRGBAColour
{
public:
    CRGBAColour( ) : m_value(g_XMOne) {}
    CRGBAColour( FXMVECTOR vec );
    CRGBAColour( const XMVECTORF32& vec );
    CRGBAColour( float r, float g, float b, float a = 1.0f );
    CRGBAColour( uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8 );
    explicit CRGBAColour( uint32_t rgbaLittleEndian );
        
    float R() const { return XMVectorGetX(m_value); }
    float G() const { return XMVectorGetY(m_value); }
    float B() const { return XMVectorGetZ(m_value); }
    float A() const { return XMVectorGetW(m_value); }

    bool operator==( const CRGBAColour& rhs ) const { return XMVector4Equal(m_value, rhs.m_value); }
    bool operator!=( const CRGBAColour& rhs ) const { return !XMVector4Equal(m_value, rhs.m_value); }

    void SetR( float r ) { m_value.f[0] = r; }
    void SetG( float g ) { m_value.f[1] = g; }
    void SetB( float b ) { m_value.f[2] = b; }
    void SetA( float a ) { m_value.f[3] = a; }

    float* GetPtr( void ) { return reinterpret_cast<float*>(this); }
    float& operator[]( int idx ) { return GetPtr()[idx]; }

    void SetRGB( float r, float g, float b ) { m_value.v = XMVectorSelect( m_value, XMVectorSet(r, g, b, b), g_XMMask3 ); }

    CRGBAColour ToSRGB() const;
    CRGBAColour FromSRGB() const;
    CRGBAColour ToREC709() const;
    CRGBAColour FromREC709() const;

    // Probably want to convert to sRGB or Rec709 first
    uint32_t R10G10B10A2() const;
    uint32_t R8G8B8A8() const;

    // Pack an HDR color into 32-bits
    uint32_t R11G11B10F(bool RoundToEven=false) const;
    uint32_t R9G9B9E5() const;

    operator XMVECTOR() const { return m_value; }

private:
    XMVECTORF32 m_value;
};

_forceinline CRGBAColour Max(CRGBAColour a, CRGBAColour b) { return CRGBAColour(XMVectorMax(a, b)); }
_forceinline CRGBAColour Min(CRGBAColour a, CRGBAColour b) { return CRGBAColour(XMVectorMin(a, b)); }
_forceinline CRGBAColour Clamp(CRGBAColour x, CRGBAColour a, CRGBAColour b) { return CRGBAColour(XMVectorClamp(x, a, b)); }


inline CRGBAColour::CRGBAColour( FXMVECTOR vec )
{
    m_value.v = vec;
}

inline CRGBAColour::CRGBAColour( const XMVECTORF32& vec )
{
    m_value = vec;
}

inline CRGBAColour::CRGBAColour( float r, float g, float b, float a )
{
    m_value.v = XMVectorSet(r, g, b, a);
}

inline CRGBAColour::CRGBAColour( uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth )
{
    m_value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / ((1 << bitDepth) - 1));
}

inline CRGBAColour::CRGBAColour( uint32_t u32 )
{
    float r = (float)((u32 >>  0) & 0xFF);
    float g = (float)((u32 >>  8) & 0xFF);
    float b = (float)((u32 >> 16) & 0xFF);
    float a = (float)((u32 >> 24) & 0xFF);
    m_value.v = XMVectorScale( XMVectorSet(r, g, b, a), 1.0f / 255.0f );
}

inline CRGBAColour CRGBAColour::ToSRGB( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
    result = XMVectorSelect(result, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline CRGBAColour CRGBAColour::FromSRGB( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
    result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline CRGBAColour CRGBAColour::ToREC709( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(0.45f)), 1.099f), XMVectorReplicate(0.099f));
    result = XMVectorSelect(result, XMVectorScale(T, 4.5f), XMVectorLess(T, XMVectorReplicate(0.0018f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline CRGBAColour CRGBAColour::FromREC709( void ) const
{
    XMVECTOR T = XMVectorSaturate(m_value);
    XMVECTOR result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.099f)), 1.0f / 1.099f), XMVectorReplicate(1.0f / 0.45f));
    result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 4.5f), XMVectorLess(T, XMVectorReplicate(0.0081f)));
    return XMVectorSelect(T, result, g_XMSelect1110);
}

inline uint32_t CRGBAColour::R10G10B10A2( void ) const
{
    XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value), XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
    result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
    uint32_t r = XMVectorGetIntX(result);
    uint32_t g = XMVectorGetIntY(result);
    uint32_t b = XMVectorGetIntZ(result);
    uint32_t a = XMVectorGetIntW(result) >> 8;
    return a << 30 | b << 20 | g << 10 | r;
}

inline uint32_t CRGBAColour::R8G8B8A8( void ) const
{
    XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value), XMVectorReplicate(255.0f)));
    result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
    uint32_t r = XMVectorGetIntX(result);
    uint32_t g = XMVectorGetIntY(result);
    uint32_t b = XMVectorGetIntZ(result);
    uint32_t a = XMVectorGetIntW(result);
    return a << 24 | b << 16 | g << 8 | r;
}
