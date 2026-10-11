# DRAW-01 scenario-tagged drawing inventory

The machine-readable [route register](drawing-inventory.json) implements DRAW-01
accounting for the pinned No-CD build. It records individual entry routes,
aggregate producer operations and original queue kinds separately. Each route
has stable IDs, behavior links, a subsystem owner, classification, explicit
support scope, required side effects, confidence and remaining gaps.

**DRAW-01 is not globally complete.** The route checker is implemented; fourteen
required scenario censuses, unclassified kinds, alternate/default dispatch and
unattributed callers remain visible. Existing bounded captures do not prove that
every original pixel writer was instrumented. The immutable original binary
inventory and all historical evidence hashes remain unchanged.

## Reproduce the accounting

```sh
python3 -B tests/test-drawing-inventory.py
python3 tools/report-drawing-inventory.py --require-accounted \
  --json working/tests/draw01-new.json --markdown working/tests/draw01-new.md
python3 tools/report-drawing-inventory.py --require-complete
```

The first report command fails for newly observed identities or producer hook
entries that lack reviewed routes. `--require-complete` additionally fails while
required scenario censuses, dispatch tables or support decisions remain open.
Report files must be new; the reporter refuses writes under `original/` or
`.git/`, including symlinked roots. It reads repository evidence without opening
original game/media artifacts and performs no binary changes or game execution.

Eight retained observation cohorts seed the inventory: sixteen-queue startup
producer operations and raw input kinds, the later four-return producer shadow,
the guarded sixteen-queue exact-entry World batch comparison, glyph shadow and
takeover cohorts, both enabled native intros, finite minimap motion and the
later 25-call guarded fade-loop takeover. The
scenario labels are bounded evidence collections. They do not imply that every
required scenario, phase, wrapper or caller was exercised. Operation totals never
become entry totals; independent cohorts are not summed into runtime frequencies.

The retained static export supplies twenty-four selected instruction windows,
all their direct caller sites, indirect calls and computed jumps. The checker
requires a route for every window and every current producer-hook entry, plus the
separate interior fade splice. An interior entry must name an explicit registered
original anchor and lie within that function's pinned discovered ranges; it does
not become an additional discovered function. Exact
CALL bytes determine a return PC; no nearest-function or nearest-PC assignment
is made. The historical `shared_sprite_dispatch` window at `581ec0` is labeled
as the recovered SFT glyph route. The unassigned `58d3b0..58d4a7` WORD-copy window
remains unassigned. Static call counts and instrumented entries are never live
activity evidence. Native support decisions are scoped to linked research;
they do not promote central implementation/comparison/replacement statuses.

## Expanded exact caller census — 2026-10-10

The supplemental [caller export](drawing-callers-expanded-20261010.json) covers
all 61 registered entry identities across the pinned executable disassembly,
retaining 909 exact rel32 CALL sites. It complements the historical 24-window
export without replacing it. The importer verifies each CALL's bytes, target
and exact return PC, accepts only registered targets, and rejects conflicts.
Discovered owners come only from containing recorded function ranges; empty
owner lists remain empty. Instrumented indirect calls still require observed
callee identities and separately reviewed ownership.

Against the diagnostic preferred-base interpretation of the retained four-return
journal, 53 of its 63 previously unattributed caller/operation groups have an
exact static match; ten raster groups still lack one. This is a static crosswalk,
not fresh process-base evidence or semantic runtime attribution. Historical
journal evidence retains its original hash and 63-group result. Fourteen scenario
closures, unknown writers and incomplete dispatch remain pending.

Reproduce in a new output file:

```sh
python3 -B tools/export-drawing-census.py --executable <pinned-Chaos.exe> \
  --output <new-report.json>
python3 -B tests/test-drawing-census.py
python3 -B tests/test-drawing-inventory.py
```

The full authorized work targets all fourteen required single-player journeys,
including sustained battles, interactions, transitions and shutdown. Incremental
commits do not close this milestone until scenario, caller, graphics-side-effect,
failure/lifecycle and dispatch evidence supports the strict completion gate.

## Original-active call metadata — 2026-10-10

The observation-only `runtime/drawing-census/` hook and V1 wire journal retain
actual callee entries, thread, incoming return address, paired return, unsigned
EAX, flags/LastError, six diagnostic argument words, selected drawing globals and
host scenario phases. The loader requires the actual module base `0x400000`,
checks all selected original signatures before mutation, and relocates copied
relative instructions. Staging checks the pinned executable hash and preserves
every original file-backed section; added DLL imports live in disposable files.
The original drawing bodies and menu callbacks execute. There is no native
rendering, original-body suppression or balance change.

The current build accepts **60 of 61 catalog sites**. The fade interior
`58edb0` cannot safely take a six-byte entry splice: the original branch at
`58edde` targets `58edb1`, inside those bytes. The first live attempt exposed this
and is retained as failed evidence. The builder now refuses incoming direct
branch/CALL targets inside a splice; it does not assert that indirect entry into
all copied bytes has been excluded. Fade wrapper `58ed80` remains observable.
An interior checkpoint records no guessed caller or paired return.

The fresh scope-bound menu repeat retained 83,178 events / 41,589 calls at 18
entries. Its result is registered as
`DRAW.census.menu-observation-20261010`; the journal/ABI/guard result is
`DRAW.census.journal-guards-20261010`. Neither result closes a scenario census.

The synthetic PE32 fixture executes the actual assembly and callbacks against
callee-pop 0/8/24-byte contracts, comparing full register/flags/x87/SSE snapshots
and LastError. It checks same-stack tail-call unwinding, interior forwarding,
recorded errors and frame-exhaustion forwarding with explicit loss. The journal
parser rejects malformed build/base/tag/extent/count/reserved fields, wrong
thread/stack/callee/phase returns, loss, open calls, IO failure and absent terminal
markers. Host phase labels express fixture intent, not scenario completion.

