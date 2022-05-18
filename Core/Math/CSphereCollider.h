#pragma once

#include "VectorMath.h"

namespace CMath
{
    // This is a spherial bounding box, as this engine is currently more mathematical. I'm calling this a sphere collider.
    // It in no way is similar to the collider components like UBoxComponent in Unreal
    class CSphereCollider
    {
    public:
        CSphereCollider() {}
        CSphereCollider( float x, float y, float z, float r ) : m_repr(x, y, z, r) {}
        CSphereCollider( const XMFLOAT4* unaligned_array ) : m_repr(*unaligned_array) {}
        CSphereCollider( CVector3 center, CScalar radius );
        CSphereCollider( EZeroTag ) : m_repr(kZero) {}
        explicit CSphereCollider (const XMVECTOR& v) : m_repr(v) {}
        explicit CSphereCollider( const XMFLOAT4& f4 ) : m_repr(f4) {}
        explicit CSphereCollider( CVector4 sphere ) : m_repr(sphere) {}
        explicit operator CVector4() const { return CVector4(m_repr); }

        CVector3 GetCenter( void ) const { return CVector3(m_repr); }
        CScalar GetRadius( void ) const { return m_repr.GetW(); }

        CSphereCollider Union( const CSphereCollider& rhs );

    private:

        CVector4 m_repr;
    };

    // Inline implementations

    inline CSphereCollider::CSphereCollider( CVector3 center, CScalar radius )
    {
        m_repr = CVector4(center);
        m_repr.SetW(radius);
    }

} 
