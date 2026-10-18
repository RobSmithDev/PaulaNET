/*
 * webadf_lz4.h: the LZ4 half of webadf's PaulaNET.device variant (spec docs/superpowers/specs/2026-10-18-paulanet-lz4-
 * design.md §2-§4). Copyright (c) 2026 Stefan Skotte and contributors. MIT licence. Not Rob Smith's code: his
 * paulanetio.c (third-party/paulanet/src, by his permission) calls it through tools/paulanet-driver/webadf-lz4.patch.
 *
 * Pure -- no exec or dos calls, no globals -- so tools/paulanet-driver/test.sh runs it under vamos against the board's
 * own C. Byte access only (68000), with one carve-out: the MFM wire of a frame from the board is read as words (see
 * wl_rx_frame), as Rob's mfmDecode reads it.
 *
 * Wire, only between parties that both advertised LZ4:
 *   size word 0xC000 | n   an n-byte LZ4 block, MFM-encoded (2n bytes): toward the Amiga
 *   size word 0x4000 | n   an n-byte LZ4 block, raw, padded to even: toward the board
 * Every frame of a batch is laid back to back in a window; an LZ4 frame's matches reach into the earlier frames of the
 * same batch, never further. Every frame starts at an even window offset: after an odd-length frame both sides lay one
 * 0x00 pad byte, never sent, counted toward WL_WINDOW (spec §2). Per CPU, from AttnFlags at open:
 *                                    board -> Amiga   Amiga -> board
 *   68000 / 68010                    LZ4              RLE
 *   68020 / 68030 (table 1,024)      LZ4              LZ4
 *   68040 / 68060 (table 4,096)      LZ4              LZ4
 *   Rob's stock driver (any CPU)     MFM              RLE
 */
#ifndef WEBADF_LZ4_H
#define WEBADF_LZ4_H
#include <exec/types.h>

#define WL_WINDOW        16128UL     /* a batch's frames decoded back to back (the board's PNET_LZ4_WINDOW) */
#define WL_BLOCK_MAX     1540UL      /* the longest block of one frame (the board's PNET_LZ4_BLOCK_MAX) */
#define WL_MTU           1518UL
#define WL_BROKEN        0xFFFFFFFFUL /* a window past which nothing may link */
#define WL_SIZE_LZ4      0x4000U     /* CompressionHeader.compressedSize: an LZ4 block of (size & 0x3FFF) bytes */
#define WL_DRIVER_LZ4    0x10U       /* DataHeader.compressed: this driver decodes LZ4 */
#define WL_ST_LZ4_IN     2U          /* LiveStatus.reserved (byte 155): the board accepts LZ4 frames */
/* The longest run of 0x00 bytes a raw block may carry to the board, its word pad counted. N zero bytes between two set
 * bits are a gap of up to 8N + 15 cells (7 clear bits at the end of the byte before, 8N, 7 at the start of the byte
 * after, plus one): N = 6 is 63, N = 7 is 71. The board refuses a gap over 64 (PNET_RX_MAX_CELLS) and the whole batch
 * with it, which Rob's resend-forever would turn into a stall. wl_tx_frame counts the 0x00 pad of an odd-length block
 * as part of the run, so with 5 every gap is 55 cells or less: 9 cells of margin under the board's limit. */
#define WL_ZERO_RUN_MAX  5UL

struct WlRx { UBYTE *win; ULONG pos; UBYTE *scratch; };
struct WlTx { UBYTE *win; ULONG pos; UBYTE *out; UWORD *table; ULONG entries; BOOL on; };

/* The upload hash table for this CPU: 0 (68000/68010: no LZ4 upload), 1,024 (68020/68030), 4,096 (68040 and up). */
ULONG wl_table_entries(UWORD attnFlags);
/* One allocation for both halves on this CPU (any memory, long aligned). Not in the lowest 64 KB: lz4_68k.h sends every
 * block whose window lies below $10000 through the decoder's slow checked loop (an A500's chip RAM starts there); the
 * patch allocates it MEMF_PUBLIC | MEMF_REVERSE on exec V39+ (the top of the memory list), and opens as Rob's stock
 * driver (no 0x10, no LZ4) when it cannot get it. */
ULONG wl_mem_bytes(UWORD attnFlags);
/* The upload hash table needs no clearing, at open or per batch: the encoder verifies every entry before it uses one,
 * so clearing 8 KB on a 68040 every batch would only cost time. */
void  wl_init(struct WlRx *rx, struct WlTx *tx, UBYTE *mem, UWORD attnFlags);
/* A frame's bytes on the wire from its size word: 2n for 0xC000|n, else size & 0x7FFF (Rob's rule). */
UWORD wl_wire_bytes(UWORD field);
void  wl_rx_begin(struct WlRx *rx);   /* a new batch from the board: the window starts empty */
void  wl_rx_break(struct WlRx *rx);   /* a frame decoded outside the window (Rob's raw RLE): no LINK past it */
/* One frame of a board -> Amiga batch whose size word has bit 15 set: MFM, or (0xC000) an MFM-encoded LZ4 block.
 * `wire` must be word aligned (it is: Rob's receive walks the payload as UWORD *pkt, and every frame's wire length is
 * even). Decoded into the window; returns the frame's orig bytes, or 0 (drop it) when it does not decode. Stricter than
 * Rob's receive on plain MFM frames too: orig 0, orig over 1,518 or a size word under 2 x orig is dropped (he would
 * hand the callback a clamped or partly stale frame); a correct board never sends one. */
const UBYTE *wl_rx_frame(struct WlRx *rx, UWORD field, const UBYTE *wire, UWORD orig);
/* A new batch toward the board: LZ4 on when this CPU has a table and status byte 155 has WL_ST_LZ4_IN. */
void  wl_tx_begin(struct WlTx *tx, UBYTE statusReserved);
/* Call it only after Rob's space and packet-count checks have taken the frame into this batch: once a frame joins the
 * window it must go in this batch, or the board's window and this one no longer agree.
 * After Rob's RLE of one frame (rleLen bytes at body): the frame joins the window; returns the length of an LZ4 block
 * now at body (shorter than rleLen, padded to even with a 0, no zero run over WL_ZERO_RUN_MAX pad included), or 0:
 * keep the RLE. */
UWORD wl_tx_frame(struct WlTx *tx, const UBYTE *data, UWORD len, UBYTE *body, UWORD rleLen);
/* The longest run of 0x00 bytes in p[0..n). */
ULONG wl_zero_run(const UBYTE *p, ULONG n);
#endif
