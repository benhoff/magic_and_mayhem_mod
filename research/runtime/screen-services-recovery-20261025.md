# Screen selection, cursor selection and control dispatch recovery

This batch separates the three services that the earlier shared-tick fixture
substituted. The No-CD executable is pinned to
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The recovered models are in `reconstruction/rendering/screen_services.hpp`;
`tools/test-screen-services.py` checks local control flow, compares complete
unchanged original bodies and retains results through the SQLite report writer.
These are isolated comparisons, not a game integration or replacement.

## Selection at 5571e0

The complete local extent is 5571e0..557368. Signed depth <= 0 warns and returns.
Otherwise the helper snapshots 6e1efc into 6e1ef8, calls current slot 4, rereads
depth and visits stack entries from depth-1 down through 1. Each nonmatching
entry receives slot 4. A match publishes depth/current, clears saved cursor
background validity through the unchanged 4a4370, then calls slot 0c.

A significant recovered quirk is that ordinary exhaustion reaches index zero
and returns: an absent target is not pushed. Only a negative index, reachable
when the current callback changes depth to zero, enters the fallback. That
branch invalidates the cursor, checks depth >= 15, suspends nullable current via
slot 8, snapshots the token again, pushes the target, drains the two message
ranges, invalidates the cursor again and invokes current slot 0. Stack entries
above the new depth are not cleared. The routine itself neither allocates nor
frees screens. Calling slot 4 does not by itself prove destruction or retirement.

## Cursor selector at 4a39a0

The earlier name “special service” did not identify its actual role. Complete
4a39a0..4a3c17 selects a cursor class. Equal, negative and >=23 requests return.
With custom cursor mode enabled, a null mutex or nonzero 10ms wait result leaves
the selection unchanged. WAIT_ABANDONED is rejected along with timeout/failure.
A successful wait publishes the class. Class 22 skips frame changes; other
classes clear the frame counter, read a 68-byte-stride descriptor and derive a
40-pixel rectangle using signed division/remainder. Release rereads the mutex.
Division faults are outside the admitted model domain, not fallback behavior.

Without custom mode, the class is published before LoadCursorA/SetCursor. The
20-entry jump table is completely resolved; classes 20..22 use resource 0x9d.
Resource IDs for 0..19 are 6c,68,a0,6a,6d,6b,92,93,94,95,97,98,9a,99,9b,9c,
9d,9e,9f,a1. A null LoadCursorA result still reaches SetCursor. OS resource
ownership, actual cursor artwork and real mutex concurrency remain separate.

## Control dispatcher at 4a5070

The complete local extent is 4a5070..4a5842. It consumes ten-DWORD events from a
32-entry ring, advancing the reader before callbacks. Idle hover and ordinary
selection scan controls in reverse order using signed half-open rectangles,
nonzero enabled fields and slot-4 hit callbacks where zero accepts the target.
Capture can retain an enabled target outside its rectangle. An invalid capture
is cleared before selection. Once set, an invocation-local capture latch persists
across subsequent ring entries even after a button release clears global capture;
it is not a fresh per-event boolean. The previous target gets leave flag 0x40 before
the new action. Enter/motion and left/right button flags are assembled in the
original order; capture writes and final registry-count checks reread storage.
The dispatcher calls the real recovered cursor selector, not a cursor recorder.

The inline World branch updates the clamped selection rectangle, enters drag
mode beyond a signed 32-pixel extent, and orchestrates existing selection/unit
helpers. Those helpers remain authored boundaries in this comparison, with
bounded iterator results and checked argument ABIs. This batch does not recover
unit lookup, activation, orders, notification internals or gameplay effects.
Control action and hit callback internals remain separately owned contracts.

## Vtables and lifetime boundaries

A data pointer matching shared tick 5595d0 identifies a candidate table only.
The execution report retains every aligned non-executable-section candidate
with its 14 slots and marks its ownership unrecovered. It does not infer a
dynamic type or an owning parent from those pointers.

The real candidate at 5c6564 has first four callbacks 4b9b90,4b9ba0,4b9bb0,
4b9bc0. The first clears owner byte 12 and the other three return. The fixture
executes those original callbacks unchanged for selected selection cases;
these particular endpoints do not free or retain objects. The fixture owns
storage throughout the call and does not supply constructor/destructor or
whole-session lifetime proof. Other screen classes must be recovered separately.

## Evidence and remaining work

Every comparison attempt, including failures, is retained under
`screen-services-results-20261025/` as SQLite references. Inputs are manifest
verified before and after execution. Recovered code bodies remain byte-identical;
only explicitly external world helpers, platform IAT entries and authored
callback tables are adapted in the private mapping. No original file is modified.

Confidence is high for the decoded local semantics within each stated domain.
Actual callback classes, their owning constructors/destructors and dynamic
registration/retirement are still recovery work. Arbitrary callback interleavings,
invalid pointers, nonterminating queues, asynchronous retirement and live game
replacement are not validated by these isolated fixtures.

The final [comparison](screen-services-results-20261025/attempt08.json) passes 192 selection, 9984 cursor, 18432 control and 110 tick-transition cases in optimized i386, i386 ASan/UBSan and native64 ASan/UBSan builds, with identical checksums. Earlier results retain compiler temporary quota failure, dependency/fixture admission errors, the initial capture-latch discrepancy, the interrupted slow sanitizer run and the initial null tick receiver. No failed result is treated as validation.

The refreshed [coverage summary](coverage/summary-screen-services-20261025.md) and [audit](screen-services-results-20261025/audit01.json) retain the repository-wide historical staleness and unknown dispatch backlog. They do not turn those historical results into current-code comparisons.
