#pragma once

#include <cstddef>

/// <summary>
/// The Smart Fat pointer is a smart pointer that is aware of its size -- It knows how fat it is!
/// it's similar to unique pointer.
/// But is used solely for creative engine DXR stuff.
///
/// He cannot be moved because he is fat.
/// He cannot be copied because he is fat.
/// </summary>

template<class object>
class CSmartFatPointer
{
public:

    CSmartFatPointer()
    {
        object* nullptr;
        Size = 0;
    }

    CSmartFatPointer(object* _ptr, UINT _size)
    {
        m_RawPointer* = _ptr;
        Size = _size;
    }

    ~CSmartFatPointer()
    {
        delete(m_RawPointer);
        if (m_OldPointer != nullptr)
        {
            delete(m_OldPointer);
        }
        m_Size = NULL;
    }

public:

    UINT GetSize() const
    {
        return m_Size;
    }

    object* GetPointer() const
    {
        return m_RawPointer;
    }

    void Reset()
    {
        m_OldPointer = m_RawPointer;
        m_RawPointer = std::nullptr_t;
    }


private:

    object* m_RawPointer;

    //Cached old pointer -- Smart, remembers what he was originally.
    object* m_OldPointer;
    UINT        m_Size;

};

