# Shared protocol migration evidence

Reviewed 2026-10-04. Scope is frame v1, input v1 and media v1. This is a
behavior-preserving ownership extraction, not a new live replacement milestone.

## Integration

- Qt frame/input/media clients include the generated C-compatible headers.
  Native playback and media CLI result values use the media status constants.
- Injected frame publishers, input polling, media requests and mapped-file
  self-tests consume the same versioned headers. File mapping and atomic
  publication stay local to the adapters.
- Python fixture creators, launcher channel validation and startup debugging
  consume the generated Python bindings. Literal expected bytes and offsets in
  independent assertions remain intentional reference values.
- The render bridge build checks generated-file freshness and records hashes of
  the shared headers, schemas and generator in `protocol_sources` alongside its
  existing source hashes.

The input timer in `apps/qt-shell/main.cpp` remains `setInterval(50)`. That file,
Qt build/menu files, asset build files and audio work were active in other
sessions and were left alone. Replace that timer argument with
`MNM_INPUT_V1_HEARTBEAT_MS` when its owning session is ready. Audio v2, render
capture/replay files and menu observation formats are separate contracts outside
this extraction.

## Checks and evidence

| Check | Result / evidence |
| --- | --- |
| `python3 -B protocols/tests/test_contracts.py` | Four tests pass: independent identities/layouts/values, malformed identity/size rejection and generated-file freshness |
| PE32 compile/link, live and self-test bridge builds | Pass; protocol dependency hashes recorded in build manifests |
| PE32 executable section comparison against commit `9e09ab5ee402c1364f6ce277024ea8e5a9409485` | `.text` bytes match for live `bridge.c`, self-test `bridge.c` and `selftest.c`; `working/protocol-binary-equivalence.txt` |
| `python3 -B tools/test-render-bridge.py` | Pass; [frame report](../working/tests/render/run-kpar43jo/report.json): pixels, diagnostics, fallback/forwarding, CPU/OpenGL capture replay and Qt readback |
| `python3 -B tools/test-render-input.py` | Pass; [input report](../working/tests/render-input/run-wmg746g3/report.json): Qt snapshot through PE32 key/cursor APIs, generations, focus/rejection/lease fallback and ABI preservation |
| `python3 -B tools/test-native-media.py` | Pass; [media report](../working/tests/native-media/run-y8j3_9fv/report.json): exact decoded RGBA/PCM, completion/skip, sound replacement/stop/busy, rejected paths, stale-host fallback and ABI guards |
| Focused CTest: `frame-stream`, `render-pixels`, `qt-shell-input`, `qt-shell-input-hidpi` | Four tests pass |
| `python3 -B tests/render-movie-preferences-test.py` | Three tests pass; importlib loading of launcher remains functional |
| Original input manifest | Verified 2927 files before and after validation |

[Migration report](../working/protocol-migration-report.json) records the scope,
baseline commit and evidence paths. Reports and build manifests in `working/`
are disposable artifacts; retain/export them if needed for longer-term audits.

Initial test attempts encountered temporarily missing test sources from concurrent
CMake edits. The standard scripts passed once those sources appeared. Wine/Xvfb
checks required execution outside the filesystem sandbox because local IPC
socket creation was blocked. Neither issue required edits to another session's
sources. An additional isolated Qt shell build under `working/build/protocol-qt`
passed with unrelated tests disabled.

## Confidence and remaining boundaries

High confidence within the synthetic v1 integration scope. Executable-section
comparison supports preservation of injected code; it does not by itself prove
relocation/data equivalence or live behavior. The independent byte expectations
and existing integration tests provide separate evidence.

No new commands, versions, synchronization algorithm or gameplay behavior were
introduced. No real-game playback/input/rendering scenario was run for this
migration. Existing live coverage remains as recorded in the ledger. Other
sessions can change these consumers after this review; rerun the checks when
integrating their work.
