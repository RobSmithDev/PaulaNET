# LZ4 in PaulaNET.device

An optional LZ4 encoding for Ethernet frames, on top of the existing MFM and RLE encodings. It is used only
between a driver and a board that have both said they support it; with anything else the driver behaves exactly as
before.

Written by Stefan Skotte (webadf) for Rob Smith's PaulaNET. `webadf_lz4.c/.h`, `lz4_68k.asm/.h` and this file are
MIT-licensed (`LICENSE-lz4`; the sources' "LICENSE in this directory" means that file). The changes to
`paulanetio.c` and `Makefile` are a few lines each, marked `webadf LZ4`.

## Which directions use LZ4

The driver reads `SysBase->AttnFlags` at open:

| CPU | board -> Amiga | Amiga -> board |
|---|---|---|
| 68000 / 68010 | LZ4 | RLE (as today) |
| 68020 / 68030 | LZ4 | LZ4 (encoder hash table 1,024 entries) |
| 68040 / 68060 | LZ4 | LZ4 (encoder hash table 4,096 entries) |
| a driver without this change (any CPU) | MFM | RLE |

A 68000 decodes LZ4 quickly but encodes it too slowly to gain anything, so it keeps RLE for uploads. Uploads use LZ4
only where they used RLE (`FLAGS_AMIGA_COMPRESSION` set); with that flag off they stay MFM.

## Wire format

Each frame in a batch has a 16-bit size word, as today:

- Board -> Amiga: bit 15 = MFM-encoded payload; clear = raw RLE.
- Amiga -> board: bit 15 = RLE; clear = MFM.

New, only between parties that have both advertised LZ4 (next section):

- **Bit 14 (`0x4000`) = LZ4 block.** Bits 0-13 give the LZ4 block's length in bytes (at most 16,383).
- Board -> Amiga, an LZ4 frame is `0xC000 | n`. The n LZ4 bytes are MFM-encoded on the wire (2n bytes), the same way
  plain frames are, because raw bytes toward the Amiga break its reads (see the RLE -> MFM pull request).
- Amiga -> board, an LZ4 frame is `0x4000 | n`. The n LZ4 bytes go out raw, as the driver sends RLE today, padded to
  an even length. No raw block carries more than 5 zero bytes in a row (pad included), so no gap in the bit stream
  exceeds 55 cells.
- Blocks use the standard LZ4 block format (compatible with liblz4 1.9 `LZ4_compress_default`, little-endian
  offsets). There is no LZ4 frame header and no content checksum; the batch checksum already covers the bytes.
- **Linked within a batch.** The decoded frames of one batch lie back to back in a 16,128-byte window, so an LZ4
  frame's matches may reach back into the earlier frames *of the same batch*. A match never crosses into another
  batch, so a lost batch cannot corrupt the next one. Every frame starts at an even window offset: after an
  odd-length frame both sides lay one zero pad byte. The pad is never sent, counts toward the 16,128 bytes, and is
  written as 0 every batch because a match may cover it. A raw RLE frame from the board is decoded outside the
  window, so no later frame of that batch may link past it.
- **Chosen frame by frame.** The encoder uses LZ4 only when the block is shorter than the frame's present encoding:
  toward the Amiga, the raw length (both then MFM-doubled); toward the board, the RLE length. Otherwise the frame
  goes out exactly as today, so incompressible data is never larger than before.

## Capability exchange

- **Board -> driver:** `LiveStatus.reserved` (cylinder-76 status, byte 155; always 0 today, and the Config tool
  never writes it). Bit 0 means "the board sends LZ4 to a driver that asks for it"; bit 1 means "the board accepts
  LZ4 frames".
- **Driver -> board:** this driver sets `0x10` in `DataHeader.compressed` on every batch it writes, meaning "I
  decode LZ4". The board sends LZ4 only while the most recent batch from the Amiga carried `0x10`. A driver restart,
  or a swap back to a driver without `0x10`, returns the board to MFM only from its next batch on. A swap made
  partway through a session can lose one LZ4 batch, which IP resends.
- **Upload gate:** LZ4 frames go to the board only on a 68020 or faster, and only while status bit 1 is set.

Your Pico firmware leaves byte 155 at 0, so with it this driver behaves exactly as today (MFM down, RLE up). The
board half is in webadf's firmware, `wifi-floppy/firmware/src/pnet_lz4.c` (the C LZ4 codec, linked within a batch)
and `pnet_pack.c` (choosing an encoding for each frame and packing the batch). Both are MIT, there for you to port.

## Memory

The driver makes one extra `AllocMem` at open: the window, the scratch buffer and, on 68020+, the upload hash table.
It requests `MEMF_PUBLIC`, plus `MEMF_REVERSE` on exec V39+, so the block does not land below `$10000`, where the
68000 decoder runs its slower checked loop. If that allocation fails, the device still opens and runs as the stock
driver: it does not set `0x10` and decodes or sends no LZ4.

## Testing

`lz4_68k.asm` is checked against liblz4 and its own C model under the vamos emulator (68000, 68020, 68040). The
driver's LZ4 half (`webadf_lz4.c`) is checked against webadf's board C under vamos on the same CPUs: decoding,
linking, window resets per batch, malformed input, the even-offset pad and the zero-run limit.
