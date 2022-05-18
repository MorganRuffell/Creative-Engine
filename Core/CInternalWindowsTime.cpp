

#include "pch.h"
#include "CInternalWindowsTime.h"

double CInternalWindowsTime::sm_CpuTickDelta = 0.0;

// Query the performance counter frequency
void CInternalWindowsTime::Initialize( void )
{
    LARGE_INTEGER frequency;
    ASSERT(TRUE == QueryPerformanceFrequency(&frequency), "Unable to query performance counter frequency");
    sm_CpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
}

// Query the current value of the performance counter
int64_t CInternalWindowsTime::GetCurrentTick( void )
{
    LARGE_INTEGER currentTick;
    ASSERT(TRUE == QueryPerformanceCounter(&currentTick), "Unable to query performance counter value");
    return static_cast<int64_t>(currentTick.QuadPart);
}

void CInternalWindowsTime::BusyLoopSleep( float SleepTime )
{
    int64_t finalTick = (int64_t)((double)SleepTime / sm_CpuTickDelta) + GetCurrentTick();
    while (GetCurrentTick() < finalTick);
}
