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

template <size_t LempelHashTableBucketCount>
template<typename AllocatorIF>
size_t T2CT<LempelHashTableBucketCount>::compress(const void * source, size_t len, u8 * dest, AllocatorIF * alloc)
{
    static_assert(  LempelHashTableBucketCount + MatchMin >= MatchMax 
                    && ((LempelHashTableBucketCount - 1) & LempelHashTableBucketCount) == 0, "The hash table is too small to find matches or is not a power of 2");

    typedef unsigned short u16;
    constexpr const size_t hashTableSize = sizeof(u16) * LempelHashTableBucketCount;
    int copyMask = 1 << 7;
    
    u16 * hashTable = alloc ? (u16*)alloc->alloc(hashTableSize) : (u16*)alloca(hashTableSize);
    const u8 * end = (const u8*)source + len, * src = (const u8*)source; 
    u8 * copyMap;
    size_t dlen = 0;

    while (src < end) 
    {
        // Is it time to output a bitmap ?
        copyMask = copyMask << 1;
        if (copyMask == (1 << 8)) 
        {
            // Check if the compression is worthless, that is if even with the best match here, it would still consume more space than the source buffer
            if (dlen >= len - 1 - 2 * 8) 
            {
                if (dest) ::memcpy(dest, source, len);
                if (alloc) alloc->release(hashTable);
                return len;
            }
            copyMask = 1;
            if (dest) {
                dest[dlen] = 0;
                copyMap = &dest[dlen];
            }
            dlen++;
        }
        // Skip searching if it can't match anymore
        if (src > end - MatchMax) 
        {
            if (dest) dest[dlen] = *src;
            dlen++; src++;
            continue;
        }
        // Find the bucket that the source would hash into
        u16 * htp = &hashTable[computeHash(src) & LempelTableMask];
        // Optimization: Compute the offset from the current source position to the stored position in the hash table. 
        // If it points to an uninitialized bucket it'll be ignored later on. Valgrind might scream here, but it's valid here 
        int offset = (intptr_t)(src - *htp) & OffsetMask;
        // Store the position of the source in the bucket
        *htp = (u16)(uintptr_t)src;
        
        const u8 * cpy = src - offset;
        // Check if the match is valid: if it's inside the expected range 
        if (cpy >= (const u8*)source && cpy != src 
        // and that the initial bytes are the same (not the case upon hash collision)        
            && src[0] == cpy[0] && src[1] == cpy[1] && src[2] == cpy[2]) 
        {
            // Mark the position in the bitmap to say that a match will happen now instead of the data
            if (dest) *copyMap |= copyMask; 
            // Find the maximum match len
            int mlen = MatchMin;
            while (src[mlen] == cpy[mlen] && mlen < MatchMax) mlen++;
            // Store the match in the destination now
            if (dest) 
            {
                dest[dlen++] = ((mlen - MatchMin) << (8 - MatchBits)) | (offset >> 8);
                dest[dlen++] = (u8)offset;
            } else dlen += 2;
            src += mlen;
        } 
        else
        {
            if (dest) dest[dlen] = *src;
            dlen++; src++;
        }
    }
    if (alloc) alloc->release(hashTable);
    return dlen;
}