# Read-only asset file interface contract

## Status, scope, and evidence

Chunk 1 is this design contract. Chunks 2 and 3's path resolver and read-only
file access are implemented in `assets/`; chunk 4 validates installed raw bytes.
These are
native design decisions, not discovered original-game
filesystem semantics. Compatibility with the original loader's path encoding,
supported paths, and failure behavior remains unverified.

Use Qt Core behind a standard C++17 interface. QString and QFile stay in the
backend; reconstructed audio code remains independent of Qt. No GUI or event
loop is required. Future interface, backend, and comparison CLI files belong
under `assets/`; focused tests belong under `tests/`.

The first consumer is `audio/upload_main.cpp`: asset read -> `audio::readWave`
-> reconstructed `uploadStatic` -> native PCM storage. Scope is installed loose
files. Archives, writes, overlays, live Wine hooks, and reconstruction of the
original manager's file/config loader are separate work.

Evidence inspected at commit `79186f4543aa98037d7ad7f9326190f0365c3b98`:

- The upload CLI uses binary `std::ifstream`, a 32 MiB file cap, an owned byte
  vector, `readWave`, and `uploadStatic`. Confidence: high from source inspection.
- `audio/buffers.hpp` separates owned `Wave::samples`, Device storage, and
  temporary write-lock regions. Preserve those existing ownership rules.
- `tools/test-audio-buffers.py` independently parses installed WAVs with Python
  and compares uploaded PCM. That covers decoded output, not raw reads through
  this proposed interface. See [PCM WAV evidence](pcm-wav-loading.md).
- `apps/qt-shell/media_broker.cpp::asset` provides a limited media resolver;
  it does not implement this general interface.

No original artifact was consumed to write this contract.

Resolver evidence: `assets/path_resolver.cpp`, `tests/asset-path-test.cpp`, and
`ctest --test-dir working/build/assets --output-on-failure`. Synthetic Linux
fixtures verify resolution and rejection policies, including permission-denied
enumeration and symlink boundaries. Confidence: high for tested native policies;
no installed assets or original loader were exercised. Qt 6.11.2 and GNU C++
16.2.1 were used. See [build and test details](../../assets/README.md).

File-access evidence: `assets/asset_file.cpp` and `tests/asset-file-test.cpp`.
Both asset CTests pass with the same Qt/compiler versions. Temporary binary
fixtures validate reads, seeks, sizes, ownership, limits, denied opens, and
truncation; controlled streams validate partial/error propagation in generic
helpers. Confidence: high for tested native behavior. Installed-file byte
comparisons are now recorded in [raw comparison evidence](asset-file-comparison.md);
native loader integration and live-game behavior remain pending.

## Installation root and path rules

Configure an existing installation directory explicitly, initially
`working/game-nocd`. Canonicalize it; a missing or non-directory root fails with
`invalidRoot`. Relative requests resolve from that root, never from the process
working directory. The interface provides read-only access.

Requests are nonempty ASCII byte strings. Reject NUL, control bytes, DEL, and
non-ASCII bytes as `invalidPath`; no implicit code-page conversion. Internal
spaces are allowed. The host root itself may use a Unicode path. Non-ASCII game
requests require a later encoding decision supported by evidence.

1. Replace backslashes with `/`. Accept repeated internal separators and `.`
   components. Reject every `..` component before normalization.
2. Accept relative requests such as `Sounds\\Spell click.wav`.
3. Accept drive-absolute requests only through explicit installation-prefix
   aliases, for example `C:\\MagicMayhem` -> the installation root. Match whole
   prefix components case-insensitively using ASCII folding, then resolve the
   remainder under the root. `C:\\MagicMayhemOther` does not match. Reject
   overlapping aliases at configuration time. Never strip an arbitrary drive
   letter or search for a familiar suffix.
4. Reject drive-relative paths (`C:Sounds\\x.wav`), leading-slash/root-relative
   paths, UNC/device paths, and unconfigured drive-absolute prefixes with
   `unsupportedPath`.
5. Reject `:`, `<`, `>`, `"`, `|`, `?`, and `*` in remaining name components,
   trailing dots/spaces, and Windows device names CON, PRN, AUX, NUL, COM1-9,
   LPT1-9 (including names with extensions). No trimming, wildcard/environment
   expansion, or 8.3-name translation.
6. Enumerate each directory and match each component by folding ASCII A-Z to
   a-z. No match returns `notFound`; multiple matches return `ambiguousPath`,
   even if one matches the spelling exactly. Preserve on-disk spelling.
   Unicode entries do not match ASCII requests through transliteration.
7. Intermediate components must be directories; the target must be a regular
   file. Reject trailing separators and requests resolving only to the root.
   Canonical targets must remain inside the root using component boundaries
   for containment. Symlinks within the root are allowed; escaping links return
   `outsideRoot`.

