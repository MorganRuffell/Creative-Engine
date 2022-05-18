

#pragma once

#include "EngineCore.h"

class CCommandContext;

/// <summary>
/// GPUs need to be timed, this manages the timing of the GPU installed on the computer
/// It does this through using the CommandContext class and a timer index.
/// </summary>
namespace CGPUTimerManager
{
    void Initialize( uint32_t MaxNumTimers = 4096 );
    void Shutdown();

    // Reserve a unique timer index
    uint32_t NewTimer(void);

    // Write start and stop time stamps on the GPU timeline
    void StartTimer(CCommandContext& Context, uint32_t TimerIdx);
    void StopTimer(CCommandContext& Context, uint32_t TimerIdx);

    // Bookend all calls to GetTime() with Begin/End which correspond to Map/Unmap.  This
    // needs to happen either at the very start or very end of a frame.
    void BeginReadBack(void);
    void EndReadBack(void);

    // Returns the time in milliseconds between start and stop queries
    float GetTime(uint32_t TimerIdx);
}
