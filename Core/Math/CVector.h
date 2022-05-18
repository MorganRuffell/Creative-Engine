#pragma once

#include "pch.h"
#include "CScalar.h"

namespace CMath
{
    class CVector4;

    // A 3-vector with an unspecified fourth component.  Depending on the context, the W can be 0 or 1, but both are implicit.
    // The actual value of the fourth component is undefined.
    class CVector3
    {
    public:

        INLINE CVector3() {}
        INLINE CVector3( float x, float y, float z ) { m_vec = XMVectorSet(x, y, z, z); }
        INLINE CVector3( const XMFLOAT3& v ) { m_vec = XMLoadFloat3(&v); }
        INLINE CVector3( const CVector3& v ) { m_vec = v; }
        INLINE CVector3( CScalar s ) { m_vec = s; }
        INLINE explicit CVector3( CVector4 vec );
        INLINE explicit CVector3( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit CVector3( EZeroTag ) { m_vec = SplatZero(); }
        INLINE explicit CVector3( EIdentityTag ) { m_vec = SplatOne(); }
        INLINE explicit CVector3( EXUnitVector ) { m_vec = CreateXUnitVector(); }
        INLINE explicit CVector3( EYUnitVector ) { m_vec = CreateYUnitVector(); }
        INLINE explicit CVector3( EZUnitVector ) { m_vec = CreateZUnitVector(); }

        INLINE operator XMVECTOR() const { return m_vec; }

        INLINE CScalar GetX() const { return CScalar(XMVectorSplatX(m_vec)); }
        INLINE CScalar GetY() const { return CScalar(XMVectorSplatY(m_vec)); }
        INLINE CScalar GetZ() const { return CScalar(XMVectorSplatZ(m_vec)); }

    public:

        INLINE void SetX( CScalar x ) { m_vec = XMVectorPermute<4,1,2,3>(m_vec, x); }
        INLINE void SetY( CScalar y ) { m_vec = XMVectorPermute<0,5,2,3>(m_vec, y); }
        INLINE void SetZ( CScalar z ) { m_vec = XMVectorPermute<0,1,6,3>(m_vec, z); }

    public:

        INLINE CVector3 operator- () const { return CVector3(XMVectorNegate(m_vec)); }
        INLINE CVector3 operator+ ( CVector3 v2 ) const { return CVector3(XMVectorAdd(m_vec, v2)); }
        INLINE CVector3 operator- ( CVector3 v2 ) const { return CVector3(XMVectorSubtract(m_vec, v2)); }
        INLINE CVector3 operator* ( CVector3 v2 ) const { return CVector3(XMVectorMultiply(m_vec, v2)); }
        INLINE CVector3 operator/ ( CVector3 v2 ) const { return CVector3(XMVectorDivide(m_vec, v2)); }
        INLINE CVector3 operator* ( CScalar  v2 ) const { return *this * CVector3(v2); }
        INLINE CVector3 operator/ ( CScalar  v2 ) const { return *this / CVector3(v2); }
        INLINE CVector3 operator* ( float  v2 ) const { return *this * CScalar(v2); }
        INLINE CVector3 operator/ ( float  v2 ) const { return *this / CScalar(v2); }

        INLINE CVector3& operator += ( CVector3 v ) { *this = *this + v; return *this; }
        INLINE CVector3& operator -= ( CVector3 v ) { *this = *this - v; return *this; }
        INLINE CVector3& operator *= ( CVector3 v ) { *this = *this * v; return *this; }
        INLINE CVector3& operator /= ( CVector3 v ) { *this = *this / v; return *this; }

        INLINE friend CVector3 operator* ( CScalar  v1, CVector3 v2 ) { return CVector3(v1) * v2; }
        INLINE friend CVector3 operator/ ( CScalar  v1, CVector3 v2 ) 	{ return CVector3(v1) / v2; }
        INLINE friend CVector3 operator* ( float   v1, CVector3 v2 ) { return CScalar(v1) * v2; }
        INLINE friend CVector3 operator/ ( float   v1, CVector3 v2 ) 	{ return CScalar(v1) / v2; }

    protected:
        XMVECTOR m_vec;
    };

