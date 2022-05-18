
#include "pch.h"
#include "FileUtility.h"
#include <fstream>
#include <mutex>
#include <zlib.h> // From NuGet package 

using namespace std;
using namespace CUtility;

namespace CUtility
{
    CSmartByteVector NullFile = make_shared<vector<cs_byte> > (vector<cs_byte>() );
}

CSmartByteVector DecompressZippedFile( wstring& fileName );

CSmartByteVector ReadFileHelper(const wstring& fileName)
{
    struct _stat64 fileStat;
    int fileExists = _wstat64(fileName.c_str(), &fileStat);
    if (fileExists == -1)
        return NullFile;

    ifstream file( fileName, ios::in | ios::binary );
    if (!file)
        return NullFile;

    CUtility::CSmartByteVector CSmartByteVector = make_shared<vector<cs_byte> >( fileStat.st_size );
    file.read( (char*)CSmartByteVector->data(), CSmartByteVector->size() );
    file.close();

    return CSmartByteVector;
}

CSmartByteVector ReadFileHelperEx( shared_ptr<wstring> fileName)
{
    std::wstring zippedFileName = *fileName + L".gz";
    CSmartByteVector firstTry = DecompressZippedFile(zippedFileName);
    if (firstTry != NullFile)
        return firstTry;

    return ReadFileHelper(*fileName);
}

CSmartByteVector Inflate(CSmartByteVector CompressedSource, int& err, uint32_t ChunkSize = 0x100000 ) 
{
    // Create a dynamic buffer to hold compressed blocks
    vector<unique_ptr<cs_byte> > blocks;

    z_stream strm  = {};
    strm.data_type = Z_BINARY;
    strm.total_in  = strm.avail_in  = (uInt)CompressedSource->size();
    strm.next_in   = CompressedSource->data();

    err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib

    while (err == Z_OK || err == Z_BUF_ERROR)
    {
        strm.avail_out = ChunkSize;
        strm.next_out = (cs_byte*)malloc(ChunkSize);
        blocks.emplace_back(strm.next_out);
        err = inflate(&strm, Z_NO_FLUSH);
    }

    if (err != Z_STREAM_END) 
    {
        inflateEnd(&strm);
        return NullFile;
    }

    ASSERT(strm.total_out > 0, "Nothing to decompress");

    CUtility::CSmartByteVector CSmartByteVector = make_shared<vector<cs_byte> >( strm.total_out );

    // Allocate actual memory for this.
    // copy the bits into that RAM.
    // Free everything else up!!
    void* curDest = CSmartByteVector->data();
    size_t remaining = CSmartByteVector->size();

    for (size_t i = 0; i < blocks.size(); ++i)
    {
        ASSERT(remaining > 0);

        size_t CopySize = remaining < ChunkSize ? remaining : ChunkSize;

        memcpy(curDest, blocks[i].get(), CopySize);
        curDest = (cs_byte*)curDest + CopySize;
        remaining -= CopySize;
    }

    inflateEnd(&strm);

    return CSmartByteVector;
}

CSmartByteVector DecompressZippedFile( wstring& fileName )
{
    CSmartByteVector CompressedFile = ReadFileHelper(fileName);
    if (CompressedFile == NullFile)
        return NullFile;

    int error;
    CSmartByteVector DecompressedFile = Inflate(CompressedFile, error);
    if (DecompressedFile->size() == 0)
    {
        CUtility::Printf(L"Couldn't unzip file %s:  Error = %d\n", fileName.c_str(), error);
        return NullFile;
    }

    return DecompressedFile;
}

CSmartByteVector CUtility::ReadFileSync( const wstring& fileName)
{
    return ReadFileHelperEx(make_shared<wstring>(fileName));
}

task<CSmartByteVector> CUtility::ReadFileAsync(const wstring& fileName)
{
    shared_ptr<wstring> SharedPtr = make_shared<wstring>(fileName);
    return create_task( [=] { return ReadFileHelperEx(SharedPtr); } );
}
