
#pragma once

namespace CMath
{
	// To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress
	// upconversion.
	INLINE bool operator<  ( CScalar lhs, float rhs ) { return (float)lhs <  rhs; }
	INLINE bool operator<= ( CScalar lhs, float rhs ) { return (float)lhs <= rhs; }
	INLINE bool operator>  ( CScalar lhs, float rhs ) { return (float)lhs >  rhs; }
	INLINE bool operator>= ( CScalar lhs, float rhs ) { return (float)lhs >= rhs; }
	INLINE bool operator== ( CScalar lhs, float rhs ) { return (float)lhs == rhs; }
	INLINE bool operator<  ( float lhs, CScalar rhs ) { return lhs <  (float)rhs; }
	INLINE bool operator<= ( float lhs, CScalar rhs ) { return lhs <= (float)rhs; }
	INLINE bool operator>  ( float lhs, CScalar rhs ) { return lhs >  (float)rhs; }
	INLINE bool operator>= ( float lhs, CScalar rhs ) { return lhs >= (float)rhs; }
	INLINE bool operator== ( float lhs, CScalar rhs ) { return lhs == (float)rhs; }

#define CREATE_SIMD_FUNCTIONS( TYPE ) \
	INLINE TYPE Sqrt( TYPE s ) { return TYPE(XMVectorSqrt(s)); } \
	INLINE TYPE Recip( TYPE s ) { return TYPE(XMVectorReciprocal(s)); } \
	INLINE TYPE RecipSqrt( TYPE s ) { return TYPE(XMVectorReciprocalSqrt(s)); } \
	INLINE TYPE Floor( TYPE s ) { return TYPE(XMVectorFloor(s)); } \
	INLINE TYPE Ceiling( TYPE s ) { return TYPE(XMVectorCeiling(s)); } \
	INLINE TYPE Round( TYPE s ) { return TYPE(XMVectorRound(s)); } \
	INLINE TYPE Abs( TYPE s ) { return TYPE(XMVectorAbs(s)); } \
	INLINE TYPE Exp( TYPE s ) { return TYPE(XMVectorExp(s)); } \
	INLINE TYPE Pow( TYPE b, TYPE e ) { return TYPE(XMVectorPow(b, e)); } \
	INLINE TYPE Log( TYPE s ) { return TYPE(XMVectorLog(s)); } \
	INLINE TYPE Sin( TYPE s ) { return TYPE(XMVectorSin(s)); } \
	INLINE TYPE Cos( TYPE s ) { return TYPE(XMVectorCos(s)); } \
	INLINE TYPE Tan( TYPE s ) { return TYPE(XMVectorTan(s)); } \
	INLINE TYPE ASin( TYPE s ) { return TYPE(XMVectorASin(s)); } \
	INLINE TYPE ACos( TYPE s ) { return TYPE(XMVectorACos(s)); } \
	INLINE TYPE ATan( TYPE s ) { return TYPE(XMVectorATan(s)); } \
	INLINE TYPE ATan2( TYPE y, TYPE x ) { return TYPE(XMVectorATan2(y, x)); } \
	INLINE TYPE Lerp( TYPE a, TYPE b, TYPE t ) { return TYPE(XMVectorLerpV(a, b, t)); } \
    INLINE TYPE Lerp( TYPE a, TYPE b, float t ) { return TYPE(XMVectorLerp(a, b, t)); } \
	INLINE TYPE Max( TYPE a, TYPE b ) { return TYPE(XMVectorMax(a, b)); } \
	INLINE TYPE Min( TYPE a, TYPE b ) { return TYPE(XMVectorMin(a, b)); } \
	INLINE TYPE Clamp( TYPE v, TYPE a, TYPE b ) { return Min(Max(v, a), b); } \
	INLINE BoolVector operator<  ( TYPE lhs, TYPE rhs ) { return XMVectorLess(lhs, rhs); } \
	INLINE BoolVector operator<= ( TYPE lhs, TYPE rhs ) { return XMVectorLessOrEqual(lhs, rhs); } \
	INLINE BoolVector operator>  ( TYPE lhs, TYPE rhs ) { return XMVectorGreater(lhs, rhs); } \
	INLINE BoolVector operator>= ( TYPE lhs, TYPE rhs ) { return XMVectorGreaterOrEqual(lhs, rhs); } \
	INLINE BoolVector operator== ( TYPE lhs, TYPE rhs ) { return XMVectorEqual(lhs, rhs); } \
	INLINE TYPE Select( TYPE lhs, TYPE rhs, BoolVector mask ) { return TYPE(XMVectorSelect(lhs, rhs, mask)); }


