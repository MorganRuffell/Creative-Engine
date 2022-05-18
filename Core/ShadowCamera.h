
#pragma once

#include "CViewportCamera.h"
#include "VectorMath.h"

//Manages how the camera percieves shadows
class CShadowCamera : public CMath::CBaseCamera
{
public:

    CShadowCamera() {}

    void UpdateMatrix( 
        CMath::CVector3 LightDirection,		
        CMath::CVector3 ShadowCenter,		
        CMath::CVector3 ShadowBounds,		
        uint32_t BufferWidth,		
        uint32_t BufferHeight,		
        uint32_t BufferPrecision	
        );

    // Used to transform world space to texture space for shadow sampling
    const CMath::CMatrix4& GetShadowMatrix() const { return m_ShadowMatrix; }

private:

    CMath::CMatrix4 m_ShadowMatrix;
};
