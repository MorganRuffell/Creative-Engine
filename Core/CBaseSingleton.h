#pragma once


/// <summary>
/// This is base singleton command and graphics contexts utilise this class
/// because they cannot be copied.
/// </summary>
class CBaseSingleton
{
public:
	CBaseSingleton() = default;
	CBaseSingleton(const CBaseSingleton&) = delete;
	CBaseSingleton& operator= (const CBaseSingleton&) = delete;
};

