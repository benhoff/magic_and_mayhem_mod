# Application copy route census V1

`NP.native-copy-route-census` observes application-facing Blt/BltFast callbacks before terminal RenderStop
only when resident World and `MNM_NATIVE_COPY_DIAGNOSTICS` are enabled. The
mapping is `lock-capture/copy-routes-v1.bin`, created with CREATE_NEW; it never
supplies rendering inputs. All words are little-endian unsigned32. Process
object tokens and return PCs are evidence identities, never dereferenceable
host pointers or assumptions about future process addresses.

The file is exactly10,485,888bytes:128header bytes followed by65,536rows of
160bytes (40words). The protocol header checks C/C++ sizes. Mapped creation
failure leaves graphics admission unchanged; a missing file cannot pass evidence
validation. The existing ordered application-copy lease serializes row writers.
Skipped active command/copy callbacks or a failed tracker snapshot set sticky invalid
and missed markers. Every published row follows its complete scalar snapshot.
Capacity/attempt-counter exhaustion makes evidence invalid without counter wrap
or changing a graphics result. A normal DLL detach marks closure; the campaign
separately requires normal game Quit, balanced ownership and private Wine stop.
This is not an independent semantic COM/context-loss teardown proof.

## Header words

| Word | Meaning |
| --- | --- |
|0–1|ASCII `MNMCRT01`|
|2–5|Version1, header128bytes, row160bytes, capacity65536|
|6|Published rows|
|7|Sticky invalid marker (overflow, exhaustion or missed observation)|
|8|Attempted row publications; must equal published rows|
|9|Sticky missed-callback/snapshot marker|
|10–13|GPU-prepared, CPU-prepared, unprepared and fill-prepared counts|
|14|Normal detach closure marker1|
|15–16|Observed executable module base and PE SizeOfImage|
|17–20|Self-alias, destination attached-clipper, requested source-key and GPU-source counts|
|21|Count of post-retirement command-lease bypasses, excluded from branch claims|
|22|Active missed-observation reason mask:1command gate,2copy lease,4tracker snapshot; must be zero|
|23–31|Reserved zero|

## Row words

| Word | Meaning |
| --- | --- |
|0|Publication ordinal, starting1|
|1–3|Application return PC, checked module-relative RVA (or ffffffff), thread ID|
|4–7|Requested target/source tokens, tracker target/source representative tokens|
|8–9|Frozen target/source generation before graphics execution|
|10|Frozen state mask below|
|11–13|Fast boolean, actual application flags, actual HRESULT|
|14–15|Preparation valid boolean; route0unprepared,1CPU,2GPU,3fill|
|16–18|Source width, height, bits per pixel|
|19–21|Target width, height, bits per pixel|
|22–23|Observed source key, destination clipper token|
|24–27|Preparation source rectangle (signed32 interpretation)|
|28–31|Preparation destination rectangle (signed32 interpretation)|
|32|FNV1a32 of the existing preparation reason string (offset2166136261, multiplier16777619)|
|33|Exact ordinal in the separate GPU-copy census, only for GPU-prepared calls|
|34–37|Frozen source generation origin/caller, target generation origin/caller|
|38–39|Tracker epoch, reserved zero|

State-mask bits:0source tracked,1target tracked,2source GPU history,3target GPU
history,4target clipper known,5target clipper attached,6source key known,
7same tracked surface (including observed aliases),8same nonnull raw interface,
9source CPU allocation present,10target CPU allocation present,11source RECT
pointer supplied,12destination RECT pointer supplied,13effects pointer supplied,
14source supplied,15tracker snapshot captured. CPU allocation presence does not
assert currency: source/target GPU-history bits keep stale storage explicit.

Geometry represents only what the existing preparation path inspected; early
untracked/refused calls can leave default/partial rectangles. The observer adds
no rectangle dereference or COM metadata query. Preparation is distinct from
actual HRESULT and independent destination proof. CPU-prepared calls are routing
observations, not an assertion of original-driver output equivalence.

`tools/native_copy_routes.py` rejects nonclosed/incomplete/overflowed evidence,
unknown alias/clipper state, invalid identities, counts/tail bytes and non-bijective GPU linkage. Every
GPU-prepared call must match the separate census caller/RVA, source/target tokens,
generations, alias/clip/key flags and actual result. Existing GPU diagnostics
separately compare complete native destinations with independent expectations.
Zero branch counts establish only absence from the closed active interval.
The terminal command lease marks later bypasses explicitly retired; those calls
are counted without inspecting destroyed tracker metadata. A retired-tail counter
cannot wrap; exhaustion invalidates evidence. Unknown tail branches are retained
as an exclusion, without a complete renderer or post-retirement drawing claim. Unknown
higher callers, indirect flow, other configurations and all-game reachability
remain outside this format.
