# Optional colour rectangle refusal records

`MNM_COLOUR_RECT_DIAGNOSTICS=1` enables at most eight process-local
`colour-rect-refusal-N.bin` records in the existing native colour splice.
Ordinary launches leave it disabled. Admission, original-once refusal and pixel
production do not change. A record is 256 bytes, little-endian, version 1.

| DWORD offsets | Content |
| --- | --- |
| 0..3 | Magic `0x4652434d`, version 1, exact source refusal line, reserved zero |
| 4..17 | Fourteen saved caller-context words |
| 18..23 | Six owned-plane query words, zero where no query completed |
| 24..25 | Current destination base and engine pixel format |
| 26..30 | Shape width, height, mode and prepared source x/y |
| 31..34 | Prepared visible width/height and destination x/y |
| 35..38 | Constant red/green/blue WORD values and optional triple-source address |
| 39 | Caller scratch at stack+0x2c, only when its span is readable |
| 40..45 | Up to 22 readable shape bytes, zero padded |
| 46..54 | First 36 triple-source bytes when that span is readable |
| 55..63 | Reserved zero |

Addresses are observations within one process; they do not establish ownership,
cross-launch identity or complete source content. The exact bridge source hash
is needed to interpret a refusal line. No destination pixels are read by this
diagnostic. Missing, partial or exhausted records cannot prove that no other
refusal occurred. The normal route counters and whole-journey validator retain
their independent zero-forward requirement.

`tools/diagnose-colour-rect-live.py` invokes the ordinary campaign validator with
this single diagnostic enabled. It forwards all CLI arguments and retains the
normal source, installation, original-manifest and private-prefix checks.
