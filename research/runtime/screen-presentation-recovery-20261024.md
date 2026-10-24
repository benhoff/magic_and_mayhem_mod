# Shared screen presentation recovery — 2026-10-24

## Result

The complete local caller at5595d0..55981e is recovered and compared against
an independent native model. The original `-win` configuration also provides a
bounded way past the enabled-setting presentation error on this Wine host.
No crop,copy suppression,pointer publisher,synthetic HRESULT or executable
instruction patch is used. Original alternate ownership remains unresolved.

## Complete local caller

The closed590-byte control-flow graph has no unresolved internal branch,overlap
or unassigned local byte. All direct and indirect calls remain explicit external
service boundaries. Complete original-body execution and the independent model
agree in96000 admitted cases,including all owned object bytes,screen-stack and
selected global state,callback order and state observed at each callback.
Optimized i386,i386 ASan/UBSan and independent native64 sanitized executions agree.
The original caller bytes remain unchanged throughout the comparison. Vtable
pointers and authored screen handles in request/stack fields are normalized to
stable fixture IDs;all other owned object bytes and selected globals compare
directly. The opaque6e1efc/6e1ef8 token names carry no inferred window semantics.

- Push at owner+33 has priority when nonzero and fade byte+0c is zero. Depth>=15
  issues the original warning and returns. Otherwise the optional current slot8
  precedes depth increment,stack publication,opaque6e1efc→6e1ef8 snapshot,two
  finite PeekMessage drains,4a4370 and the new screen's slot0.
- Selection owner+37 is next,again gated by a zero fade byte;5571e0 is called
  and the function returns.
- Pop owner+43 is next. Depth<=0 warns;otherwise the current slot4 precedes
  decrement,opaque token snapshot,stack restore,4a4370 and nullable slot0c.
  These returns precede presentation.
- Ordinary redraw requires owner+8 nonzero. Screen class22 invokes4a39a0 with
  ECX16 when not fading;the fade byte is reread after this callback.
- A nonzero fade byte and counter+0d selects58ed80. The counter is reread after
  that callback,incremented with DWORD wrap,and fade clears when the old signed
  counter>=5. Otherwise slot28 draws and its counter is incremented afterward.
- Owner+12 suppresses the presentation block. Otherwise642c60 becomes0,the
  foreground helper4a45b0 runs when the current fade byte is zero,58bf40 copies
  source6a49e0 to destination6a4fe0 at0,0,4a4380 runs,and642c60 becomes1.
  Slot2c and4a5070 follow regardless of presentation suppression.

Tests admit finite message queues,owned positive stack indexes and valid required
screens. They include nullable current/back screens,fade/count signed/wrap
boundaries and named authored special/draw/fade callback mutations. Arbitrary
reentrancy,negative-depth push indexing,invalid pointers,nonterminating queues,
external helper semantics and screen lifetime remain outside this contract.
The returned CPU register value is not interpreted as a semantic return value.

The original global function inventory remains immutable. The execution report
re-exports this focused caller's exact ranges/hash/CFG,cross-checks its overlap
against the baseline and retains the original unassigned ranges. A separate
execution-backed `screen-tick-complete-5595d0` range exception covers590bytes;
the historical eight-byte entry exception is preserved. This does not declare
any external computed targets complete or recover every menu behavior.

## Original configuration route

The pinned executable is NoCD SHA256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The caller has no direct alternate-setting or video-mode branch. Those choices
arrive through upstream surface dimensions and the copy wrapper's route state.

Command parser4e8480 compares token`win` at5e3f64,using delimiters`/- `,
and sets6a2920 at4e85be. Initializer4e40d0 selects58aa90 when that flag is nonzero
and58a9b0 otherwise. Fullscreen setup explicitly sets6f68d8 to1 at58aa6f;
window setup stores0 at58ac02. These are pinned static byte/branch witnesses,
not a claim to have reconstructed every parser or device-setup branch.

With a private1024x768 16-bit desktop,the observed original window mode uses
Surface2 Blt at58c03a with WAIT0x1000000,original client-relative destination
rectangle and complete padded source rectangle. Fullscreen uses original
BltFast58c05a with flags0x10. Both modes retain the original foreground/restore
calls and shared screen caller5597fd;the debugger only reads child memory and
updates hardware debug registers/resume flag.

| Mode | Video | Padded setting | Source rectangle | Original result | World prefix |
|---|---|---|---|---|---|
| Windowed | 640x480 | enabled | (0,0,736,480) | success | observed |
| Windowed | 800x600 | enabled | (0,0,896,600) | success | observed |
| Windowed | both | disabled | exact video extent | success | observed |
| Fullscreen | both | enabled | video width+96 | invalid rectangle | blocked |
| Fullscreen | both | disabled | exact video extent | success | observed |

Windowed primary metadata is1024x768. In the initial admitted640x480 case,the
complete destination rectangle is(192,157,928,637);in800x600 it is
(112,97,1008,697). The live runner verifies bounds using each session's actual
rectangles and primary metadata,not those example coordinates. Successful Blt
and World dispatch do not establish visible padding pixels,whole-frame pixel
correctness,clipper ownership,format portability or performance.

All reported child threads retain hardware watches on6e1f68/6e1f6c;mandatory
main/worker controls catch eight transient authored stores. No natural writes
or alternate-pointer dispatch are observed in either configuration. The reached
World path is the ordinary path with the original padded offscreen surface.
This is new configuration reachability evidence,not recovered alternate allocation.
The menu adapter and bounded termination remain explicit;normal Quit is untested.
No production launcher,default configuration or application integration changes.

## Reproduction and evidence

Use `tools/test-screen-tick.py` with new `--output` and `--report` paths for the
isolated original/native comparison. Use `tools/test-screen-presentation-live.py`
with `--trace-blt --enabled-first --desktop-depth 16`,adding `--windowed` for the
original`-win` route,and the existing owned source-game/idle-prefix arguments.
The installation supplies assets only;the executable is restaged from the
immutable original with the established menu import. Source/installation/provider
fingerprints and original manifests are verified before/after each experiment.

Final records: [caller comparison](screen-presentation-results-20261024/control-final02.json),
[windowed observations](screen-presentation-results-20261024/window02.json),
[matching fullscreen controls](screen-presentation-results-20261024/fullscreen01.json).
Earlier failures are retained:control01 compiler indentation diagnostic,
control02 sanitizer stack-alignment failure corrected by the PE32 adapter's
`-mstackrealign`,and window01's completed cohorts followed by a rejected final
input check due to a runner variable shadowing the source-installation path.
Intermediate passing control03/control-final retain their original fingerprints.

To retain the90GiB main-drive reserve,large temporary installations and review
workspaces run in private RAM mounts. Unique raw outputs are retained in
`working/coverage/screen-presentation-raw.sqlite3`;identical assets reference the
already SHA-verified private-backed ACD1 artifact archive
`bc7d6ddfaddfa74832b574f017875c6ee250a034bcd24f269214fe5a1879c414`.
Each raw artifact records size,hash and storage location. Compressed SQLite review
snapshots are restored and integrity-checked before commit and private-backup
acceptance. This storage method supplies no additional engine validation.
