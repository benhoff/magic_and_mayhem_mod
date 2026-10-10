# Owned byte text producer

NoCD build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This increment separates recovered byte behavior from deliberate native refusal
policy. The prior [glyph raster comparison](font-glyph-raster.md) covers the
shared pixel writer; this work adds its contour and cursor producer.

Static evidence: retained `working/decompiled/sft-support-pxjq5rm6/advance.asm`
and `character_draw.asm`, plus hash-verified bytes at `0x5e0578`: `!?:;` followed
by NUL. `0x59c8e0` is the executable's byte-search helper. At `0x4a58e0`, global
`0x6e1ed4 == 1` adds tracking plus one for these four punctuation bytes. Tracking
is also added normally. Bytes `60/91/92/27/b4` (hex) override the extra with two.
The profile difference, final additions and cursor addition wrap at 32 bits;
the running contour maximum uses signed comparison starting at zero.

`0x4a59e0` calls advance with updates enabled, adds it to object cursor X at
`+0x218`, and then chooses frame `byte-33`. Cursor Y at `+0x21c` is unchanged.
Space and underscore use the `a` contour but suppress glyph output; tab returns
the configured `+0x224` advance, and newline returns zero. This routine does
not move to a new line. The advance routine bounds against the header glyph
count at `+0x0c`, not its metric count at `+0x24`. The draw routine lacks a safe
frame check for other controls or unavailable bytes. Those draws are refused
before any native state change; no original malformed-input execution is claimed.

`reconstruction/rendering/font_byte.*` holds the build-specific arithmetic.
`compat/legacy/font_text.*` owns decoded font data, stages the recovered state
change and commits it after the atomic owned canvas glyph operation succeeds.
It requires complete contours and at most 128 rows, matching the bounded object
profile area. This is a reusable byte producer without widgets or hook addresses.

Reproduce the bounded comparison with prospective declarations:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.font-byte-consumer \
  --behavior NR.font-byte-producer --scenario font-byte-atomic-20261010 \
  --output working/tests/font-byte-claims-next.json
python3 tools/test-font-byte-producer.py --claims working/tests/font-byte-claims-next.json
```

Recorded execution: `working/tests/font-byte-producer/run-2t3xh_m9/`, retained
in [comparison report](font-byte-producer-comparison-range-20261010.json). All 3,776
cases matched, covering 64,793 byte operations and 59,424,768 RGB565 WORDs in
3,627 complete draw canvases. The corpus includes all byte values for advances,
both update settings, modes -1/0/1/2, negative/zero/positive tracking, every
available installed glyph followed by suppressed-byte chains, extended quote
bytes, long mixed sequences, vertical cropping/horizontal rejection, synthetic
32-bit wrap and zero-row contours. Six atomic native refusals, read-only probes
and suppressed-byte updates passed, including ASan/UBSan. Source/input hashes and
immutable manifests remained stable. Confidence is high within this bounded
domain. Earlier attempts remain retained: a reference compile error and a
matching preliminary execution with a source change are not promoted. The
earlier successful `run-av3x761q` result remains historical after the runner
gained pinned entry anchors and the draw reference gained an explicit recovered
range. The final rerun binds that range without changing original code or
refreshing earlier hashes. `0x4a59e0..0x4a5a49` is absent from the discovered
function inventory; the exception records its selected draw scope, not complete
discovery of its callers or malformed branches.

The isolated original harness maps the pinned PE
privately, checks entry signatures and punctuation bytes, supplies initialized
font/cursor/raster state and invokes both unchanged entries. The native harness
independently decodes the SFT source. Font bytes, canvas guards and row padding
are checked. Original outputs are separate comparison oracles, never native inputs.
The runner retains compiler dependency checks, source/input hashes and immutable
manifest verification before/after. Higher string layout, original reader and
state reset/lifecycle, arbitrary floating state, Win32 ABI and text takeover remain
pending. Live glyph observation uses the separately registered canvas-producer
pipeline; it does not imply that this byte-state adapter replaces a live consumer.