The menu fixture traverses Main -> Preferences -> Main -> Quick Battle -> Single
Player setup -> Map selection -> cancel back through Main -> original quit. A
closed diagnostic run retained 94,256 events / 47,128 calls at 20 entries in 112
entry/caller groups. These are that cohort's counts only. Empty discovered-owner
sets stay empty. Physical return addresses may include observation trampolines
for tail calls or the separate menu action bridge; they require explicit
call-flow classification before semantic attribution. Global snapshots identify
possible state changes; they do not inventory every side effect or pixel writer.

```sh
python3 -B tools/prepare-drawing-census.py --source-game <pinned-installation>
python3 -B tools/capture-drawing-census-game.py <fresh-experiment> \
  --prefix-template <stopped-Wine-prefix> --journey menus
python3 -B tools/test-drawing-census-abi.py \
  --prefix-template <stopped-Wine-prefix>
python3 -B tests/test-drawing-census-trace.py
```

Use a fresh experiment for each journey. The fixture owns a private Xvfb display
and Wine prefix, maintains the menu host heartbeat, retains failed reports and
checks staged binaries. A successful terminal proves only bounded journal
integrity: loss/open/IO counters must be zero. Busy loader detach does not wait on
terminated workers and cannot produce a valid terminal. Dynamic unload is
unsupported; partial hook installation refuses startup. Capture overhead has no
hard timing promise. Movies, all writer/API/borrowed-access routes, failures and
recovery, save/load, later map transitions, results and full dispatch still need
separate evidence. DRAW-01 remains incomplete.

## Queue dispatch and helper census — 2026-10-10

The [expanded dispatch caller export](drawing-callers-dispatch-expanded-20261010.json)
contains 937 exact direct CALL sites for 68 registered entry identities. Seven
previously omitted consumer helper/cleanup targets are now explicit original-only
routes: `4df840`, `57e8e0`, `57fe10`, `580260`, `580a90`, `581200`, and `4d8ab0`.
The metadata observer now admits 67 safe sites; the unsafe fade interior remains
refused. This increases the route register to 107 routes without asserting that
every executable writer or indirect target has been discovered.

The [original-consumer result](drawing-dispatch-anchors-20261010.json) retains
2,432 cases executing the pinned consumer's unchanged instructions with synthetic
backend bindings. Each binding's stack cleanup is checked against explicit
original RET instructions. Cases span all three dispatch tables, numeric kinds
0..34 and selected unsigned defaults, indexed/direct object markers, selected
auxiliary spans, signed offsets, and four backend return values. They confirm
1,280 literal wave calls and 240 literal colour calls. Kinds 16, 17 and 20 use
wave amplitudes 2, 3 and 6 respectively. Kind 9 stores zero metadata, ordinary
returns truncate to AX, defaults can preserve prior metadata, and one indexed
sprite branch normalizes its result to a boolean before storage.

The static slicer retains all 105 raw table targets, both conditional arms,
shared exits and opaque CALL operands. It refuses unresolved indirect jumps or
missing instruction boundaries. Static slices do not establish virtual target
semantics. Numeric 33 also has pre-table routing in the Video-enabled paths;
the raw table alone does not describe the consumer's full behavior.

The fresh [expanded ABI result](drawing-census-caller-source-abi-20261010.json)
checks callee-pop sizes 0/4/8/12/20/24 and preserves the forwarding checks from
the earlier fixture. Original raster pixels, arbitrary virtual objects, helper
global effects, changed prior metadata, clip-boundary auxiliary paths, enabled
postlude cleanup, borrowed graphics API lifetimes and complete scenario caller
ownership remain pending. All four dispatch gates remain incomplete. Earlier
results and source fingerprints are retained, including those superseded by
catalog or scope changes; no historical hash was refreshed.

```sh
python3 -B tools/test-drawing-dispatch.py --executable <pinned-Chaos.exe>
python3 -B tests/test-drawing-dispatch.py
```

Campaign attempts are retained separately from the successful menu cohort. The
Novice tutorial and the Adept Preferences return require distinct input
fixtures. These attempts have incomplete journals and cannot close a scenario.
Compressed partial journals retain their original byte hashes and verified
archive hashes. Difficulty/input phase labels express intent, not complete
camera, interaction or battle-result coverage.

## Bounded original campaign journal — 2026-10-10

The [fresh campaign observation](drawing-census-campaign-observation-20261010.json)
retains 2,416,882 events / 1,208,441 paired calls, with zero loss, open calls or
I/O failure, unchanged staged binaries/sources and normal original exit. The
journey visits Main/setup, all four difficulty choices (entering Adept), World,
camera/cursor input, Mini/Preferences, World return, original Quit confirmation,
defeat report, Main and shutdown. Phase labels still describe fixture intent.
They do not establish full camera semantics, complete battle results or a
closed census for any of the fourteen required scenarios.

The cohort reaches 44 entries in 674 phase/entry/caller groups: 615 have exact
direct CALL attribution and 59 retain pending call-flow review. Nine of the ten
formerly unmatched preferred-base caller PCs now have observed callee identities
in this closed capture; `4dcbdd` was not exercised. This does not retroactively
normalize the historical journal or establish ownership for indirect callers.

Original Hermes dialogues can hold input across camera/menu transitions. The
fixture retains screenshots and uses original mouse/Escape input before Mini;
it does not mutate tutorial or simulation state. Failed attempts remain retained.
One completed journey was correctly rejected for two phase-control races. The
reader now retries boundedly during host seqlock changes; stable invalid phase,
dead host or persistently odd sequence still records loss. The fresh
[phase/ABI fixture](drawing-census-phase-abi-20261010.json) checks those refusals
alongside register, flags, FP, LastError and stack cleanup forwarding.

The new observation does not invent owners for undiscovered function ranges or
classify observer/bridge return PCs as original direct callers. Pixels, arbitrary
writers, borrowed API intervals, resource generations, failures/recovery,
save/load and later map transitions remain independent obligations.

