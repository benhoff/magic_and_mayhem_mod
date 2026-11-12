# Screen concrete providers and exceptional ownership

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This batch implements the concrete-provider and exceptional-path deliverables.
Real Main/Realm/World transition routing and installed/live acceptance remain
separate deliverables. It changes no gameplay rules or balance.

## Recovered behavior

`ScreenProviderComposition` composes the screen-specific providers with the
existing raw SPR, palette-chain, heap and retirement reconstructions. Platform
file/profile APIs, font realization/localization, allocator primitives, COM and
the original JPEG codec are explicit interfaces. An interface in a comparison
fixture is not a recovered implementation of the OS or driver.

| Provider | Recovered responsibility |
| --- | --- |
| `558580` | All eleven layout section prefixes; owned layout properties; case-insensitive font/flags; prefix decimal values; partial token lists; optional allocated colour triples; profile writeback. |
| `559820/559880` | Exact initialized layout fields and colour-list destruction. The destructor does not clear its pointer. |
| `4cc720/4d27b0`, setters and destructors | Control construction, copied text, registration, independent control/text destruction and callback-visible vector selection. |
| `4cabd0/4def80/4cad00/4caf00` | Backing-vector ownership, insertion with aliased value storage, selection and callback order. |
| `58ad90/58b3e0` | Surface creation and publication, primary/palette branches, registry compaction and COM reference retirement. |
| `58b660/58d320` | Pixel lock, lost-surface restoration and retry, JPEG decode and unlock. These pointers are pixel storage, not HDCs. |
| `58b270` | Optional JPEG DLL's seven-symbol publication, complete clearing on failed lookup, and cached availability. The original retains the loaded module. |
| `57daa0/584430/589280` | World effects mutate existing second/fourth palette nodes; they neither allocate nor relink those nodes. |

Missing `Rect1` scales `Rect2` by **0.8**, truncates the signed values, and writes
`Rect1` back through `WritePrivateProfileString`. This is recovered behavior,
separate from the stricter native configuration policy. Rect token absence can
fault; missing optional sprite indices preserve the corresponding fields.

After a lost pixel lock, `58b660` records Restore's result but ignores the second
Lock HRESULT when deciding failure. `58ad90` can diagnose a failed CreateSurface
and continue to QueryInterface, or tear down the drawing service and call
DestroyWindow after later failures. These are original observations, not native
transactional recovery promises.

The primary-surface CreatePalette branch initializes RGB as the 0..255 gray
ramp (`58b004..58b031`), leaving the fourth palette flag byte indeterminate.
The fixture compares the defined RGB bytes and retains the flag-byte exclusion.
Expanded primary-surface comparison exposed and corrected an initial all-black
model; the failed proof09 is retained. Proof10 subsequently passed every
provider configuration and constructor-prefix check, then refused three omitted
transitive header declarations before the composition proof. Proof11 includes
the reviewed dependency closure and passes the complete run.

Word and DWORD World effects have distinct x87 luminance addition order and
output table extents. The new model preserves the original power sequence,
coefficient spills, conditional tint branches, packed 555/565 values and cached
lighting powers. Its admitted domain is bounded, finite owned lighting and RGB
storage, power-of-two shade counts 2..256 and three finite tint coefficients.
Other floating-point/environment and malformed backing cases remain outside it.

## Ownership composition

The Main → World → Report → Main fixture now uses the recovered layout parser,
control constructors/text, vector operations, raw SPR loader and palette chains,
surface creation/locking and resource retirement. World enters with populated
SPR buffers and four-node palette chains at `6de558` and `6a28f8`. The recovered
World exit retires those actual allocations. **No World resource fields are
cleared before exit.**

The selection, markers and lines resources, and the score-service object, have
session lifetimes. The fixture retains them through World exit and retires them
at explicit session shutdown. It also releases Report's retained vector backing.
The score producer/items, simulation services, fonts, COM implementation and
JPEG codec remain named supplied endpoints in this composition; this is not a
claim of real transition routing, simulation closure or actual pixel rendering.

## Failure and native policy

Original text setters free the previous text before allocating/copying its
replacement. Null allocation can therefore fault with the new null field already
published. Main/Report/button cleanup funclets free or unregister only the
currently constructing object. They do not roll back every previously published
screen resource. Direct parent-EBP invocation tests those cleanup bodies;
Windows SEH dispatch is not emulated or validated by that test.

The CRT array constructor's native model destroys only completed constructors,
in reverse order, after a thrown constructor. Its reverse destruction helper
`59bf87` consumes the **end pointer**, not the beginning of the array.

