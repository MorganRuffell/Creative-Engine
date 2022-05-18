#pragma once

#include <cstdint>

//
// An animation is composed of multiple animation curves.
//
struct CAnimationSet
{
    float duration;             // Time to play entire animation
    uint32_t firstCurve;        // Index of the first curve in this set (stored separately)
    uint32_t numCurves;         // Number of curves in this set
};
