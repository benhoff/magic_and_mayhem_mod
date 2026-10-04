# Native asset access

Chunk 2 implements read-only path resolution for installed loose assets using
Qt Core and C++17. It does not open/read asset contents yet. The public API in
`path_resolver.hpp` uses standard C++ types, allowing native loaders to consume
it without Qt types entering reconstructed algorithms.

```bash
cmake -S assets -B working/build/assets
cmake --build working/build/assets --parallel 4
ctest --test-dir working/build/assets --output-on-failure
```

Requires Qt 6.8+ Core, CMake, and a C++17 compiler. No Wine, display server,
application event loop, game installation, or original artifacts are needed for
the fixture tests. Test fixtures use QTemporaryDir and are removed on exit.

Call `PathResolver::create(installationRoot, windowsPrefixes)` and inspect its
`Result<PathResolver>` variant. Prefixes such as `C:\\MagicMayhem` explicitly
map that installation to the configured host root. `resolve(request)` returns
`Result<ResolvedAsset>` containing both the matched host spelling and canonical
target, or a structured `Error`. Paths do not grant access outside that root.
No file handles, buffers, or original-game pointer values are exposed.

Matching folds ASCII case component by component. Collisions fail even for an
exact spelling. Requests must use ASCII names, though host installation roots
can contain Unicode. Traversal, reserved names, unsupported absolute paths,
and escaping symlinks fail explicitly. Prefix configuration accepts trailing
and repeated separators but rejects dot/parent components and overlapping
prefixes. See the [complete contract](../research/formats/asset-file-interface.md).

Qt handles host path/name representation through QFileInfo. Checked C++
filesystem iteration, canonicalization, and metadata queries retain operating
system errors; directory enumeration must not silently turn permission or I/O
failures into `notFound`. No directory listings are cached.

Tests cover relative and aliased Windows paths, nested mixed-case names,
Unicode roots, missing paths, file/directory types, name rejection, collisions,
contained/escaping/broken/looped symlinks, current-directory independence,
fresh listings, diagnostics, and denied directory enumeration. Collision tests
skip on case-insensitive fixture filesystems; permission tests skip when the
process can enumerate a mode-000 directory. Symlink fixtures require a host
that permits creating links. Linux fixture validation is confirmed; other host
platforms and original-game path compatibility remain unvalidated. The root
must remain trusted and stable during resolution.

Next: chunk 3 adds owned read-only handles and read/seek/size operations;
chunks 4 and 5 compare installed bytes and connect the existing WAV pipeline.
