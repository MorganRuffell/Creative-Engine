
#pragma once

#include "pch.h"
#include "CEngineBespokeTypes.h"

class CComputeContext;

namespace CGlobalPostEffects
{
    extern CETBoolean EnableHDR;			// Turn on tone mapping features

    // Tone mapping parameters
    extern ExponentialCETFloat Exposure;				// Brightness scaler when adapative exposure is disabled
    extern CETBoolean EnableAdaptation;	// Automatically adjust brightness based on perceived luminance

    // Adapation parameters
    extern ExponentialCETFloat MinExposure;
    extern ExponentialCETFloat MaxExposure;
    extern CETFloat TargetLuminance;
    extern CETFloat AdaptationRate;

    // Bloom parameters
    extern CETBoolean BloomEnable;
    extern CETFloat BloomThreshold;
    extern CETFloat BloomStrength;

    void Initialize( void );
    void Shutdown( void );
    void Render( void );

    // Copy the contents of the post effects buffer onto the main scene buffer
    void CopyBackPostBuffer( CComputeContext& Context );
}