World entry stores the zero rate and then the original `1000 / rate` faults.
The reconstructed entry now throws a domain error at that boundary, preserving
its partial wire state and avoiding C++ undefined division. Its normal-return
caller comparisons are rerun under new evidence, preserving the old evidence.

The original Report uses a seven-byte decimal scratch area. The previous
[negative observation](screen-acquisition-results-20261026/acquire05-gdb.log)
records `INT_MIN` overwriting a loop local and eventually faulting. That remains
historical negative evidence; it is not converted into an original success case.
`screenScoreText` deliberately handles the full signed DWORD range using owned,
bounded native text.

`assets/resources/screen_resources.*` adds a CPU-only native screen owner:

- It owns copied controls and actual SPR/BMP/PCX/JPEG resource recipes and images.
- It stages a complete replacement, retaining the current screen on recoverable
  binding, allocation, loading, decoder or cancellation exceptions.
- Checkpoints expose completed binding/decoded prefixes. Reset or a nested newer
  replacement supersedes staging, preventing an older operation from publishing.
- Worker requests own their inputs. Adoption checks both screen lifetime and the
  ResourceManager binding identity, rejecting retired and foreign completions.

Allocation injection covers C++ binding/staging allocations before decoding,
plus `bad_alloc` at each completed decoded-provider prefix. The early broad
injection attempts terminated in Qt's `noexcept` allocation paths; those raw
negative logs are retained. Qt-internal/global catastrophic exhaustion is not
advertised as a recoverable native error. Ordinary loader errors and worker
cancellation have their separate checks. Resource adoption is thread confined;
this service does not install a widget or GL replacement.

## Validation and reproduction

Execution results are registered separately from the implementation. The driver
pins the original hash, verifies the immutable input manifest before and after,
reviews local direct edges and the layout/shade dispatch tables, and checks
executed original instruction bytes against a generated protected census.
Individual endpoint substitutions are excluded from that fixture's protected
set and exercised as real original bodies in the corresponding lower-provider
fixture. Independent value-address storage compares owned bytes, published
fields, calls and expected fault partial state. Void/caller-saved register residue
and temporary stack identities are not semantic outputs.

Run `tools/test-screen-providers.py --output <new-bounded-build-dir> --report
research/runtime/screen-provider-results-20261026/<new-id>.json` under the storage
guard. Use a bounded RAM build directory and a new report identity. It runs
optimized original/native PE32 comparisons, 32-bit ASan/UBSan comparisons,
64-bit sanitized native checks, completed-prefix exceptions, composed ownership,
and the native resource-owner and existing manager/preparation suites. Durable
logs and report payloads are SQLite evidence; generated binaries are temporary.

The temporary 88 GiB storage floor and 2 GiB durable-output cap were explicitly
approved for this batch on 2026-10-26. They authorize neither publication nor
cleanup and do not change the repository's default 90 GiB reserve.

Fresh `SP26.original` and `SP26.native` evidence is recorded in
[proof11](screen-provider-results-20261026/proof11.json). Each optimized PE32,
ASan/UBSan PE32 and sanitized native64 configuration passes 2,551 provider
schedules: 1,174 controls/vector/registry cases, 144 surface cases, 704 palette
cases, 528 layout cases and one zero-rate World partial-state comparison.
All configurations match. Native exception tests pass 325 constructor-prefix
schedules. Sixteen composed Main/World/Report journeys retire their actual
screen allocations and session resources without pre-clearing World fields.
The native owner passes concrete SPR/BMP/PCX/JPEG loading, rollback at each
completed provider prefix, allocation injection, cancellation, stale/foreign
worker rejection, reentrant replacement and full signed score formatting.
Existing ResourceManager/preparation regressions and the fresh 768-schedule
caller regression (`RSA26.provider-regression`) also pass. Immutable input
manifests verify before and after. No original live replacement is installed.

The earlier original Report score overflow is retained as negative evidence;
full signed score handling is an explicit native policy. Actual Windows SEH
delivery, physical drivers/codecs, simulation and score content, real transition
routing and installed widget/GL acceptance remain outside these results.


The committed-history review covers `bdf87f55..1ffeb53e`: three commits and 95
exact parent/current payload transitions, with no missing receipts. The
[history report](screen-provider-results-20261026/history01.json) distinguishes
that review from this batch's uncommitted changes and makes no retrospective
engine-validation or historical gate-pass claim.
