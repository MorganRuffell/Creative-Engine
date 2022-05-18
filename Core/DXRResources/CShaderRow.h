#pragma once

#include "../CSmartFatPointer.h"
#include <stdio.h>
#include <vcruntime_string.h>

/// <summary>
/// Singular row element of  CShaderTable - These are information management
/// tools on the back end of creative.
/// </summary>
///
class CShaderRow
{
public:

    CShaderRow(void* pShaderIdentifier, int shaderIdentifierSize)
        : ShaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
    {

    }

    CShaderRow(void* pShaderIdentifier, int shaderIdentifierSize, void* pLocalRootArguments, int localRootArgumentsSize)
        : ShaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
         LocalRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
        
    }

    void CopyTo(void* dest) const
    {
        int* byteDest = static_cast<int*>(dest);
        memcpy(byteDest, ShaderIdentifier.ptr, ShaderIdentifier.size);
        if (LocalRootArguments.ptr)
        {
            memcpy(byteDest + ShaderIdentifier.size, LocalRootArguments.ptr, LocalRootArguments.size);
        }
    }

public:

    struct PointerWithSize {
        void* ptr;
        int size;

        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, int _size) : ptr(_ptr), size(_size) {};
    };

    PointerWithSize ShaderIdentifier;
    PointerWithSize LocalRootArguments;

};

