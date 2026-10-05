# Shared wire contracts

This directory owns the toolkit's frame, input and media wire definitions.
Injected C producers/readers, Qt clients, launcher validation and Python
fixture creation consume these bindings. Version 1 wire bytes and publication
behavior are preserved.

## Ownership and integration

`schemas/*-v1.json` own numeric layouts, magic values, versions, sizes, enums,
masks and documented timing constants. `CONTRACTS.md` owns field publication
and lifecycle semantics. Generated C/C++ headers and Python bindings are checked
in for use by independent builds; edit schemas and regenerate, never edit the
bindings directly. There is no package installation or root build change.

```bash
python3 protocols/generate.py
python3 protocols/generate.py --check
python3 -B protocols/tests/test_contracts.py
```

C/C++ consumers can add `protocols/include` to their include path and include
`mnm/frame_v1.h`, `mnm/input_v1.h` or `mnm/media_v1.h`. Macros are versioned,
for example `MNM_MEDIA_V1_RESPONSE_SEQUENCE_OFFSET`. Headers require no platform
or standard library headers and contain no host structs or pointers.

Python tools can add `protocols/python` to their module search path and import
`mnm_protocols.frame_v1`, `input_v1` or `media_v1`. `initial_header()` returns
private initialization bytes; input/media callers must zero the rest of the
new file. `valid_header()` checks identity and exact file length in private
bytes. Neither helper reads a live shared mapping or implements synchronization.

File mapping, atomic operations, platform APIs, Qt event translation, playback
and guarded game hooks remain in their adapters. The injected render build
checks binding freshness and hashes protocol headers, schemas and the generator
in its manifest. Relative header includes support existing standalone Qt/test
builds without changing their build files; Python tools add this repository's
`protocols/python` directory to their module search path.

The main window's input timer still uses a literal 50 ms: `main.cpp` became
active work in another session during migration and was left untouched. Its
follow-up is replacing `inputTimer_.setInterval(50)` with
`inputTimer_.setInterval(MNM_INPUT_V1_HEARTBEAT_MS)`; the macro is available
through the existing input headers. Independent tests intentionally retain
literal expected offsets and bytes.

## Compatibility

Each channel has its own version. Its magic, exact file size and declared size
must match. Frame offset 12 declares **header size**; input and media offset 12
declare **whole file size**. Do not normalize that difference in v1.

Changing field layout or established semantics requires a new version. Enum
extensions require an explicit reader compatibility decision and tests. Reserved
bytes are initialized to zero; existing readers do not uniformly reject nonzero
reserved bytes. They are not an implicit capability negotiation mechanism.
Unknown media operations/statuses must not be treated as success. Existing
media writer behavior for unknown response statuses is to keep waiting until
a timeout; adopting constants must preserve that behavior unless separately
versioned. Unknown frame statuses may be displayed as unknown diagnostics.

Before adding commands, specify writer ownership, publication ordering, bounds,
acceptance, completion, cancellation and fallback, and how older peers behave.
Do not assume these three channels share a common request/response model.

## Evidence and boundaries

This v1 baseline was transcribed from the existing sources and format documents
on 2026-10-04:

- [Frame format](../research/formats/render-frame-stream.md),
  `runtime/render/bridge.c`, `apps/qt-shell/frame_stream.cpp`.
- [Input format](../research/formats/render-input-state.md),
  `runtime/render/input_polling.h`, `apps/qt-shell/input_state.cpp`.
- [Media format](../research/formats/render-media-channel.md),
  `runtime/render/media_bridge.h`, `apps/qt-shell/media_broker.cpp`.

The standalone checks use independently written expected byte layouts and
numeric values. They establish this package's agreement with that baseline,
not automatic agreement with future edits in other sessions.
[Migration validation](VALIDATION.md) and its evidence are recorded in
`research/runtime/coverage-ledger.md`; no new live-game validation or replacement
claim follows from this extraction.

Audio's existing v2 contract in `runtime/audio/protocol.h` is a separate future
migration. It is not moved or redesigned here.

The optional `render_commands-v1.json` schema defines bounded append-only native
render command transport. It does not extend frame v1. Its generated bindings
are `mnm/render_commands_v1.h` and `mnm_protocols.render_commands_v1`; publication
and failure semantics are documented in [the channel format](../research/formats/render-command-channel.md).
