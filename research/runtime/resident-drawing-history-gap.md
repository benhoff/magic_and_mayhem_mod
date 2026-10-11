# Resident drawing-history contention investigation

2026-10-11. This investigates native scheduling policy, not recovered driver
ordering or original pixel equivalence. Historical reports and hashes remain
unchanged. The visible command refusal `reason 2` means GAP, independently of
the producer's local `session_gap` subreason.

## Confirmed failure chain

The retained stage-profile capture failures `run-5cef8x_c` and `run-u4m1wj53`
record an ordered copy timeout against `bridge.c:307`, the Unlock wrapper in
their declared source version. The observed holder elapsed times are559ms and
604/669ms; the native admission budget is50ms. Timeout forwards the original
copy without a captured transaction, increments `game_lock_epoch`, and marks
target uncertainty before/after that original. `game_session_sync()` then emits
local `session_gap(1)` and command-channel GAP2. This is intentional conservative
refusal after incomplete history, not a GPU pixel mismatch or archive exhaustion.
Clock resolution and separate atomic diagnostic reads limit exact timing claims.

Fresh phase instrumentation narrows Unlock ownership to snapshot preparation
(20..24), successful publication/cleanup (30..33), resident UI delta scan/packing
(40..43), and tracker release (44..48). Only the owning thread may change its
phase; external World binding must not overwrite another callback's marker.
Phase33 precedes final snapshot cleanup and tracker release. Phase45 covers the
bounded diagnostic call; phase47 covers inline command-channel pumping after
tracker release. All markers are local diagnostic integers, with no wire change.
The50ms ordering deadline, original results/LastError, byte/ownership budgets and
conservative refusal remain intact in this diagnostic milestone.

## Fresh negative observations

* [Combat/retreat negative](resident-history-combat-retreat-negative-20261011.json):
  2,880 balanced completed/imported/original-returned Worlds and no recorded
  copy timeout/GAP; failed the post-combat retreat waypoint. This is not a
  completed campaign or a GAP fix.
* [Unlock cleanup GAP](resident-history-unlock-cleanup-gap-negative-20261011.json):
  longer camera/menu capture fails at576 balanced Worlds. Unlock owner phase33
  is recorded with325ms holder age and306ms waiter age, then local epoch GAP1.
  Cleanup, diagnostics and inline pumping are not yet distinguished by this
  result; recording phase33 does not prove a heap or mapped-I/O cause.
* [World bind tracker refusal](resident-history-bind-tracker-refusal-negative-20261011.json):
  expanded markers reach651 balanced Worlds then the resident producer cancels.
  `RenderResidentWorldV1` requests the tracker while another thread holds
  `game_blit_after`; the8ms tracker budget expires at observed16ms. The bind
  export currently skips the ordered-copy admission domain used by callbacks.
  This is a separate confirmed admission failure, not a copy-timeout GAP.

All three runs use private NVIDIA/Zink TITAN rendering, retain failed journeys,
stop their owned Wine sessions and verify immutable originals afterwards. They
do not claim uninstrumented cadence, whole-frame comparison or completed Quit.

## Next bounded changes

Serialize public resident binding with the existing callback/copy admission
domain before acquiring the tracker. Prove that a short admitted callback hold
longer than8ms lets binding succeed without losing history, while same-thread,
timeout, shutdown and invalid-input guards retain explicit refusal.

Investigate inline transport publication inside ordered callbacks. The existing
publication worker already owns retry/drain capability; a source-backed
regression should distinguish drawing callbacks from worker pumping and preserve
FIFO bytes, full frames and shutdown ACKs. Phase33 alone does not identify the
slow suboperation. Mapped page stalls, heap stalls, diagnostics, arbitrary owner
preemption and other tracker users remain explicit hypotheses/boundaries.
