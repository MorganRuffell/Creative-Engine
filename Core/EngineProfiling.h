

#pragma once

#include <string>
#include "TextRenderer.h"

class CCommandContext;

namespace EngineProfiling
{
    void Update();

    void BeginBlock(const std::wstring& name, CCommandContext* Context = nullptr);
    void EndBlock(CCommandContext* Context = nullptr);

    bool IsPaused();
}

#ifdef RELEASE
class CScopeTimingBlock
{
public:
    CScopeTimingBlock(const std::wstring&) {}
    CScopeTimingBlock(const std::wstring&, CCommandContext&) {}
};
#else
class CScopeTimingBlock
{
public:
    CScopeTimingBlock( const std::wstring& name ) : m_Context(nullptr)
    {
        EngineProfiling::BeginBlock(name);
    }
    CScopeTimingBlock( const std::wstring& name, CCommandContext& Context ) : m_Context(&Context)
    {
        EngineProfiling::BeginBlock(name, m_Context);
    }
    ~CScopeTimingBlock()
    {
        EngineProfiling::EndBlock(m_Context);
    }

private:
    CCommandContext* m_Context;
};
#endif
