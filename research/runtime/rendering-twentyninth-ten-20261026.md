# Ten owned native sprite timeline components

These are ten small component implementations within the partial native TAG,
sprite-resource and animation-presentation work. The previous native session
supports a contiguous name-occurrence range and uniform frame ticks. This batch
adds a separate variable-timed service; it does not change that session, decode
original ANI opcodes, or reinterpret original timing/events. Widget presentation,
recovered animation players and native drawing remain independent suppliers.

| `NP.sprite-timeline.` component | Native implementation |
| --- | --- |
| explicit-clip | Owned noncontiguous name/occurrence entries, resolved before publication |
| variable-loop | Repeating explicit per-entry durations |
| variable-once | Saturating durations with the last frame visible at completion |
| ping-pong | Forward/back traversal with each endpoint held once per cycle |
| reverse | Reversed loop/one-shot/ping-pong itineraries |
| pause | Paused host time is discarded; seeking/rate edits remain explicit |
| seek | Virtual-tick and schedule-entry/local-tick positioning without events |
| rational-rate | Positive rational host-to-virtual ticks with retained fractional carry |
| bounded-events | Ordered entry notifications with complete preflight and rollback on overflow |
| checkpoint-reload | Owned validated state and exact-name table rebinding preserving phase |

`TimelineClip` owns1..4096 selectors, each with1..1e9 ticks and an arbitrary
signed32 event value. Names retain all eight bytes and occurrences use the
existing strict native `SpriteTable`. Repeats and noncontiguous frames are valid;
an unknown name/occurrence, null table or invalid duration refuses construction.
The immutable clip holds the table and resolved frame indices. Reload first
resolves the complete clip against the new table, then publishes it atomically.
Old plans and checkpoints retain their original sources.

For entries A,B,C,D, forward ping-pong is A,B,C,D,C,B; reverse is D,C,B,A,B,C.
One/two-entry clips are respectively A and A,B, reversed when requested. Each
visit retains that entry's full duration. The maximum cycle is8190e9 ticks.
Loop phases are strictly below the cycle; once phases may equal their duration,
with the final frame visible and its local tick equal to its exhausted duration.

Rates have numerator and denominator1..1e6. A rate edit clears the old fractional
remainder. Host deltas may reach UINT64_MAX: production reduces or caps the
quotient before multiplying, avoiding overflow and unbounded iteration. Seeking
uses virtual time, ignores rate/pause, emits no events and clears the remainder.
Entry seeks name a playback schedule position, so the two visits to an interior
ping-pong entry are unambiguous. Invalid entry/local-tick seeks preserve state.

Events are opt-in values returned by `advance`, with a requested budget0..4096.
They occur on entering a new schedule slot, including a wrap to the same frame
in a singleton loop. Construction, seeking and one-shot terminal completion emit
no entry event. Large non-collecting advances stay constant-time. Collection
performs a bounded local walk and publishes neither time, fraction nor an event
prefix if the full result exceeds its budget. Paused or completed advances
return no events; an oversized budget still refuses. No callback is invoked.

Checkpoints own the clip, mode/direction, phase, pause state, rate and fractional
carry. All fields are validated before replacement, including terminal fraction
zero. Reload preserves these temporal fields. Plans reuse the existing owned
`SpriteDrawPlan`: palette overrides are copied by value and direct RGB565 sources
refuse them. Neither this library nor the renderer depends on widgets or original
game objects. CPU/GPU delivery uses the existing bounded batch contracts.

Prospective source/contract declarations precede Release and ASan/UBSan controls.
An independent expanded tick tape and per-host-tick rational model check all
itineraries, durations, pause, seeks, event order and rates. A separate binary
rational accumulation oracle checks maximal host deltas and schedule/rate limits.
CPU/GPU pixels are independently constructed from fixture masks, literal frame
selectors and signed origins, after producer handles are retired. Checkpoint,
reload, malformed input, event-cap rollback and empty-frame controls are separate.
The existing native sprite session and CTest wiring are supplier regressions.

Results are immutable SQLite references under
`rendering-twentyninth-ten-results-20261026/`. No original artifacts are consumed.
Original ANI/timing/event semantics, application/world integration, hardware,
performance and live replacement remain unvalidated. The partial TAG parent and
all independent status axes retain their prior scope; no subsystem completion
percentage is inferred from these ten components.
