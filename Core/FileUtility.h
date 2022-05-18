
#pragma once

#include "pch.h"
#include <vector>
#include <string>
#include <ppl.h>



namespace CUtility
{
    using namespace std;
    using namespace concurrency;

    typedef shared_ptr<vector<cs_byte> > CSmartByteVector;
    extern CSmartByteVector NullFile;

    // Reads the entire contents of a binary file.  If the file with the same name except with an additional
    // ".gz" suffix exists, it will be loaded and decompressed instead.
    // This operation blocks until the entire file is read.
    CSmartByteVector ReadFileSync(_In_ const wstring& fileName);

    // Same as previous except that it does not block but instead returns a task.
    task<CSmartByteVector> ReadFileAsync(const wstring& fileName);

} // namespace CUtility
