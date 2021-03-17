# Tinytinycompress (T2C)

This is a very very small data compressor and decompressor for embedded system with a minimal flash requirement.
It using less than 100 lines of code.


It's based on LZJB (the same algorithm found in OpenSolaris's ZFS source code).
The original code was written in C and under the CDDL license, while this code is written in C++ and under the GPLv3 license.

The description for the algorithm is copied from [the original source code](https://web.archive.org/web/20120608132900/http://src.opensolaris.org/source/xref/onnv/onnv-gate/usr/src/uts/common/os/compress.c):
```
This compression algorithm is a derivative of LZRW1, which I'll call
LZJB in the classic LZ* spirit.  All LZ* (Lempel-Ziv) algorithms are
based on the same basic principle: when a "phrase" (sequences of bytes)
is repeated in a data stream, we can save space by storing a reference to
the previous instance of that phrase (a "copy item") rather than storing
the phrase itself (a "literal item").  The compressor remembers phrases
in a simple hash table (the "Lempel history") that maps three-character
sequences (the minimum match) to the addresses where they were last seen.

A copy item must encode both the length and the location of the matching
phrase so that decompress() can reconstruct the original data stream.
For example, here's how we'd encode "yadda yadda yadda, blah blah blah"
(with "_" replacing spaces for readability):

Original:

y a d d a _ y a d d a _ y a d d a , _ b l a h _ b l a h _ b l a h

Compressed:

y a d d a _ 6 11 , _ b l a h 5 10

In the compressed output, the "6 11" simply means "to get the original
data, execute memmove(ptr, ptr - 6, 11)".  Note that in this example,
the match at "6 11" actually extends beyond the current location and
overlaps it.  That's OK; like memmove(), decompress() handles overlap.

There's still one more thing decompress() needs to know, which is how to
distinguish literal items from copy items.  We encode this information
in an 8-bit bitmap that precedes each 8 items of output; if the Nth bit
is set, then the Nth item is a copy item.  Thus the full encoding for
the example above would be:

0x40 y a d d a _ 6 11 , 0x20 _ b l a h 5 10

Finally, the "6 11" isn't really encoded as the two byte values 6 and 11
in the output stream because, empirically, we get better compression by
dedicating more bits to offset, fewer to match length.  LZJB uses 6 bits
to encode the match length, 10 bits to encode the offset.  Since copy-item
encoding consumes 2 bytes, we don't generate copy items unless the match
length is at least 3; therefore, we can store (length - 3) in the 6-bit
match length field, which extends the maximum match from 63 to 66 bytes.
Thus the 2-byte encoding for a copy item is as follows:

 byte[0] = ((length - 3) << 2) | (offset >> 8);
 byte[1] = (uint8_t)offset;

In our example above, an offset of 6 with length 11 would be encoded as:

 byte[0] = ((11 - 3) << 2) | (6 >> 8) = 0x20
 byte[1] = (uint8_t)6 = 0x6

Similarly, an offset of 5 with length 10 would be encoded as:

 byte[0] = ((10 - 3) << 2) | (5 >> 8) = 0x1c
 byte[1] = (uint8_t)5 = 0x5

Putting it all together, the actual LZJB output for our example is:

0x40 y a d d a _ 0x2006 , 0x20 _ b l a h 0x1c05

The main differences between LZRW1 and LZJB are as follows:

(1) LZRW1 is sloppy about buffer overruns.  LZJB never reads past the
    end of its input, and never writes past the end of its output.

(2) LZJB allows a maximum match length of 66 (vs. 18 for LZRW1), with
    the trade-off being a shorter look-behind (1K vs. 4K for LZRW1).

(3) LZJB records only the low-order 16 bits of pointers in the Lempel
    history (which is all we need since the maximum look-behind is 1K),
    and uses only 256 hash entries (vs. 4096 for LZRW1).  This makes
    the compression hash small enough to allocate on the stack, which
    solves two problems: (1) it saves 64K of kernel/cprboot memory,
    and (2) it makes the code MT-safe without any locking, since we
    don't have multiple threads sharing a common hash table.

(4) LZJB is faster at both compression and decompression, has a
    better compression ratio, and is somewhat simpler than LZRW1.

Finally, note that LZJB is non-deterministic: given the same input,
two calls to compress() may produce different output.  This is a
general characteristic of most Lempel-Ziv derivatives because there's
no need to initialize the Lempel history; not doing so saves time.
```


# Benchmark

The source file used for this benchmark is a G-Code file of 309 564 bytes

|  Algorithm  | Compressor binary size | Decompressor size | Internal memory    | Compressed data size |
|-------------|------------------------|-------------------|--------------------|----------------------|
| Heatshrink  | 1964 bytes (ARM)       | 1336 bytes (ARM)  | < 40 bytes (claim) | 135 398 bytes        |
| Heatshrink  | 2176 bytes (AVR)       | 1268 bytes (AVR)  | < 40 bytes (claim) |                      |
| T2C         | 528 bytes (ARM)        | 292 bytes (ARM)   | 512 bytes (temp)   | 196 217 bytes        |
| T2C         | 618 bytes (AVR)        | 300 bytes (AVR)   | 512 bytes (temp)   |                      |
| T2C(128)    | 528 bytes (ARM)        | 292 bytes (ARM)   | 256 bytes (temp)   | 199 021 bytes        |
| T2C(128)    | 618 bytes (AVR)        | 300 bytes (AVR)   | 256 bytes (temp)   |                      |

T2C can be build with a smaller hashtable (down to 128 buckets / 256 bytes) but it implies a smaller compression factor.

On average, T2C is 8x faster to compress and 4.5x faster to decompress then Heatshrink


# Documentation

More information about this code can be found on [this blog page](https://blog.cyril.by/en/software/tinytinycompressor-t2c).