## Phase projection and enabled movie requests — 2026-10-10

The checker now projects retained entry/caller-group counts by exact host phase
within a single closed metadata cohort. Campaign phases 2/3/5/6/9/10/13/14 are
linked to their declared scenarios. Counts from different experiments remain
separate. The recorded hook manifest is reviewed independently from hits: 67
accepted entry/checkpoint sites and one explicitly refused fade interior. These
do not replace the 52 legacy producer hooks or establish writer completeness.
The [fresh guards](drawing-inventory-phase-guards-20261010.json) cover phase
filtering, malformed excluded rows, unknown entries, duplicate hook identities
and route conflicts. All 37 checks pass; the report still has 18 closure blockers.

The [enabled original movie-request cohort](drawing-census-enabled-movie-requests-20261010.json)
retains 84,752 events / 42,376 paired calls and normal original shutdown, with
zero loss/open/IO and stable sources/binaries. Both original startup calls reach
`469a00`; their returns are 1 after 313 and 189 milliseconds. This is a fast
return path, not evidence of full movie playback. The wrapper's return boolean
does not independently establish stream creation or sample decoding. Original
stream setup/playback/failure paths, controls, destination placement and graphics
side effects remain pending. The movie-enabled fixture's successful journal
must not be presented as successful audiovisual playback or scenario closure.

```sh
python3 -B tools/prepare-drawing-census.py --source-game <pinned-installation> --movies
python3 -B tools/capture-drawing-census-game.py <fresh-experiment> \
  --prefix-template <stopped-Wine-prefix> --journey movies
```

## Inspect an existing producer journal

The optional importer reads only 64-byte envelopes and 96-byte record headers,
skipping source payloads. It validates V1/V2/V3 build/version/bounds, contiguous
record and World-return sequences, payload extents and a matching successful
`MNMPDONE` marker. It requires the journal SHA-256 recorded by the capture, checks
both inputs stayed unchanged, and preserves unknown opcodes and caller PCs.
Source assets and destination-oracle pixels are not opened.

```sh
python3 tools/report-drawing-inventory.py \
  --producer-stream <capture>/canvas-producers.bin \
  --done <capture>/canvas-producers.done \
  --stream-sha256 <recorded-journal-sha256> \
  --stream-scenario quick-battle-entry \
  --runtime-image-base <observed-process-module-base> \
  --json working/tests/draw01-journal-new.json
```

The runtime module base is explicit; the tool does not infer it from raw pointer
values or assume it persists across launches. Verify it against the captured
process before using normalized caller identities semantically. Exact static
callers may identify an entry. Other callers remain unattributed even when the
aggregate operation is understood. The importer checks journal accounting, not
the original success semantics of every drawing call or complete instrumentation.

## Completion and ownership

Scenario completion requires a separately retained live census report with an
explicit reviewed closure: complete caller attribution, required side-effect and
owner/classification inventories, zero lost events and zero unknown routes,
bound to the exact route register digest. A complete producer journal or matching
pixel corpus alone cannot supply this closure. A closure must explain how paths
outside the producer hooks, borrowed DC/Lock intervals and indirect/alternate
dispatch were accounted for. Updating the route register invalidates that closure
until it is reviewed against the new census. The initial fourteen scenarios have
no such closure; no new original observation is asserted here.

Rendering-side state/readbacks, allocation/generation transitions, original
caller results and retry/failure branches are inventory obligations alongside
visible pixel producers. The owner fields name repository subsystem boundaries,
not host pointers or named contributors. Unknown caller ownership, opaque import/
COM targets and unclassified table/default branches remain pending research.

Use the [coverage register](coverage/register.json) for behavior/evidence status,
the [ledger](coverage-ledger.md) for achieved scope, and the
[replacement plan](../../docs/live-drawing-replacement.md) for sequencing. This
route register complements the rendering API support report; neither replaces
the other or claims an overall completion percentage.


## Original save/load fixture under validation — 2026-10-10

The isolated staging tool now creates the original `Save\\` output directory.
Without it, the original Save dialog reports "Unable to save file" and creates
no save. With it, the original fresh campaign produces nonempty `.sav` files;
retained copies remain in each attempt's `saved-game-artifacts/` or
`save-artifacts/` directory. These are original-generated artifacts, not proof
of native persistence compatibility or a successful round trip.

The first Load fixture typed the filename without selecting its list row and
faulted reading `00000010` at `005b74dc`. The same failure repeats with drawing
entry hooks absent, leaving the menu adapter active; it is not a fully unhooked
baseline. The revised fixture requires exactly one new `draw01.sav`, retains
its bytes/hash, selects that original list row, and verifies Load heading,
World/Mini return and normal original quit. Results remain pending until the
fresh attempt finishes; neither crashes nor screen labels close save/load.

The optional `--menu-only` staging diagnostic omits the drawing DLL import and
reports `call_census_enabled:false`. No drawing journal or terminal is inferred.
Two later attempts exposed host-admission/lease timing: one has no channel
publication despite visible Main; another reports retired status5. The fixture
keeps its initial host snapshot stable during admission, then starts heartbeat
renewal 100ms after the observer log header appears, while original assets load.
This timing policy needs fresh live validation and is not an engine recovery fix.
All failed attempts and partial journals remain retained; compressed partial
journals retain their original byte hashes and explicit incomplete status.


The [selected-row menu-only control](drawing-census-save-load-menu-only-success-20261010.json)
now finishes the bounded original Save/Load journey: a retained 353,641-byte save,
original World/Mini and Preferences return, original defeat/Main and exit0.
Sources and staged binaries remain stable. Drawing entry hooks are absent and
no call journal is claimed. The visible loading transition begins on list-row
selection, before the later explicit Load-button input; input ordering is retained.
The same corrected fixture with full drawing metadata remains under validation.
The two failed Load reports are retained under distinct immutable evidence IDs;
the successful control does not retroactively validate those attempts.


