#include "pch.h"
#include "EngineInput.h"
#include "CRGBAColour.h"
#include "Display.h"
#include "CCommandContext.h"

using namespace std;
using namespace CMath;
using namespace CGraphics;

namespace EngineTuning
{
    // For delayed registration.  Some objects are constructed before we can add them to the graph (due
    // to unreliable order of initialization.)
    enum { kMaxUnregisteredTweaks = 1024 };
    char s_UnregisteredPath[kMaxUnregisteredTweaks][128];
    CreativeEngineType* s_UnregisteredVariable[kMaxUnregisteredTweaks] = { nullptr };
    int32_t s_UnregisteredCount = 0;

    float s_ScrollOffset = 0.0f;
    float s_ScrollTopTrigger = 1080.0f * 0.2f;
    float s_ScrollBottomTrigger = 1080.0f * 0.8f;

    // Internal functions
    void AddToVariableGraph( const string& path, CreativeEngineType& var );
    void RegisterVariable( const string& path, CreativeEngineType& var );

    CreativeEngineType* sm_SelectedVariable = nullptr;
    bool sm_IsVisible = false;
}

// Not open to the public.  Groups are auto-created when a tweaker's path includes the group name.
class VariableGroup : public CreativeEngineType
{
public:
    VariableGroup() : m_IsExpanded(false) {}

    CreativeEngineType* FindChild( const string& name )
    {
        auto iter = m_Children.find(name);
        return iter == m_Children.end() ? nullptr : iter->second;
    }
     
    void AddChild( const string& name, CreativeEngineType& child )
    {
        m_Children[name] = &child;
        child.m_GroupPtr = this;
    }

    bool IsExpanded( void ) const { return m_IsExpanded; }

    virtual void Increment( void ) override { m_IsExpanded = true; }
    virtual void Decrement( void ) override { m_IsExpanded = false; }
    virtual void Bang( void ) override { m_IsExpanded = !m_IsExpanded; }

    virtual void SetValue( FILE*, const std::string& ) override {}
    
    static VariableGroup sm_RootGroup;

private:
    bool m_IsExpanded;
    std::map<string, CreativeEngineType*> m_Children;
};

VariableGroup VariableGroup::sm_RootGroup;

//=====================================================================================================================
// CreativeEngineType implementations

CreativeEngineType* CreativeEngineType::NextVar(void)
{
    return nullptr;
}

CreativeEngineType* CreativeEngineType::PrevVar(void)
{
    return nullptr;
}

CreativeEngineType::CreativeEngineType( void ) : m_GroupPtr(nullptr)
{
}

CreativeEngineType::CreativeEngineType( const std::string& path, ActionCallback pfnCallback) : m_GroupPtr(nullptr), m_ActionCallback(pfnCallback)
{
    EngineTuning::RegisterVariable(path, *this);
}



CETBoolean::CETBoolean( const std::string& path, bool val, ActionCallback pfnCallback )
    : CreativeEngineType(path, pfnCallback)
{
    m_Flag = val;
}


std::string CETBoolean::ToString( void ) const
{
    return m_Flag ? "on" : "off";
} 

void CETBoolean::SetValue(FILE* file, const std::string& setting)
{	
    std::string pattern = "\n " + setting + ": %s";
    char valstr[6];

    // Search through the file for an entry that matches this setting's name
    fscanf_s(file, pattern.c_str(), valstr, _countof(valstr));

    // Look for one of the many affirmations
    m_Flag = (
        0 == _stricmp(valstr, "1") ||
        0 == _stricmp(valstr, "on") ||
        0 == _stricmp(valstr, "yes") ||
        0 == _stricmp(valstr, "true") );
}

CETFloat::CETFloat( const std::string& path, float val, float minVal, float maxVal, float stepSize, ActionCallback pfnCallback )
    : CreativeEngineType(path, pfnCallback)
{
    ASSERT(minVal <= maxVal);
    m_MinValue = minVal;
    m_MaxValue = maxVal;
    m_Value = Clamp(val);
    m_StepSize = stepSize;
}

std::string CETFloat::ToString( void ) const
{
    char buf[128];
    sprintf_s(buf, "%f", m_Value);
    return buf;
} 

