

#include "pch.h"
#include "CUtility.h"
#include <string>
#include <locale>

// A faster version of memcopy that uses SSE instructions.  TODO:  Write an ARM variant if necessary.
void SIMDMemCopy( void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords )
{
    ASSERT(CMath::IsAligned(_Dest, 16));
    ASSERT(CMath::IsAligned(_Source, 16));

    __m128i* __restrict Dest = (__m128i* __restrict)_Dest;
    const __m128i* __restrict Source = (const __m128i* __restrict)_Source;

    // Discover how many quadwords precede a cache line boundary.  Copy them separately.
    size_t InitialQuadwordCount = (4 - ((size_t)Source >> 4) & 3) & 3;
    if (InitialQuadwordCount > NumQuadwords)
        InitialQuadwordCount = NumQuadwords;

    switch (InitialQuadwordCount)
    {
    case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
    case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
    case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
    default:
        break;
    }

    if (NumQuadwords == InitialQuadwordCount)
        return;

    Dest += InitialQuadwordCount;
    Source += InitialQuadwordCount;
    NumQuadwords -= InitialQuadwordCount;

    size_t CacheLines = NumQuadwords >> 2;

    switch (CacheLines)
    {
    default:
    case 10: _mm_prefetch((char*)(Source + 36), _MM_HINT_NTA);	// Fall through
    case 9:  _mm_prefetch((char*)(Source + 32), _MM_HINT_NTA);	// Fall through
    case 8:  _mm_prefetch((char*)(Source + 28), _MM_HINT_NTA);	// Fall through
    case 7:  _mm_prefetch((char*)(Source + 24), _MM_HINT_NTA);	// Fall through
    case 6:  _mm_prefetch((char*)(Source + 20), _MM_HINT_NTA);	// Fall through
    case 5:  _mm_prefetch((char*)(Source + 16), _MM_HINT_NTA);	// Fall through
    case 4:  _mm_prefetch((char*)(Source + 12), _MM_HINT_NTA);	// Fall through
    case 3:  _mm_prefetch((char*)(Source + 8 ), _MM_HINT_NTA);	// Fall through
    case 2:  _mm_prefetch((char*)(Source + 4 ), _MM_HINT_NTA);	// Fall through
    case 1:  _mm_prefetch((char*)(Source + 0 ), _MM_HINT_NTA);	// Fall through

        // Do four quadwords per loop to minimize stalls.
        for (size_t i = CacheLines; i > 0; --i)
        {
            // If this is a large copy, start prefetching future cache lines.  This also prefetches the
            // trailing quadwords that are not part of a whole cache line.
            if (i >= 10)
                _mm_prefetch((char*)(Source + 40), _MM_HINT_NTA);

            _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
            _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
            _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
            _mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

            Dest += 4;
            Source += 4;
        }

    case 0:	// No whole cache lines to read
        break;
    }

    // Copy the remaining quadwords
    switch (NumQuadwords & 3)
    {
    case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
    case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
    case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
    default:
        break;
    }

    _mm_sfence();
}

void SIMDMemFill( void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords )
{
    ASSERT(CMath::IsAligned(_Dest, 16));

    register const __m128i Source = _mm_castps_si128(FillVector);
    __m128i* __restrict Dest = (__m128i* __restrict)_Dest;

    switch (((size_t)Dest >> 4) & 3)
    {
    case 1: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
    case 2: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
    case 3: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
    default:
        break;
    }

    size_t WholeCacheLines = NumQuadwords >> 2;

    // Do four quadwords per loop to minimize stalls.
    while (WholeCacheLines--)
    {
        _mm_stream_si128(Dest++, Source);
        _mm_stream_si128(Dest++, Source);
        _mm_stream_si128(Dest++, Source);
        _mm_stream_si128(Dest++, Source);
    }

    // Copy the remaining quadwords
    switch (NumQuadwords & 3)
    {
    case 3: _mm_stream_si128(Dest++, Source);	 // Fall through
    case 2: _mm_stream_si128(Dest++, Source);	 // Fall through
    case 1: _mm_stream_si128(Dest++, Source);	 // Fall through
    default:
        break;
    }

    _mm_sfence();
}

std::wstring CUtility::UTF8ToWideString( const std::string& str )
{
    wchar_t wstr[MAX_PATH];
    if ( !MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), -1, wstr, MAX_PATH) )
        wstr[0] = L'\0';
    return wstr;
}

std::string CUtility::WideStringToUTF8( const std::wstring& wstr )
{
    char str[MAX_PATH];
    if ( !WideCharToMultiByte(CP_ACP, MB_PRECOMPOSED, wstr.c_str(), -1, str, MAX_PATH, nullptr, nullptr) )
        str[0] = L'\0';
    return str;
}

std::string CUtility::ToLower(const std::string& str)
{
    std::string lower_case = str;
    std::locale loc;
    for (char& s : lower_case)
        s = std::tolower(s, loc);
    return lower_case;
}

std::wstring CUtility::ToLower(const std::wstring& str)
{
    std::wstring lower_case = str;
    std::locale loc;
    for (wchar_t& s : lower_case)
        s = std::tolower(s, loc);
    return lower_case;
}

std::string CUtility::GetBasePath(const std::string& filePath)
{
    size_t lastSlash;
    if ((lastSlash = filePath.rfind('/')) != std::string::npos)
        return filePath.substr(0, lastSlash + 1);
    else if ((lastSlash = filePath.rfind('\\')) != std::string::npos)
        return filePath.substr(0, lastSlash + 1);
    else
        return "";
}

std::wstring CUtility::GetBasePath(const std::wstring& filePath)
{
    size_t lastSlash;
    if ((lastSlash = filePath.rfind(L'/')) != std::wstring::npos)
        return filePath.substr(0, lastSlash + 1);
    else if ((lastSlash = filePath.rfind(L'\\')) != std::wstring::npos)
        return filePath.substr(0, lastSlash + 1);
    else
        return L"";
}

std::string CUtility::RemoveBasePath(const std::string& filePath)
{
    size_t lastSlash;
    if ((lastSlash = filePath.rfind('/')) != std::string::npos)
        return filePath.substr(lastSlash + 1, std::string::npos);
    else if ((lastSlash = filePath.rfind('\\')) != std::string::npos)
        return filePath.substr(lastSlash + 1, std::string::npos);
    else
        return filePath;
}

std::wstring CUtility::RemoveBasePath(const std::wstring& filePath)
{
    size_t lastSlash;
    if ((lastSlash = filePath.rfind(L'/')) != std::string::npos)
        return filePath.substr(lastSlash + 1, std::string::npos);
    else if ((lastSlash = filePath.rfind(L'\\')) != std::string::npos)
        return filePath.substr(lastSlash + 1, std::string::npos);
    else
        return filePath;
}

std::string CUtility::GetFileExtension(const std::string& filePath)
{
    std::string fileName = RemoveBasePath(filePath);
    size_t extOffset = fileName.rfind('.');
    if (extOffset == std::wstring::npos)
        return "";

    return fileName.substr(extOffset + 1);
}

std::wstring CUtility::GetFileExtension(const std::wstring& filePath)
{
    std::wstring fileName = RemoveBasePath(filePath);
    size_t extOffset = fileName.rfind(L'.');
    if (extOffset == std::wstring::npos)
        return L"";

    return fileName.substr(extOffset + 1);
}

std::string CUtility::RemoveExtension(const std::string& filePath)
{
    return filePath.substr(0, filePath.rfind("."));
}

std::wstring CUtility::RemoveExtension(const std::wstring& filePath)
{
    return filePath.substr(0, filePath.rfind(L"."));
}
