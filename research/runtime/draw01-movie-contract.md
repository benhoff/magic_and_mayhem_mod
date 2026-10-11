# DRAW-01 original movie setup, playback and cleanup

Pinned NoCD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred and reference mapping base `00400000`.

The isolated reference executes unchanged original bodies `4696e0`, `469830`
and `469a00..469a5d`. Only logger `59ca38`, three IAT entries and explicit mock
COM vtables are bound externally. The original body bytes are compared after
every case. This recovers original control flow with synthetic external results;
it does not run DirectShow codecs or establish original audiovisual output.

## Argument and state findings

The wrapper receives the path in ECX and the DirectDraw **device** in EDX.
Its first stack word is the destination DirectDraw **surface**, followed by x,
y and a control word whose low byte is sign-extended into `5fbf5c`. The earlier
HWND/EDX-surface label in `qt-native-media.md` was incorrect. Distinct mock
objects check the device passed to video stream admission and the destination
used as the Blt receiver. Confidence is high for these accesses within the
unchanged bodies; backend-internal retention remains excluded.

The wrapper writes x/y to `5fbf50/54` and control to `5fbf5c` after setup returns, before the saved setup-result flags choose playback.
It returns the inverse boolean of byte `5fbf58`; **return1 does not prove
successful stream creation or movie playback**. Setup failure leaves the
previous skip byte intact. Entering sample playback resets it to zero, including
before the later selected acquisition/state failures. Escape sets it to one.

## Selected external stages and failures

| Stage | Original external operation | Selected negative-result effect |
|---|---|---|
| 0 | Create multimedia stream, IAT `5c5318` | No owned stream, no playback; previous skip retained |
| 1 | Stream initialize, vtable `+30` | Release created stream; no later setup calls |
| 2 | Add video stream, `+3c`, device and purpose `5c92d0` | Release stream; no audio/open/playback |
| 3 | Add audio stream, `+3c`, null device, purpose `5c92e0`, flags1 | Release stream; no open/playback |
| 4 | Open converted path, `+40` | Release stream; no playback |
| 5 | Get video-purpose media stream, `+10` | Playback skip already reset; wrapper releases stream |
| 6 | Query video interface, `+0`, IID `5c9270` | Release acquired media then wrapper stream |
| 7 | Create sample, video `+34` | Release media/video then wrapper stream |
| 8 | Get source surface and rectangle, sample `+20` | Release media/video/sample then wrapper stream |
| 9 | Set stream state1, `+1c` | Release media/video/sample/source then wrapper stream |

Successful setup AddRefs the returned stream and releases the local reference.
Selected playback cleanup releases acquired media, video, sample and source in
that order; the wrapper releases its stream afterward. These are explicit
original Release calls and balanced mock references, not proof of backend
internal ownership or a native lifecycle policy.

The loop polls Escape through IAT `5c5204`, then calls sample Update `+18` with
zero flags/event/callback/data. Only result0 continues to drawing. Both selected
E_FAIL and positive1 terminate sampling. Control0 calls destination Blt `+14`
with flags `01000000`; selected nonzero controls consume samples without this
Blt. The selected Blt E_FAIL is ignored by the wrapper result policy. Source
rectangle is unchanged; destination coordinates add x/y with 32-bit wraparound.

## Reproduction and limits

```sh
python3 tools/test-drawing-movie.py --executable <pinned-NoCD-Chaos.exe> \
  --claims <prospective-coverage-claims.json>
```

The selected 1,760-case matrix crosses success/ten negative HRESULT stages,
prior skip0/1, controls0/1/-1/127/-128, Escape0/1, Update and Blt failure0/1,
and ordinary/wrapping positions. It checks exact external arguments, stage
short-circuit, wrapper/global results and write ordering, placement, reference balance, release
order and unchanged original instructions. Per-case JSON and compiler/execution
logs are retained, including failed exploratory assertions in
`working/tests/drawing-movie-prototype/`.

ASCII path conversion is a synthetic external success fixture. Actual Windows
conversion failures, positive HRESULT setup variants, noncanonical COM outputs,
exceptions, arbitrary control values, codecs, internal retained references,
concurrent surfaces, actual pixels/audio and complete live playback/recovery
remain pending. The original live startup wrapper's fast return is a separate
observation. Native Qt decoding and replacement evidence must remain separate.


The [current isolated-original result](drawing-movie-scoped-original-20261010.json)
binds the headless scenario prospectively. Earlier retained reference reports
used an incorrect original-comparison scenario label; their claims stay
historical rather than being rewritten. Fresh execution with the corrected
classification supersedes those scope bindings. No native movie implementation
comparison/integration/replacement status is promoted.

The headless scenario also has a separate `TOOL.drawing-movie.fixture-bindings`
evidence ID for the native host's mapped-image/mock-callback test infrastructure.
It shares the executed report, validates those bindings only, and does not
promote native movie engine integration. Original findings use the distinct
isolated-original evidence definition.


## Original instruction probes and early end of stream

Fourteen declared SITE probes use the existing metadata ABI without a function
return-slot substitution. Each requires an explicitly named containing original
function; signature bytes and all retained direct incoming control-flow targets
are checked against the whole pinned image. Unknown modes, missing/outside/boolean
anchors and a legacy interior mislabeled as a function entry are refused.
Indirect entry into a splice remains unresolved. Relocated CALL instructions
have an observer-trampoline return PC; unperturbed timing and arbitrary
return-PC-sensitive external behavior are not asserted.

| Probe addresses | Before-site interpretation |
|---|---|
| `469721`, `46974a`, `469777`, `4697a2`, `4697e0` | Selected setup HRESULT failure before the original log CALL |
| `469872`, `46989e`, `4698c9`, `4698f6`, `469917` | Selected sample acquisition/state HRESULT failure before logging |
| `469962` | Sample loop before Escape poll; EAX is general diagnostic state |
| `469981` | Update returned0; before reading original control global |
| `46998a` | Control0 selected original destination Blt dispatch |
| `4699b1` | Cleanup entry; EAX is last Update on ordinary loop termination, but needs branch context on other exits |

The first retained83-hook run `run-13j4m4__` has82,328events,41,162paired
function calls and4SITE events, zero loss/open/IO and normal shutdown. Each intro
hits the loop once, then cleanup with EAX`00040003`. Neither successful-Update,
Blt-dispatch nor the ten selected error-log sites fires. This indicates that
both initial sample Updates ended without producing a sample in this observed
Wine path; it does not explain the immediate termination or establish audio
absence, codec failure, all platforms or complete movie playback.

Microsoft defines `00040003` as `MS_S_ENDOFSTREAM`, indicating an ended stream
with no updated sample. See [multimedia streaming codes](https://learn.microsoft.com/en-us/windows/win32/directshow/multimedia-streaming-error-and-success-codes)
and [IStreamSample::Update](https://learn.microsoft.com/en-us/previous-versions/ms786825(v=vs.85)).
The original loop tests result0, so this positive end-of-stream result exits.
The inferred API meaning is separate from the measured register value.

The decoder retains the wire `calls` identity counter (ENTER plus SITE), and
adds `paired_calls`/`site_events` separately. `site_eax_counts` are before-site
register values, not function return values or generally HRESULTs. Corrupt
pairing, invented SITE callers, loss/open/IO, phases and unknown identities
still refuse a valid journal. All fourteen scenario closures remain open.


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
