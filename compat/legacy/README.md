# Original scene observation adapter

The [complete live drawing replacement plan](../../docs/live-drawing-replacement.md)
owns the cross-subsystem implementation order and takeover gates. This adapter
currently provides bounded observation and replay while original drawing continues.

This host adapter decodes owned `MNMSCNE1` snapshots and maps normalized SPR
frame content to explicit, SHA-256-pinned native resource bindings. It consumes
original draw order and anchors without importing original pointers into native
services. It does not reconstruct world entities or advance simulation.

Build and exercise the adapter:

```sh
cmake -S compat/legacy -B working/build/scene-snapshot -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/scene-snapshot -j4
xvfb-run -a python3 tools/test-scene-snapshot.py
```

For installed assets and original simulation queues:

```sh
python3 tools/index-scene-bindings.py --help
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/capture-scene-game.py
xvfb-run -a python3 tools/test-scene-snapshot.py \
  --capture-report working/experiments/scene-observer/run-EXAMPLE/report.json \
  --bindings working/tests/scene-bindings-EXAMPLE.json
```

The capture uses the repository launcher with an isolated staged game and Wine
prefix. Synthetic forwarding checks use a dedicated initialized Wine prefix at
`working/tests/scene-selftest-wine`; a fresh prefix can be created with
`WINEPREFIX="$PWD/working/tests/scene-selftest-wine" wineboot -u` under Xvfb.
All outputs use fresh directories or prefixes; assets remain read-only.

The preview executable requires `--unshaded`. It refuses incomplete mappings
unless `--supported-only` explicitly permits a partial preview. Its JSON reports
ordered draws, missing identities, unsupported records, and remaining surface
ownership. Rendering uses native SPR palettes and a diagnostic background.
Original shade remapping, effects, background, HUD, continuous scene streaming
and live replacement remain separate milestones. See
[the runtime evidence](../../research/runtime/native-scene-snapshot.md) and
[the wire format](../../research/formats/scene-snapshot-v1.md).
