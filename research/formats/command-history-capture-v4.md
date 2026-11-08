# Bounded command-history capture, version 4

This is an intentional native diagnostic format, not a recovered original-game
file structure. `MNM_ACCEPTANCE_COMMAND_HISTORY=1` selects it only for the
`commands` role. Rendering, simulation and shared runtime-channel layouts do not
change. Legacy capture versions 1, 2 and 3 retain their existing budgets.

The envelope remains `MNMACE01`. Each record has two little-endian uint32
lengths followed by JSON metadata and an optional Qt `qCompress` body. That body
has a big-endian uint32 decoded length and zlib bytes. Command payloads remain
opaque command-stream bytes; this version does not use viewport delta planes.

The first JSON record is `kind: open`, `version: 4`, `role: commands` and
`policy: command-history-lossless-v1`. The writer and reader require these exact
bounds:

| Field | Value |
| --- | ---: |
| `row_limit` | 65,536, including open and terminal close |
| `raw_limit_bytes` | 8 GiB of decoded payloads |
| `stored_limit_bytes` | 1 GiB |
| `queue_limit_bytes` | 256 MiB, including in-flight work |
| `queue_limit_rows` | 4,096, including in-flight work |

Metadata indices are contiguous and host timestamps are monotonic. The writer
reserves terminal capacity. Overflow or any earlier explicit/compression/I/O
refusal preserves the first error, drains owned work and emits an incomplete
close with the contiguous written row/decoded-byte counts. A successful close
requires `complete: true`, an empty error, the exact prior-row count and the
exact decoded-byte count. Readers reject incomplete evidence and malformed
policy/role/budget declarations; declaring a larger limit on version 1 does not
increase its 16,384-row allowance.

The motivating PA07 high-resolution capture has a complete viewport stream but
an explicitly refused 16,384-row command stream. It earns no pixel-comparison
or pause-row credit. The new mode admits a 17,000-fragment native test and still
refuses its own row cap and the unchanged blocked-compressor queue cap. A
viewport cannot select this command policy. Optimized and ASAN/UBSAN writer
controls, strict Python reader guards and the existing complete legacy worker
controls are retained under `research/runtime/rendering-pause-results-20261107/`.

Producer: `apps/qt-shell/acceptance_capture.hpp`. Consumer:
`tools/rendering_capture.py`. Executable controls:
`tests/acceptance-capture-test.cpp`, `tools/test-acceptance-capture.py`,
`tests/test-rendering-command-capture.py` and `tests/test-rendering-capture.py`.
The independently scoped binding is `command-history-controls-bound-01.json`.
These controls establish the diagnostic policy, not original simulation,
rendering pixel equivalence, physical-provider support or whole-player closure.
