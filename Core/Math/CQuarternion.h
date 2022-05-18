
#pragma once

#include "CVector.h"

namespace CMath
{
    /// <summary>
    /// Quarternions represent rotations, in creative the mathematics are similar to what you would find in a mathematics textbook
    /// This is because this is a student project, and is designed to be an interesting personal interest project to end my bachelors degree
    /// </summary>
    class CQuarternion
    {
    public:
        INLINE CQuarternion() { m_vec = XMQuaternionIdentity(); }
        INLINE CQuarternion( const CVector3& axis, const CScalar& angle ) { m_vec = XMQuaternionRotationAxis( axis, angle ); }
        INLINE CQuarternion( float pitch, float yaw, float roll) { m_vec = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }
        INLINE explicit CQuarternion( const XMMATRIX& matrix ) { m_vec = XMQuaternionRotationMatrix( matrix ); }	
        INLINE explicit CQuarternion( FXMVECTOR vec ) { m_vec = vec; }
        INLINE explicit CQuarternion( EIdentityTag ) { m_vec = XMQuaternionIdentity(); }

        INLINE operator XMVECTOR() const { return m_vec; }

        INLINE CQuarternion operator~ ( void ) const { return CQuarternion(XMQuaternionConjugate(m_vec)); }
        INLINE CQuarternion operator- ( void ) const { return CQuarternion(XMVectorNegate(m_vec)); }

        INLINE CQuarternion operator* ( CQuarternion rhs ) const { return CQuarternion(XMQuaternionMultiply(rhs, m_vec)); }
        INLINE CVector3 operator* ( CVector3 rhs ) const { return CVector3(XMVector3Rotate(rhs, m_vec)); }

        INLINE CQuarternion& operator= ( CQuarternion rhs ) { m_vec = rhs; return *this; }
        INLINE CQuarternion& operator*= ( CQuarternion rhs ) { *this = *this * rhs; return *this; }

    public:

        INLINE XMVECTOR GetDotProduct(XMVECTOR& VectorToCompareAgainst) { return XMQuaternionDot(VectorToCompareAgainst, m_vec); }

    protected:
        XMVECTOR m_vec;

    protected:


        /// <summary>
        /// Do not directly modify these.
        /// </summary>
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 0;
    };

    INLINE CQuarternion Normalize(CQuarternion q) { return CQuarternion(XMQuaternionNormalize(q)); }
    INLINE CQuarternion Slerp(CQuarternion a, CQuarternion b, float t) { return Normalize(CQuarternion(XMQuaternionSlerp(a, b, t))); }
    INLINE CQuarternion Lerp(CQuarternion a, CQuarternion b, float t) { return Normalize(CQuarternion(XMVectorLerp(a, b, t))); }
}
