# Native TXT and WBT input

Reviewed 2026-10-04. Owned byte text, typed scroll entries and a read-only WBT
statement tree. Evidence is complete installed-file inspection and independent
native/reference comparisons, not original parser execution. Confidence is high
for stored bytes and installed syntax; original parsing, display and script
execution behavior remain unverified.

## Installed inventory and roles

The installation contains 43 TXT files (141,904 bytes) and two WBT files
(346 bytes). All 45 inputs contain ASCII bytes, with no NULs; all stored line
terminators are CRLF. Some region descriptions and the sound README end without
a terminator. The native text service preserves arbitrary non-NUL byte values,
including high bytes and BOMs; it does not infer or transcode an encoding.

| Input | Native handling / boundary |
| --- | --- |
| 36 Realm Viewer region descriptions | Exact byte document and line spans |
| Four `*regionNames.txt` lists | Exact byte document; existing typed persistence reader also remains available |
| `Interface/Grimoire/Grimoire.txt` | Exact byte document; existing application parser handles its book syntax |
| `Text/scrolls.txt` | Exact byte document plus 26 typed ID/heading/body entries |
| `Sounds/Readme.txt` | Exact byte document; development prose, no game semantics inferred |
| `AI/BugSearch.wbt`, `AI/Train.wbt` | Exact byte document plus six statements each; parsed, never executed |

The earlier inventory label “AI data” for WBT was misleading. Their content
requests a directory change, deletion of a log, invocation of `Debug\Chaos`
with `/win/bugsearch=7500` or `/win/train`, copying of that log and a jump back
to a label. This establishes an automation-script role from content, not AI
parameter definitions. Whether these scripts generated either DAT file is
unresolved. No original game WBT consumer was identified in this milestone.

## Text API and exact lines

`mnm-text-loader` exposes `decodeText`/`loadText` in `assets/text.hpp`.
`TextDocument` owns exact source bytes and ordered line spans with byte offset,
content length and explicit `none`, `lf`, `crlf` or `cr` ending.
`lineText(index)` returns an owned byte string without the terminator.
No whitespace/comment stripping, encoding conversion or final-empty-line
invention occurs: empty source has zero lines; `a\n` has one terminated line;
`\n` has one empty terminated line. A BOM remains part of the first line.

NUL is rejected as a native text policy, with its source offset. Defaults are
1 MiB input, 65,536 lines, 256 KiB per line and 4 MiB decoded payload/metadata.
The parser validates/counts lines before allocating output. The decoded budget
counts exact source bytes and `sizeof(TextLine)` for each span. Allocator and
container bookkeeping are not a process-wide memory cap. File errors preserve
structured backend information, including input limit errors.

The existing [Grimoire application parser](grimoire-text.md) already decodes
nested comments and chapter/section/level declarations. This change supplies
shared byte loading; it does not duplicate that parser or migrate its widget.
Likewise, [region-name parsing](persistence-native-loading.md) is already present.

## Typed scroll syntax

`decodeScrollText`/`loadScrollText` accept the installed sequence:

```text
[Scroll1]
~HS heading~HE
~BS body~BE
```

IDs are positive DWORD decimals, unique and preserved in source order. Leading
zeros are accepted as a native policy. Blank lines and semicolon-prefixed comment
lines are skipped outside fields. Heading/body retain exactly the bytes between
their markers, including spaces and embedded newlines; whitespace is not
silently trimmed for display. Five installed headings contain a single space.

A declaration must be followed by one heading and one body; closing markers
allow only whitespace to end of line. Unknown/nested tilde markers, duplicate
IDs, absent markers and unexpected declarations are rejected with byte offsets.
These are bounded native grammar policies; original parser equivalence is not
claimed. Empty scroll catalogs are accepted synthetically. The entry limit is
4,096 by default; the same decoded budget also counts `ScrollEntry` objects and
owned heading/body bytes. Lookup, substitution, wrapping and game triggers are
consumer work.

## WBT statement subset

`assets/wbt.hpp` exposes `decodeWbt`/`loadWbt`, `WbtScript`, `WbtStatement`
and typed string/boolean/identifier arguments. The service reads and validates
an AST only; it invokes no file operations, processes or jumps.

| Syntax | AST operation and arguments |
| --- | --- |
| `:start` | Label and identifier |
| `DirChange("..")` | Directory-change command, one string |
| `FileDelete("path")` | File-delete command, one string |
| `RunWait("program", "arguments")` | Run-wait command, two strings |
| `FileCopy("from", "to", @FALSE)` | File-copy command, two strings and boolean |
| `Goto start` | Jump and target identifier |

Identifiers use ASCII letters/underscore followed by letters/digits/underscore.
Commands, constants and label matching are ASCII case-insensitive native
policies; original spelling remains in the exact source. Quoted strings keep
backslashes, commas and high bytes literally; this subset defines no C-style
escape handling. Blank lines and semicolon-prefixed lines are ignored in the
AST but retained in the document. Each statement retains its physical line's
source offset. Forward label references are allowed; duplicates and unresolved
jump targets fail. Unknown commands/constants, expressions, multiline commands,
additional arguments and quoted-string escape extensions are unsupported.
This is the installed subset, not a complete language implementation.

Statement count defaults to 4,096. The shared decoded budget counts document
bytes/spans, statement and argument objects and their owned string bytes.
No execution, AI training or gameplay changes accompany this loader.

## Reproduction and evidence

```bash
cmake -S assets -B working/build/text -DBUILD_TESTING=ON
cmake --build working/build/text --parallel 4
ctest --test-dir working/build/text --output-on-failure
python3 tests/test-text-loader.py working/build/text/mnm-text-inspect \
    --installation working/game-clean \
    --report working/tests/text-wbt/comparison.json
```

`mnm-text-inspect ROOT PATH [text|scrolls|wbt]` emits complete raw source and
per-line hex, offsets and terminators; typed modes add scroll entries or WBT
statements. Default mode is raw `text`, avoiding filename-based semantic guesses.
All decoding finishes and the file closes before reporting owned output.

The independent Python reference uses line/marker/argument regexes rather than
the native scanners. It compares every field and reconstructs exact source
bytes from reported lines. Installed runs verify the immutable manifest before
and after, and recheck all 45 input hashes. Five synthetic comparisons cover
mixed newlines, empty input, BOM/high bytes, multiline scroll text and all WBT
operations. C++ tests additionally cover ownership, NUL rejection, malformed
markers/arguments, duplicate IDs/labels, missing jumps and exact budget limits.
All 33 asset CTests and both TXT/WBT ASan/UBSan tests pass (leak detection
disabled). [Retained evidence](txt-wbt-native-loading.json) records inputs,
comparison hashes and tool versions by source hash.
