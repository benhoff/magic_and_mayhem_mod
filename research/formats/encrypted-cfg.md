# Encrypted CFG container

## Scope and provenance

This describes the files in `CFG/Encrypted/*.cfg` in the clean working
installation. The findings were reproduced against:

- `Chaos.exe` SHA-256
  `124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`
- PE timestamp: 1998-11-13 07:13:14 (value reported by `objdump`)
- `CFG/Encrypted/creature.cfg` SHA-256
  `05be54554522af9645d1fa22e7711fdbd23d44756973f68a3b902edf79f3108a`

Run `./tools/decode-cfg.py` to validate every default input without writing
anything. Run `./tools/decode-cfg.py --output-dir working/decoded-cfg` to
extract them. Existing output is not replaced without an interactive
confirmation or `--force`.

## Confirmed structure

Confidence: high. Every one of the 11 shipped containers passes both
checksums, decodes to its recorded length, and matches the plaintext file
written by the game byte-for-byte.

The complete file is first obfuscated in place except for the seed at offset
zero. After undoing that transform, the little-endian header is:

| Offset | Size | Meaning |
|---:|---:|---|
| `0x00` | 4 | Cipher seed: `GetTickCount() XOR 0x5A2387C3` |
| `0x04` | 4 | Decoded byte length |
| `0x08` | 4 | Checksum of the packed payload |
| `0x0c` | 4 | Checksum of the decoded bytes |
| `0x10` | 4 | Packing mode: 0, 1, or 2 |
| `0x14` | rest | Packed payload |

The checksum considers complete little-endian 32-bit words only. Starting at
zero, it XORs words with even indices and adds words with odd indices modulo
2^32. Trailing one to three bytes are ignored.

### Outer transform

The seed initializes a 250-word pseudo-random generator. Each complete word
after offset `0x04` is XORed with one generated 32-bit value. Each trailing
byte also consumes one complete generated value and is XORed with its low
byte. The implementation is in `tools/decode-cfg.py`.

Relevant executable routines (preferred image base `0x00400000`):

- `0x00498720`: symmetric outer transform
- `0x00538770`: generator initialization
- `0x00538800`: generator output
- `0x00498646`, `0x004988e0`, `0x00499333`: construct the seed with
  `GetTickCount()` and `0x5A2387C3`

### Packing modes

- Mode 0 is an uncompressed byte copy.
- Mode 1 is byte-oriented run-length encoding. A positive signed control byte
  repeats the following byte that many times. A negative control byte copies
  that many following literal bytes. Zero emits nothing.
- Mode 2 is a bit-oriented LZSS variant. Bits are read most-significant first.
  A one bit precedes an eight-bit literal. A zero bit precedes a 12-bit
  absolute ring-buffer position; zero is the end marker. A nonzero position is
  followed by a four-bit value and copies `value + 2` bytes. The 4096-byte
  ring starts filled with zero and its initial write position is one.

Relevant unpacking routines are `0x00498990` for mode 1 and `0x00499150` for
mode 2. All 11 shipped encrypted CFGs use mode 2.

## Behavioral evidence

Launching the game causes it to emit plaintext siblings in `CFG/` and rewrite
the encrypted containers. Successive writes retain the same decoded bytes and
packed length but change the four-byte seed and ciphertext. After decoding the
outer transform, the clean-media containers and runtime-written copies have
the same payload except where the game changed configuration data.

The following clean-media examples all validated:

| File | Container bytes | Decoded bytes | Mode |
|---|---:|---:|---:|
| `ai.cfg` | 1,417 | 6,041 | 2 |
| `chaos.cfg` | 4,487 | 8,914 | 2 |
| `creature.cfg` | 4,898 | 21,599 | 2 |
| `effectani.cfg` | 3,640 | 22,503 | 2 |
| `effects.cfg` | 1,314 | 8,364 | 2 |
| `HTH.cfg` | 3,643 | 18,421 | 2 |
| `mitems.cfg` | 500 | 1,913 | 2 |
| `objects.cfg` | 2,540 | 16,024 | 2 |
| `printfx.cfg` | 1,682 | 6,180 | 2 |
| `spells.cfg` | 7,770 | 42,383 | 2 |
| `tables.cfg` | 470 | 881 | 2 |

## Confirmed precedence and remaining unknowns

- High confidence: when both forms exist before startup, the encrypted file is
  decoded over its plaintext sibling. The comment-marker experiment is
  documented in `cfg-precedence.md`.
- A deterministic mode-0 writer is implemented in `tools/encode-cfg.py`, passes
  byte-for-byte local round-trip validation, and is accepted by Chaos.exe. The
  successful reversible probe is documented in `cfg-writer.md`.
- The semantic schema of each decoded CFG is not yet cataloged.

The smallest next experiment is to change one well-understood numeric setting,
package it with the mode-0 writer, and verify the corresponding gameplay effect.
