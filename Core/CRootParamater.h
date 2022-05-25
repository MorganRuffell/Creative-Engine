#pragma once

class CRootParameter
{
    friend class CRootSignature;
public:

    CRootParameter()
    {
        m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
    }

    ~CRootParameter()
    {
        Clear();
    }

    void Clear()
    {
        if (m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            delete[] m_RootParam.DescriptorTable.pDescriptorRanges;

        m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
    }

    void InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        m_RootParam.ShaderVisibility = Visibility;
        m_RootParam.Constants.Num32BitValues = NumDwords;
        m_RootParam.Constants.ShaderRegister = Register;
        m_RootParam.Constants.RegisterSpace = Space;
    }

    void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        m_RootParam.ShaderVisibility = Visibility;
        m_RootParam.Descriptor.ShaderRegister = Register;
        m_RootParam.Descriptor.RegisterSpace = Space;
    }

    void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        m_RootParam.ShaderVisibility = Visibility;
        m_RootParam.Descriptor.ShaderRegister = Register;
        m_RootParam.Descriptor.RegisterSpace = Space;
    }

    void InitAsBufferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        m_RootParam.ShaderVisibility = Visibility;
        m_RootParam.Descriptor.ShaderRegister = Register;
        m_RootParam.Descriptor.RegisterSpace = Space;
    }

    void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
    {
        InitAsDescriptorTable(1, Visibility);
        SetTableRange(0, Type, Register, Count, Space);
    }

    void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
    {
        m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        m_RootParam.ShaderVisibility = Visibility;
        m_RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
        m_RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
    }

    void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space = 0)
    {
        D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
        range->RangeType = Type;
        range->NumDescriptors = Count;
        range->BaseShaderRegister = Register;
        range->RegisterSpace = Space;
        range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    const D3D12_ROOT_PARAMETER& operator() (void) const { return m_RootParam; }


protected:

    D3D12_ROOT_PARAMETER m_RootParam;
};

