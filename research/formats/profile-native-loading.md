# Native audio profile input

The bounded native profile reader is implemented in
[profile.hpp](../../assets/profile.hpp) and [profile.cpp](../../assets/profile.cpp).
It owns a snapshot loaded through the existing Qt-backed `AssetFile`; it neither
calls Windows profile functions nor uses registry mappings or Qt QSettings.
No game-specific IDs, addresses or reconstruction dependency enter the assets
library. `mnm-profile-loader` depends only on `mnm-assets`.

This is a native input policy with synthetic evidence, not a recovered complete
INI parser. No installed `Sounds.ini` file or original artifact was read for this
chunk. The [manager configuration research](../runtime/audio-manager-configuration.md)
describes the original caller capacities and table behavior separately.

## Supported subset

Input is at most 1 MiB, printable ASCII with tab/CR/LF. The loader rewinds and
bounds the file read, then releases its handle. A snapshot remains valid after
the source file changes; explicit reopen obtains a new snapshot. NUL, BOM,
non-ASCII and other control bytes reject. This restriction is native policy,
not evidence that the game's profile files use no other encoding.

Section headers use `[name]` on their own trimmed line. Blank lines and leading
semicolon comments are skipped. Entries require a section; sections and keys
compare ASCII case-insensitively. Surrounding space/tab/CR is trimmed from lines,
section names, keys and unquoted value storage. Entry order is retained. Bare
entries are supported for the recovered per-map sound lists. Duplicate section
or key names reject; no guessed first/last-write override is applied. This is
stricter than the decoded table model, which can represent duplicate ID slack.

Section reads preserve stored value quote marks and emit normalized `key=value`
or bare-key strings. The returned count includes each string terminator but not
the final MULTI_SZ terminator. If capacity cannot fit the whole list plus its final
terminator and one spare byte (the measured Wine exact-fit boundary), the reader returns `capacity - 2` and no purportedly complete entries.
Partial section contents are intentionally not exposed; the audio caller rejects
that marker. String reads remove matching single/double quotes around the whole
value and reserve one byte for termination when truncating. Inline semicolons
remain literal data; the recovered source-entry comment scan is a separate step.
Missing string keys use the caller default after removing trailing ASCII spaces
(leading spaces and trailing tabs remain). Missing integer keys use the caller
fallback; present integers require complete unsigned decimal with no sign, hex,
trailing junk or DWORD overflow. Malformed integers reject explicitly.

The selected case/quote/count behavior is informed by Microsoft's
[GetPrivateProfileString documentation](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofilestring),
[GetPrivateProfileSection documentation](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofilesection)
and [GetPrivateProfileInt documentation](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofileinta).
Those references do not establish that every normalization/rejection policy above
matches historical Windows or Wine. UTF/ANSI conversion, registry mapping,
repeated sections/keys, complete numeric conversion,
partial enumeration and concurrent file update semantics remain outside scope.
The underlying store additionally enforces trusted-root/path resolution policies.

## Validation

`./tools/test-audio-native-manager.py` exercises the profile reader through Qt
files and synthetic fixtures: case/quote/inline-comment behavior, section/string
capacities, bare entries, integer fallbacks/rejections, malformed/duplicate input,
encoding bounds, file rewind, snapshot lifetime, explicit refresh and size limits.
Native audio integration adds actual WAV preloads/admission/mixing and lifecycle
checks. Evidence: `working/tests/audio-native-manager/run-xauygdrw/report.json`;
29 current audio/assets CTests and address/undefined/leak sanitizer checks pass.

The [PE32 Wine profile API comparison](profile-api-comparison.md) now records
29 selected calls, two supported semantic fixes and seven intentional native
policy differences. Next compare installed profile/catalog reads under the
guarded immutable-input workflow. Synthetic parsing and native PCM
agreement do not establish original profile behavior or live replacement.
