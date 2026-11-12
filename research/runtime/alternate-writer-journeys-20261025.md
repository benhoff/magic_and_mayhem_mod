# Alternate pointer watches through campaign and graphics rebuilds

This extends the [complete Quick Battle observations](alternate-ownership-sessions-20261024.md)
using only the supplied original NoCD image and the admitted Wine environment.
It investigates computed and transient writes directly: hardware watchpoints
on both DWORD slots catch executed writes without knowing their address formula.
No native allocator, renderer, or original pointer publication is installed.

## Observed domain

The original executable remains SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The runner stages it from immutable input, verifies every original stored
section before launch, and adds only the established menu-observer import.
Original manifests pass before/after each attempt. Installation assets are
copied to an owned output; source asset/provider hashes are retained and checked.
The original Save directory must exist in that isolated installation.

`tools/test-alternate-ownership-session.py --journey campaign-rebuild` runs a
fresh Apprentice campaign, Save/resume, Mini Load/resume, original Quit/defeat/
Main return, an original resolution change and saved-map Main Load, another
Quit/Main return, the reverse resolution change and Main Load, then final
Quit/defeat/Main and Main Quit. Both initial modes and both values of
`DEBUG/UseMallocedBackBuffer` define the prospective matrix. Explicit
`--debug-choice` and `--initial-video` filters select subsets without changing
the default repeated-Quick-Battle path. Each of five World-dispatch
checkpoints requires80additional entries; original child exit0, observed
graphics shutdown and no bounded-stop flag are required for normal completion.
The saved file must remain byte-identical through all three Loads.

V13 menu commands use their existing versioned wire, generation and availability
guards. New Save names belong only to the owned game copy; Load rows must match
that exact new name once. Original callbacks perform serialization, display
rebuild and Quit. Quit Yes is a physical click after an observed original
confirmation, rather than a host-authored answer or patched callback.

All reported created process threads receive the existing two pointer write
watches plus dispatcher and shutdown execution watches before resumption.
The authored main/worker positive control must catch eight transient writes,
four per slot, and its two dispatcher/two shutdown visits. It is instrumentation
validation, not a natural original publication. The two original slots are
`6e1f68` and `6e1f6c`; padding/configuration remain separate globals.

## Private input and diagnostic recordV3

The original runs windowed on the admitted1024x76816-bit private Wine desktop.
Its game client is a Win32 window inside that desktop and need not be a named
X11 client. Diagnostic recordV3 retains the fixed256-byte header,64-byte rows
and256-byte error-text tail, and defines these additional header words:

| Words | Observation |
| --- | --- |
|39 | Original caption HWND, accepted only when its PID matches the observed child |
|40–43 | Signed client x/y, unsigned width/height |
|44 | Geometry sample count |
|45–48 | Signed outer window rectangle |
|49–50 | Foreground HWND and its owning PID |

`GetClientRect`, `ClientToScreen`, `GetWindowRect`, `GetForegroundWindow` and
`GetWindowThreadProcessId` query USER32 only. Handles are launch-specific scalar
identities in a versioned diagnostic record, not shared host pointers. No new
target-memory write API is introduced. All three existing record consumers
retain V1/V2 compatibility and reject unknown versions.

The driver uses the witnessed title bar to activate the game, then moves the
pointer into the client without a scene click. Escape and Quit confirmation
clicks use XTest on the authenticated private display. A Hermes card authorizes
a dialogue click only when the existing complete-title-line OCR rule matches.
Unrecognized captures authorize no click. Screenshots use ImageMagick's
16-bit-capable XGetImage route; capture and OCR pump the original two-second
host lease rather than relaxing it. A dedicated timer services that same lease
across synchronous desktop queries; one lock serializes heartbeat and command
seqlock writes, and the timer joins before the mapping closes. Mini readiness is checked again after
capture because opening Mini pauses World dispatch.

Sixteen controls cover bounded/duplicate/missing Save rows, engine seqlock
changes, all three readers' V1/V2/V3 behavior, unknown diagnostic versions,
heartbeat supervision and timed-out child cleanup, Mini becoming ready during
capture, an already-open Mini, retired-menu input refusal, and heartbeat progress
while the main host blocks followed by a joined stop.

A short third Hermes card exposes background inside the broad crop; OCR can
append punctuation to its title and correctly fail the exact-line rule. A
second bounded title crop `(380,70,510,150)` in normalized800x600 coordinates
uses that same rule without punctuation normalization. Both OCR observations
are retained. A recognized card receives a physical click at `(450,155)`,
inside the witnessed cards, instead of the earlier scene-center point.

## Retained refusals

campaign01 reaches original Save but its owned asset copy lacks an empty Save
directory. The original displays “Unable to save file”; the diagnostic image
and bounded cleanup are retained. This is fixture preparation, not a game-code
repair or successful ownership journey.

campaign02 completes Save but the initial host locator expects a separately
named X11 game window and refuses. RecordV3 instead queries the child-owned
Win32 rectangle. campaign03 recognizes two original Hermes cards but capture/
OCR lets the menu host lease expire; its permanent retirement is preserved.
The heartbeat-aware command runner fixes that host error without changing the
lease or original game.

campaign04 completes Save/Mini Load and enters the saved map after a display
rebuild, then fails its required World-dispatch prefix. campaign05 retains an
overly eager unrecognized-capture refusal after the two recognized Hermes cards;
the image shows ordinary World. Neither counts as normal completion.

