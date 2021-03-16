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

#include "../T2C.hpp"

#include <stdio.h>

int help(const char * name) { return fprintf(stderr, "Usage is: %s -c/-d [inFile] [outFile]\n", name); }

#ifndef HashTableSize
    #define HashTableSize 256
#endif

int main(int argc, char ** argv)
{
    if (argc < 4) return help(argv[0]);


    FILE * in = fopen(argv[2], "rb");
    if (!in) return fprintf(stderr, "Can not open file: %s\n", argv[2]);
    fseek(in, 0, SEEK_END);
    size_t inSize = ftell(in);
    fseek(in, 0, SEEK_SET);

    // The compression algorithm we are using (only important for the compressor, the decompressor doesn't care here)
    typedef T2CT<HashTableSize> T2C;

    // Allocate the memory to store the source file
    u8 * inBuffer = new u8[inSize];
    if (fread(inBuffer, 1, inSize, in) != inSize) return fprintf(stderr, "Failed to read input file\n");

    if (strncmp(argv[1], "-c", 2) == 0) {
        // Compress here
        // First pass to figure out the required output size:
        size_t outSize = T2C::compress(inBuffer, inSize, 0);
        u8 * outBuffer = new u8[outSize];
        // Second pass to capture the compressed file
        outSize = T2C::compress(inBuffer, inSize, outBuffer);
        FILE * out = fopen(argv[3], "wb");
        if (!out) return fprintf(stderr, "Can not open output file: %s\n", argv[3]);
        if (fwrite(outBuffer, 1, outSize, out) != outSize) return fprintf(stderr, "Failed to write output file\n");
        fclose(out);
        fclose(in);
        printf("Compressed %u bytes down to %u bytes: ratio: %.2f%%\n", inSize, outSize, outSize * 100.0 / inSize);
        return 0;
    }
    else if (strncmp(argv[1], "-d", 2) == 0) {
        // Decompress here
        // First pass to figure out the required output size:
        size_t outSize = T2C::decompress(inBuffer, inSize, 0, 0);
        u8 * outBuffer = new u8[outSize];
        // Second pass to capture the decompressed file
        outSize = T2C::decompress(inBuffer, inSize, outBuffer, outSize);
        FILE * out = fopen(argv[3], "wb");
        if (!out) return fprintf(stderr, "Can not open output file: %s\n", argv[3]);
        if (fwrite(outBuffer, 1, outSize, out) != outSize) return fprintf(stderr, "Failed to write output file\n");
        fclose(out);
        fclose(in);
        printf("Decompressed %u bytes up to %u bytes: ratio: %.2f%%\n", inSize, outSize, outSize * 100.0 / inSize);
        return 0;
    }
    else return help(argv[0]);

    return 0; 
}