    // A 4-vector, each component (X,Y,Z,W) is defined and will have a value.
    class CVector4
    {
    public:
        INLINE CVector4() {}
        INLINE CVector4( float x, float y, float z, float w ) { m_vec = XMVectorSet(x, y, z, w); }
        INLINE CVector4( const XMFLOAT4& v ) { m_vec = XMLoadFloat4(&v); }
        INLINE CVector4( CVector3 xyz, float w ) { m_vec = XMVectorSetW(xyz, w); }
        INLINE CVector4( const CVector4& v ) { m_vec = v; }
        INLINE CVector4( const CScalar& s ) { m_vec = s; }
        INLINE explicit CVector4( CVector3 xyz ) { m_vec = SetWToOne(xyz); }
        INLINE explicit CVector4( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit CVector4( EZeroTag ) { m_vec = SplatZero(); }
        INLINE explicit CVector4( EIdentityTag ) { m_vec = SplatOne(); }
        INLINE explicit CVector4( EXUnitVector	) { m_vec = CreateXUnitVector(); }
        INLINE explicit CVector4( EYUnitVector ) { m_vec = CreateYUnitVector(); }
        INLINE explicit CVector4( EZUnitVector ) { m_vec = CreateZUnitVector(); }
        INLINE explicit CVector4( EWUnitVector ) { m_vec = CreateWUnitVector(); }

        INLINE operator XMVECTOR() const { return m_vec; }

    public:

        INLINE CScalar GetX() const { return CScalar(XMVectorSplatX(m_vec)); }
        INLINE CScalar GetY() const { return CScalar(XMVectorSplatY(m_vec)); }
        INLINE CScalar GetZ() const { return CScalar(XMVectorSplatZ(m_vec)); }
        INLINE CScalar GetW() const { return CScalar(XMVectorSplatW(m_vec)); }
        INLINE void SetX( CScalar x ) { m_vec = XMVectorPermute<4,1,2,3>(m_vec, x); }
        INLINE void SetY( CScalar y ) { m_vec = XMVectorPermute<0,5,2,3>(m_vec, y); }
        INLINE void SetZ( CScalar z ) { m_vec = XMVectorPermute<0,1,6,3>(m_vec, z); }
        INLINE void SetW( CScalar w ) { m_vec = XMVectorPermute<0,1,2,7>(m_vec, w); }
        INLINE void SetXYZ( CVector3 xyz ) { m_vec = XMVectorPermute<0,1,2,7>(xyz, m_vec); }
    public:

        //Operator overloads for when we want to work with multiple bespoke math types,
        //Creative has operator overloads for adding vectors to scalars.
        INLINE CVector4 operator- () const { return CVector4(XMVectorNegate(m_vec)); }
        INLINE CVector4 operator+ ( CVector4 v2 ) const { return CVector4(XMVectorAdd(m_vec, v2)); }
        INLINE CVector4 operator- ( CVector4 v2 ) const { return CVector4(XMVectorSubtract(m_vec, v2)); }
        INLINE CVector4 operator* ( CVector4 v2 ) const { return CVector4(XMVectorMultiply(m_vec, v2)); }
        INLINE CVector4 operator/ ( CVector4 v2 ) const { return CVector4(XMVectorDivide(m_vec, v2)); }
        INLINE CVector4 operator* ( CScalar  v2 ) const { return *this * CVector4(v2); }
        INLINE CVector4 operator/ ( CScalar  v2 ) const { return *this / CVector4(v2); }
        INLINE CVector4 operator* ( float   v2 ) const { return *this * CScalar(v2); }
        INLINE CVector4 operator/ ( float   v2 ) const { return *this / CScalar(v2); }

        INLINE void operator*= ( float   v2 ) { *this = *this * CScalar(v2); }
        INLINE void operator/= ( float   v2 ) { *this = *this / CScalar(v2); }

        INLINE friend CVector4 operator* ( CScalar  v1, CVector4 v2 ) { return CVector4(v1) * v2; }
        INLINE friend CVector4 operator/ ( CScalar  v1, CVector4 v2 ) 	{ return CVector4(v1) / v2; }
        INLINE friend CVector4 operator* ( float   v1, CVector4 v2 ) { return CScalar(v1) * v2; }
        INLINE friend CVector4 operator/ ( float   v1, CVector4 v2 ) 	{ return CScalar(v1) / v2; }


    protected:
        XMVECTOR m_vec;
    };

    // Defined after Vector4 methods are declared
    INLINE CVector3::CVector3( CVector4 vec ) : m_vec((XMVECTOR)vec)
    {
    }

    // For W != 1, divide XYZ by W.  If W == 0, do nothing
    INLINE CVector3 MakeHomogeneous( CVector4 v )
    {
        CScalar W = v.GetW();
        return CVector3(XMVectorSelect( XMVectorDivide(v, W), v, XMVectorEqual(W, SplatZero()) ));
    }

    class BoolVector
    {
    public:
        INLINE BoolVector( FXMVECTOR vec ) { m_vec = vec; }
        INLINE operator XMVECTOR() const { return m_vec; }
    protected:
        XMVECTOR m_vec;
    };

} 
