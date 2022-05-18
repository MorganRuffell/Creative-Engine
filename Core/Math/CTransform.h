
#pragma once

#include "CMatrix3.h"
#include "BoundingSphere.h"

namespace CMath
{
    // Orthonormal basis (just rotation via quaternion) and translation
    class CMOrthoTransformation;

    // A 3x4 matrix that allows for asymmetric skew and scale
    class CMAffineTransform;

    // Uniform scale and translation that can be compactly represented in a vec4
    class CMScaleAndTranslation;

    // Uniform scale, rotation (quaternion), and translation that fits in two vec4s
    class CMUniformTransform;

    // This transform strictly prohibits non-uniform scale
    class CMOrthoTransformation
    {
    public:
        INLINE CMOrthoTransformation() : m_rotation(kIdentity), m_translation(kZero) {}
        INLINE CMOrthoTransformation( CQuat rotate ) : m_rotation(rotate), m_translation(kZero) {}
        INLINE CMOrthoTransformation( CVector3 translate ) : m_rotation(kIdentity), m_translation(translate) {}
        INLINE CMOrthoTransformation( CQuat rotate, CVector3 translate ) : m_rotation(rotate), m_translation(translate) {}
        INLINE CMOrthoTransformation( const CMatrix3& mat ) : m_rotation(mat), m_translation(kZero) {}
        INLINE CMOrthoTransformation( const CMatrix3& mat, CVector3 translate ) : m_rotation(mat), m_translation(translate) {}
        INLINE CMOrthoTransformation( EIdentityTag ) : m_rotation(kIdentity), m_translation(kZero) {}
        INLINE explicit CMOrthoTransformation( const XMMATRIX& mat ) { *this = CMOrthoTransformation( CMatrix3(mat), CVector3(mat.r[3]) ); }

        INLINE void SetRotation( CQuat q ) { m_rotation = q; }
        INLINE void SetTranslation( CVector3 v ) { m_translation = v; }

        INLINE CQuat GetRotation() const { return m_rotation; }
        INLINE CVector3 GetTranslation() const { return m_translation; }

