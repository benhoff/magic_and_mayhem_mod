# Passive installed font custody, versions 1/2

`MNM_FONT_LIFETIME=1` opts into metadata in the native resident scene observer.
It changes no font allocation, source admission or renderer operation. The five
entry detours verify exact NoCD40209ca7 bytes before installation. Original
addresses remain private lookup keys; records identify owned observations with
monotonic font IDs, role IDs and relative source offsets.

The 64-byte little-endian header starts `MNMFLT01`. Words 2..8 are version 2 (historical version1 remains readable),
header bytes 64, build `40209ca7`, byte budget 67108864, font budget 128,
record budget 131072 and fixed record header bytes 128. Other words are zero.
Each record has 32 words and an optional body: size, ordinal, kind, owner thread,
tick, scene queue, CPS sequence. Word22 is the CRC32C of its body, or zero for
an empty body. The strict reader rejects foreign owners, missing/reordered
load/return pairs, malformed sources, unbound glyphs and incomplete closure.

Kinds 1 and 2 are loader entry/return. Words7..11 identify font ID, role,
previous generation, original caller/result, and owned-field bit mask. Word21
is256: the body begins with a zero-tailed ASCII path. Successful returns append
the complete loaded SFT image. Words12..20 are image length, glyph count, rows,
contours/table/frame-base relative offsets, line height, tab and tracking.
Every frame's process-local palette pointer at frame+28 is normalized to zero.
The independently read installed input is normalized identically before exact
comparison. Source bytes and palette generation remain separately checked.

Kind3 binds a font generation and glyph ordinal to an actually committed CPS
font operation. Words7..14 are font, role, glyph, committed sequence, canvas,
frame length, frame CRC32C and scene queue. Words15..20 carry X/Y, the three
record colour fields and original caller. Word21 is the current role generation;
word23 marks a superseded source. Generations are never inferred from pointer
reuse. CPS sequence numbers reset at independently captured source chunks and
World generations, so retained-data analysis preserves complete emission order.

Kinds4/5 are teardown entry/return: generation, role, previous generation,
caller/result and owned-field mask. The mask covers original object offsets
0,8,12,16,20; the borrowed palette pointer at+4 is intentionally excluded.
A successful original teardown must clear every owned field. Original reload
retains replaced allocations; it does not constitute their deallocation.

Kind6 is the final footer. Words7..13 hold loss, load attempts, glyph count,
active roles, pending calls, bytes before footer and superseded retained fonts.
A complete positive trace has zero loss, active roles and pending calls. Its
superseded count explicitly preserves original baseline retention until process
retirement. It grants no leak-free original claim or acceptance by itself.

The separate CPU reader journal records DC image acquire/release. Installed SFT
text is not GDI text. Wire source joins, actual output comparison and normal
native/original retirement remain necessary in addition to this metadata.

Evidence: `font-dc-lifetime-results-20261108/controls-01.json` retains 16 controls,
24 production i386 entry/return ABI calls, exact authored fixture and production
binaries. This is high-confidence protocol/control evidence; actual installed
lifetime/output acceptance is a separate milestone.

## Version2 consumed-source identity

Kind3words24/25are the current continuous World generation low/high; words26/27
are the current source-prefix ordinal low/high. Both values must be nonzero.
Together with the actual CPS sequence they identify one source operation across
all resets and reloads. The private pointer lookup is unchanged.

The acceptance reader matches every consumed CPS glyph to exactly one observed
font generation and full normalized frame. A final unconsumed hidden-menu prefix
may remain only after every matched glyph, in the exact next World/source
context, with fewer than16384glyphs and no middle gap. Those observations are
reported separately and grant no consumed-output credit. Normal retirement and
independent complete output comparison remain required.

`controls-02` retains17strict controls,24actual i386 hook ABI calls and both
production/selftest binaries for version2. Version1execution/failures and hashes
remain historical; no source context is invented for them.