The [full original-active save/load journal](drawing-census-save-load-observation-20261010.json)
also completes with exit0: 3,607,228 events / 1,803,614 paired calls, zero
loss/open calls, verified actual base400000 and stable source/binary fingerprints.
The original-created 354,045-byte save is retained with SHA-256
`76bd3f35812ce21ad19dc18bf073772c54725dd65eb80e8c14afb55ffda040c4`.
This validates the bounded selected-row journey and journal; exhaustive caller,
borrowed graphics, state/pixel/lifetime/recovery and scenario closure stay pending.


### DRAW-01 original movie contract and helper census — 2026-10-10

The [unchanged original movie reference](drawing-movie-scoped-original-20261010.json)
passes1,760selected external HRESULT/control/Escape/Update/Blt/placement/cleanup
cases, including sentinel checks of wrapper state-write ordering. It corrects
EDX to DirectDraw device and the first stack word to destination surface.
[Contract findings and exclusions](draw01-movie-contract.md) retain exact
short-circuit, skip/result and media/video/sample/source release order. Native
Qt decoding/replacement and actual original codec/pixel/audio output stay separate.

The catalog now has109routes/70entry identities; its new exact caller export
has939direct CALL sites. The [expanded original movie-helper journal](drawing-census-movie-helpers-observation-20261010.json)
has69safe metadata hooks plus the original unsafe fade-site refusal,82,380events /
41,190paired calls, zero loss/open/IO, stable sources/binaries and normal exit.
Both intros enter original setup/sample: each helper returns0 and wrapper1.
These quick returns do not establish complete playback; exact sample Update/Blt
counts/results and original audiovisual output remain pending. The driver now
binds the actual menu source/header compiler inputs prospectively; older records
retain their original narrower source declarations.

The [fresh ABI fixture](drawing-census-movie-pop16-abi-20261010.json) adds the
movie wrapper's16-byte callee cleanup to0/4/8/12/20/24, with independent tail8
baseline and retained flags/registers/x87/SSE/LastError/loss/phase guards. It uses
synthetic bodies; actual69original splice manifests and live observations are
separate. Committed history5abde11..7bb1f46 has zero missing exact receipts.
All14scenario closures and4DRAW-01 dispatch obligations remain open.


The [fresh accounted movie-site repeat](drawing-census-movie-sites-accounted-observation-20261010.json)
again observes two immediate `MS_S_ENDOFSTREAM` exits:82,316events,41,156paired
calls and4SITE events; zero loss/open/IO, stable declared sources/binaries and
exit0. Its catalog projection correctly addresses `/decoded/original_sha256`;
the earlier record and failed inventory guards remain unchanged. The inventory
reports123routes,83metadata sites/1refusal,939direct CALL sites and0unaccounted
activity; all18DRAW-01 closure blockers remain. [54fresh guards](drawing-census-movie-site-guards-20261010.json)
cover mode/anchor refusal, mixed counters and result separation, exact callers
and scenario/unknown-route accounting. Source-only decoder/catalog changes leave
older original reference, ABI and live evidence stale; historical stages/hashes
are retained without claiming current full original/native equivalence.

## Bounded original Quick Battle and results — 2026-10-10

The Quick Battle driver selects the original second offered map (Celtic Forest
Two), keeps all17original setup rules and the offered empty spell assignments,
and observes World plus bounded camera/HUD/cursor inputs. Original contexts1/2
route Escape to results; Continue resumes World, and a second Escape/results/Quit
returns through Quick Battle/Main to normal shutdown. The existing
[results contract](quick-battle-results-engine-bridge.md) and
[Mini findings](mini-menu-engine-bridge.md) define that boundary. Experimental
Quick Mini remains disabled. The first attempt incorrectly expected Preferences
after Escape, retained the actual Game Over screen and timed out; its partial
journal and source hashes remain historical diagnostics.

The first successful cohort retained2,509,020events/1,254,510paired calls at41
entries, no sites, loss or open calls. Phase projections are declared separately
before a repeat. Camera inputs and HUD hovering express intent; this empty
loadout does not validate summoning, effects, actual camera motion or every
semantic tooltip. Results rows are copied from original display strings, never
calculated by the host. Writer/caller/side-effect/borrowed API/lifetime/recovery
and14scenario/4dispatch closure remain pending.

```sh
python3 -B tools/prepare-drawing-census.py --source-game <pinned-installation>
python3 -B tools/capture-drawing-census-game.py <fresh-experiment> \
  --prefix-template <stopped-Wine-prefix> --journey quick-battle \
  --claims <fresh-prospective-claims.json>
```

The prospectively registered [repeat](drawing-census-quick-battle-accounted-observation-20261010.json) retains2,447,382events/1,223,691pairs, zero loss/open calls, stable sources/binaries and exit0. Both original Game Over headings were checked. Each phase projection stays a bounded cohort; all18closure blockers remain.

## Original campaign summon and effect cohort — 2026-10-11

The [campaign effects cohort](drawing-census-campaign-effects-observation-20261011.json)
uses the existing opt-in signature-pinned original creature/spell observers with
original drawing active. After bounded OCR checks and original mouse dismissal
of introductory Hermes dialogues, a physical Zombie cast must create a newly
living, player-owned type14 creature in the sampled original pool. Failed casts
cannot satisfy the fixture merely by returning an EAX value. The inspected
screenshot shows the summon effect and original `1/15` HUD count. Later bounded
hover/animation/camera/cursor inputs precede original Mini/Preferences/World,
Quit confirmation/defeat/Main and normal shutdown.

The retained original spell callback records the same player wizard's type14
cast and a raw mana debit of 4,352 within the sampled creation interval. The
existing `campaign_spell.validate_spell_observation` validator was applied
read-only after the run; its result and explicit post-run scope are retained
separately inside the cohort report. The 514 creature samples and 12 spell rows
have checked framing/sequences and retained hashes. These auxiliary formats have
no journal terminal/loss counter: they do not prove every tick, callback,
resource generation or lifetime. Earlier scripted Fireball/damage records belong
to their actual source/target identities; they are not assigned to the player's
physical Zombie cast. Effect-to-queue-to-pixel attribution remains pending.

