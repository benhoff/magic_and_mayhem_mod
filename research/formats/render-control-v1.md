# Rendering recovery control v1

Toolkit-designed IPC, not an original game format. Schema:
`protocols/schemas/render_control-v1.json`; generated C/Python constants share
that source. Exact size 576 bytes; magic `MNMRCV01`, version 1 and declared size
576. Little endian, aligned atomic u32 on supported x86 hosts.

| Offset | Meaning | Writer |
| --- | --- | --- |
| 0, 8, 12 | Magic, version, exact declared size | Creator |
| 16 | Nonzero immutable initial command session / launch ID | Creator |
| 20 | Request sequence, starting at 1; release after request fields | Qt |
| 24, 28 | Operation RECOVER=1 or CHECKPOINT=2; expected new session | Qt |
| 32, 36 | Response sequence; READY=1 or REFUSED=2 | PE32 |
| 40 | Permanent cancellation, initially 0 | Qt |
| 44 | Nonempty path length, at most 511 | Qt |
| 48 | WAITING=0, RUNNING=1, STOPPED=2 | PE32 |
| 52..63 | Zero reserved bytes | Creator |
| 64..575 | NUL-terminated ANSI candidate path, zero-filled tail | Qt |

Only one request is outstanding. Request metadata and path remain immutable
until the matching response. Producer reads the release-published sequence,
copies and validates fields, rechecks identity and sequence, and verifies the
request again before acceptance. Response status precedes release publication
of its sequence. Online state is independent; READY acknowledges admission and
worker startup. For CHECKPOINT, READY additionally acknowledges complete owned
resource serialization and a queued initial PRESENT; GPU completion remains
independent. RECOVER now prefers a complete owned checkpoint when the same
strict state/ownership/budget admission succeeds; otherwise it waits for fresh
full observations. A checkpoint publication/startup failure after claim refuses
and retires that candidate, without retrying it as a fresh stream. The direct
PE32 RenderRecover export continues to require fresh pixels. No host pointers,
COM pointers, wire C++ objects or game addresses cross this channel.

Initial files are exclusive, fresh and zeroed. Launch staging requires continuous
v2 production, paths under `working/` and matching initial launch/session IDs.
A recovery candidate is a fresh exact-size v2 file with distinct physical file
identity and a strictly higher session ID. Producer verifies the expected ID
before claiming it. Three requests per launch, no request sequence wrap.

Host reply deadline is 6000ms; complete frame deadline is 10000ms. Cancellation
on timeout, fallback or producer exit is permanent; late READY cannot revive a
cancelled consumer. Native worker observes cancellation before and after
admission. Qt never polls candidate command bytes before matching READY.
Unknown operations, malformed identities/reserved bytes/sequences/statuses,
missing workers, ownership contention and exhausted budgets fail closed into
original-window presentation. Cancellation is not an original process kill.

CHECKPOINT=2 is an additive operation in the existing versioned envelope; older
readers refuse its unknown opcode. It uses the same immutable publication,
request budget and cancellation rules. The producer serializes a complete
independently owned checkpoint into bounded storage behind exclusive callback
admission before replying. Incomplete or borrowed state refuses. See the
[native policy and validation boundary](../runtime/opengl-command-checkpoint.md).

## Terminal stop addition (2026-10-06)

Operation 3 is a terminal application stop. Its target session, path length and
all 512 path bytes must be zero. It uses the next sequence after zero to three
completed recovery requests (at most sequence four). The producer latches stop,
refuses further startup/recovery, quiesces observed borrowing and drains cleanup
through END with an owned-copy ACK deadline. READY records successful terminal
publication cleanup; REFUSED records inability to establish it. Matching reply
and renderer END are independently observed. The worker publishes ONLINE_STOPPED
when returning; the normal public stop/ExitProcess path still owns joining its
handle and releasing its control view. A pending recovery cannot be overwritten
with STOP: the host cancels and reports refusal instead. Old v1 producers reject
the unknown operation and the host falls back explicitly. No wire layout or status
numbers changed. See [ownership and validation](../runtime/opengl-application-shutdown.md).