void CETFloat::SetValue(FILE* file, const std::string& setting) 
{
    std::string scanString = "\n" + setting + ": %f";
    float valueRead;
    
    //If we haven't read correctly, just keep m_Value at default value
    if (fscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead; 
}

#if _MSC_VER < 1800
__forceinline float log2( float x ) { return log(x) / log(2.0f); }
__forceinline float exp2( float x ) { return pow(2.0f, x); }
#endif

ExponentialCETFloat::ExponentialCETFloat( const std::string& path, float val, float minExp, float maxExp, float expStepSize, ActionCallback pfnCallback )
    : CETFloat(path, log2f(val), minExp, maxExp, expStepSize, pfnCallback)
{
}

ExponentialCETFloat& ExponentialCETFloat::operator=( float val )
{
    m_Value = Clamp(log2f(val));
    return *this;
}

ExponentialCETFloat::operator float() const
{
    return exp2f(m_Value);
}

std::string ExponentialCETFloat::ToString( void ) const
{
    char buf[128];
    sprintf_s(buf, "%f", (float)*this);
    return buf;
} 

void ExponentialCETFloat::SetValue(FILE* file, const std::string& setting) 
{
    std::string scanString = "\n" + setting + ": %f";
    float valueRead;
    
    //If we haven't read correctly, just keep m_Value at default value
    if (fscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead;
}

CETInt::CETInt( const std::string& path, int32_t val, int32_t minVal, int32_t maxVal, int32_t stepSize, ActionCallback pfnCallback )
    : CreativeEngineType(path, pfnCallback)
{
    ASSERT(minVal <= maxVal);
    m_MinValue = minVal;
    m_MaxValue = maxVal;
    m_Value = Clamp(val);
    m_StepSize = stepSize;
}

std::string CETInt::ToString( void ) const
{
    char buf[128];
    sprintf_s(buf, "%d", m_Value);
    return buf;
} 

void CETInt::SetValue(FILE* file, const std::string& setting) 
{
    std::string scanString = "\n" + setting + ": %d";
    int32_t valueRead;
    
    if (fscanf_s(file, scanString.c_str(), &valueRead))
        *this = valueRead;
}


CETStaticEnum::CETStaticEnum( const std::string& path, int32_t initialVal, int32_t listLength, const char** listLabels, ActionCallback pfnCallback )
    : CreativeEngineType(path, pfnCallback)
{
    ASSERT(listLength > 0);
    m_EnumLength = listLength;
    m_EnumLabels = listLabels;
    m_Value = Clamp(initialVal);
}

std::string CETStaticEnum::ToString( void ) const
{
    return m_EnumLabels[m_Value];
} 

void CETStaticEnum::SetValue(FILE* file, const std::string& setting) 
{
    std::string scanString = "\n" + setting + ": %[^\n]";
    char valueRead[14];
        
    if (fscanf_s(file, scanString.c_str(), valueRead, _countof(valueRead)) == 1)
    {
        std::string valueReadStr = valueRead;
        valueReadStr = valueReadStr.substr(0, valueReadStr.length() - 1);

        //if we don't find the string, then leave m_EnumLabes[m_Value] as default
        for(int32_t i = 0; i < m_EnumLength; ++i)
        {
            if (m_EnumLabels[i] == valueReadStr)
            {
                m_Value = i;
                break;
            }
        }
    }
}

CETDynmEnum::CETDynmEnum( const std::string& path, ActionCallback pfnCallback )
    : CreativeEngineType(path, pfnCallback)
{
    m_EnumCount = 0;
    m_Value = 0;
}

std::string CETDynmEnum::ToString( void ) const
{
    return CUtility::WideStringToUTF8(m_EnumLabels[m_Value]);
} 

void CETDynmEnum::SetValue(FILE* file, const std::string& setting) 
{
    std::string scanString = "\n" + setting + ": %[^\n]";
    char valueRead[14];

    if (fscanf_s(file, scanString.c_str(), valueRead, _countof(valueRead)) == 1)
    {
        std::string valueReadStr = valueRead;
        valueReadStr = valueReadStr.substr(0, valueReadStr.length() - 1);

        std::wstring wvalue = CUtility::UTF8ToWideString(valueReadStr);

        //if we don't find the string, then leave m_EnumLabes[m_Value] as default
        for (int32_t i = 0; i < m_EnumCount; ++i)
        {
            if (m_EnumLabels[i] == wvalue)
            {
                m_Value = i;
                break;
            }
        }
    }
}


CallbackTrigger::CallbackTrigger( const std::string& path, std::function<void (void*)> callback, void* args )
    : CreativeEngineType(path)
{
    m_Callback = callback;
    m_Arguments = args;
    m_BangDisplay = 0;
}

void CallbackTrigger::SetValue(FILE* file, const std::string& setting) 
{
    //Skip over setting without reading anything
    std::string scanString = "\n" + setting + ": %[^\n]";
    char skippedLines[100];
    fscanf_s(file, scanString.c_str(), skippedLines, _countof(skippedLines));
}

//=====================================================================================================================
// EngineTuning namespace methods

void EngineTuning::Initialize( void )
{

    for (int32_t i = 0; i < s_UnregisteredCount; ++i)
    {
        ASSERT(strlen(s_UnregisteredPath[i]) > 0, "Register = %d\n", i);
        ASSERT(s_UnregisteredVariable[i] != nullptr);
        AddToVariableGraph(s_UnregisteredPath[i], *s_UnregisteredVariable[i]);
    }
    s_UnregisteredCount = -1;

}

void HandleDigitalButtonPress( EngineInput::DigitalInput button, float timeDelta, std::function<void ()> action )
{
    if (!EngineInput::IsPressed(button))
        return;

    float durationHeld = EngineInput::GetDurationPressed(button);

    // Tick on the first press
    if (durationHeld == 0.0f)
    {
        action();
        return;
    }

    // After ward, tick at fixed intervals
    float oldDuration = durationHeld - timeDelta;

    // Before 2 seconds, use slow scale (200ms/tick), afterward use fast scale (50ms/tick).
    float timeStretch = durationHeld < 2.0f ? 5.0f : 20.0f;

    if (Floor(durationHeld * timeStretch) > Floor(oldDuration * timeStretch))
        action();
}

void EngineTuning::Update( float frameTime )
{
    if (EngineInput::IsFirstPressed( EngineInput::kKey_back ))
        sm_IsVisible = !sm_IsVisible;

    if (!sm_IsVisible)
        return;

    if (sm_SelectedVariable == nullptr)
        return;

    HandleDigitalButtonPress(EngineInput::kKey_right, frameTime, []{ sm_SelectedVariable->Increment(); } );
    HandleDigitalButtonPress(EngineInput::kKey_left,	frameTime, []{ sm_SelectedVariable->Decrement(); } );
    HandleDigitalButtonPress(EngineInput::kKey_down,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->NextVar(); } );
    HandleDigitalButtonPress(EngineInput::kKey_up,	frameTime, []{ sm_SelectedVariable = sm_SelectedVariable->PrevVar(); } );

}

/*
void StartSave(void*)
{
    FILE* settingsFile;
    fopen_s(&settingsFile, "engineTuning.txt", "wb");
    if (settingsFile != nullptr)
    {
        VariableGroup::sm_RootGroup.SaveToFile(settingsFile, 2 );
        fclose(settingsFile);
    }
}
std::function<void(void*)> StartSaveFunc = StartSave;
static CallbackTrigger Save("Save Settings", StartSaveFunc, nullptr); 

void StartLoad(void*)
{
    FILE* settingsFile;
    fopen_s(&settingsFile, "engineTuning.txt", "rb");
    if (settingsFile != nullptr)
    {
        VariableGroup::sm_RootGroup.LoadSettingsFromFile(settingsFile);
        fclose(settingsFile);
    }
}
std::function<void(void*)> StartLoadFunc = StartLoad;
static CallbackTrigger Load("Load Settings", StartLoadFunc, nullptr); 
*/

void EngineTuning::AddToVariableGraph( const string& path, CreativeEngineType& var )
{
    vector<string> separatedPath;
    string leafName;
    size_t start = 0, end = 0;

    while (1)
    {
        end = path.find('/', start);
        if (end == string::npos)
        {
            leafName = path.substr(start);
            break;
        }
        else
        {
            separatedPath.push_back(path.substr(start, end - start));
            start = end + 1;
        }
    }

    VariableGroup* group = &VariableGroup::sm_RootGroup;

    for (auto iter = separatedPath.begin(); iter != separatedPath.end(); ++iter )
    {
        VariableGroup* nextGroup;
        CreativeEngineType* node = group->FindChild(*iter);
        if (node == nullptr)
        {
            nextGroup = new VariableGroup();
            group->AddChild(*iter, *nextGroup);
            group = nextGroup;
        }
        else
        {
            nextGroup = dynamic_cast<VariableGroup*>(node);
            ASSERT(nextGroup != nullptr, "Attempted to trash the tweak graph");
            group = nextGroup;
        }
    }

    group->AddChild(leafName, var);
}

void EngineTuning::RegisterVariable( const std::string& path, CreativeEngineType& var )
{
    if (s_UnregisteredCount >= 0)
    {
        int32_t Idx = s_UnregisteredCount++;
        strcpy_s(s_UnregisteredPath[Idx], path.c_str());
        s_UnregisteredVariable[Idx] = &var;
    }
    else
    {
        AddToVariableGraph( path, var );
    }
}

bool EngineTuning::IsFocused( void )
{
    return sm_IsVisible;
}