The drawing journal closes with 4,184,314 events / 2,092,157 paired calls,
zero loss/open calls and stable binaries/sources. This is close to the fixed
4,194,304-record budget; larger journeys must be bounded explicitly, and any loss
invalidates the journal. The opt-in adds passive original hooks only; no orders,
AI, health, mana, positions or balance rules are patched. Scenario phase7 covers
the physical summon and later animation interval; phase8 records HUD/minimap
hover intent. Neither projection closes a scenario census.

```sh
python3 -B tools/prepare-drawing-census.py --source-game <pinned-installation> \
  --gameplay-observation
python3 -B tools/capture-drawing-census-game.py <fresh-experiment> \
  --prefix-template <stopped-Wine-prefix> --journey campaign-effects \
  --claims <fresh-prospective-claims.json>
```


## Exact API CALL-site accounting, 2026-10-11

The route census now separately includes380 exact original CALL instructions,
bringing the reviewed catalog to503routes. CALL points have an `api_site` selector,
not a function `entry`: their complete instruction bytes and return PC must match
the retained pinned static export, and every exported site needs a reviewed route.
Target mode and exact import slots are checked independently against bytes. Empty
discovered owner sets stay empty, and offset128 never supplies a COM method/type.
All380API routes preserve original execution; no native replacement is admitted.

Four independent expanded original API-only movie/save/load/results/effects reports
project CALL-site activity by the exact host phase. The projection requires an
intact build-bound journal and validates excluded rows as well as included rows.
Repeated per-run target instances accumulate only within their own phase/cohort.
Unknown observed sites remain unaccounted; no nearest function/entry is assigned.
The selected cohorts observe101unique sites across all14host phase labels. This
records selected activity, with279admitted sites still unexecuted; it establishes
neither complete writer instrumentation nor semantic caller/effect/lifetime scope.
Resource intervals and host phase intent remain separately bounded evidence.

Forty-five native guards pass. `--require-accounted` passes with zero unknown
identities, while `--require-complete` exits1 with18pending obligations:14scenario
reviews and four dispatch gates. None of those reviews is promoted. Their remaining
text now reflects the original save/load, results and summon evidence rather than
claiming those observations do not exist. The retained byte export/original reports
keep their historical source hashes; changing the catalog creates explicit source
staleness, not fresh original execution. The new [guard result](drawing-inventory-api-site-guards-20261011.json)
validates current offline accounting only.

History002633e..c43bfde contains9exact file/12behavior receipt matches and no
missing intermediate versions. Its review does not assert a past gate pass or a
new graphics result. The branch remains isolated from the shared worktree.


## Original consumer client state fixture, 2026-10-11

A separate headless fixture executes the unchanged pinned5002a0 consumer across
58,320 unique cells. It complements the earlier2432routing matrix without changing
its sources or hashes. Empty/one/two rows, Video0/1, kind9/15/default, first-span
sentinel and signed boundary values, changed prior16-bit metadata, nine vertical
positions, 480/600height flags, raw0/high/negative returns and optional cleanup are
explicit axes. All backend/clip/cleanup calls use synthetic bindings whose pops
are checked against actual original RET instructions. This fixture integration is
registered separately from native renderer comparison, which remains none.

Original consumer instructions remain byte-identical throughout. The row/container
checks retain all untouched words and both trailing rows. With Video enabled and
prior metadata different from8ad0, original row y becomes old-y minus signed prior
plus object height16 in this explicit object fixture; unprocessed rows stay intact.
Kind9 stores zero, selected wave metadata truncates to16bits, and defaults retain
prior metadata. Opening clips require strict top/bottom inclusion. Their exact
return PCs500337/500388 are separate from auxiliary clip5008b9 and restoration
500d25, including equal raw arguments. Auxiliary dispatch requires its endpoint
strictly below480or600according to the flag and below the clip bottom; negative
auxiliary tops clamp to zero.

The native integration records75,312clip,25,500wave and19,440copy-backend calls.
Optional postlude produces29,160calls, including empty queues: exact receiver
6ddf78, original three counter arguments, last-call ordering, and original zeroing
of6e8db0/6e8db4/6e8db8 occur independently of the synthetic raw return. Disabled
postlude preserves those counters. Raw returns are not assigned HRESULT semantics.
Actual helper body state, raster pixels, other object layouts and changed-prior
kinds, real drivers/interfaces/lifetimes and all14scenario/four dispatch completion
remain pending. The table entries retain only the new selected client scope.

[Final result](drawing-queue-client-state-20261011.json) retains the raw-case file,
compiled fixture, command logs and hashes. [Attempt limitations](drawing-queue-client-state-attempts-20261011.json)
preserve the zero-row default-sandbox SIGSYS repeat, passing debugger diagnostic,
unsupported first scenario declaration and intermediate pass before unique-cell
validation. The final successor uses a new supported headless declaration and
runs with required32-bit syscall permissions; no old hash is refreshed.
Historyc43bfde..0b40de2 has11exact file/21behavior receipts, no missing versions;
retrospective review asserts neither a past gate pass nor new graphics validation.


## All-kinds original consumer client state, 2026-10-11

The widened fixture covers984960unique cells over all kinds-2..35, raw returns
0/1/12345678/ffffffff, Video0/1, empty/one/two rows, first-span sentinel/signed clip
boundaries, changed prior metadata and optional cleanup. It uses a direct object
marker and ordinary offset; earlier indexed/-127 routing evidence remains separate.
Every original row/container assertion, opening/auxiliary/restoration clip caller,
strict480/600threshold and cleanup ordering/counter reset passes. Kind9 keeps zero;
defaults and Video-disabled7/32 preserve prior metadata; ordinary results truncate
to16bits. Wave kinds15..30 retain literal amplitude kind-14 across primary/auxiliary
routes. Consumer instructions and original input hashes stay unchanged.

