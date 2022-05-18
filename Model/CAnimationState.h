#pragma once

class CAnimationState
{
public:
    enum eMode { kStopped, kPlaying, kLooping };
    eMode state;
    float time;
    CAnimationState() : state(kStopped), time(0.0f) {}
};
