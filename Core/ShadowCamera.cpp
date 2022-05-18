

#include "pch.h"
#include "ShadowCamera.h"

using namespace CMath;

void CShadowCamera::UpdateMatrix(
    CVector3 LightDirection, CVector3 ShadowCenter, CVector3 ShadowBounds,
    uint32_t BufferWidth, uint32_t BufferHeight, uint32_t BufferPrecision )
{
    SetLookDirection( LightDirection, CVector3(kZUnitVector) );

    // Converts world units to texel units so we can quantize the camera position to whole texel units
    CVector3 RcpDimensions = Recip(ShadowBounds);
    CVector3 QuantizeScale = CVector3((float)BufferWidth, (float)BufferHeight, (float)((1 << BufferPrecision) - 1)) * RcpDimensions;

    //
    // Recenter the camera at the quantized position
    //

    // Transform to view space
    ShadowCenter = ~GetRotation() * ShadowCenter;
    // Scale to texel units, truncate fractional part, and scale back to world units
    ShadowCenter = Floor( ShadowCenter * QuantizeScale ) / QuantizeScale;
    // Transform back into world space
    ShadowCenter = GetRotation() * ShadowCenter;

    SetPosition( ShadowCenter );

    SetProjMatrix( CMatrix4::MakeScale(CVector3(2.0f, 2.0f, 1.0f) * RcpDimensions) );

    Update();

    // Transform from clip space to texture space
    m_ShadowMatrix = CMatrix4( CMAffineTransform( CMatrix3::MakeScale( 0.5f, -0.5f, 1.0f ), CVector3(0.5f, 0.5f, 0.0f) ) ) * m_ViewProjMatrix;
}