The case archive records93exact original CALL return PCs,1271936clip calls,
544000wave calls and492480cleanup calls. These are this offline fixture's counts,
not live runtime frequencies. Fifteen direct backend RET contracts and separate
opaque/global/virtual bindings remain explicit. Actual helper bodies are synthetic,
so helper global effects, pixels, arbitrary objects, changed-prior indexed/-127
branches and complete live API/lifetime/scenario/dispatch closure remain pending.
The tables link the new client-state contract for every numeric/default entry while
keeping their completeness flags false and all recovered native implementation/
comparison/integration/replacement statuses none.

[Fresh result](drawing-queue-all-kinds-state-20261011.json) binds the prospective
scope/source declaration and984960unique matrix cells. The bounded producer records
actual process exit/timeout and streams a lossless gzip archive. It hashes the raw
stream and independently decoded archive:559379392bytes, identical SHA-256
524f76b1099db750656eaa8494a2fca2d3fef3224bd602fafe999ad9125584de.
Compressed size16054745bytes avoids retaining duplicate raw storage. Interrupted
or nonzero runs cannot become successes; native helper/lifetime completion is not
inferred from compression integrity. The earlier58320cell result and failed/
intermediate attempts retain all historical fingerprints.

History0b40de2..23bd1c0 has14file/85behavior exact receipts and no missing versions.
Retrospective accounting does not assert a past gate pass or new graphics result.


## Original optional render postlude, 2026-10-11

The late consumer helper at `4d8ab0` performs rendering as well as metadata work;
its earlier lifecycle/cleanup label was incomplete. The route is now classified
as composition. Historical reports and their hashes retain the terminology used
at execution; this finding does not rewrite their evidence or imply actual global
counter semantics. Only the first of the three consumer arguments contributes
to this helper's rendered numeric text in the tested paths.

The [new fixture](drawing-render-postlude-client-20261011.json) executes unchanged
original postlude, frame/clip setters, clip getter and signed decimal conversion
instructions across12288owned client cells. Disabled, not-ready and null-Lock exits
preserve the initialized frame/clip state and row arrays. Successful mock Lock
produces the original19byte-prefix plus six selected signed values, including
INT_MIN/INT_MAX; font calls retain five trailing zero arguments. Fill and slot128
failure returns are ignored by the original client. This verifies continuation
only, not successful external drawing or resource release.

The real57dc20setter stores pixel pointer at658174, pitch/2 at6a2dc8 and supplied
dimension at6def50. Real57dc60/57dc90write/read the clip rectangle. Drawn rows
store width100, a four-byte-aligned returned font width and centered/aligned
placement in640/800mode. Other object fields/rows remain byte-identical. Canvas
fill/Lock, font and virtual slot128 bodies are synthetic; no real font pixels,
COM type from slot offset, reference/lifetime/driver, arbitrary layout or full
scenario/dispatch completion is claimed. The separate57dcc0scatter getter remains
unassigned rather than being attached to a nearby discovered function.

[First-attempt limits](drawing-render-postlude-attempts-20261011.json) preserve the
fixture's wrong expected dimension global6e1f70 and exit1 after10752early-exit
cells. The corrected fixture uses a new prospective source declaration and
passes12288cases with original input and helper bytes unchanged. Original input
manifest verification passes before/after both executions. Actual fixture totals are3072fill/3072Lock/1536font/
1536slot128 calls.

The [current offline45guards](drawing-inventory-postlude-guards-20261011.json)
validate the corrected catalog against retained original reports. They retain
zero unknown identities and18open completion obligations; catalog changes leave
historical original evidence source-stale rather than refreshing its hashes.
The committed23bd1c0..1fe53d9range has10file/29behavior exact receipt matches and
no missing intermediate versions. This history review asserts no past gate pass
or new graphics validation. Recovered native implementation/comparison/integration/
replacement remain none; the fixture is bounded original-contract discovery.


## Original indexed and offset consumer states, 2026-10-11

The [successor fixture](drawing-queue-object-variants-state-20261011.json) passes
4924800unique unchanged-original consumer cells, covering direct/indexed object
markers and ordinary/-127offsets with all kinds-2..35, prior16-bit metadata,
empty/one/two rows, strict clips/480and600height branches and optional postlude.
Five raw backend results include12340001, separating high EAX bits from low AX1.
For Video-enabled indexed/-127kinds0and31, returned metadata normalizes the low
16-bit comparison to one; other defined routes retain zero/default/truncation
policies. Every original row/container, opening/auxiliary/restoration clip caller,
wave amplitude and postlude opaque-argument/reset assertion passes.

Native execution exits0without timeout. The lossless gzip archive independently
decodes to2972130848bytes with identical raw/decoded SHA-256
69f3ff36525adae3e3040e7d541a71180a8633e30fd435008405202bf92f06e5.
There are2462400postlude calls,6359680clip calls and2720000wave calls in this
fixture, not live frequencies. External backend/clip/postlude bodies remain
synthetic. Actual helper state/pixels, arbitrary object layouts, graphics lifetime
and14scenario/four dispatch closure remain pending; native recovered statuses
remain none. Earlier reports retain their original sources and narrower scopes.

The [first launch](drawing-queue-object-variants-attempts-20261011.json) failed
before compilation/original execution because declaration creation was unfinished;
the retry waited for its completed declaration. Manifest verification passes
before/after the successful run and the rejected launch. History1fe53d9..b85df54
has15file/86behavior exact receipts, no missing intermediate versions, and asserts
neither a past gate pass nor new graphics validation.


## Original Lock recovery client, 2026-10-11

