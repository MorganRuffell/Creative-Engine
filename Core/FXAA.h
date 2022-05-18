

#pragma once

class CRGBBuffer;
class CETBoolean;
class CETFloat;
class CComputeContext;

namespace FXAA
{
    extern CETBoolean Enable;
    extern CETBoolean DebugDraw;
    extern CETFloat ContrastThreshold;	// Default = 0.20
    extern CETFloat SubpixelRemoval;		// Default = 0.75

    void Initialize( void );
    void Shutdown( void );
    void Render( CComputeContext& Context, bool bUsePreComputedLuma );

}
