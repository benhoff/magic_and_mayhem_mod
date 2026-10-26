# Recorder colour-rectangle refusals

The original-active producer recorder writes at most 32
`producer-colour-refusal-NNNN.bin` files per process when a colour-rectangle
input fails admission. Each is 192 bytes, little-endian. The ordinary
failure18/tag52 journal entry still follows; no diagnostic grants admission,
clears failures or completes a source session. Missing, partial or exhausted
files cannot prove absence of other refusals.

| DWORD offsets | Content |
| --- | --- |
| 0..1 | Eight-byte magic `MNMCRF01` |
| 2..3 | Version1, exact size192 |
| 4 | Reason: 1 unbound destination, 2 unreadable shape, 3 unsupported pixel format, 4 trace uniform policy, 5 uniform channel range, 6 primitive extent budget, 7 unreadable plane, 8 plane channel range, 9 source/destination alias |
| 5..8 | Pre-failure producer sequence, next failure ordinal, original caller, shape address |
| 9..11 | Current original binding, pixel format, known surface ID or zero |
| 12..17 | Signed clip left/top/right/bottom and requested x/y |
| 18..19 | Trace-mode flag; captured-span bits1 shape and2 plane prefix |
| 20..25 | First22 admitted readable shape bytes, followed by two zero bytes |
| 26..34 | First36 source-plane bytes when readable and disjoint from known destination storage, otherwise zero |
| 35..36 | World queue ordinal and local thread ID |
| 37..47 | Reserved zero |

The shape contains mode at byte0, signed dimensions at4/8, three channel
WORDs at12/14/16 and an unaligned source-plane address at18. A plane prefix
is a diagnostic sample, never complete source ownership. Binding/shape/source
addresses identify this process only. No destination pixels are read.

`tools/drawing_colour_refusals.py` validates the envelope, flags and reserved
bytes. Interpret reasons with the exact recorder source hash; historical
failure-only streams cannot be assigned a subreason retrospectively.

Diagnostic spans overlapping known destination storage are omitted even when
an earlier format guard refused first. Unknown destination bindings omit source
sampling. These omissions do not change failure or source-session admission.