The [13824cell fixture](drawing-lock-recovery-client-20261011.json) executes
unchanged58b660instructions with owned synthetic interfaces and external result
schedules. It complements existing [fill/copy scheduling](original-surface-retry.md)
and [cursor sentinel](original-surface-sentinel.md) evidence; it does not rerun
or replace those reports. Null object+8 skips calls and returns null after
clearing the global108byte descriptor. A present interface gets Lock with null
rectangle/event and flags1; returned pitch is stored at object+18.

Only exact887601c2 triggers Restore and one more Lock. The original wrapper
retains the Restore result at6f8150 and never replaces that global with the
retried Lock result.1296selected cells have differing stored/retry results.
Even failed Restore permits the retry. Successful Restore optionally reapplies
low16saved source key; key failure is ignored. Object+28 selects none, a callback
with argument0, or the -1cursor sentinel endpoint. The real sentinel/asset bodies
are outside this new fixture; their earlier independent evidence remains scoped.

Nonzero stored status triggers selected graphics teardown and DestroyWindow,
then reads final descriptor pitch/pointer and optionally calls auxiliary Unlock
through object+4 with that same pointer. Nonexclusive teardown releases the
held auxiliary interface, while exclusive teardown calls RestoreDisplayMode;
both release three other selected globals and clear the recorded state globals.
The object list is explicitly empty, so no populated-list destructor or
reentrant/null-interface callback behavior is claimed. External Release/display/
window/Unlock results are synthetic and do not prove physical retirement.

2304cells return a nonnull scripted descriptor pointer with nonzero stored
status. Mock Lock deliberately supplies controlled outputs on errors, too: this
is a client continuation observation, not a claim that real drivers supply valid
pixels after failure or that returned storage survives teardown. Driver loss,
reference counts, actual reload pixels, UI behavior, concurrency and live recovery
remain pending. Caller-effect classifications now cover14exact CALL routes in
this wrapper; list-cleanup CALL58b75aremains unclassified/unexecuted by this fixture.
No nearest function, universal COM type or full scenario ownership is inferred.

[Retained first attempt](drawing-lock-recovery-attempts-20261011.json) passed all
actual13824client cells, then failed its erroneous27648count assertion. The
corrected cardinality has a new contract/source declaration and fresh successful
execution. Original manifest and sources are stable before/after; instructions
remain unchanged. Recovered native implementation/comparison/integration/
replacement stay none. [45current offline guards](drawing-inventory-lock-recovery-guards-20261011.json)
account503routes with no unknown identities and preserve18completion obligations.
Changing the catalog preserves old live source hashes and their staleness.
Historyb85df54..d3cd04fhas11file/30behavior exact receipts and no missing versions;
retrospective accounting asserts no past gate pass or new validation.


## Reviewed exact API client declarations, 2026-10-11

The route inventory now binds141existing [reviewed client layouts](drawing-surface-api-layouts-20261011.json)
to their exact380CALL census points. A static review's build and source hash,
CALL bytes/mode, selected primary PE32 prototype/formal count, explicit receiver
provenance and route contract/classification must all agree. The review's20reload
callbacks remain excluded from these declarations. Duplicate/excluded layouts,
changed types/formals/provenance/classes, unsafe paths, stale source hashes and
unreviewed sites claiming a contract are rejected. A virtual displacement alone
still supplies no type. Empty discovered owner sets stay empty.

Known declarations describe submitted client composition, surface access, state
and lifecycle operations. They do not assign a universal runtime implementation,
reference lifetime, valid output buffer/pixels or complete semantic caller owner.
The other239sites have no reviewed client contracts; selected Lock wrapper
caller-effect labels remain separately bounded by their original-client fixture.
All support decisions retain original execution. No native graphics replacement
or recovered behavior status is promoted.

[51current native guards](drawing-inventory-reviewed-api-client-guards-20261011.json)
pass, including a strengthened duplicate declaration case that reaches its actual
rejection guard rather than an earlier review-count assertion. The preceding
51guard run and prospective hashes are retained as intermediate evidence only.
Current offline accounting has503routes,141reviewed clients,101observed API sites,
zero unknown identities and18completion obligations. This neither reruns original
code nor refreshes historical live sources. Existing layout review hashes remain
unchanged; current guards validate their interpretation and exact route binding.

Historyd3cd04f..f30b3b6has17file/45behavior exact receipts and no missing versions.
Retrospective accounting asserts no past gate pass or new graphics validation.
The branch remains isolated from the shared worktree.

## Exact core allocation bindings, 2026-10-11

The observation DLL now writes a separate `core-bindings.bin` before patching
any original entry. Its versioned 128-byte header, 96-byte rows and 64-byte
terminal preserve the actual core module bounds, named `census_leave`, entry
stubs, each prepared allocation, original prefix, relocated prefix and final
continuation bytes. The existing 128-byte call journal is unchanged. Partial
maps are refused; a complete map proves preparation, not installation.

`tools/drawing_census_bindings.py` independently reconstructs every relocation
and continuation and refuses mismatched build, tags, RET/SITE declarations,
bytes, padding, extents, overlapping allocations or duplicate stub identities.
Only a recorded relocated E8's exact physical return maps to its original CALL,
return PC and target. E9 transfers, other allocation/module addresses and the
named leave remain diagnostic; logical tail attribution still requires separate
thread/stack/flow review. Existing journals without the sidecar retain their
original limitations.

Thirteen offline mutation guards pass. A fresh synthetic PE32 fixture executes
the actual writer and decoder across nine prepared allocations, including an
E8 and an E9; the E9 acquires no CALL identity. The existing register, selected
flags, x87/SSE, LastError, 0–24-byte cleanup, same-stack tail, phase and exhaustion
checks also pass. Synthetic prefixes are not original game execution.

The original menu cohort `run-t3mn_i6s` independently closes 42,021 pairs across
84,042 events with no unassigned phase, loss, open calls or IO failure. All 83
safe prepared allocations pass exact byte reconstruction; the existing unsafe
fade remains refused. The original exits0 with stable sources, staged binaries
and metadata; both immutable-input manifest checks pass. Of 109 distinct observed
physical callers, one is an exact relocated E8 return. That identity is available
for further call-flow review; the other addresses do not acquire instrumentation
or logical owner classifications. Raw artifacts and exact staged binaries are
retained; only the stopped private installation/prefix was removed. This adds
no scenario or dispatch closure.

