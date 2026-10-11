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