        static INLINE CMOrthoTransformation MakeXRotation( float angle ) { return CMOrthoTransformation(CQuat(CVector3(kXUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeYRotation( float angle ) { return CMOrthoTransformation(CQuat(CVector3(kYUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeZRotation( float angle ) { return CMOrthoTransformation(CQuat(CVector3(kZUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeTranslation( CVector3 translate ) { return CMOrthoTransformation(translate); }

        INLINE CVector3 operator* ( CVector3 vec ) const { return m_rotation * vec + m_translation; }
        INLINE Vector4 operator* ( Vector4 vec ) const { return
            Vector4(SetWToZero(m_rotation * CVector3((XMVECTOR)vec))) +
            Vector4(SetWToOne(m_translation)) * vec.GetW();
        }
        INLINE BoundingSphere operator* ( BoundingSphere sphere ) const {
            return BoundingSphere(*this * sphere.GetCenter(), sphere.GetRadius());
        }

        INLINE CMOrthoTransformation operator* ( const CMOrthoTransformation& xform ) const {
            return CMOrthoTransformation( m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation );
        }

        INLINE CMOrthoTransformation operator~ () const { CQuat invertedRotation = ~m_rotation;
            return CMOrthoTransformation( invertedRotation, invertedRotation * -m_translation );
        }

    private:

        CQuat m_rotation;
        CVector3 m_translation;
    };

    //
    // A transform that lacks rotation and has only uniform scale.
    //
    class CMScaleAndTranslation
    {
    public:
        INLINE CMScaleAndTranslation()
        {}
        INLINE CMScaleAndTranslation( EIdentityTag )
            : m_repr(kWUnitVector) {}
        INLINE CMScaleAndTranslation(float tx, float ty, float tz, float scale)
            : m_repr(tx, ty, tz, scale) {}
        INLINE CMScaleAndTranslation(CVector3 translate, CScalar scale)
        {
            m_repr = Vector4(translate);
            m_repr.SetW(scale);
        }
        INLINE explicit CMScaleAndTranslation(const XMVECTOR& v)
            : m_repr(v) {}

        INLINE void SetScale(CScalar s) { m_repr.SetW(s); }
        INLINE void SetTranslation(CVector3 t) { m_repr.SetXYZ(t); }

        INLINE CScalar GetScale() const { return m_repr.GetW(); }
        INLINE CVector3 GetTranslation() const { return (CVector3)m_repr; }

        INLINE BoundingSphere operator*(const BoundingSphere& sphere) const
        {
            Vector4 scaledSphere = (Vector4)sphere * GetScale();
            Vector4 translation = Vector4(SetWToZero(m_repr));
            return BoundingSphere(scaledSphere + translation);
        }

    private:
        Vector4 m_repr;
    };

    //
    // This transform allows for rotation, translation, and uniform scale
    // 
    class CMUniformTransform
    {
    public:
        INLINE CMUniformTransform()
        {}
        INLINE CMUniformTransform( EIdentityTag )
            : m_rotation(kIdentity), m_translationScale(kIdentity) {}
        INLINE CMUniformTransform(CQuat rotation, CMScaleAndTranslation transScale)
            : m_rotation(rotation), m_translationScale(transScale)
        {}
        INLINE CMUniformTransform(CQuat rotation, CScalar scale, CVector3 translation)
            : m_rotation(rotation), m_translationScale(translation, scale)
        {}

        INLINE void SetRotation(CQuat r) { m_rotation = r; }
        INLINE void SetScale(CScalar s) { m_translationScale.SetScale(s); }
        INLINE void SetTranslation(CVector3 t) { m_translationScale.SetTranslation(t); }


        INLINE CQuat GetRotation() const { return m_rotation; }
        INLINE CScalar GetScale() const { return m_translationScale.GetScale(); }
        INLINE CVector3 GetTranslation() const { return m_translationScale.GetTranslation(); }


        INLINE CVector3 operator*( CVector3 vec ) const
        {
            return m_rotation * (vec * m_translationScale.GetScale()) + m_translationScale.GetTranslation();
        }

        INLINE BoundingSphere operator*( BoundingSphere sphere ) const
        {
            return BoundingSphere(*this * sphere.GetCenter(), GetScale() * sphere.GetRadius() );
        }

    private:
        CQuat m_rotation;
        CMScaleAndTranslation m_translationScale;
    };

    // A AffineTransform is a 3x4 matrix with an implicit 4th row = [0,0,0,1].  This is used to perform a change of
    // basis on 3D points.  An affine transformation does not have to have orthonormal basis vectors.
    class CMAffineTransform
    {
    public:
        INLINE CMAffineTransform()
            {}
        INLINE CMAffineTransform( CVector3 x, CVector3 y, CVector3 z, CVector3 w )
            : m_basis(x, y, z), m_translation(w) {}
        INLINE CMAffineTransform( CVector3 translate )
            : m_basis(kIdentity), m_translation(translate) {}
        INLINE CMAffineTransform( const CMatrix3& mat, CVector3 translate = CVector3(kZero) )
            : m_basis(mat), m_translation(translate) {}
        INLINE CMAffineTransform( CQuat rot, CVector3 translate = CVector3(kZero) )
            : m_basis(rot), m_translation(translate) {}
        INLINE CMAffineTransform( const CMOrthoTransformation& xform )
            : m_basis(xform.GetRotation()), m_translation(xform.GetTranslation()) {}
        INLINE CMAffineTransform( const CMUniformTransform& xform )
        {
            m_basis = CMatrix3(xform.GetRotation()) * xform.GetScale();
            m_translation = xform.GetTranslation();
        }
        INLINE CMAffineTransform( EIdentityTag )
            : m_basis(kIdentity), m_translation(kZero) {}
        INLINE explicit CMAffineTransform( const XMMATRIX& mat )
            : m_basis(mat), m_translation(mat.r[3]) {}

        INLINE operator XMMATRIX() const { return (XMMATRIX&)*this; }

        INLINE void SetX(CVector3 x) { m_basis.SetX(x); }
        INLINE void SetY(CVector3 y) { m_basis.SetY(y); }
        INLINE void SetZ(CVector3 z) { m_basis.SetZ(z); }
        INLINE void SetTranslation(CVector3 w) { m_translation = w; }
        INLINE void SetBasis(const CMatrix3& basis) { m_basis = basis; }

        INLINE CVector3 GetX() const { return m_basis.GetX(); }
        INLINE CVector3 GetY() const { return m_basis.GetY(); }
        INLINE CVector3 GetZ() const { return m_basis.GetZ(); }
        INLINE CVector3 GetTranslation() const { return m_translation; }
        INLINE const CMatrix3& GetBasis() const { return (const CMatrix3&)*this; }

        static INLINE CMAffineTransform MakeXRotation( float angle ) { return CMAffineTransform(CMatrix3::MakeXRotation(angle)); }
        static INLINE CMAffineTransform MakeYRotation( float angle ) { return CMAffineTransform(CMatrix3::MakeYRotation(angle)); }
        static INLINE CMAffineTransform MakeZRotation( float angle ) { return CMAffineTransform(CMatrix3::MakeZRotation(angle)); }
        static INLINE CMAffineTransform MakeScale( float scale ) { return CMAffineTransform(CMatrix3::MakeScale(scale)); }
        static INLINE CMAffineTransform MakeScale( CVector3 scale ) { return CMAffineTransform(CMatrix3::MakeScale(scale)); }
        static INLINE CMAffineTransform MakeTranslation( CVector3 translate ) { return CMAffineTransform(translate); }

        INLINE CVector3 operator* ( CVector3 vec ) const { return m_basis * vec + m_translation; }
        INLINE CMAffineTransform operator* ( const CMAffineTransform& mat ) const {
            return CMAffineTransform( m_basis * mat.m_basis, *this * mat.GetTranslation() );
        }

    private:
        CMatrix3 m_basis;
        CVector3 m_translation;
    };
}
