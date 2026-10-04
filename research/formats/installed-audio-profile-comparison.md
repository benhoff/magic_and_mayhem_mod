# Installed audio profile and catalog comparison

`./tools/test-installed-audio-profile.py` compares the installed `Sounds.ini`
through Qt-backed `AssetFile`/`loadProfile` with actual PE32 Wine profile APIs.
It copies the input bytes unchanged into an evidence directory, inventories
section/key names solely to select queries, and runs neither the game nor an
injected adapter. Original artifacts are verified before and after, including
failure paths. The installed input is checked for changes during the run.

## Input and method

Default input: `working/game-nocd/Sounds/Sounds.ini`, **15,672 bytes**, SHA-256
`72f9c5b34a3325976562247bb16be1e720ad47636a7d546c2b31df171ce1c412`.
The runtime installation's `working/runtime/game-nocd/Sounds/Sounds.ini` has the
same hash. This is an installed input comparison, not a direct extraction from
original media or a historical Windows run.

The native fixture uses the existing Qt-backed asset store and bounded profile
loader. The Wine fixture is a freestanding PE32 i386 console program importing
only kernel32 profile/file/process functions. No audio output device is opened.
Both execute a shared version-1 little-endian query file, bounded at 4,096 queries
and 128-byte section/key fields. Results hold a DWORD return plus 16,384 bytes.
Every output byte is compared, including terminators and zeroed unused capacity.
The reference checks writes and a buffer guard; native decoding bounds records
and checks termination. These are test contracts, not a new runtime bridge.

Calls cover all eight installed sections with capacity `0x4000`, all 486
key/value entries with string capacity 260 (Sounds/other settings) or 256
(Randomised), the recovered `MaxSimultaneousSounds` integer read with fallback
16, and four absent map-0/map-3 preload sections. Tuning values are compared as
strings; original tuning numeric conversion has not been established here.

Both result files are separately fed through the same reconstructed
`soundTable`, `groupMembers` and `sourceEntryName` functions. This verifies
agreement of decoded inputs and derived tables/names, not independent recovery
of those functions or an original in-memory catalog capture.

## Findings and confidence

The installed profile revealed a supported parser gap: its
`[1 Load Permanent]` header has a trailing semicolon comment. The native reader
previously rejected the file. A populated synthetic commented-header section
established that Wine accepts this syntax for both string and section reads:
`working/tests/profile-api/run-7mp0o2v4/wine-results.bin` (native rejected).
The parser now permits an empty suffix or a semicolon comment after `]`; other
suffixes still reject under native policy. The regression covers populated
header comments and rejection of a non-comment suffix.

Installed evidence: `working/tests/installed-audio-profile/run-cu79arb9/report.json`.
On **Wine 11.16**, all **499 calls match exactly**, with **zero mismatches** and
identical derived catalogs:

- 412 source IDs and derived source names.
- 69 randomized groups containing 168 member references; all resolve to known
  source/group IDs.
- Simultaneous-sound limit 12.
- Empty installed permanent/temporary lists for maps 1 and 2, and empty results
  for absent map-0/map-3 lists.

Confidence is high for these installed bytes and selected API calls on the
recorded Wine runtime. The report records hashes for inputs, queries, sources,
binaries and result/catalog artifacts, as well as successful manifest checks.
Each call's return counts and agreement status remain reviewable.

The expanded synthetic suite has 31 calls: 24 exact matches, seven deliberate
native integer/partial-list policy differences, zero unexpected mismatches.
Evidence: `working/tests/profile-api/run-pcfa7l91/report.json`.
All 35 current audio/assets CTests pass:
`working/tests/audio-native-manager/run-6eczsua_/report.json`.
Address/undefined/leak sanitizer runs additionally cover installed profile
loading and decoding both result sets; their evidence is recorded with the
installed comparison.

## Boundaries and next work

Empty installed map lists cannot establish nonempty classification/preload
agreement; synthetic manager tests cover the selected native contract separately.
No Windows 98 equivalence, tuning integer conversion, registry/encoding/duplicate
policy equivalence, filesystem-stat semantics, active-map ordering, original COM
ownership, concurrent update semantics or live/audible replacement is claimed.
WAV payload compatibility has its separate loader evidence.

The next useful offline step is to run the installed catalog through native
manager startup and source loading/admission, using controlled time and RNG,
and check PCM and cleanup without an output device or game process. That extends
installed API agreement into installed file-to-manager integration; it still
cannot establish live caller ordering or timing.
