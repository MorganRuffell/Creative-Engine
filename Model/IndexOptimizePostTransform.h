
#pragma once

//-----------------------------------------------------------------------------
//  OptimizeFaces
//-----------------------------------------------------------------------------
//  Parameters:
//      indexList
//          input index list
//      indexCount
//          the number of indices in the list
//      newIndexList
//          a pointer to a preallocated buffer the same size as indexList to
//          hold the optimized index list
//      lruCacheSize
//          the size of the simulated post-transform cache (max:64)
//-----------------------------------------------------------------------------
template <typename SrcIndexType, typename DstIndexType>
void OptimizeFaces(_Inout_ const SrcIndexType* indexList, _In_ size_t indexCount, _Inout_ DstIndexType* newIndexList, _In_ size_t lruCacheSize);

template void OptimizeFaces<uint16_t, uint16_t>(_In_ const uint16_t* indexList, size_t indexCount, uint16_t* newIndexList, _In_ size_t lruCacheSize);
template void OptimizeFaces<uint32_t, uint16_t>(_In_ const uint32_t* indexList, size_t indexCount, uint16_t* newIndexList, _In_ size_t lruCacheSize);
template void OptimizeFaces<uint32_t, uint32_t>(_In_ const uint32_t* indexList, size_t indexCount, uint32_t* newIndexList, _In_ size_t lruCacheSize);
