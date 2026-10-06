# Installed effect animation catalog

This is a native complete-entry producer and headless integration, not execution
of the original collection loader or a live effect renderer. It feeds the
[owned forward binding](effect-animation-binding.md) from installed assets.

## Recovered producer boundaries

Static inspection of the unchanged No-CD executable, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
identifies function `0049c5b0`. The retained experiment records hashes of its
selected windows and exact printer/Data1 string anchors. Confidence is high
within these static windows; loader execution equivalence remains pending.

- `0049c653..0049c735`: HEADER/NumberofAnimations determines the allocation of
  16-byte entries at collection `+438`. NumberOfEffectsFile determines the ANI
  object count; the original clamps values at or below one to one.
- `0049c841..0049caae`: ANI_<ordinal>/AnimationNo populates entry `+0` through
  original atoi (`0059c5a4`). AnimationFileRef is compared against generated
  EFFECTS<number> strings using `0059db40`, writing the matched index at `+4`.
  SpritePrinter comparisons populate `+8`: NORMAL=31, TRANSPARENT=2,
  TRANSPARENT25=3, TRANSPARENT50=4, TRANSPARENT75=5. Data1 populates `+c`.
  Each word is conditionally written; missing keys/unmatched strings can retain
  uninitialized allocation bytes. Those values are not a defined fallback.
- `0049cabe..0049cc11`: each ordinal constructs sprites\\EFFECTS<number>.spr
  and .ani paths and loads paired objects. ANI table stride is `114`, SPR
  object stride is `24` (hex). Original caching/construction and fallback calls
  in this window are not reproduced by the new catalog.

The fourth entry word was retained as `opaque` by the earlier binding. Its
producer is now identified as Data1, without assigning gameplay semantics.
Printer numeric codes are retained by the binding's `property` word; this does
not implement drawing, blending, sorting or lighting.

## Native policy and ownership

`assets/effect_animation_catalog.*` uses the existing checksum-checked packed
CFG decoder and Config parser. Section/key names are ASCII case insensitive;
values permit trailing semicolon annotations and surrounding whitespace or
matched quotes. References and printer names are ASCII folded and compared
with exact generated names. EFFECTS00 is not an alias for EFFECTS0.

All four fields must be present and nonempty. Numbers require complete unsigned
32-bit decimal; negative, overflow, junk-suffixed and unknown values reject.
Counts must be positive and within explicit limits (4096 entries, 64 files by
default); zero file count rejects rather than using the original clamp. These
are intentional native safety rules, not malformed-input equivalence. Config
retains its existing duplicate-key rejection and annotation handling.

The producer opens CFG/Encrypted/effectani.cfg and all declared ANI files once
through the Windows-path/case-insensitive AssetStore. Limits bound packed bytes,
file/entry counts and each ANI decode. Every selected base sequence must exist.
It returns owned metadata, normalized ANI records and paired SPR paths. It does
not load SPR pixels or infer direction variants from base sequences. Successful
and failed loads close handles through RAII; no partial catalog is returned.
Errors identify the failing asset or metadata category.

`reconstruction/rendering/effect_animation_catalog_binding.*` converts one
native recipe into the recovered binding words and delegates to the existing
forward player. It preserves the original ordinal and copies the sequence.
Catalog destruction does not invalidate playback. The native asset service has
no dependency on reconstruction, Qt widgets, Wine or original host pointers.
The adapter and a standalone CMake project make this integration reusable.
The older mode-one preview remains an independently scoped facing-aware caller.

## Offline evidence

Reproduce with `python3 tools/test-effect-animation-catalog.py`. Each run retains
new logs, native/sanitized builds, static byte windows and report under working/;
immutable original files are verified before and after, including on failure.
The [report](effect-animation-catalog.json) fingerprints current sources and
installed CFG, all 13 ANI/SPR pairs and the unchanged executable.

Independent Python checksum decoding and INI parsing produce all **222**
16-byte recipe entries and compare their bytes with native output. Native and
ASan/UBSan each load the **13** ANI/SPR pairs, check every sprite record in each
selected sequence against its paired SPR frame count, and observe **14,430**
forward states (65 per entry). Metadata/ordinal propagation, one selector-to-
catalog binding, playback after catalog destruction and restart are exercised.
Units cover all five printer codes, case folding, full-width Data1, exact
reference spelling and missing/negative/overflow/unknown/capacity refusals.
Separate copied fixture roots test missing CFG, missing third ANI after two
successful loads, and malformed first ANI; failures name the correct path.

This proves installed native loading/binding and bounded playback/sprite-index
safety. It does not compare the whole original collection loader, execute its
Win32 profile/filesystem API, validate ANI cache sharing, draw pixels, cover
base-plus-facing variants, interpret Data1, construct effects through the full
89-type dispatcher, or establish lifecycle/scheduling/live replacement.
Historical controller/binding evidence remains unchanged; this run does not
refresh old fingerprints or promote their original-comparison scopes.
