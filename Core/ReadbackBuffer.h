
#pragma once

#include "CGPUBuffer.h"

/// <summary>
/// This specialised buffer manages texel data
/// </summary>
class ReadbackBuffer : public CGPUBuffer
{
public:
    virtual ~ReadbackBuffer() { Destroy(); }

    void Create( const std::wstring& name, uint32_t NumElements, uint32_t ElementSize );

    void* Map(void);
    void Unmap(void);

protected:

    void CreateDerivedViews(void) {}

};
