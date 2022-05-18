#pragma once
#include <cstdint>

class CAnimationCurve
{
public:
    enum {
        kTranslation,
        kRotation,
        kScale,
        kWeights
    }; // targetPath

    enum {
        kLinear,
        kStep,
        kCatmullRomSpline,
        kCubicSpline
    }; // interpolation type

    enum {
        kSNorm8,
        kUNorm8,
        kSNorm16,
        kUNorm16,
        kFloat
    }; // format

    uint32_t targetNode : 28;           // Which node is being animated
    uint32_t targetPath : 2;            // What aspect of the transform is animated
    uint32_t interpolation : 2;         // The method of interpolation

public:

    uint32_t keyFrameOffset : 26;       // Byte offset to first key frame
    uint32_t keyFrameFormat : 3;        // Data format for the key frames
    uint32_t keyFrameStride : 3;        // Number of 4-byte words for one key frame

public:

    float numSegments;                  // Number of evenly-spaced gaps between keyframes
    float startTime;                    // Time stamp of the first key frame
    float rangeScale;                   // numSegments / (endTime - startTime)
};