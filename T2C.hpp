/* Copyright (C) 2021 X-Ryl669 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

// We need size_t
#include <stdint.h>
// We need malloc and free for the (optional) allocator
#include <stdlib.h>
// We need memcpy too
#include <string.h>

/** Define this only if you only need the decompressor */
// #define OnlyDecompressor

typedef unsigned char u8;

/** A simple allocator interface that's used by the compressor. 
    The engine does not need that the returned memory is initialized */
struct Allocator
{
    void * alloc(size_t size) { return ::malloc(size); }
    void release(void * ptr) { return ::free(ptr); }
};


/** The LZJB compression and decompression engine 
    @param LempelHashTableBucketCount   The number of buckets in the Lempel hash table, each bucket takes 16 bits. This is the amount of internal memory used by the algorithm. */
template <size_t LempelHashTableBucketCount = 256>
class T2CT
{
    /** The algorithm parameters */
    enum Parameters
    {
        MatchLen     = 16,                                  //!< The length of the word used to store a match in bits
        /** The logarithm of the maximum size of a match (currently log2(64) = 6 bits). Since the overhead of the algorithm is 2 bytes to store a match, there is no point to store value [0-2] so the actual match length is 66 bytes */
        MatchBits    = 6,
        MatchMin     = 3,                                   //!< The minimum match length that's efficient to store is 3 bytes (since writting a match takes 2 bytes)
        MatchMax     = (1 << MatchBits) + (MatchMin - 1),   //!< The maximum match length is (64 + 3 - 1) = 66 bytes
        /** The match size takes 16 bits where 6 bits are used to store the length of the match, so the maximum offset is 2^10 - 1 */
        OffsetMask   = (1 << (MatchLen - MatchBits)) - 1,

        LempelTableMask = LempelHashTableBucketCount - 1,   //!< Used to compute the modulo with only a AND operation
    };

public:
    #ifndef OnlyDecompressor
        /** Compute a hash from the given bytes */
        static inline size_t computeHash(const u8 * src) { return (src[0] + 13) ^ (src[1] - 13) ^ src[2]; }

        /** Compress the given source buffer into the destination buffer
            @param src  A pointer to a source buffer of len bytes
            @param len  The length of the buffer pointed by src in bytes
            @param dest A pointer to the destination buffer
            @param allocator    If provided, this allocator is used to create the Lempel hash table, else it's allocated on the stack
            @return The destination size in bytes. At worst the destination buffer will use (len * 9) / 8 bytes.
            It's safe to call with a null dest pointer to query the required size for compressed buffer */
        template <typename AllocatorIF = Allocator>
        static size_t compress(const void * src, size_t len, u8 * dest, AllocatorIF * allocator = nullptr);
    #endif

    /** Decompress the given source buffer into the destination buffer
        @param src  A pointer to a source buffer of slen bytes
        @param slen The length of the buffer pointed by src in bytes
        @param dest A pointer to the destination buffer allocated to at least dlen bytes
        @param dlen  The length of the buffer pointed by dest in bytes
        @return The used destination size in bytes. At worst the destination buffer will use (slen * max compression ratio) bytes.
        It's safe to call with a null dest pointer to query the required size for decompressed buffer */
    static size_t decompress(const u8 * src, size_t slen, u8 * dest, size_t dlen);
};


#ifndef OnlyDecompressor
    #include "Compress.inc"
#endif

#include "Decompress.inc"