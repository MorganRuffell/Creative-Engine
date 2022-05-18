
#include "pch.h"
#include "CInternalWindowsTime.h"
#include "Display.h"
#include "TextRenderer.h"
#include "EngineInput.h"
#include "CGPUTimerManager.h"
#include "CCommandContext.h"
#include <vector>
#include <unordered_map>
#include <array>

using namespace CGraphics;
using namespace CMath;
using namespace std;

#define PERF_GRAPH_ERROR uint32_t(0xFFFFFFFF)
namespace EngineProfiling
{
    bool Paused = false;
}

class StatHistory
{
public:
    StatHistory()
    {
        for (uint32_t i = 0; i < kHistorySize; ++i)
            m_RecentHistory[i] = 0.0f;
        for (uint32_t i = 0; i < kExtendedHistorySize; ++i)
            m_ExtendedHistory[i] = 0.0f;
        m_Average = 0.0f;
        m_Minimum = 0.0f;
        m_Maximum = 0.0f;
    }

    void RecordStat( uint32_t FrameIndex, float Value )
    {
        m_RecentHistory[FrameIndex % kHistorySize] = Value;
        m_ExtendedHistory[FrameIndex % kExtendedHistorySize] = Value;
        m_Recent = Value;

        uint32_t ValidCount = 0;
        m_Minimum = FLT_MAX;
        m_Maximum = 0.0f;
        m_Average = 0.0f;

        for (float val : m_RecentHistory)
        {
            if (val > 0.0f)
            {
                ++ValidCount;
                m_Average += val;
                m_Minimum = min(val, m_Minimum);
                m_Maximum = max(val, m_Maximum);
            }
        }

        if (ValidCount > 0)
            m_Average /= (float)ValidCount;
        else
            m_Minimum = 0.0f;
    }

    float GetLast(void) const { return m_Recent; }
    float GetMax(void) const { return m_Maximum; }
    float GetMin(void) const { return m_Minimum; }
    float GetAvg(void) const { return m_Average; }

    const float* GetHistory(void) const { return m_ExtendedHistory; }
    uint32_t GetHistoryLength(void) const { return kExtendedHistorySize; }

private:
    static const uint32_t kHistorySize = 64;
    static const uint32_t kExtendedHistorySize = 256;
    float m_RecentHistory[kHistorySize];
    float m_ExtendedHistory[kExtendedHistorySize];
    float m_Recent;
    float m_Average;
    float m_Minimum;
    float m_Maximum;
};

class StatPlot
{
public:
    StatPlot(StatHistory& Data, CRGBAColour Col = CRGBAColour(1.0f, 1.0f, 1.0f))
        : m_StatData(Data), m_PlotColor(Col)
    {
    }

    void SetColor( CRGBAColour Col )
    {
        m_PlotColor = Col;
    }

private:
    StatHistory& m_StatData;
    CRGBAColour m_PlotColor;
};

class StatGraph
{
public:
    StatGraph(const wstring& Label, D3D12_RECT Window)
        : m_Label(Label), m_Window(Window), m_BGColor(0.0f, 0.0f, 0.0f, 0.2f)
    {
    }

    void SetLabel(const wstring& Label)
    {
        m_Label = Label;
    }

    void SetWindow(D3D12_RECT Window)
    {
        m_Window = Window;
    }

    uint32_t AddPlot( const StatPlot& P )
    {
        uint32_t Idx = (uint32_t)m_Stats.size();
        m_Stats.push_back(P);
        return Idx;
    }

    StatPlot& GetPlot( uint32_t Handle );

    void Draw( CGraphicsContext& Context );

private:
    wstring m_Label;
    D3D12_RECT m_Window;
    vector<StatPlot> m_Stats;
    CRGBAColour m_BGColor;
    float m_PeakValue;
};

class GraphManager
{
public:

private:
    vector<StatGraph> m_Graphs;
};

class CGPUTimer
{
public:

    CGPUTimer::CGPUTimer()
    {
        m_TimerIndex = CGPUTimerManager::NewTimer();
    }

    void Start(CCommandContext& Context)
    {
        CGPUTimerManager::StartTimer(Context, m_TimerIndex);
    }

    void Stop(CCommandContext& Context)
    {
        CGPUTimerManager::StopTimer(Context, m_TimerIndex);
    }

    float CGPUTimer::GetTime(void)
    {
        return CGPUTimerManager::GetTime(m_TimerIndex);
    }

    uint32_t GetTimerIndex(void)
    {
        return m_TimerIndex;
    }
private:

    uint32_t m_TimerIndex;
};

