# Native asset access

Chunks 2 and 3 implement path resolution and owned read-only file handles for
installed loose assets using Qt Core and C++17. The public APIs in
`path_resolver.hpp` and `asset_file.hpp` use standard C++ types, allowing native
loaders to consume them without Qt types entering reconstructed algorithms.

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

## Read-only handles

Create an `AssetStore` with the same root/prefix configuration, then call
`open(request)`. It returns `Result<std::unique_ptr<AssetFile>>`; inspect the
variant for an `Error` before taking ownership. Handles are noncopyable, have
independent positions, close on destruction, and can outlive the store.

`AssetFile` exposes `size()`, `position()`, absolute `seek(offset)`, and
`read(destination, capacity)`. Sizes/counts/offsets are signed 64-bit integers.
Size and position return `Result<std::int64_t>`; seek returns `Status` (a
`Result<std::monostate>`). Read returns `ReadResult` with a transferred count
and optional error. The caller owns the destination; the backend retains no
pointers. Negative counts/offsets, null nonempty destinations, and seeks past
EOF fail before changing position. Seeking to EOF and null zero-length reads
are allowed. Thread confinement applies to every handle.

`readExact` loops over successful short reads, preserving partial counts and
I/O errors. EOF before the requested count returns `unexpectedEof`. Failed
reads leave any transferred bytes in the caller's destination. `readWhole`
requires an explicit size limit, allocates an owned byte vector, seeks to zero,
and reads the entire reported size. It returns complete output or an error;
successful output remains valid after the file closes. Rejecting a limit occurs
before allocation/seek. The generic helpers work with any `AssetFile` backend.

The QFile backend uses binary ReadOnly/Unbuffered mode and caps each read at
64 KiB, so callers must handle short reads or use `readExact`. It performs no
text conversion, exposes no write operation, and caches no file size. On Linux,
Qt may report a denied open as generic OpenError; immediately captured errno
provides the permission/missing/resource category without parsing error text.
Unknown backend failures remain `ioError`. Non-Linux error mapping remains
unvalidated. Files must remain stable during normal use; this is not a snapshot.

`asset-file-io` verifies binary bytes (including NUL, CRLF, and high bytes),
positions, independent handles, empty files, EOF/partial exact reads, seek
bounds, size limits, multi-read files, source preservation, store/handle/buffer
lifetimes, permission-denied opens, and truncation without stale read-ahead.
A controlled `AssetFile` fixture checks repeated short reads, partial I/O
failure counts, premature EOF after a size query, and size/seek failures.
Allocation-limit rejection is checked, but actual memory exhaustion is not
forced. Real device failures and descriptor exhaustion are not induced.

Next: chunks 4 and 5 compare installed raw bytes and connect the existing WAV
pipeline. This increment reads only temporary fixtures and does not launch or
hook the game.
