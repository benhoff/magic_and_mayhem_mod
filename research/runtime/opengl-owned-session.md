# Ordered replay from owned rendering state

The opt-in producer in `runtime/render/owned_session.h` combines successful
CPU Unlock updates, supported Blt/BltFast operations, two-buffer Flips and
observed palette changes into one `MNMCMD01` file. It extends the existing
[partial Lock capture](opengl-partial-locks.md); it makes no additional COM calls
or observer Locks and allocates no additional native image buffers.

```bash
MNM_RENDER_OWNED_SESSION=1 ./tools/run-qt-shell.sh --capture-locks
```

The generated experiment's `lock-capture/session-00000001.bin` is created
exclusively. This flag is disabled by default. Existing independent Lock,
UPDATE, blit and Flip captures continue alongside the ordered session.

## Ownership and ordering

IDs identify application-observed interface alias groups within this session;
they are not persistent addresses. A surface's first usable complete owned
checkpoint seeds CREATE. Full CPU reseeds of an existing surface and partial
CPU writes emit one UPDATE per normalized row. Partial writes CHECK the old
complete base first. Supported copies CHECK both inputs, emit COPY and CHECK
the reconstructed destination. Flips CHECK both inputs, emit SWAP and CHECK
both results. Expected CHECK pixels are comparison data, never GPU uploads.

Complete opaque destination bootstrap can seed synthetic zero bytes before a
COPY overwrites every pixel. These zeros are not an observation of engine
memory. Existing native retention and application forwarding remain unchanged.
Failed original calls produce no session operation. Known palette assignments
stay with their surface IDs while SWAP rotates native indices. Presentation
requires a complete observed indexed palette; no default palette is inferred.

The file ends after 16 logical accepted operations or at DLL detach, with
DESTROY records and END. At least one PRESENT and no outstanding tracked Lock
are required. It is a bounded replay checkpoint, not continuous gameplay
capture or a rendering replacement.

## Incompleteness and bounds

The producer limits a file to 64 MiB, 4096 commands, 32 surface identities and
16,777,216 live pixels. It reserves command/byte capacity for cleanup or GAP.
State invalidation, missing required pixels or palettes, alias conflicts,
contention epochs and outstanding Locks close the file with GAP, without END.
An incomplete file is rejected before OpenGL replay. Capturing does not resume
within that file after a gap. I/O failure likewise leaves no valid END and does
not stop the original operation or native cache commit.

GAP payloads are one little-endian uint32:

| Reason | Meaning |
|---|---|
| 1 | Observation epoch changed, including contention |
| 2 | Byte, command or resource bound |
| 3 | Tracked surface invalidation, layout change or outstanding Lock |
| 4 | Missing/conflicting identity or required native checkpoint |
| 5 | Incomplete observed indexed palette |
| 6 | No recorded presentation at completion |

`lifecycle.log` records `session_started`, `session_finished`, `session_gap`
and `session_file_failed`. If DLL detach cannot acquire the guard, it leaves
the session incomplete rather than manufacturing a complete trace.

## Offline validation

```bash
./tools/test-render-owned-session.py
```

Synthetic PE32 i386 fixtures own independent engine pixels, poison unlocked
live storage and verify original call counts, return values and LastError.
Mixed full/partial writes, copies and repeated swaps compare every CHECK with
independent native snapshots, then replay through the CPU oracle and actual
OpenGL consumer. Qt validates the RGBA stream. Failed calls, aliases, the
16-operation boundary, Restore, held Locks, exclusive file failure, byte and
command limits have separate cases. An indexed fixture combines keyed COPY,
three Flips and palette changes, checking every presentation against independent
engine indices and colors.

Confidence: confirmed only for these synthetic x86 ownership and replay
contracts. No original game artifact is consumed by these tests. Real game
coverage, long sessions, unsupported DirectDraw operations and concurrent
rendering remain unvalidated.

Evidence: all 10 cases passed in
`working/tests/render-owned-session/run-2gkywf0c/report.json`. The mixed session
contains 45 commands, 22 native CHECKs and four presentations. The indexed
session contains 39 commands, 20 CHECKs, three palette uploads and five
presentations. Byte/command limits produced GAP files below both parser bounds;
the consumer rejected them without creating an output. The report records the
selftest DLL SHA-256 and OpenGL implementation. The production DLL was also
built successfully as PE32 Intel i386.

Regression evidence: all 25 partial-Lock cases passed in
`working/tests/render-partial-locks/run-bu_166zi/report.json`; all 14 baseline
Lock/Unlock cases passed in
`working/tests/render-lock-lifecycle/run-ehd8_ete/report.json`.
