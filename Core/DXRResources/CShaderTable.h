#pragma once
#include "CDXRGPUUploadBuffer.h"
#include "CShaderRow.h"
#include <unordered_map>
#include <iosfwd>
#include <string>

/// <summary>
/// This is the CShaderTable class, it contains information about each DXR shader in the scene
/// Stored in a SQL Style database, with primary keys.
/// </summary>
class CShaderTable : public CDXRGPUUploadBuffer
{
public:
    CShaderTable() = default;

    CShaderTable(ID3D12Device8* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr)
    {
        m_shaderRecordSize = (shaderRecordSize + (D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1) & ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1));

        m_shaderRecords.reserve(numShaderRecords);
        UINT bufferSize = numShaderRecords * m_shaderRecordSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedShaderRecords = MapCPUWriteOnly();
    }

    void push_back (const CShaderRow& shaderRecord)
    {
        if (m_shaderRecords.size() < m_shaderRecords.capacity() != true)
        {
            m_shaderRecords.push_back(shaderRecord);
            shaderRecord.CopyTo(m_mappedShaderRecords);
            m_mappedShaderRecords += m_shaderRecordSize;
        }
        else
        {

        }
        
    }

    UINT GetShaderRecordSize() { return m_shaderRecordSize; }

public:

    /* void DebugPrint(std::unordered_map<void*, std::wstring> shaderIdToStringMap)
     {
         std::cout << L"|--------------------------------------------------------------------\n";
         std::cout << L"|Shader table - " << m_name.c_str() << L": "
             << m_shaderRecordSize << L" | "
             << m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

         for (int i = 0; i < m_shaderRecords.size(); i++)
         {
             std::cout << L"| [" << i << L"]: ";
             std::wcout << shaderIdToStringMap[m_shaderRecords[i].ShaderIdentifier.ptr()] << L", ";
             std::cout << m_shaderRecords[i].ShaderIdentifier.size << L" + " << m_shaderRecords[i].LocalRootArguments.size << L" bytes \n";
         }
         std::cout << L"|--------------------------------------------------------------------\n";
         std::cout << L"\n";
     }*/


private:

    std::wstring                    m_name;
    std::vector<CShaderRow>         m_shaderRecords;

    uint8_t*                        m_mappedShaderRecords;
    UINT                            m_shaderRecordSize;
};

