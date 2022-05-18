
#pragma once

#include "CEngineBespokeTypes.h"

// Forward declarations
namespace CMath { 
    class CMatrix4; 
    class CViewportCamera; 
}
class CRGBBuffer;
class CCommandContext;

namespace CMotionBlurSettings
{
    extern CETBoolean Enable;

    void Initialize( void );
    void Shutdown( void );

    void GenerateCameraVelocityBuffer( CCommandContext& Context, const CMath::CViewportCamera& camera, bool UseLinearZ = true );
    void GenerateCameraVelocityBuffer( CCommandContext& Context, const CMath::CMatrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ = true);

    void RenderCameraBlur( CCommandContext& Context, const CMath::CViewportCamera& camera, bool UseLinearZ = true );
    void RenderCameraBlur( CCommandContext& Context, const CMath::CMatrix4& reprojectionMatrix, float nearClip, float farClip, bool UseLinearZ = true);

    // Generate proper motion blur that takes into account the velocity of each pixel.  Requires a pre-generated
    // velocity buffer (R16G16_FLOAT preferred.)
    void RenderObjectBlur( CCommandContext& Context, CRGBBuffer& velocityBuffer );
}
