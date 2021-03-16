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
size_t T2C<LempelHashTableBucketCount>::decompress(const u8 * source, size_t slen, u8 * dest, size_t dlen)
{
    const u8 * send = source + slen;
    size_t ulen = 0;

    u8  copyMap = 0;
    int copyMask = 1 << 7;

    // If the output size is too small to store the source, it means it's not compressed, so just copy the data here
    if (slen >= dlen && dlen) 
    {
        if (dest) ::memcpy(dest, source, dlen);
        return dlen;
    }

    // Ok, let's decompress
    while (source < send && (!dest || ulen < dlen)) 
    {
        copyMask = copyMask << 1;
        // Is it the position of the bitmap ?
        if (copyMask == (1 << 8)) 
        {
            copyMask = 1;
            copyMap = *source++;
        }
        // Do we have a match for this position in the bitmap ?
        if (copyMap & copyMask) 
        {
            // Decode the match len and offset
            int mlen = (*source >> (8 - MatchBits)) + MatchMin;
            int offset = ((source[0] << 8) | source[1]) & OffsetMask;
            source += 2;
            // Then copy the matched bytes
            if (ulen >= offset) 
            {
                u8 * cpy = &dest[ulen] - offset;            
                while (--mlen >= 0 && (!dest || ulen < dlen)) 
                {
                    if (dest) dest[ulen] = *cpy++;
                    ulen++;
                }
            }
            else 
            {
                // The offset points to invalid position, so fail decoding here
                return ulen;
            }
        } else 
        {
            // No we don't, so just copy the byte here
            if (dest) dest[ulen] = *source;
            source++; ulen++;
        }
    }
    return ulen;
}