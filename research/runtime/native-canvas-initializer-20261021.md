# Native canvas initializer — 2026-10-21

The owned `initializeOriginalCanvas` reconstruction and its original comparison
fixture are now committed with a dedicated reproducible runner:

```sh
python3 tools/working-retention.py run --reserve-gib 0.3 -- python3 tools/test-canvas-initializer.py --output working/tests/canvas-initializer-NEW
```

The normal-return provider contract supplies window/mode setup, wrapper creation,
zero-fill requests and pixel-format results. The model owns all 36 neighboring
global bytes. It preserves both alternate pointers, reproduces unsigned padding
wrap and the configuration override, publishes the three low-WORD masks and
RGB555 indicator, and retains complete primary/offscreen descriptors and call
order. Confidence is high within this declared owned-provider domain.

`RS.canvas-startup-initializer` is a separate scoped supplier to the existing
`RS.alternate-canvas-initializer` investigation. The original initializer at
004e40d0 and pixel-format wrapper at 0058d540 execute unchanged in private mapped
memory. Script-verified executable SHA-256 and boundary-byte checks precede
private callback redirection. Original files are never patched. Mode/window,
create and fill bodies are explicitly owned boundaries, not claimed original
physical driver behavior.

The fresh original PE32, native64 and Clang ASan/UBSan cohorts agree over 46,656
cases and 18,475,776 complete serialized bytes per cohort. Source fingerprints
remain stable; original manifest verification passes before and after. The
fixture asserts alternate-pointer preservation and checks original initializer /
query bytes after execution. Evidence is
`CANVAS-INIT.original-native-sanitized-20261021`, retained in
`native-canvas-initializer-result-20261021.json`; raw results are pinned under
`working/tests/canvas-initializer-20261021/execution/` in the integration worktree.

Actual alternate-buffer allocation, initial pixels, upstream publication of
006e1f68/006e1f6c, external writer/lifetime ownership and physical provider
failure/loss semantics remain unrecovered. This initializer does not invent an
allocator or enable a supposedly original alternate route. No integration,
physical NVIDIA, complete original session or live replacement milestone follows
from this comparison. Existing historical evidence and hashes remain unchanged.
