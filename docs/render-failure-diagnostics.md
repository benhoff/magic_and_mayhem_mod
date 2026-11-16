# Native render failure diagnostics

Handled native rendering failures retain diagnostics automatically. No capture
environment variable is needed. This includes the campaign Quit followed by
new-campaign failure reported on2026-11-16; that particular failure has not yet
been reproduced with these diagnostics, and its underlying cause remains open.

Read `render_failure` in the failed session's `resident-world-terminal.json`,
`resident-world-report.json` or `native-only-session.json`. These existing bounded
runtime interchange reports retain their existing consumers. The terminal worker
report survives the narrower resident teardown report. The native-only receipt
keeps the first failure if later shutdown errors occur. A failed report write is
reported to stderr with the original diagnostic envelope. No report path is a
promise of persistence after an abrupt process kill, disk failure or power loss.

The snapshot contains the following metadata:

- `reason`, the original failure message, `schema: 1`, executable path and
  SHA-256 (or explicit hash unavailability). Executable hashing runs only on
  the failure path.
- `active`, up to16 enclosing calls, and `recent`, the last64 instrumented calls
  on that owner thread. Each call has a monotonic decimal-string `serial`, call
  name, decimal-string `words`, optional resource-name prefix `label`, payload
  length and exception-unwinding flag. Active scopes survive recent-history wrap.
  A snapshot taken before throwing still shows the active call as unfailed.
- Active source payload SHA-256 and at most64 prefix bytes as hex, when present.
  Historical payload pointers and payload bytes are never retained. Payloads are
  hashed only while their owner scope is alive, on the failure path.
- For undefined-pixel reads, `detail` names the renderer operation, source file
  and line, rejected rectangle, first unknown pixel and validity-mask SHA-256.
  It also records the checked surface and all live surface IDs, creation serials,
  dimensions, bit depths, RGB masks, clipper rectangles, unknown counts and
  validity-definition/invalidation/restore counters. Validity travels with native
  storage across copies/restores/swaps; these counters describe that knowledge.
  The live driver, renderer revision and batch depth are included.

`world_generation.words` is `[generation, completed_generation, stream_version]`.
`world_record.words` and `source_record.words` are the24 original producer fields;
field22 is the original caller address. `source_chunk.words` is
`[completed_generation, stream_version, record_count]`; `source_delivery.words`
is `[generation, source_ordinal]`. `ui_command.words` starts with
`[stream_version, command_sequence, operation]`, followed by all13 command words.
`command_poll.words` is `[session, transport_version, consumed_bytes, byte_budget]`.
`scene_draw.words` is `[draw_index, batch_end, frame, anchor_x, anchor_y,
composite_mode, renderer_canvas]`, with the resource name in `label`.
`render_turn.words` is `[owner_turn, source_generation_before_poll]`.

Other call names are renderer methods; their words follow that method's numeric
arguments in declaration order, with rectangles expanded left/top/right/bottom,
optional keys represented by presence/value and formats by bit depth/RGB masks.
The instrumented call site is authoritative for ancillary arguments. Signed
renderer coordinates use64-bit two's-complement decimal strings; raw producer
fields retain their unsigned32-bit wire values. IDs are native numeric identities,
never shared host pointers. The containing session reports supply the session
path and World nonce. Match source file/line against the executable used for
that session, rather than assuming a later checkout has the same line numbers.

Snapshots are compressed into the existing exception string with the marker
`MNM_RENDER_FAILURE_V1:` so string-only adapters, GPU queues and the isolated
renderer service preserve the original failure through cleanup. Report writers
decode the same envelope into `render_failure`; normal UI messages show its
human-readable prefix. A later wrapper preserves the original snapshot. The
decoder bounds encoded input to128KiB and decoded JSON to256KiB. Labels retain
at most159 UTF-8 bytes. This is bounded diagnostic context, not a complete call
log, pixel capture, replay input or general machine stack trace.

Successful calls update fixed thread-local metadata only. They perform no
diagnostic JSON serialization, hashing, I/O or GPU readback. The existing
undefined-pixel refusal remains enforced; recording a failure does not repair
the surface or establish original-game equivalence.

Focused controls are `render-failure-diagnostics-test`, `native-render-worker-test`
and `native-only-session-test` in the Qt-shell CMake build. They cover real
undefined GPU storage, original source caller fields, UI rejection before
cleanup, history wrap, thread/string forwarding and sticky terminal reports.
