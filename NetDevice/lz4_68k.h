/*
 * lz4_68k.h: an LZ4 block decoder and encoder for the plain 68000 (lz4_68k.asm), and the C
 * reference model of the encoder (lz4_68k.c).
 *
 * Copyright (c) 2026 Stefan Skotte and contributors. MIT licence (LICENSE in this directory).
 * Written from the LZ4 block format description (github.com/lz4/lz4, doc/lz4_Block_format.md).
 * The decoder's fast path uses the technique of Arnaud Carre's lz4_fastest (MIT,
 * github.com/arnaud-carre/lz4-68k): a token jump table into unrolled copy blocks. No code was
 * copied from it or from any other implementation.
 *
 * Format: the standard LZ4 *block* (no frame header, no checksum): little-endian offsets, the
 * last 5 bytes literals, the last match starting at least 12 bytes before the end. Blocks from
 * the reference liblz4 decode here, and blocks from here decode with liblz4's
 * LZ4_decompress_safe (or LZ4_decompress_safe_usingDict for a linked block).
 *
 * "Linked" blocks: the matches of a block may reach back before its own first byte into data
 * that sits directly before it in the same buffer (earlier frames of the same batch). The
 * encoder gets that buffer's start as `window`, the decoder the same start as `window`.
 *
 * All entry points are 68000 code: no 68010+ instructions, no word or long access at an odd
 * address. Buffers may be at any address (odd or even) unless stated.
 *
 * ---- Register entries (asm callers; every register not carrying the result is preserved) ----
 *
 * lz4_decode
 *   in:  a0 = src, d0.l = src length (the block's exact size)
 *        a1 = dst (where the decoded bytes go)
 *        a2 = dst_end (hard limit: nothing is written at or past it)
 *        a3 = window start: the lowest address a match may copy from. a3 = a1 for a plain
 *             block; a3 = the batch buffer's start for a linked block (a3 <= a1).
 *   out: d0.l = the decoded length, or -1 if the block is malformed, a match reaches below a3
 *        (or has offset 0), or the output would pass dst_end.
 *   Never writes outside [a1, a2), never reads src outside [a0, a0 + d0), never reads a match
 *   source outside [a3, the current output position). src must not overlap [a3, a2).
 *   Rejects an offset of 0 (the format says it is invalid; liblz4 does not check it).
 *   Returns -1 at once if a2 < a1 or a3 > a1. A window below $10000 sends the whole block
 *   through the slow checked loop (so that no match source can wrap below address 0).
 *   Built with LZ4_68K_UNCHECKED_READS = 1, the fast path does not check the window or offset
 *   0: writes still stay inside [a1, a2) and src is never read past its end, but a malformed
 *   block may make it copy from memory below a3 (and return a length, not -1).
 *   Accepts any block that is structurally valid (it does not enforce the encoder-side end
 *   rules "last 5 bytes literals / last match 12 bytes before the end"); an empty src (d0 = 0)
 *   is malformed (-1), the 1-byte block 0x00 decodes to 0 bytes.
 *
 * lz4_encode
 *   in:  a0 = src, d0.l = n (0..65535)
 *        a1 = dst, d1.l = cap: the most bytes dst may take
 *        a2 = hash table: d2.l entries of 16 bits (word aligned, so at an even address)
 *        d2.l = table entries, a power of two from 256 to 16384 (1024 = 2 KB, 4096 = 8 KB)
 *   out: d0.l = the block's size (1 .. cap), or 0 if the block would be larger than cap, or
 *        if n > 65535.
 *        Nothing is written at or past dst + cap, in either case.
 *   The table needs no clearing, ever: any contents are valid (stale entries are verified
 *   before use), so keep it between calls. Its contents only change which matches are found,
 *   never correctness. The same table may serve both entry points and any table size.
 *
 * lz4_encode_linked
 *   the same as lz4_encode, plus a3 = window start (a3 <= a0): matches may copy from
 *   [a3, a0 + position). Requires (a0 - a3) + n <= 65536 and a3 <= a0, else returns 0. Decode the block with the same
 *   window: lz4_decode with a3 = the window, a1 = the frame's place right after the earlier
 *   frames. A table carried over from encoding the earlier frames against the same window
 *   finds matches into them; that is how linking gains.
 *
 * Stack use (peak, below the caller's return address, saved registers included): the decoder
 * 64 bytes, the encoder 80; the C entries add 12 (decode, encode) or 16 (encode_linked).
 * Code: about 4.2 KB decoder, 1.0 KB encoder, 0.1 KB C entries (5.3 KB in all).
 *
 * ---- C entries (stack arguments; m68k-amigaos gcc, vbcc) ----
 * The same functions with the arguments below; the result in d0.
 */
#ifndef LZ4_68K_H
#define LZ4_68K_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Recommended table sizes (entries): 1024 (2 KB) for a 68000/68020, 4096 (8 KB) on 68040+. */
#define LZ4_68K_TABLE_SMALL 1024u
#define LZ4_68K_TABLE_LARGE 4096u

#if defined(__mc68000__) || defined(__m68k__) || defined(__VBCC__) || defined(__amigaos__) || \
    defined(LZ4_68K_ASM)
/* lz4_68k.asm */
long lz4_decode(const void *src, unsigned long srclen, void *dst, void *dst_end,
                const void *window);
unsigned long lz4_encode(const void *src, unsigned long n, void *dst, unsigned long cap,
                         uint16_t *table, unsigned long entries);
unsigned long lz4_encode_linked(const void *src, unsigned long n, void *dst, unsigned long cap,
                                uint16_t *table, unsigned long entries, const void *window);
#endif

/* lz4_68k.c, the reference model: the same bytes as lz4_encode / lz4_encode_linked from the
 * same arguments and table contents. Portable C (any host). */
unsigned long lz4_encode_ref(const void *src, unsigned long n, void *dst, unsigned long cap,
                             uint16_t *table, unsigned long entries);
unsigned long lz4_encode_linked_ref(const void *src, unsigned long n, void *dst,
                                    unsigned long cap, uint16_t *table, unsigned long entries,
                                    const void *window);

#ifdef __cplusplus
}
#endif
#endif
