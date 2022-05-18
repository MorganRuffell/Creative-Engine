
#pragma once

#include "CEngineBespokeTypes.h"

class CCommandContext;

namespace CTAAEffects
{
    // Temporal antialiasing involves jittering sample positions and accumulating color over time to 
    // effectively supersample the image.
    extern CETBoolean EnableTAA;

    void Initialize( void );

    void Shutdown( void );

    // Call once per frame to increment the internal frame counter and, in the case of TAA, choosing the next
    // jittered sample position.
    void Update( uint64_t FrameIndex );

    // Returns whether the frame is odd or even, relevant to checkerboard rendering.
    uint32_t GetFrameIndexMod2( void );

    // Jitter values are neutral at 0.5 and vary from [0, 1).  Jittering only occurs when temporal antialiasing
    // is enabled.  You can use these values to jitter your viewport or projection matrix.
    void GetJitterOffset( float& JitterX, float& JitterY );

    void ClearHistory(CCommandContext& Context);

    void ResolveImage(CCommandContext& Context);

}
