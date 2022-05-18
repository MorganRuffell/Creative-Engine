
#pragma once

#include "CMatrix3.h"
#include "CSphereCollider.h"

/*
    Transforms -- A note!

    As well as possesing implementation code for vectors, scalars, 3x3 & 4x4 matrixes. 
    Creative posses code that allows for transforms, these are instrumental to a lot of the navigation of the 3D world
    that creative renders.

    A transform at least in the context of creative is four types:

    CMScaleAndTranslation
        
           The basic transform type, does not use quarternions, this manages simple scale and movement.

    CMUniformTransform

           Uniform transformation takes all types, that are being used inside of the ScaleAndTranslation transformation 'Calculation'
           and adds on top of it the ability to manage rotations as well (Through Quarternions) I eventually plan to add a Rotator overload, similar to FMath.

    CMOrthoTransform
        
           Allows for all matrixes (Similar to uniform transform), but done in an orthographic way.  

    CMAffineTransform

            *See Implementation*
  
    The implementation code is below, but these manage different rotations, scales and transforms.
*/

namespace CMath
{
    //Pre declaring all of the transforms that we are going to use -- These are more mathematical
    //relegated to the realm of matrix mathematics
    //This is not related to programming, but is built upon matrix mathematics
	class CMUniformTransform;
    class CMOrthoTransformation;
    class CMAffineTransform;
    class CMScaleAndTranslation;

    class CMOrthoTransformation
    {
    public:
        INLINE CMOrthoTransformation() : m_rotation(kIdentity), m_translation(kZero) {}
        INLINE CMOrthoTransformation( CQuarternion rotate ) : m_rotation(rotate), m_translation(kZero) {}
        INLINE CMOrthoTransformation( CVector3 translate ) : m_rotation(kIdentity), m_translation(translate) {}
        INLINE CMOrthoTransformation( CQuarternion rotate, CVector3 translate ) : m_rotation(rotate), m_translation(translate) {}
        INLINE CMOrthoTransformation( const CMatrix3& mat ) : m_rotation(mat), m_translation(kZero) {}
        INLINE CMOrthoTransformation( const CMatrix3& mat, CVector3 translate ) : m_rotation(mat), m_translation(translate) {}
        INLINE CMOrthoTransformation( EIdentityTag ) : m_rotation(kIdentity), m_translation(kZero) {}
        INLINE explicit CMOrthoTransformation( const XMMATRIX& mat ) { *this = CMOrthoTransformation( CMatrix3(mat), CVector3(mat.r[3]) ); }

        INLINE void SetRotation( CQuarternion q ) { m_rotation = q; }
        INLINE void SetTranslation( CVector3 v ) { m_translation = v; }

        INLINE CQuarternion GetRotation() const { return m_rotation; }
        INLINE CVector3 GetTranslation() const { return m_translation; }

        static INLINE CMOrthoTransformation MakeXRotation( float angle ) { return CMOrthoTransformation(CQuarternion(CVector3(kXUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeYRotation( float angle ) { return CMOrthoTransformation(CQuarternion(CVector3(kYUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeZRotation( float angle ) { return CMOrthoTransformation(CQuarternion(CVector3(kZUnitVector), angle)); }
        static INLINE CMOrthoTransformation MakeTranslation( CVector3 translate ) { return CMOrthoTransformation(translate); }

        INLINE CVector3 operator* ( CVector3 vec ) const { return m_rotation * vec + m_translation; }
        INLINE CVector4 operator* ( CVector4 vec ) const { return
            CVector4(SetWToZero(m_rotation * CVector3((XMVECTOR)vec))) +
            CVector4(SetWToOne(m_translation)) * vec.GetW();
        }
        INLINE CSphereCollider operator* ( CSphereCollider sphere ) const {
            return CSphereCollider(*this * sphere.GetCenter(), sphere.GetRadius());
        }

        INLINE CMOrthoTransformation operator* ( const CMOrthoTransformation& xform ) const {
            return CMOrthoTransformation( m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation );
        }

        INLINE CMOrthoTransformation operator~ () const { CQuarternion invertedRotation = ~m_rotation;
            return CMOrthoTransformation( invertedRotation, invertedRotation * -m_translation );
        }

    private:

        CQuarternion m_rotation;
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
            m_repr = CVector4(translate);
            m_repr.SetW(scale);
        }
        INLINE explicit CMScaleAndTranslation(const XMVECTOR& v)
            : m_repr(v) {}

        INLINE void SetScale(CScalar s) { m_repr.SetW(s); }
        INLINE void SetTranslation(CVector3 t) { m_repr.SetXYZ(t); }

        INLINE CScalar GetScale() const { return m_repr.GetW(); }
        INLINE CVector3 GetTranslation() const { return (CVector3)m_repr; }

        INLINE CSphereCollider operator*(const CSphereCollider& sphere) const
        {
            CVector4 scaledSphere = (CVector4)sphere * GetScale();
            CVector4 translation = CVector4(SetWToZero(m_repr));
            return CSphereCollider(scaledSphere + translation);
        }

    private:
        CVector4 m_repr;
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
        INLINE CMUniformTransform(CQuarternion rotation, CMScaleAndTranslation transScale)
            : m_rotation(rotation), m_translationScale(transScale)
        {}
        INLINE CMUniformTransform(CQuarternion rotation, CScalar scale, CVector3 translation)
            : m_rotation(rotation), m_translationScale(translation, scale)
        {}

        INLINE void SetRotation(CQuarternion r) { m_rotation = r; }
        INLINE void SetScale(CScalar s) { m_translationScale.SetScale(s); }
        INLINE void SetTranslation(CVector3 t) { m_translationScale.SetTranslation(t); }


        INLINE CQuarternion GetRotation() const { return m_rotation; }
        INLINE CScalar GetScale() const { return m_translationScale.GetScale(); }
        INLINE CVector3 GetTranslation() const { return m_translationScale.GetTranslation(); }


        INLINE CVector3 operator*( CVector3 vec ) const
        {
            return m_rotation * (vec * m_translationScale.GetScale()) + m_translationScale.GetTranslation();
        }

        INLINE CSphereCollider operator*( CSphereCollider sphere ) const
        {
            return CSphereCollider(*this * sphere.GetCenter(), GetScale() * sphere.GetRadius() );
        }

    private:
        CQuarternion m_rotation;
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
        INLINE CMAffineTransform( CQuarternion rot, CVector3 translate = CVector3(kZero) )
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