Example: `sOuNdS\\SPELL CLICK.WAV` resolves to `Sounds/Spell click.wav` under
the configured root if each component has exactly one match.

Assume a trusted installation that is not concurrently modified. Canonical
checks do not guarantee protection against hostile symlink replacement between
checking and opening. Do not cache directory listings initially.

## Operations and buffer ownership

This table specifies behavior; `assets/asset_file.hpp` is the implemented API.
Use signed 64-bit
sizes/positions, checked conversions to allocation sizes, and structured
results rather than sentinels or persistent last-error state.

| Operation | Contract |
|---|---|
| `AssetStore::open(path)` | Resolve and open in binary read-only mode; return a uniquely owned `AssetFile` or an error. |
| `AssetFile::size()` | Return current file length or an error without changing position. |
| `AssetFile::position()` | Return current byte position or an error. |
| `AssetFile::seek(offset)` | Absolute offset in `[0, size]`, including EOF. Reject negatives and beyond-EOF offsets without changing position. |
| `AssetFile::read(destination, capacity)` | Read up to capacity bytes into caller-owned storage; return bytes transferred and an optional error. |
| `readExact(file, destination, count)` | Repeat reads until filled; return transferred count and optional error. Premature EOF returns `unexpectedEof`. |
| `readWhole(file, limit)` | Query size, enforce limit before allocating, seek to zero, and read exactly that size into an owned byte vector. Return complete output or an error; success leaves position at EOF. |

Each handle starts at zero, owns its file, is noncopyable, and closes on
destruction. Separate handles have independent positions. Handles may outlive
their store. Each handle is thread confined; callers coordinate concurrent use.

Reject negative capacities and sizes unrepresentable by the backend before
I/O. Zero-capacity reads succeed without dereferencing the destination; null
destinations with nonzero capacity return `invalidArgument`. Successful short
reads are allowed. EOF returns zero bytes without an error. I/O failure returns
`ioError` plus any bytes already transferred by the operation. `readExact`
retains the partial count and error. Neither operation rolls back consumed
bytes or written destinations. Bytes beyond the reported count remain untouched.

Invalid arguments do not change position. An underlying seek failure returns
`ioError`; callers query position or reopen before continuing. Allocation
failure returns `limitExceeded`. `readWhole` never returns partial output on
failure and does not promise to restore position.

The caller owns destination storage, which must remain valid for the synchronous
call. The backend retains no buffer pointer. `readWhole` returns an owned
`std::vector<std::uint8_t>` that survives file closure. No mapped-memory views
or borrowed pointers cross the interface.

Sizes are not snapshots. Truncation after a size query is detected by exact
reads; concurrent growth/mutation is outside the consistency guarantee.
Comparison runs record input hashes before/after and fail if inputs changed.

## Errors and diagnostics

Stable categories: `invalidRoot`, `invalidPath`, `unsupportedPath`, `notFound`,
`ambiguousPath`, `outsideRoot`, `notDirectory`, `notRegularFile`,
`permissionDenied`, `invalidArgument`, `unexpectedEof`, `limitExceeded`,
and `ioError`. Enumeration/permission failures must not become `notFound`.
Attach operation, requested path, resolved path when available, and readable
backend detail. Diagnostic text does not replace structured categories.

## First integration and completion checks

The WAV adapter uses `readWhole` with the existing 32 MiB input cap, then
`readWave` and `uploadStatic`. Keep Device's default 16 MiB sample cap. Raw
file bytes, extracted PCM, and Device storage retain separate owners. Preserve
parser checks and audio policy without conversion, resampling, or balance edits.

Chunk 2 implements the resolver; chunk 3 implements this file interface.
Chunk 4 compares complete raw asset bytes with independent binary reference
reads, reports sizes and first differing byte offsets, and exercises mixed-case
paths plus sequential and seek-based reads. Existing Python WAV comparisons
remain a separate decoded PCM check. Chunk 5 connects the WAV adapter; chunk 6
finishes build integration, commands, documentation, and recorded evidence.

Focused tests cover the path policies, independent positions, empty files,
embedded zero bytes, zero-length reads, EOF, partial exact reads, seek bounds,
size/position preservation, allocation limits, and buffer lifetime after close.
Document backend failures needing controlled fixtures. Both raw-file and PCM
comparisons are required for the WAV milestone.

Experiments consuming original artifacts run
`./tools/original-manifest.sh verify` before and after, including failure paths.

Backend references: [QFile](https://doc.qt.io/qt-6/qfile.html),
[QIODevice](https://doc.qt.io/qt-6/qiodevice.html), and
[QDir](https://doc.qt.io/qt-6/qdir.html). Use binary mode; Windows-path policy
and case matching remain explicit application behavior.
