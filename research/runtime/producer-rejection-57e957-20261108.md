# Fixed primary opaque-wrapper producer rejection

The producer guard now admits exactly original return `0x57e957`, tag17, with
its ten pinned continuation bytes. The generator independently checks the29-byte
argument/call/cleanup/AX block and retains this entry even in an empty observed
stream. Unknown PCs, other tags, changed/missing bytes, source validation,
ownership and all existing production limits remain strict.

The original40209ca7 wrapper calls57de00 at57e952 after its indexed/black path
returns AX1. It forwards frame,x,y,shade,palette as five cdecl arguments, cleans
20bytes and masks EAX to16bits before the unchanged auxiliary tail. The existing
table omitted that continuation. This omission is independent of the missing
live operands: every such call was rejected before request capture.

## Direct regression and fix

The [pre-fix reproduction](producer-rejection-57e957-results-20261108/negative-01.json)
executes21384complete original-wrapper cases and reproduces5184exact primary
guard refusals. The [final current-source comparison](producer-rejection-57e957-results-20261108/positive-02.json)
accepts all5184fallbacks, with zero refusals or output differences across all
21384cases and6500736destination WORDs including prefix/suffix guards and row
padding. Low16AX, RET12wrapper/cdecl20callee cleanup and nonvolatile registers
agree. Deliberately nonzero native high-EAX bits are discarded by the original
mask. Both before buffers are authored seeds; original output is never a native
input. Complete original wrapper bytes stay unchanged. Only the selected callee
entry is redirected in the private mapped test image after checking its bytes;
the original executable file is never patched.

The finite domain is two existing indexed entries5947b2/59521a, a7x4indexed RLE
frame, nine signed origin pairs, two clips, eleven X/nine Y positions, shades
-127/0/127 and palette-chain ordinals0/1 with shift0/neutral127. A16x12canvas has
20-word stride. Empty auxiliary frames execute the unchanged blend-enabled tail.
This proves the primary continuation, including fully hidden and edge-clipped
cases; it does not admit unobserved secondary returns or a full volatile ABI.

[Affected existing controls](producer-rejection-57e957-results-20261108/black-guards-01.json)
pass393216black AX-normalization cases,3766production caller/tag/signature
refusals,4922production and4884regenerated sprite-context controls, with628
protected short spans in each. Production now has48ordinary callers/55pairs;
the empty-stream generated table has17/17. Both production/selftest DLLs compile.
Exact tested ELF/DLL/EXE bytes, manifests, sources and logs are SQLite-backed
artifacts in the final comparison. Positive01 is preserved; final02corrects its
hook-artifact collector paths and binds the complete final dependency list.

## Remaining boundary

No original-game start was used. The exhausted HS08nine-request/six-start cap
is unchanged. [The failed World590](cursor-hotspot-results-20261108/pilot-diagnosis-01.json)
remains negative: its rejected operands cannot be reconstructed, and that live
run was not replayed to normal completion. This fix removes the demonstrated
caller-guard omission; it does not certify full corner/pan reliability, installed
player integration, secondary wrapper sites, arbitrary palettes/formats, other
unsupported callers, full ABI or either final rendering decision.

The acceptance frontier stays114/156passed42pending. No additional acceptance
row is promoted and no historical source hash or failed result is refreshed.
