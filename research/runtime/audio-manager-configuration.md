# Audio manager configuration and pool startup

Selected offline reconstruction for the pinned NoCD executable, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high for the listed instructions and file-backed strings; agreement
with an original live startup has not been measured. No game was launched.

## Evidence and implementation

`./tools/export-audio-manager-config.py` verifies the immutable-input manifest
before and after export, checks the executable hash, and exports assembly,
callers and strings with artifact hashes. This run produced
`working/decompiled/audio-support-objgyz0h/manifest.json` and
`manager-config-strings.json`. The source executable was read, not modified.

Implementation: [manager_configuration.hpp](../../reconstruction/audio/manager_configuration.hpp)
and [manager_configuration.cpp](../../reconstruction/audio/manager_configuration.cpp).
It consumes decoded profile API results through `ConfigurationBackend`;
it does not make the native assets/audio services depend on recovered engine
contracts, and it is not a new general-purpose INI parser.

| Address | Recovered block |
|---|---|
| `0x56dd80` | Manager constructor and disabled wrapper defaults |
| `0x56df60` | Root/profile paths, source and randomized group tables |
| `0x56ffc0` | Comma-separated positive group member parsing |
| `0x59e142` | Linear table lookup, without insertion or count reduction |
| `0x56e910` | Per-map classifications, source budget, pool and permanent preloads |
| `0x5719b0` | Scheduler allocation/reset and ring linkage |
| `0x572110`, `0x572120` | Group descriptor construction/destruction |

## Profile and table contracts

The profile path is `<root>\Sounds.ini`. Section reads request 16,384 bytes;
return value 16,382 means truncation. `Sounds` must contain at least one
positive ID. `Randomised` may be empty. The adapter supplies the returned
count and decoded NUL-separated entries, preserving API ordering.

Entries beginning with `;` are skipped. Keys before `=` must occupy one through
six bytes. CRT `atoi` accepts a numeric prefix (including whitespace/sign) and
IDs <= 0 are ignored. The first scan counts positive entries. The second scan
inserts only IDs not already present in the zero-filled array, and sorts the
entire original allocation. Duplicate IDs therefore leave leading zero slots;
this model deliberately does not compact them away. Group lookup includes those
zero slots. Class storage is initialized by per-map pool setup, not the table phase.

Group member reads request 256 bytes and decimal ID keys. `strtok` splits on
commas only: `10 20` parses as the single ID 10. Positive member IDs preserve
order and duplicates; zero/negative tokens are ignored. Missing values leave
empty groups. The default string address `0x5faf28` is in a zero-initialized
virtual tail; later runtime writes have not been ruled out, so decoded default
behavior remains the adapter's responsibility.

After successful primary setup/control, the original reads
`Optimisation / MaxSimultaneousSounds`, with the current global `0x5f0404` as
fallback, stores the returned global value, then initializes scheduler storage.
`initializeConfiguredSchedules` models that selected sequence; it does not
create a DirectSound device or claim to implement all of `0x56df60`.

## Per-map sources and scheduler

When manager initialization is false, pool setup returns success without changes.
Otherwise it releases the old source ring first, walking backward from the
former tail, destroying duplicate chains before each root buffer/free.
It sets every source class to 1, then reads `<map> Load Permanent` (class 0)
and `<map> Load Temporary` (class 2), in that order. These section entries are
whole strings of length one through six, rather than `key=value` pairs.
Unknown IDs are ignored. Each matched permanent entry increments `+0x22c`,
even repeats and entries subsequently overridden by the temporary list.

For every final class-0 table slot, the loader obtains the `Sounds` profile
value, builds the same quoted-comment/path form as the source cache, and calls
CRT stat (`0x59e270`). Successful size words are added with DWORD wrap to
`+0x230`; missing profile values and failed stat calls contribute zero. This
counts file size, not decoded PCM size.

`+0x234 = +0x22c + ((0x100000 - +0x230) / 46080)`, with unsigned wrapping
subtraction and addition. The division implementation preserves the original
multiply/shift sequence (`0x56ed55..0x56ed74`). It allocates that many default
40-byte source wrappers, links their `+0x18` previous / `+0x1c` next ring,
then preloads class-0 IDs in table order via the existing recovered cache loader.
The first preload error is returned unchanged; the ring and earlier successful
preloads remain. Profile failure occurs after old-ring disposal, without rollback.

Scheduler records are 32 bytes: deadline/voice/output zero, volume -5000, x/y -1,
with adjacent next/previous links. Reinitialization frees old records without
stopping voices or clearing their caller output slots. The host model uses stable
owned nodes; raw pointer words remain zero because no process addresses are known.
Source wrapper identities are fixture-local tokens, not recovered heap addresses.

## Validation and remaining boundaries

Run `./tools/test-audio-manager-configuration.py`. Synthetic fixtures check table
slack, numeric-prefix keys, invalid/truncated sections, comma-only group parsing,
profile path bounds, global scheduler limit publication, map override/repeat
behavior, stat-success-only budget, bidirectional ring defaults, exact backward
old-pool disposal, failed preload retention and absent-manager gating.
An independent division oracle checks 542,368 values including threshold and
wraparound cases. All 23 audio/assets CTests passed. Final evidence:
`working/tests/audio-manager-configuration/run-_umqq5yn/report.json`.
Address/undefined/leak sanitizer evidence is stored alongside that report.

This is selected static reconstruction plus synthetic fixtures, not observation
or live replacement. Windows quote/case/truncation/encoding semantics and CRT
stat special-path behavior are delegated, not reconstructed by Qt asset loading.
Device creation/cooperative-level ordering, primary setup, global-device
publication, full constructor/destructor/table failure ownership and an aggregate
manager startup controller remain separate work. Existing primary-control/setup
models do not establish end-to-end startup equivalence.

Original null allocation, oversized stack-copy and count-0/1 scheduler cases can
perform invalid accesses. The model rejects those domains rather than claiming
safe original error behavior. Host pools cap allocations at 65,536 nodes, a
fixture safety boundary, not a recovered engine limit; source count 1 is valid,
scheduler count must be >=2. Tests do not establish live wrapper/scheduler
identity ownership, thread behavior, real map configuration or audible timing.