	CREATE_SIMD_FUNCTIONS(CScalar)
	CREATE_SIMD_FUNCTIONS(CVector3)
	CREATE_SIMD_FUNCTIONS(CVector4)

#undef CREATE_SIMD_FUNCTIONS

	INLINE float Sqrt( float s ) { return Sqrt(CScalar(s)); }
	INLINE float Recip( float s ) { return Recip(CScalar(s)); }
	INLINE float RecipSqrt( float s ) { return RecipSqrt(CScalar(s)); }
	INLINE float Floor( float s ) { return Floor(CScalar(s)); }
	INLINE float Ceiling( float s ) { return Ceiling(CScalar(s)); }
	INLINE float Round( float s ) { return Round(CScalar(s)); }
	INLINE float Abs( float s ) { return s < 0.0f ? -s : s; }
	INLINE float Exp( float s ) { return Exp(CScalar(s)); }
	INLINE float Pow( float b, float e ) { return Pow(CScalar(b), CScalar(e)); }
	INLINE float Log( float s ) { return Log(CScalar(s)); }
	INLINE float Sin( float s ) { return Sin(CScalar(s)); }
	INLINE float Cos( float s ) { return Cos(CScalar(s)); }
	INLINE float Tan( float s ) { return Tan(CScalar(s)); }
	INLINE float ASin( float s ) { return ASin(CScalar(s)); }
	INLINE float ACos( float s ) { return ACos(CScalar(s)); }
	INLINE float ATan( float s ) { return ATan(CScalar(s)); }
	INLINE float ATan2( float y, float x ) { return ATan2(CScalar(y), CScalar(x)); }
	INLINE float Lerp( float a, float b, float t ) { return a + (b - a) * t; }
	INLINE float Max( float a, float b ) { return a > b ? a : b; }
	INLINE float Min( float a, float b ) { return a < b ? a : b; }
	INLINE float Clamp( float v, float a, float b ) { return Min(Max(v, a), b); }

	INLINE CScalar Length( CVector3 v ) { return CScalar(XMVector3Length(v)); }
	INLINE CScalar LengthSquare( CVector3 v ) { return CScalar(XMVector3LengthSq(v)); }
	INLINE CScalar LengthRecip( CVector3 v ) { return CScalar(XMVector3ReciprocalLength(v)); }
	INLINE CScalar Dot( CVector3 v1, CVector3 v2 ) { return CScalar(XMVector3Dot(v1, v2)); }
	INLINE CScalar Dot( CVector4 v1, CVector4 v2 ) { return CScalar(XMVector4Dot(v1, v2)); }
	INLINE CVector3 Cross( CVector3 v1, CVector3 v2 ) { return CVector3(XMVector3Cross(v1, v2)); }
	INLINE CVector3 Normalize( CVector3 v ) { return CVector3(XMVector3Normalize(v)); }
	INLINE CVector4 Normalize( CVector4 v ) { return CVector4(XMVector4Normalize(v)); }

	INLINE CMatrix3 Transpose( const CMatrix3& mat ) { return CMatrix3(XMMatrixTranspose(mat)); }
    INLINE CMatrix3 InverseTranspose( const CMatrix3& mat )
    {
        const CVector3 x = mat.GetX();
        const CVector3 y = mat.GetY();
        const CVector3 z = mat.GetZ();

        const CVector3 inv0 = Cross(y, z);
        const CVector3 inv1 = Cross(z, x);
        const CVector3 inv2 = Cross(x, y);
        const CScalar  rDet = Recip(Dot(z, inv2));

        // Return the adjoint / determinant
        return CMatrix3(inv0, inv1, inv2) * rDet;
    }

	// inline Matrix3 Inverse( const Matrix3& mat ) { TBD }
	// inline Transform Inverse( const Transform& mat ) { TBD }

	// This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
	INLINE CMAffineTransform OrthoInvert( const CMAffineTransform& xform )
	{
		CMatrix3 basis = Transpose(xform.GetBasis());
		return CMAffineTransform( basis, basis * -xform.GetTranslation() );
	}

	INLINE CMOrthoTransformation Invert( const CMOrthoTransformation& xform )	 { return ~xform; }

	INLINE CMatrix4 Transpose( const CMatrix4& mat ) { return CMatrix4(XMMatrixTranspose(mat)); }
	INLINE CMatrix4 Invert( const CMatrix4& mat ) { return CMatrix4(XMMatrixInverse(nullptr, mat)); }

	INLINE CMatrix4 OrthoInvert( const CMatrix4& xform )
	{
		CMatrix3 basis = Transpose(xform.Get3x3());
		CVector3 translate = basis * -CVector3(xform.GetW());
		return CMatrix4( basis, translate );
	}

}