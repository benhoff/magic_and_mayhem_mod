# DRAW-02 integration into current master

The reviewed merge combines DRAW-02 f305809d with master76a75c0c, preserving
both histories and all existing evidence. Master advanced from b30ff77f during
integration; the two intervening native-pixel/RTS accounting commits were included
before final validation. Incoming history review matches1812 exact file versions
across74 commits to receipts, plus514 versions across16 target-history commits
and214 versions in the two newer target commits, with no missing receipts.
This retrospective hash review does not assert old gate passes or new validation.

Resolution keeps current menu V22, multiplayer/campaign features, audio/music,
GUI rendering worker and hard restart together with DRAW-02 producer v4/v5,
semantic/HUD/latch observers, source/cursor diagnostics and observer shutdown
admission fixes. The producer merge preserves native glyph resident entry/return
publication, checked constants and installed state while retaining diagnostic-only
extra hook admission and v5 quiet-close capture. These retain separate scopes.
Results intents use the fresh-read/explicit-pre-callback rejection policy; active
media pauses suppress new actions/publication/deadline checks and rebase that
intent's deadline along with the existing request/lease clocks on resume.

Final [synthetic02](draw02-master-results-synthetic-02-20261015.json) passes14
Qt/session mapped-peer readiness/identity/duplicate/once-only/inactive/media-pause
cases. Six focused Qt checks pass, including hard restart and normal/HiDPI input.
The three native scene/session/replay-runner regressions pass. The bounded
original Results bytecode fixture passes20 callback cases with ABI, error/thread,
ownership, stale and once-only guards; raw run-pzri5wlp remains pinned. Common
engine services are stubbed there, so it is separate from the actual app journey.

Final [app04](draw02-master-merged-app-04-20261015.json) passes Qt Main → Quick →
setup/map2 → Spells → World → Results Continue → World → Results Quit → Quick →
Main → original normal Quit. It observes two stale rejections, retains their
intents and dispatches each original Continue/Quit exactly once. Final ACK12
corresponds to ten exact original callbacks on one thread. There are415 native
command frames/322 real paints, zero native fallback/recovery/error, Qt-forwarded
Right/focus-release/resize and retired menu/input channels. Native audio/music
stay on; intro movies are explicitly skipped to bound this menu/World journey.
The600ms Escape interval is a scheduled release; normal focus loss may clear the
mapped key earlier. Binary/source hashes stay stable and all2927 originals verify
before/after. Raw run-_jry9jfp and experiment run-yd65brjo remain pinned.

App01 lacks prepared music assets; App02 lacks the isolated installation
manifest; both stop before a successful game journey. App03 reaches original
intro playback and times out before Main. Its false original-executed flag is
menu-event-derived and cannot establish absence of original execution. None of
these retained failures supports a passing claim. Synthetic01 is historical after
the later master integration and explicit intro-skip harness change.

The active main checkout's preexisting game-nocd executable has SHA2561060a739,
while the supported clean No-CD source/installation manifest expects40209ca7.
Integration uses a separate copied, verified40209ca7 installation and leaves that
existing modified game copy intact. Source game selection is recorded in staging
metadata; no original artifact is changed or historical hash refreshed.

Earlier323-frame source-owned offline replay and observer proofs remain their
recorded results. Shared sources/build metadata can make them historical after
integration; this merge does not assert fresh whole-frame equivalence, hardware
coverage, arbitrary transitions, universal teardown or complete DRAW-02 takeover.
Fresh native Qt readiness evidence has an explicit current app/runtime source
closure, including compiler dependencies and staged adapter/backend imports.
