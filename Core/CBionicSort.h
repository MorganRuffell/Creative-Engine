
#pragma once

#include "CGPUBuffer.h"

namespace CAlgorithims
{
    void Sort(
        CComputeContext& Context,

        CGPUBuffer& KeyIndexList,

        CGPUBuffer& CountBuffer,

        uint32_t CounterOffset,

        bool IsPartiallyPreSorted,

        bool SortAscending
    );

    void Test( void );

} 