## Exact caller and return-continuation review, 2026-10-11

`tools/drawing_census_callers.py` independently checks a closed original core
journal and its exact binding sidecar. It preserves raw physical callers and
requires matching copied E8 bytes, original direct CALL metadata and observed
callee before classifying a relocated original CALL. Entry/return active counts
and the newest matching thread/normalized-stack frame must agree.

An observed native leave pointer can inherit a return continuation only from an
active frame on the same thread with the identical original return slot and
normalized cleanup. The report retains immediate parent and root call IDs. An
inherited continuation supplies no original CALL edge to the child; that transfer
edge remains an explicit gap. Unknown roots, differing cleanup, orphan leaves,
wrong thread/slot, neighboring module/allocation PCs and absent discovered owner
ranges remain unresolved. The raw journal is never rewritten.

Fourteen synthetic guards pass. A current-source replay of the unchanged
`run-t3mn_i6s` journal classifies 17,182 original direct pairs and eight relocated
Lock pairs among 42,021 pairs. There are no observed inherited leave continuations
in this cohort; synthetic chain admission is separate. The remaining 24,831 pairs
retain pending call-flow review. Of these, 24,825 use the raster callee `596cb8`
with five original indirect continuations; six use surface destruction `58b3e0`
with other caller continuations. The source hashes and exact retained artifacts
are recorded under new evidence IDs. Original execution was not repeated by this
replay and no semantic caller, writer, lifetime, scenario or dispatch closure is
claimed. Future core captures include the independently checked caller report.

## Raster global CALL and immediate writer census, 2026-10-11

The build-pinned CPU census scans the whole original image for exact CALLs to
`689b78`, `689b7c`, `6c4e3c` and `6903dc`, plus the exact register CALL at CRT
exit callback site `59d98a`. Each site is checked against original file-backed
bytes and retains its exact return PC/mode and discovered owner list. These are
CPU function pointers and an untyped callback site; no COM method is inferred.

The export finds 127 global CALLs: two through `689b78`, three through `689b7c`,
59 through `6c4e3c` and 63 through `6903dc`. With the CRT register CALL, there are
128 exact selected CALL sites. This extends the inventory beyond the previous
380 selected graphics API/candidate boundaries; live target admission and route
integration remain pending rather than being inferred from the old accounting
report. Twelve synthetic selection/refusal guards pass.

Eighteen exact C7 05 absolute immediate assignments are retained separately.
Nine belong to discovered initializer `484470`; nine sibling assignments have
empty discovered owners and retain that gap. Static assigned targets are:

| Global | Exact assigned target set |
| --- | --- |
| `689b78` | `596490`, `5968a4` |
| `689b7c` | `59603e`, `595b47` |
| `6c4e3c` | `59521a`, `5947b2`, `57de00` |
| `6903dc` | `596cb8`, `597086` |

These sets record immediate store bytes, not executed branch outcomes or a
complete writer/alias inventory. In particular, the existing menu's observed
callee `596cb8` and exact indirect return PCs do not by themselves prove each
before-CALL pointer value. Register, indirect, aliased and other possible writes
remain separate obligations. Raster body/pixel, lifetime and complete dispatch
claims are unchanged.

Reproduce with new output paths:

```sh
python3 tests/test-drawing-cpu-sites.py
python3 tools/export-drawing-cpu-sites.py --executable working/game-nocd/Chaos.exe \
  --output working/tests/new-cpu-sites.json
python3 tools/export-drawing-api-splices.py --executable working/game-nocd/Chaos.exe \
  --boundaries working/tests/new-cpu-sites.json --output working/tests/new-cpu-plan.json
```

Guarded planning admits all 128 CALL sites beside the 83 safe core hooks: 127
ordinary CALL splices and one single-byte breakpoint for CRT `call eax`. The
existing jump at `59d988` targets `59d98c`, so a wider patch would overwrite an
incoming continuation. The breakpoint preserves those bytes. The plan has zero
refused CPU sites; compilation, actual target capture and integration remain
pending. Both export/planning experiments verify all 2,927 immutable inputs
before and after reading the pinned binary.

## Original CPU before-CALL target observation, 2026-10-11

`--cpu-observation` prepares a separate approved 128-site CPU/global/CRT target
cohort beside the 83 safe original-active core hooks. It reuses the existing
native CALL forwarder and V2 wire with unknown formal layouts and no COM output
snapshots. The private CPU builder selects its own immutable plan; the graphics
builder's default plans remain separate. Explicit `cpu_backend_dispatch` role
validation rejects graphics/core-absent/mixed admission; twelve role guards pass.
No original body, argument or result is replaced.

Original menu cohort `run-0q3q92uc` closes 25,071 CPU CALL pairs and 42,008 core
pairs with zero loss/open/IO, unreadable stack/target or unassigned phase. Both
per-run allocation maps agree on the actual core module and named leave pointer.
Sources, prepared metadata and binaries stay stable; exit0 and both 2,927-input
manifest checks pass. The phase14 screenshot shows the original rendered Main.

Five observed menu CALL sites capture 24,825 before-target values `596cb8` from
`6903dc`. The CRT site captures 246 completed callbacks with 177 distinct targets;
those targets remain observations, not newly recovered functions or drawing
roles. Six of the 128 CPU sites are observed; 122 remain unexecuted. There is no
graphics API cohort in this experiment, and no formal CPU/COM signature, permanent
or atomic pointer binding, side-effect/pixel/lifetime or scenario/dispatch closure
is inferred. Physical core caller records remain intact; their new target-side
crosswalk still requires exact thread/stack/flow review. All raw artifacts and
staged binaries are retained; the stopped private game/prefix is removed.
