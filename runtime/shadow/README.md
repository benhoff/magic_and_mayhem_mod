# Neighbor shadow instrumentation

This is new process instrumentation, not a reconstruction of original engine
code. It lives separately from `reconstruction/pathfinding/` and the native
Qt application in `apps/qt-shell/`.

Build the PE32 DLL with `./tools/build-shadow-bridge.py`. Run the synthetic
Wine ABI/capture test with `./tools/test-shadow-bridge.py`. Outputs remain in
`working/`; relocation from `reconstruction/shadow/` changes the source path
only, not the hook implementation or expected executable bytes.

See [runtime scope and evidence](../../research/runtime/pathfinding-neighbor-shadow.md)
and [capture format](../../research/formats/neighbor-expansion-capture.md).
