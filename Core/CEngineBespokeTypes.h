
#pragma once

#include <string>
#include <stdint.h>
#include <float.h>
#include <map>
#include <set>
#include <functional>
#include <vector>

class VariableGroup;

/*
    Creative Engine utlilizes callback data types.

    These act in a similar way to the way that delegates work in unreal C++
    they act as wrappers around std::function

*/

class CreativeEngineType
{
public:

    virtual ~CreativeEngineType() {}

    virtual void Increment( void ) { m_ActionCallback(ActionType::Increment); }	// DPad Right
    virtual void Decrement( void ) { m_ActionCallback(ActionType::Decrement); }	// DPad Left
    virtual void Bang( void ) { m_ActionCallback(ActionType::Bang); }		// A Button

    virtual std::string ToString( void ) const { return ""; }
    virtual void SetValue( FILE* file, const std::string& setting) = 0; //set value read from file

    CreativeEngineType* NextVar( void );
    CreativeEngineType* PrevVar( void );

    enum class ActionType
    {
        Increment,
        Decrement,
        Bang
    };

    typedef std::function<void(ActionType)> ActionCallback;

protected:

    CreativeEngineType( void );
    CreativeEngineType( const std::string& path, ActionCallback pfnCallback = DefaultActionHandler );

    static void DefaultActionHandler(ActionType)
    {
        // nothing
    }

private:
    friend class VariableGroup;
    VariableGroup* m_GroupPtr;

    ActionCallback m_ActionCallback;
};

class CETBoolean : public CreativeEngineType
{
public:
    CETBoolean( const std::string& path, bool val, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler );
    CETBoolean& operator=( bool val ) { m_Flag = val; return *this; }
    operator bool() const { return m_Flag; }

    virtual void Increment( void ) override { m_Flag = true; CreativeEngineType::Increment(); }
    virtual void Decrement( void ) override { m_Flag = false; CreativeEngineType::Decrement(); }
    virtual void Bang( void ) override { m_Flag = !m_Flag;  CreativeEngineType::Bang(); }

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting) override;

private:
    bool m_Flag;
};

class CETFloat : public CreativeEngineType
{
public:
    CETFloat( const std::string& path, float val, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler);
    CETFloat& operator=( float val ) { m_Value = Clamp(val); return *this; }
    operator float() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = Clamp(m_Value + m_StepSize); CreativeEngineType::Increment(); }
    virtual void Decrement( void ) override { m_Value = Clamp(m_Value - m_StepSize); CreativeEngineType::Decrement(); }

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting)  override;

protected:
    float Clamp( float val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    float m_Value;
    float m_MinValue;
    float m_MaxValue;
    float m_StepSize;
};

//Exponential Value type, builds on CETFloat
class ExponentialCETFloat : public CETFloat
{
public:
    ExponentialCETFloat( const std::string& path, float val, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler);
    ExponentialCETFloat& operator=( float val );	// m_Value = log2(val)
    operator float() const;			// returns exp2(m_Value)

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

};

class CETInt : public CreativeEngineType
{
public:
    CETInt( const std::string& path, int32_t val, int32_t minValue = 0, int32_t maxValue = (1 << 24) - 1, int32_t stepSize = 1, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler);
    CETInt& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = Clamp(m_Value + m_StepSize); CreativeEngineType::Increment(); }
    virtual void Decrement( void ) override { m_Value = Clamp(m_Value - m_StepSize); CreativeEngineType::Decrement(); }

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

protected:
    int32_t Clamp( int32_t val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    int32_t m_Value;
    int32_t m_MinValue;
    int32_t m_MaxValue;
    int32_t m_StepSize;
};

class CETStaticEnum : public CreativeEngineType
{
public:
    CETStaticEnum( const std::string& path, int32_t initialVal, int32_t listLength, const char** listLabels, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler);
    CETStaticEnum& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = (m_Value + 1) % m_EnumLength; CreativeEngineType::Increment(); }
    virtual void Decrement( void ) override { m_Value = (m_Value + m_EnumLength - 1) % m_EnumLength; CreativeEngineType::Decrement(); }

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

    void SetListLength(int32_t listLength) { m_EnumLength = listLength; m_Value = Clamp(m_Value); }

private:
    int32_t Clamp( int32_t val ) { return val < 0 ? 0 : val >= m_EnumLength ? m_EnumLength - 1 : val; }

    int32_t m_Value;
    int32_t m_EnumLength;
    const char** m_EnumLabels;
};

class CETDynmEnum : public CreativeEngineType
{
public:
    CETDynmEnum( const std::string& path, ActionCallback pfnCallback = CreativeEngineType::DefaultActionHandler);
    CETDynmEnum& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    virtual void Increment( void ) override { m_Value = (m_Value + 1) % m_EnumCount; CreativeEngineType::Increment(); }
    virtual void Decrement( void ) override { m_Value = (m_Value + m_EnumCount - 1) % m_EnumCount; CreativeEngineType::Decrement(); }

    virtual std::string ToString( void ) const override;
    virtual void SetValue( FILE* file, const std::string& setting ) override;

    void AddEnum(const std::wstring& enumLabel) { m_EnumLabels.push_back(enumLabel); m_EnumCount++; }

private:
    int32_t Clamp( int32_t val ) { return val < 0 ? 0 : val >= m_EnumCount ? m_EnumCount - 1 : val; }

    int32_t m_Value;
    int32_t m_EnumCount;
    std::vector<std::wstring> m_EnumLabels;
};

class CallbackTrigger : public CreativeEngineType
{
public:
    CallbackTrigger( const std::string& path, std::function<void (void*)> callback, void* args = nullptr );

    virtual void Bang( void ) override { m_Callback(m_Arguments); m_BangDisplay = 64; }

    virtual void SetValue( FILE* file, const std::string& setting ) override;

private:
    std::function<void (void*)> m_Callback;
    void* m_Arguments;
    mutable uint32_t m_BangDisplay;
};

class CGraphicsContext;

namespace EngineTuning
{
    void Initialize( void );
    void Update( float frameTime );
    bool IsFocused( void );

}
