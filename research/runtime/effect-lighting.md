# Installed effect lighting table

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

`0049c2e0` loads the lighting portion of an effects collection. It writes 89
12-byte entries at receiver `+0x0c`, each containing diameter, affected flag,
and height-clipping flag. The loader requests the three keys documented in
[the format record](../formats/effect-lighting.md), returns 1, and does not
initialize missing fields or clear false flags. Callers must establish the
initial state separately; the native table explicitly defaults to zero.

The original `0049cf60` selects the first DWORD at
`receiver + 0x0c + type * 12`. In the lighting updater the receiver is the
collection at `00689498`, and a source's type is at record `+0x28`. This ties
the recovered light-index lookup to `effects.cfg`. Earlier static-light work
used an authored type table; this chunk recovers the installed table producer.
It does not establish that the collection represents stationary game objects.

`0059c5a4` delegates to `0059c519`: ASCII/C-locale whitespace skipping,
optional sign, decimal prefix accumulation modulo 2^32, then optional
negation. The loader compares the resulting signed value and clamps it to
0..33. `0059db40` compares strings without ASCII case sensitivity in the
default C-locale branch. Thus mixed-case `True` in the shipped file is true.
Locale-dependent CRT initialization is outside this bounded reconstruction.

The native implementation keeps asset decoding in `assets/effect_lighting.*`
and original rules in `reconstruction/rendering/effect_lighting.*`. The
`source(type, positions)` adapter selects a diameter while preserving owned
positions and activity supplied by the caller. A synthetic test connects this
result to the existing combined lighting cycle. The two metadata flags are
stored faithfully but are not used to invent filtering or clipping behavior.

Validation executes the complete unmodified `0049c2e0` and `0049cf60` in a
private PE32 mapping. Only the two OS import slots for directory and profile
reads are supplied by checked fixture callbacks. The original integer and
string routines remain intact. Every load checks all 267 reads, sections,
keys, empty defaults, 256-byte capacities and joined filenames; receiver
prefix/suffix guards check that writes stay inside the table. This is an
original-loader comparison at the profile-output boundary, not a fresh
Windows profile API equivalence claim.

Run the comparison after normal and ASan/UBSan terrain-preview builds:

```sh
xvfb-run -a python3 tools/test-effect-lighting.py \
  --build working/tests/effect-lighting/build \
  --sanitized-build working/tests/effect-lighting/sanitized
```

The durable [result](effect-lighting.json) records source/input hashes and
counts. The native asset helper opens a deliberately mixed-case Windows-style
path and compares all selected profile outputs with independently decoded
installed bytes. The comparison covers zero and nonzero prior states, reload
chains, empty/missing values, false flags, mixed case, truncation-sized values,
numeric junk and integer wrapping, followed by full native suites.

Confidence is high within the stated C-locale/profile-output/table scope.
Live observation/replacement remains absent. `0049c280` also calls
`0049c5b0`; this broader initialization path is not covered by table-loader
equivalence. Original source record creation/destruction, positions at
`+8/+0x0c/+0x10`, recount coordinates at `+0x1c2/+0x1c6`, capacity/count
ownership and region view production remain open. Diameter 1 is faithfully
loaded, but remains unsupported by native stamping; the existing cycle
refuses an active size-one source before mutating state.
