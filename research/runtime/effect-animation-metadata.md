# Executed effect animation metadata producer regions

The catalog's native complete-entry policy remains separate from recovered
baseline behavior. This experiment executes two bounded regions of No-CD
function `0049c5b0`; it does not call the whole collection loader. The executable
SHA-256 is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Original execution boundaries

`0049c6e2..0049c735` reads HEADER/NumberOfEffectsFile through the checked profile
callback, parses with the real original atoi (`0059c5a4`) and stores the result
in the existing stack slot. Signed values at or below one become one. Values
with bit31 set also become one; overflow wraps through the original 32-bit
parser before that comparison. This region stops before the global count write,
allocation, constructors or file reads. NumberofAnimations/allocation arithmetic
is outside execution coverage.

`0049c836..0049caae` takes an already allocated 16-byte-per-entry table and an
explicit positive entry/file count. It generates ANI_<ordinal> sections and
reads AnimationNo, AnimationFileRef, SpritePrinter and Data1. Real original
itoa (`0059de2f`), atoi and ASCII/C-locale stricmp (`0059db40`) run unchanged.
The recovered rules are:

- Nonempty numeric fields overwrite their word with 32-bit prefix atoi. Leading
  C whitespace/signs are accepted; digits stop at the first other character;
  no digit yields zero. Empty fields leave the original word untouched.
- References match case-insensitive generated EFFECTS<number> strings exactly.
  Unknown references, extra leading zeroes, whitespace and suffixes retain the
  original asset-index word. All declared file ordinals are checked.
- Printer matches are exact apart from ASCII case: NORMAL=31, TRANSPARENT=2,
  TRANSPARENT25=3, TRANSPARENT50=4, TRANSPARENT75=5. Empty/unknown values retain
  the original printer word.
- Nonempty Data1 is parsed into entry +c. It remains an opaque raw word for
  consumers; no gameplay or rendering meaning is inferred.

The entry ordinal spill, table pointer, count, callback register and scratch
stack are explicit authored preconditions corresponding to the preceding
original setup. The table starts with specified byte fills, not an assertion
about allocator contents. Retention of those words proves conditional writes,
not deterministic original defaults. Bounds are 1..64 files, 1..4096 positive
entries and ASCII profile-output strings of at most255 bytes; zero/negative
entry counts and unsafe allocation sizes are not executed. File-count strings
whose parsed value exceeds64 are tested only in the count region; their metadata
fixtures use a separately authored bounded count. Those pairs do not assert
whole-loader composition or safe allocation for the oversized original count.

The helper maps the unchanged PE in a private32-bit process, supplies only the
profile-output callback at its original IAT slot, and executes in a traced fork.
Hardware execution breakpoints stop before the next region; no instruction
bytes or child functions are redirected. Each stop checks the exact PC and
restored scratch stack pointer. Profile section/key/path/default/capacity and
call order are checked. Table guards, final ordinal/cursor and selected original
instruction bytes are checked. The fork is then terminated without running the
surrounding loader cleanup. This establishes region behavior, not SEH/prologue,
allocation, partial-failure cleanup or whole-function return behavior.

## Native comparison and deliberate differences

`reconstruction/rendering/effect_animation_metadata.*` models the above raw
profile-output rules with owned fields and explicit initial words. It has no
Qt, Win32 or pointer dependencies. Atomic refusal of invalid native fixture
bounds preserves the caller's inputs.

The same fixtures also run through the unchanged native catalog recipe reader.
Installed complete entries agree with the original metadata producer. The
native reader rejects incomplete/unknown fields, signed/junk/overflow numbers
and invalid counts instead of accepting prefixes or retaining indeterminate
allocation bytes. It also trims whitespace and semicolon annotations before
matching references/printers. Those accepted differences are recorded explicitly;
they are not original profile-API equivalence. Nonzero seed words prevent a
retained original asset index zero from accidentally appearing to match a
complete native recipe with asset index zero.

## Evidence and remaining work

Reproduce with `python3 tools/test-effect-animation-metadata.py`. The
[report](effect-animation-metadata.json) retains source/input/helper/output
hashes, exact fixtures and policy classification references in a new working
run. Immutable original files are verified before and after, including failure.
Normal and ASan/UBSan native streams must match all original region outputs;
strict recovered-model units run in both builds. The installed CFG is decoded
independently through the checksum-checked Python decoder.

The accepted run covers **131 file-count regions**, **393 metadata regions**
across zero/A5/FF byte fills, **2,076 entry results** and **8,435 checked profile
reads**. Every one of the installed **222 entries** agrees under all three fills.
The native catalog accepts34 complete fixture sets, refuses97 malformed sets,
and has six accepted trimming/annotation differences; these outcomes are
separate from the recovered-model equivalence result.

Confidence is high within these executed regions and authored preconditions.
The original Windows profile API, directory preparation, animation-count
allocation, object construction/destruction, ANI/SPR loading/cache sharing,
full collection reload/failure lifecycle, direction variants, dispatcher
admission, drawing and live replacement remain pending. The earlier installed
[asset/binding evidence](effect-animation-catalog.md) is preserved unchanged.
