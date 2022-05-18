
#pragma once

#include "Common.h"

namespace CMath
{
    // All of CMath Starts with CScalar, recall from A Level mathematics that 
    // this is an element of a field that is used to define a vector space
    class CScalar
    {
    public:
        INLINE CScalar() {}
        INLINE CScalar( const CScalar& s ) { m_vec = s; }
        INLINE CScalar( float f ) { m_vec = XMVectorReplicate(f); }

    public:

        //We are using explicit inline constructors here, these are where we are dictating that these XMVector splats 
        //must be a specific type
        INLINE explicit CScalar( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit CScalar( EZeroTag ) { m_vec = SplatZero(); }
        INLINE explicit CScalar( EIdentityTag ) { m_vec = SplatOne(); }

        INLINE operator XMVECTOR() const { return m_vec; }
        INLINE operator float() const { return XMVectorGetX(m_vec); }

    private:
        XMVECTOR m_vec;
    };

    INLINE CScalar operator- ( CScalar s ) { return CScalar(XMVectorNegate(s)); }
    INLINE CScalar operator+ ( CScalar s1, CScalar s2 ) { return CScalar(XMVectorAdd(s1, s2)); }
    INLINE CScalar operator- ( CScalar s1, CScalar s2 ) { return CScalar(XMVectorSubtract(s1, s2)); }
    INLINE CScalar operator* ( CScalar s1, CScalar s2 ) { return CScalar(XMVectorMultiply(s1, s2)); }
    INLINE CScalar operator/ ( CScalar s1, CScalar s2 ) { return CScalar(XMVectorDivide(s1, s2)); }
    INLINE CScalar operator+ ( CScalar s1, float s2 ) { return s1 + CScalar(s2); }
    INLINE CScalar operator- ( CScalar s1, float s2 ) { return s1 - CScalar(s2); }
    INLINE CScalar operator* ( CScalar s1, float s2 ) { return s1 * CScalar(s2); }
    INLINE CScalar operator/ ( CScalar s1, float s2 ) { return s1 / CScalar(s2); }
    INLINE CScalar operator+ ( float s1, CScalar s2 ) { return CScalar(s1) + s2; }
    INLINE CScalar operator- ( float s1, CScalar s2 ) { return CScalar(s1) - s2; }
    INLINE CScalar operator* ( float s1, CScalar s2 ) { return CScalar(s1) * s2; }
    INLINE CScalar operator/ ( float s1, CScalar s2 ) { return CScalar(s1) / s2; }

}
