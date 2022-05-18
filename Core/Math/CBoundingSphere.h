
#pragma once

#include "VectorMath.h"

namespace CMath
{
    class CBoundingSphere
    {
    public:
        CBoundingSphere() {}
        CBoundingSphere( float x, float y, float z, float r ) : m_repr(x, y, z, r) {}
        CBoundingSphere( const XMFLOAT4* unaligned_array ) : m_repr(*unaligned_array) {}
        CBoundingSphere( CVector3 center, CScalar radius );
        CBoundingSphere( EZeroTag ) : m_repr(kZero) {}
        explicit CBoundingSphere (const XMVECTOR& v) : m_repr(v) {}
        explicit CBoundingSphere( const XMFLOAT4& f4 ) : m_repr(f4) {}
        explicit CBoundingSphere( Vector4 sphere ) : m_repr(sphere) {}
        explicit operator Vector4() const { return Vector4(m_repr); }

        CVector3 GetCenter( void ) const { return CVector3(m_repr); }
        CScalar GetRadius( void ) const { return m_repr.GetW(); }

        CBoundingSphere Union( const CBoundingSphere& rhs );

    private:

        Vector4 m_repr;
    };

    //=======================================================================================================
    // Inline implementations
    //

    inline CBoundingSphere::CBoundingSphere( CVector3 center, CScalar radius )
    {
        m_repr = Vector4(center);
        m_repr.SetW(radius);
    }

} // namespace Math