campaign06 reaches original Mini after both display rebuilds, but the host waits
for more World dispatch while Mini has paused it. The later input helper checks
Mini readiness after OCR and before another ingress attempt. Concurrent source
archival also locks its evidence database during final source checking, so the
producer cannot emit its final report. Raw debugger/menu records, images,
compiler outputs and before/after manifests remain retained. Any reconstructed
negative receipt is explicitly distinguished from an original producer report
or a retained prospective declaration. No normal-Quit or current-source result
is inferred from that failed finalization. Source archival and producers are
kept serial afterwards.

campaign07 completes Save/Mini Load and the first resolution-change/Main Load,
then the original bridge permanently retires its host lease. It has no normal
Quit proof. campaign08 keeps the timer-backed lease active but cannot recognize
the short third Hermes card in its broad crop, and refuses further scene input.
The six retained screenshots show why repeated Escape does not open Mini.
Offline title-crop checks distinguish these cards from the captured ordinary
World images; neither failed attempt is counted as a completed cohort.

campaign09 completes the low-resolution/debug-disabled cohort with1,605 World
entries, both rebuilds and normal original Quit, but its next enabled process
does not expose Main before the bounded host timeout. Its zeroed menu lane is
not attributed retrospectively to a particular modal.

campaign10 starts with the low-resolution/debug-enabled configuration. Main's
New Game action is acknowledged, but original presentation displays the child-
owned “DirectDraw Error / Rectangle provided was invalid.” before any World
entry. The retained screenshot confirms the modal; observed debug/padding is
`1/96/0`, offscreen pitch1,472 bytes, both pointer slots zero. The driver neither
dismisses the error nor changes the rectangle. This is a distinct original
campaign-entry refusal; the existing enabled Quick Battle pass does not cover
it. The exact failing API, operands and upstream ownership are not inferred
from modal text or pitch alone. The subsequent full-journey run explicitly
selects debug-disabled cohorts; enabled campaign continuation stays pending.

## Remaining recovery boundary

Executed pointer writes are observable across these specific original journeys,
including register-relative, bulk and transient writes on reported threads.
This is stronger within that domain than a search for literal stores, but does
not prove whole-program absence or dormancy. Multiplayer, later campaign maps,
unobserved configuration/error paths, unreported/external writers and unresolved
indirect flows remain separate. Original alternate allocation, initial backing
pixels, base/source relationship and retirement ownership require an actual
publication chain or a sufficient alias/reachability proof.

The existing native owned alternate generation and its validated resident route
remain intentional native policy. This investigation changes observation and
test navigation, not native rendering, original pointer values, or gameplay
balance.

## Final bounded observations

| Initial mode | Debug flag | World entries | Result |
| --- | --- | ---: | --- |
| 640x480 | disabled | 1410 | Save/three Loads,two rebuilds,normal Quit |
| 800x600 | disabled | 1428 | Save/three Loads,two rebuilds,normal Quit |
| 640x480 | enabled | 0 | Original invalid-rectangle modal;bounded cleanup |
| 800x600 | enabled | 0 | Original invalid-rectangle modal;bounded cleanup |

[campaign14](alternate-writer-results-20261025/campaign14.json) retains two complete
processes and2,838 dispatcher entries. Each process observes both original graphics
rebuilds and preserves its original Save bytes across three Loads. Both pointer
slots remain zero and no hardware write event occurs on any reported created
thread. The main/worker positive control catches all eight transient writes.
These are observations of this original Wine domain, not an allocator, global
dormancy proof, original pixel comparison, native integration or replacement.

[campaign15](alternate-writer-results-20261025/campaign15.json) and
[campaign16](alternate-writer-results-20261025/campaign16.json) retain the enabled
entry refusals with their prospective source/contract bindings, modal captures,
padding96/0 and observed offscreen pitches1,472/1,792. The precise failing API
and operands remain unknown; no error is dismissed to continue.

campaign11 completes its low-mode cohort but the second process displays a
CD-ROM driver error before exposing Main. The persisted plaintext preferences
contain `CDMusicEnabled=TRUE`; the original preferences round trip re-enabled
that fixture option. The driver now reapplies the established no-CD-music and
skip-movies fixture settings before every fresh process. campaign14 then passes
both cohorts. This is owned fixture configuration, not a game-code repair.

The90GiB storage guard interrupts RAM-staged campaign12/13. Their raw records
are retained and distinguished from complete producer reports. The low-mode
process in12 reaches raw normal exit, but lost prospective declarations are
not reconstructed or promoted into acceptance. Original manifests are verified
again after stopping each owned orphaned server. Other Wine namespaces and
unrelated work are preserved. The subsequent user authorization permits a
temporary88GiB guard for this completion batch only; repository policy remains90.

[controls](alternate-writer-results-20261025/controls01.json) passes16 unit
controls and15 retained-image exact-title checks. [retention](alternate-writer-results-20261025/retention01.json)
pins intermediate sources, saves/configuration, raw records, screenshots and
compiler artifacts, with verified archive readback in SQLite. Baseline assets
remain at the pinned input installation; no recent output or pin is deleted.
RAM staging is temporary and its unique outputs are preserved durably.

The shared V3 reader/source edits leave older API-rectangle evidence stale.
Its hashes and status remain historical; this batch does not renew those API
comparisons or the separate native resident alternate-owner validation.

The [coverage audit](alternate-writer-results-20261025/audit01.json) reports
zero accounting errors. Current ACO5 execution bindings remain exact; earlier
shared-source evidence retains its recorded hashes and freshness limitations.
