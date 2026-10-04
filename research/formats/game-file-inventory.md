# Installed game file inventory

## Reproducer and source

Run `./tools/inventory-game-files.py` with no arguments for the current clean
working tree. It is read-only and defaults to `working/game-clean`. The
inventory below was produced from 4,834 files totaling 327,271,755 bytes. The
path-and-size inventory SHA-256 was
`39262cc0b66f108d4adb4c2d1a0ca975dc14495f6dcd09f203218ea3da659f31`.

The executable identifying this installation has SHA-256
`124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`.

## Inventory

| Extension | Count | Bytes | Current classification |
|---|---:|---:|---|
| JPG | 752 | 56,398,896 | Standard JPEG |
| EVT | 685 | 156,400 | Proprietary `EVT\0` container |
| MAP | 683 | 12,060,759 | Proprietary; no stable leading magic observed |
| MPS | 683 | 389,488 | Version-1 map placements; [confirmed 40-byte records/native loading](mps-native-loading.md) |
| NOD | 683 | 12,746,278 | Proprietary `NOD\0` container |
| WAV | 356 | 11,925,486 | Standard RIFF/WAVE |
| SPR | 181 | 108,134,899 | Proprietary `SPR\0` container |
| CFG | 175 | 1,599,518 | 163 plaintext; 11 encrypted containers; one other |
| ANI | 136 | 5,478,380 | Proprietary `ANI\0` container |
| PCX | 124 | 3,459,543 | Standard PCX |
| WZD | 101 | 159,665 | Plaintext wizard data |
| TAG | 85 | 1,570,692 | Proprietary, variants start `UA000S1\0`/`UC000S1\0` |
| BMP | 48 | 13,814,536 | Standard Windows bitmap |
| TXT | 43 | 141,904 | Plaintext data |
| FP | 41 | 132,356 | Proprietary `.FP\0` container |
| CUR | 20 | 10,984 | Standard Windows cursor |
| TTD | 17 | 11,154,108 | Proprietary `TTD\0` container |
| SFT | 6 | 624,508 | Proprietary `SFT\0` container |
| AVI | 5 | 83,940,352 | Standard RIFF/AVI |
| DAT | 2 | 1,269,444 | Proprietary AI data |
| WBT | 2 | 346 | Plaintext AI data |
| Other | 6 | 176,563 | BAK, DLL, DOC, EXE, INI, and WRI |

Counts and byte totals are confirmed with high confidence. Classifications
based on standard magic values are high confidence. Proprietary labels identify
only their leading magic and are not claims that their internal layouts are
known.

## Confirmed proprietary signatures

Representative first 20 bytes show a recurring four-byte tag followed by
little-endian values:

| Format | Representative header |
|---|---|
| ANI | `414e490070e90000300500000500000000000000` |
| SPR | `5350520080140000040000000100000000000000` |
| EVT | `45565400b0010000010000001a00000003000000` |
| MPS | `4d505300e0040000010000004d00000020000000` |
| NOD | `4e4f4400f4fa0000010000008200000001000000` |
| TTD | `54544400d485090004000000d906000000000000` |
| SFT | `53465400bc6c010003000000df0000000f000000` |

Confidence is high that the first four bytes are format identifiers because
they recur across files with the matching extensions. The meanings of the
following words remain hypotheses until cross-file correlations and engine
access routines are analyzed.

## Priority for gameplay work

1. Decoded CFG files are the best first target: they contain creature, spell,
   item, effect, and global tuning data and now have a verified decoder.
2. AI `.wbt` text and `Brain.dat` may affect commander behavior; determine
   whether `Brain.dat` is generated from the text before editing either.
3. EVT/MAP/MPS/NOD appear to be tightly grouped per map section and are more
   likely world geometry, event, or navigation data than global mechanics.
4. ANI/SPR/SFT and standard image/audio/video formats are asset-oriented and
   lower priority for the three initial gameplay goals.

The priority assessment is a medium-confidence inference from names, paths,
counts, and plaintext content—not yet from traced engine reads.
