# Attached sprite sibling admission

The pinned No-CD build (`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`)
has another owner-gated random sprite class, vtable `0x5c7514`, draw method
`0x549ba0`, resource bank `0x6f21e8`. Its exact 229-byte body ends at
`0x549c85` and has SHA-256
`90738588b64a862d8bf46aa076df98a24765ddf3d6ad664e03780934850e1c83`.
The effect's visible name is not established here. The existing recovered
particle model already includes this method; live class and leaf admission were
missing. A bounded static inventory found this one missing concrete class among
13 concrete and three abstract particle-family tables. This is not a census of
all rendering in the game.

| Return site | Backend tags | Original route |
| --- | --- | --- |
| `0x549c43` | 22, 36 | Three-argument raw WORD copy |
| `0x549c64` | 13, 14 | Five-argument indexed copy |
| `0x549c79` | 17 | Indexed low-AX == 1 clipped fallback |

The shared sprite metadata now recognizes this third class with owner offset
`+0x10`, 20 readable root bytes and its own bank. The protected first-table
World batch checks the exact class/method/bank, return signatures, method
prologue, primary caller/continuation, saved row, owner/player visibility, bank
count 1..4096, source selector, original owner thread, RGB565 format and inactive
auxiliary markers. Ordinary per-draw admission remains unchanged. No new wire
operation, raster algorithm, simulation, random generator, asset, balance or
original executable modification is introduced.

The owned wrapper model also recognizes the third class. Like Cure, it draws
only when `(random & 0x7fffffff) % 100 > 50`, then consumes another word for frame
selection. It retains raw/indexed selection, fallback only for low AX == 1,
zero AX return and object preservation. Construction and lifetime remain
separate recovered contracts.

## Validation

`AT14.sibling-native01` and `AT14.sibling-original01` retain the aggregate report
`attached-sprite-results-20261114/summary01.json` and exact source/artifact custody.

- An ARM-host-compatible freestanding Win32 fixture executes all three unchanged
  original sprite bodies and original primary virtual-call/store fragments.
  All 216,000 complete traces match the native owned model, including 21,408
  callbacks admitted through the actual production predicates. Chance boundaries,
  player/resource/visibility gates, frame choices, both backend variants, low/high
  return-word patterns, original object preservation and AX metadata are checked.
  Additional byte, tag, class, bank, owner, player, source, parent, thread and mode
  mutations refuse. RNG and raster callouts in this trace proof are synthetic.
- The complete producer forwarding/refusal suite passes. Its sprite fixture
  executes 15 original primary calls and five protected entry ABI controls,
  checks 1,032 admission controls including 912 actual inaccessible-page short
  spans, and emits 20 real owned CPS rasters and a canonical batch while the
  original destination remains protected and unchanged.
- Native software GL decodes the actual 20 packets, resolves two authored source
  assets and compares all 480,000 destination words with an independently authored
  scalar image. All match; 80 words are nonzero and ordinary draw/present performs
  zero readbacks. This fresh GPU proof uses authored expected words, not new
  original pixel execution. The applicable prior 77,760 trace and pixel particle
  comparisons remain separately pinned under `BP29.particle-sprites.original01`.
- Production and regenerated ordinary caller predicates retain their existing
  admission/refusal controls. The generator verifies all nine sprite sites
  against original call slots, vtables, bank prologues and continuation bytes.

Earlier setup/link/address-range refusals and raw state mismatches remain
retained. The older fixture compared unspecified empty x87 payloads and reserved
FXSAVE bytes. A tag- and TOP-aware test mask now excludes those bytes while
retaining GPR/flags, control/status, tags, live x87, FIP/FDP, MXCSR and XMM checks.
The fixture explicitly initializes physical x87 payloads before per-call seeds;
raw diagnostics also retain the WORD baseline's differing live-slot payload
before that initialization. Fifteen mutation controls and the existing 2,048
TOP/tag codec controls pass. Production save/restore code is unchanged. The
complete suite was rerun after these test corrections; no failed execution is
relabelled as a pass.

No complete game was launched for this change. Natural encounter/reachability,
other table modes, alternate formats, arbitrary malformed original pointers,
negative counts, aliases, source lifetime and whole-player acceptance remain
outside the isolated proof. All newly identified call sites are implemented in
the bounded live bridge; a natural session is still a separate validation step.
