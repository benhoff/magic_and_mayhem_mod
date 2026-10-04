# PE32 profile API comparison

`./tools/test-profile-api.py` builds a freestanding PE32 i386 reference executable
that calls `GetPrivateProfileStringA`, `GetPrivateProfileSectionA` and
`GetPrivateProfileIntA` from Wine's kernel32. A native executable applies the
same query list to `ProfileSnapshot`. Both read one generated ASCII/CRLF file;
neither launches the game nor reads installed/original artifacts.

The shared queries live in `tests/profile-api/queries.inc`; the two executables
write fixed 132-byte records containing a DWORD return and 128 output bytes.
The reference checks a buffer guard and complete writes. Native integer
exceptions are recorded separately, so rejecting `-1` cannot falsely count as
matching Wine's `0xffffffff` result. Comparison includes every output byte,
including string/MULTI_SZ termination; unused bytes start zeroed.

## Evidence and confidence

Evidence: `working/tests/profile-api/run-50r501hf/report.json`, Wine **11.16**.
The report records source, generated-input and executable/result hashes, each
query's returns, bytes and rejection status. Build and runtime logs accompany it.
The initial run `run-8a0phly1` exposed fallback-space and exact section-capacity
mismatches; these prompted the supported fixes below. Confidence is high for
these particular calls on this Wine runtime, not historical Windows equivalence.

All **29 queries** complete: **22 exact matches**, **seven intentional policy
differences**, **zero unexpected mismatches**. Coverage includes case folding,
matching quotes, inline semicolons, empty values, bare map entries, missing
sections/keys, fallback whitespace, string truncation, section enumeration and
capacities, decimal/quoted integers, and unsupported numeric conversions.

| Behavior | Measured result and native action |
|---|---|
| Missing string default | Wine strips trailing ASCII spaces, preserves leading spaces and trailing tabs; native now does the same before capacity truncation. |
| Quotes and inline semicolons | String reads remove matching surrounding quotes; section reads retain them. Inline semicolons remain data. Exact byte agreement in the selected cases. |
| Bare map entries | Section read returns `10\0` and `20\0`, count six. A direct string lookup of the bare key uses the fallback. Native agrees. |
| Exact section capacity | Those entries with capacity seven return truncation marker five; capacity eight returns six. Native now returns the same marker at the exact boundary. |
| Partial section data | At capacities four and seven, Wine publishes partial list bytes; native exposes no partial entries, by policy, with the matching truncation marker. Two deliberate differences. |
| Unsupported integer text | Wine returns hex `0x10` → 16, negative `-1` → `0xffffffff`, suffix `3junk` → 3, empty → fallback 17, overflow `4294967296` → 0. Native rejects all five explicitly. |

The native audio caller rejects truncated section results, so dropping partial
contents remains deliberate. Unsupported integers remain explicit errors rather
than adopting permissive conversion for unvalidated game input.

## Regression checks and boundaries

`tests/audio-native-manager-test.cpp` pins the corrected fallback whitespace and
section boundary behavior without needing Wine. All 29 current audio/assets
CTests pass: `working/tests/audio-native-manager/run-42jowpax/report.json`.
That directory also records address/undefined/leak sanitizer validation of the
profile-to-native-audio integration.

Reproducer dependencies: CMake, a C++17 compiler, Qt6 Core, clang, llvm-dlltool,
lld-link and Wine. It uses the existing isolated fixture prefix
`working/tests/render-wine`, clears inherited `MNM_*` options, and launches only
the generated console executable. It does not attach a debugger or modify an
existing executable. Every command has a bounded timeout; failures fail the run.
Wine may require permission for its local wineserver socket in a sandbox.

Not established: Windows 98 behavior, installed `Sounds.ini` compatibility,
duplicate section/key handling, malformed headers, ANSI/UTF conversion, registry
mapping, filesystem stat equivalence, concurrent file updates, live caller
ordering or audio replacement. The next bounded compatibility step is an
immutable-manifest-guarded installed profile/catalog comparison; it does not
require playing the game.

The [installed profile comparison](installed-audio-profile-comparison.md)
subsequently found a trailing section-header comment in the actual input. A
populated synthetic fixture now covers that accepted syntax, expanding this
runner to 31 cases (24 exact matches, seven intentional policy differences).
