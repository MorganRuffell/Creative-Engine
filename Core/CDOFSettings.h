

#pragma once

#include "CCommandContext.h"

namespace CDOFSettings
{
    extern CETBoolean Enable;

    void Initialize( void );
    void Shutdown( void );

    void Render( CCommandContext& BaseContext, float NearClipDist, float FarClipDist );
}