class NestedTimingTree
{
public:
    NestedTimingTree( const wstring& name, NestedTimingTree* parent = nullptr )
        : m_Name(name), m_Parent(parent), m_IsExpanded(false), m_IsGraphed(false) {}

    NestedTimingTree* GetChild( const wstring& name )
    {
        auto iter = m_LUT.find(name);
        if (iter != m_LUT.end())
            return iter->second;

        NestedTimingTree* node = new NestedTimingTree(name, this);
        m_Children.push_back(node);
        m_LUT[name] = node;
        return node;
    }

    NestedTimingTree* NextScope( void )
    {
        if (m_IsExpanded && m_Children.size() > 0)
            return m_Children[0];

        return m_Parent->NextChild(this);
    }

    NestedTimingTree* PrevScope( void )
    {
        NestedTimingTree* prev = m_Parent->PrevChild(this);
        return prev == m_Parent ? prev : prev->LastChild();
    }

    NestedTimingTree* FirstChild( void )
    {
        return m_Children.size() == 0 ? nullptr : m_Children[0];
    }

    NestedTimingTree* LastChild( void )
    {
        if (!m_IsExpanded || m_Children.size() == 0)
            return this;

        return m_Children.back()->LastChild();
    }

    NestedTimingTree* NextChild( NestedTimingTree* curChild )
    {
        ASSERT(curChild->m_Parent == this);

        for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
        {
            if (*iter == curChild)
            {
                auto nextChild = iter; ++nextChild;
                if (nextChild != m_Children.end())
                    return *nextChild;
            }
        }

        if (m_Parent != nullptr)
            return m_Parent->NextChild(this);
        else
            return &sm_RootScope;
    }

    NestedTimingTree* PrevChild( NestedTimingTree* curChild )
    {
        ASSERT(curChild->m_Parent == this);

        if (*m_Children.begin() == curChild)
        {
            if (this == &sm_RootScope)
                return sm_RootScope.LastChild();
            else
                return this;
        }

        for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
        {
            if (*iter == curChild)
            {
                auto prevChild = iter; --prevChild;
                return *prevChild;
            }
        }

        ERROR("All attempts to find a previous timing sample failed");
        return nullptr;
    }

    void StartTiming( CCommandContext* Context )
    {
        m_StartTick = CInternalWindowsTime::GetCurrentTick();
        if (Context == nullptr)
            return;

        m_GpuTimer.Start(*Context);

        Context->PIXBeginEvent(m_Name.c_str());
    }

    void StopTiming( CCommandContext* Context )
    {
        m_EndTick = CInternalWindowsTime::GetCurrentTick();
        if (Context == nullptr)
            return;

        m_GpuTimer.Stop(*Context);

        Context->PIXEndEvent();
    }

    void GatherTimes(uint32_t FrameIndex)
    {
        if (EngineProfiling::Paused)
        {
            for (auto node : m_Children)
                node->GatherTimes(FrameIndex);
            return;
        }
        m_CpuTime.RecordStat(FrameIndex, 1000.0f * (float)CInternalWindowsTime::TimeBetweenTicks(m_StartTick, m_EndTick));
        m_GpuTime.RecordStat(FrameIndex, 1000.0f * m_GpuTimer.GetTime());

        for (auto node : m_Children)
            node->GatherTimes(FrameIndex);

        m_StartTick = 0;
        m_EndTick = 0;
    }

    void SumInclusiveTimes(float& cpuTime, float& gpuTime)
    {
        cpuTime = 0.0f;
        gpuTime = 0.0f;
        for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
        {
            cpuTime += (*iter)->m_CpuTime.GetLast();
            gpuTime += (*iter)->m_GpuTime.GetLast();
        }
    }

    static void PushProfilingMarker( const wstring& name, CCommandContext* Context );
    static void PopProfilingMarker( CCommandContext* Context );
    static void Update( void );
    static void UpdateTimes( void )
    {
        uint32_t FrameIndex = (uint32_t)CGraphics::GetFrameCount();

        CGPUTimerManager::BeginReadBack();
        sm_RootScope.GatherTimes(FrameIndex);
        s_FrameDelta.RecordStat(FrameIndex, CGPUTimerManager::GetTime(0));
        CGPUTimerManager::EndReadBack();

        float TotalCpuTime, TotalGpuTime;
        sm_RootScope.SumInclusiveTimes(TotalCpuTime, TotalGpuTime);
        s_TotalCpuTime.RecordStat(FrameIndex, TotalCpuTime);
        s_TotalGpuTime.RecordStat(FrameIndex, TotalGpuTime);

    }

