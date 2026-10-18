# Remaining producer entry boundaries

The five remaining natural callers are PCX words (`0x54b5f0`), brighten
(`0x58d770`), row copy (`0x58d3b0`), raster15 (`0x57e1b0`) and minimap camera3
(`0x5532f0`) in No-CD build `40209ca7`. Their complete previously registered
windows have no external E8/E9 or stored preferred-address candidate in the
executable. The known camera table selects camera0/2/0/1. These findings do
not establish global unreachability.

`tools/inspect-pixel-route-entry-boundaries.py` extends that investigation to
PE entry, export, TLS callback, import and relocation boundaries. It searches
stored target RVAs and file offsets as well as preferred addresses and rel32
byte candidates across supplied media PE files and the admitted installed JPEG
dependency. Unsupported containers/images, system DLLs, unpacked payloads and
addresses constructed at runtime remain explicit unknowns. A foreign module's
preferred address is a byte candidate, not a verified runtime mapping.

The parser controls use synthetic PE32 images and check ordinal-only and
forwarded exports, TLS callbacks, ordinal/name imports, relocation extents,
raw/virtual boundaries, negative rel32 displacement and malformed input.
Empty candidate lists never grant original-game execution or a global dead-code
claim. The 43-route denominator is retained.

## Reviewed result, 2026-10-18

Seven PE32 modules parse: the pinned game, Browser, three DirectX/media installers,
the service-pack wrapper and the installed JPEG dependency. Browser2, ISDEL and
SETUP are non-PE containers and remain outside the parser. The game has no
exports, TLS callback array or base relocations. Its PE entry is `0x59e61f`,
outside the five helpers. None of the parsed modules imports `Chaos.exe`; the
JPEG dependency imports only KERNEL32 and exports one unrelated entry.

The game contains 24 RVA/file-offset interpretations at twelve unique positions.
Its text file offsets equal RVAs, so each position retains both interpretations.
Eleven positions are relative-call bytes to other targets, a small immediate
argument, object field offsets, or stack-local stores of `0x15`. The twelfth is
an unaligned rdata pattern with unassigned data semantics. Two Browser code
positions are relative-call bytes to its own helpers; resource byte coincidences
in installers remain data candidates. The two primary E8/E9 candidates are
internal to the already reviewed windows. No new natural caller follows.

The five original functions are now explicitly classified as `unused_candidate`.
This is a hypothesis supported by the bounded searches, not a global dead-code
proof or permission to delete them. All five implementations, original controls,
unknown flows and the denominator of 43 remain. The observed union stays 38/43.

The [reviewed result](pixel-route-entry-boundaries-reviewed-20261018.json) links
the [immutable raw scan](pixel-route-entry-boundaries-raw-20261018.json), exact
module hashes and retained instruction/data dumps. Twelve synthetic parser
controls pass. The earlier overlapping-interpretation refusal is retained in
the acquisition records without success credit.

The [five provisional classifications](pixel-route-unused-candidates-20261018.json)
refer to existing recovered ranges, not new discovery-inventory function entries.
The central policy uses those exact recovered-range IDs. Original inventory
boundaries remain unchanged; twelve parser controls rerun with corrected bindings.
