

#pragma once

#include "Transform.h"

namespace CMath
{
    __declspec(align(16)) class CMatrix4
    {
    public:
        INLINE CMatrix4() {}
        INLINE CMatrix4( CVector3 x, CVector3 y, CVector3 z, CVector3 w )
        {
            m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
            m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
        }

        INLINE CMatrix4(const float* data)
        {
            m_mat = XMLoadFloat4x4((XMFLOAT4X4*)data);
        }

        INLINE CMatrix4( CVector4 x, CVector4 y, CVector4 z, CVector4 w ) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
        INLINE CMatrix4( const CMatrix4& mat ) { m_mat = mat.m_mat; }
        INLINE CMatrix4( const CMatrix3& mat )
        {
            m_mat.r[0] = SetWToZero(mat.GetX());
            m_mat.r[1] = SetWToZero(mat.GetY());
            m_mat.r[2] = SetWToZero(mat.GetZ());
            m_mat.r[3] = CreateWUnitVector();
        }
        INLINE CMatrix4( const CMatrix3& xyz, CVector3 w )
        {
            m_mat.r[0] = SetWToZero(xyz.GetX());
            m_mat.r[1] = SetWToZero(xyz.GetY());
            m_mat.r[2] = SetWToZero(xyz.GetZ());
            m_mat.r[3] = SetWToOne(w);
        }
        INLINE CMatrix4( const CMAffineTransform& xform ) { *this = CMatrix4( xform.GetBasis(), xform.GetTranslation()); }
        INLINE CMatrix4( const CMOrthoTransformation& xform ) { *this = CMatrix4( CMatrix3(xform.GetRotation()), xform.GetTranslation() ); }
        INLINE explicit CMatrix4( const XMMATRIX& mat ) { m_mat = mat; }
        INLINE explicit CMatrix4( EIdentityTag ) { m_mat = XMMatrixIdentity(); }
        INLINE explicit CMatrix4( EZeroTag ) { m_mat.r[0] = m_mat.r[1] = m_mat.r[2] = m_mat.r[3] = SplatZero(); }

        INLINE const CMatrix3& Get3x3() const { return (const CMatrix3&)*this; }
        INLINE void Set3x3(const CMatrix3& xyz)
        {
            m_mat.r[0] = SetWToZero(xyz.GetX());
            m_mat.r[1] = SetWToZero(xyz.GetY());
            m_mat.r[2] = SetWToZero(xyz.GetZ());
        }

        INLINE CVector4 GetX() const { return CVector4(m_mat.r[0]); }
        INLINE CVector4 GetY() const { return CVector4(m_mat.r[1]); }
        INLINE CVector4 GetZ() const { return CVector4(m_mat.r[2]); }
        INLINE CVector4 GetW() const { return CVector4(m_mat.r[3]); }

        INLINE void SetX(CVector4 x) { m_mat.r[0] = x; }
        INLINE void SetY(CVector4 y) { m_mat.r[1] = y; }
        INLINE void SetZ(CVector4 z) { m_mat.r[2] = z; }
        INLINE void SetW(CVector4 w) { m_mat.r[3] = w; }

        INLINE operator XMMATRIX() const { return m_mat; }

        INLINE CVector4 operator* ( CVector3 vec ) const { return CVector4(XMVector3Transform(vec, m_mat)); }
        INLINE CVector4 operator* ( CVector4 vec ) const { return CVector4(XMVector4Transform(vec, m_mat)); }
        INLINE CMatrix4 operator* ( const CMatrix4& mat ) const { return CMatrix4(XMMatrixMultiply(mat, m_mat)); }

        static INLINE CMatrix4 MakeScale( float scale ) { return CMatrix4(XMMatrixScaling(scale, scale, scale)); }
        static INLINE CMatrix4 MakeScale( CVector3 scale ) { return CMatrix4(XMMatrixScalingFromVector(scale)); }

    private:
        XMMATRIX m_mat;
    };
}