    static float GetTotalCpuTime(void) { return s_TotalCpuTime.GetAvg(); }
    static float GetTotalGpuTime(void) { return s_TotalGpuTime.GetAvg(); }
    static float GetFrameDelta(void) { return s_FrameDelta.GetAvg(); }

    void Toggle()
    { 
        //if (m_GraphHandle == PERF_GRAPH_ERROR)
        //    m_GraphHandle = GraphRenderer::InitGraph(GraphType::Profile);
        //m_IsGraphed = GraphRenderer::ManageGraphs(m_GraphHandle, GraphType::Profile);
    }
    bool IsGraphed(){ return m_IsGraphed;}

private:

    void StoreToGraph(void);
    void DeleteChildren( void )
    {
        for (auto node : m_Children)
            delete node;
        m_Children.clear();
    }

    wstring m_Name;
    NestedTimingTree* m_Parent;
    vector<NestedTimingTree*> m_Children;
    unordered_map<wstring, NestedTimingTree*> m_LUT;
    int64_t m_StartTick;
    int64_t m_EndTick;
    StatHistory m_CpuTime;
    StatHistory m_GpuTime;
    bool m_IsExpanded;
    CGPUTimer m_GpuTimer;
    bool m_IsGraphed;
    static StatHistory s_TotalCpuTime;
    static StatHistory s_TotalGpuTime;
    static StatHistory s_FrameDelta;
    static NestedTimingTree sm_RootScope;
    static NestedTimingTree* sm_CurrentNode;
    static NestedTimingTree* sm_SelectedScope;

    static bool sm_CursorOnGraph;

};

StatHistory NestedTimingTree::s_TotalCpuTime;
StatHistory NestedTimingTree::s_TotalGpuTime;
StatHistory NestedTimingTree::s_FrameDelta;
NestedTimingTree NestedTimingTree::sm_RootScope(L"");
NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;
bool NestedTimingTree::sm_CursorOnGraph = false;
namespace EngineProfiling
{
    CETBoolean DrawFrameRate("Display Frame Rate", true);
    CETBoolean DrawProfiler("Display Profiler", false);
    //CETBoolean DrawPerfGraph("Display Performance Graph", false);
    const bool DrawPerfGraph = false;
    
    void Update( void )
    {
        /*if (EngineInput::IsFirstPressed( EngineInput::kKey_space ))
        {
            Paused = !Paused;
        }
        NestedTimingTree::UpdateTimes();*/
    }

    void BeginBlock(const wstring& name, CCommandContext* Context)
    {
        NestedTimingTree::PushProfilingMarker(name, Context);
    }

    void EndBlock(CCommandContext* Context)
    {
        NestedTimingTree::PopProfilingMarker(Context);
    }

    bool IsPaused()
    {
        return Paused;
    }

} // EngineProfiling

void NestedTimingTree::PushProfilingMarker( const wstring& name, CCommandContext* Context )
{
    sm_CurrentNode = sm_CurrentNode->GetChild(name);
    sm_CurrentNode->StartTiming(Context);
}

void NestedTimingTree::PopProfilingMarker( CCommandContext* Context )
{
    sm_CurrentNode->StopTiming(Context);
    sm_CurrentNode = sm_CurrentNode->m_Parent;
}

void NestedTimingTree::Update( void )
{
    ASSERT(sm_SelectedScope != nullptr, "Corrupted profiling data structure");

    if (sm_SelectedScope == &sm_RootScope)
    {
        sm_SelectedScope = sm_RootScope.FirstChild();
        if (sm_SelectedScope == &sm_RootScope)
            return;
    }

    if (EngineInput::IsFirstPressed( EngineInput::kKey_left ))
    {
        //if still on graphs go back to text
        if (sm_CursorOnGraph)
            sm_CursorOnGraph = !sm_CursorOnGraph;
        else
            sm_SelectedScope->m_IsExpanded = false;
    }
    else if (EngineInput::IsFirstPressed( EngineInput::kKey_right ))
    {
        if (sm_SelectedScope->m_IsExpanded == true && !sm_CursorOnGraph)
            sm_CursorOnGraph = true;
        else
            sm_SelectedScope->m_IsExpanded = true;
        //if already expanded go over to graphs

    }
    else if (EngineInput::IsFirstPressed( EngineInput::kKey_down ))
    {
        sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->NextScope() : nullptr;
    }
    else if (EngineInput::IsFirstPressed( EngineInput::kKey_up ))
    {
        sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->PrevScope() : nullptr;
    }
    else if (EngineInput::IsFirstPressed( EngineInput::kKey_return ))
    {
        sm_SelectedScope->Toggle();
    }

}

void NestedTimingTree::StoreToGraph(void)
{
    for (auto node : m_Children)
        node->StoreToGraph();
}
