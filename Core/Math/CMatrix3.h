

#pragma once

#include "CQuarternion.h"

namespace CMath
{
    // Represents a 3x3 matrix while occuping a 3x4 memory footprint.  The unused row and column are undefined but implicitly
    // (0, 0, 0, 1).  Constructing a Matrix4 will make those values explicit.
    __declspec(align(16)) class CMatrix3
    {
    public:
        INLINE CMatrix3() {}
        INLINE CMatrix3( CVector3 x, CVector3 y, CVector3 z ) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
        INLINE CMatrix3( const CMatrix3& m ) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
        INLINE CMatrix3( CQuarternion q ) { *this = CMatrix3(XMMatrixRotationQuaternion(q)); }
        INLINE explicit CMatrix3( const XMMATRIX& m ) { m_mat[0] = CVector3(m.r[0]); m_mat[1] = CVector3(m.r[1]); m_mat[2] = CVector3(m.r[2]); }
        INLINE explicit CMatrix3( EIdentityTag ) { m_mat[0] = CVector3(kXUnitVector); m_mat[1] = CVector3(kYUnitVector); m_mat[2] = CVector3(kZUnitVector);  }
        INLINE explicit CMatrix3( EZeroTag ) { m_mat[0] = m_mat[1] = m_mat[2] = CVector3(kZero); }

        INLINE void SetX(CVector3 x) { m_mat[0] = x; }
        INLINE void SetY(CVector3 y) { m_mat[1] = y; }
        INLINE void SetZ(CVector3 z) { m_mat[2] = z; }

        INLINE CVector3 GetX() const { return m_mat[0]; }
        INLINE CVector3 GetY() const { return m_mat[1]; }
        INLINE CVector3 GetZ() const { return m_mat[2]; }

        static INLINE CMatrix3 MakeXRotation( float angle ) { return CMatrix3(XMMatrixRotationX(angle)); }
        static INLINE CMatrix3 MakeYRotation( float angle ) { return CMatrix3(XMMatrixRotationY(angle)); }
        static INLINE CMatrix3 MakeZRotation( float angle ) { return CMatrix3(XMMatrixRotationZ(angle)); }
        static INLINE CMatrix3 MakeScale( float scale ) { return CMatrix3(XMMatrixScaling(scale, scale, scale)); }
        static INLINE CMatrix3 MakeScale( float sx, float sy, float sz ) { return CMatrix3(XMMatrixScaling(sx, sy, sz)); }
        static INLINE CMatrix3 MakeScale( const XMFLOAT3& scale) { return CMatrix3(XMMatrixScaling(scale.x, scale.y, scale.z)); }
        static INLINE CMatrix3 MakeScale( CVector3 scale ) { return CMatrix3(XMMatrixScalingFromVector(scale)); }

        // Useful for DirectXMath interaction.  WARNING:  Only the 3x3 elements are defined.
        INLINE operator XMMATRIX() const { return XMMATRIX(m_mat[0], m_mat[1], m_mat[2], XMVectorZero()); }

        INLINE CMatrix3 operator* ( CScalar scl ) const { return CMatrix3( scl * GetX(), scl * GetY(), scl * GetZ() ); }
        INLINE CVector3 operator* ( CVector3 vec ) const { return CVector3( XMVector3TransformNormal(vec, *this) ); }
        INLINE CMatrix3 operator* ( const CMatrix3& mat ) const { return CMatrix3( *this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ() ); }

    private:
        CVector3 m_mat[3];
    };

} // namespace Math
