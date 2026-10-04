# Save/load and campaign progression (No-CD build)

Reviewed 2026-10-04. This is bounded **static research**. Subsequent [offline native readers](../formats/persistence-native-loading.md)
have synthetic/installed-config evidence; there is no live persistence service,
live observation, or demonstrated save compatibility. It covers the
save envelope, serializer dispatch, realm initialization, battle-return effects,
and the configured realm transition. Original code still owns these behaviors.

## Evidence and reproduction

- Input: `working/game-nocd/Chaos.exe`, PE32/i386, preferred base `0x00400000`.
- SHA-256: `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
- Method: bounded Intel disassembly and direct references, with read-only
  Ghidra 12.1.4 pseudocode cross-checked against instructions. Installed
  `Realms/{Celtic,Greek,Medieval}/RealmView.cfg` supplies independent data evidence.
- **High confidence** means explicit bytes, stores, branches, sizes or call
  order. **Medium confidence** means a descriptive gameplay interpretation.
  Labels below are ours, not recovered source symbols. Unnamed fields remain
  unnamed rather than being assigned speculative gameplay meanings.
- Addresses are preferred-image VAs for this hash. Allocated pointers and
  runtime thread ownership have no established stability across launches.

```bash
./tools/original-manifest.sh verify
python3 tools/export-persistence-progression.py
./tools/original-manifest.sh verify
```

The exporter checks the hash and instruction anchor before disassembly, exports
bounded ranges plus direct call/jump contexts, hashes every artifact, and rechecks
the input hash. Its `realm-configs.json` records installed configuration hashes
and values when those files are present. Direct textual references do not cover
indirect callbacks. Some ranges include adjacent context; they are not all exact
function extents. Objdump can decode inline data as instructions.

Initial instruction evidence is
`working/decompiled/persistence-progression-b4bptod9/`; the loader follow-up
exports 45 ranges under `working/decompiled/persistence-progression-hmv3gw3a/`.
Nested wizard/generator pseudocode is in
`working/decompiled/persistence-loader-schema-0_dr4pkp/`. Optional pseudocode was collected in
`working/decompiled/persistence-selected/`, `persistence-deep/`,
`progression-result/` and `progression-outcome/`. These are disposable artifacts.

For optional regeneration, use an existing analyzed No-CD project with
`-process Chaos.exe -readOnly -noanalysis -scriptPath tools/ghidra -postScript
ExportPersistenceProgression.java NEW_OUTPUT_DIRECTORY ENTRY_HEX ...`.
Use actual entry points from the tables below. Pseudocode signatures often
omit ECX receivers; assembly is authoritative. In particular, the realm tick
starts at `0x005517a0`; `0x005517e0` is not a function entry.

Validation on the review date: all 40 original exported ranges were nonempty, all 42
artifact hashes matched, the three installed realm configuration values and
fixed block-size arithmetic were checked, and an unsupported input was rejected
before disassembly/output creation. The new Ghidra script exported five exact
entry points successfully in a read-only project; its log and pseudocode are
under `working/decompiled/persistence-script-check/`. Document links and
`git diff --check` passed. Original-manifest verification passed before and
after the analysis for all 2,927 immutable files. These checks establish
reproducible static evidence and input preservation, not save/load equivalence.
The loader follow-up also verified all 47 artifact hashes from the 45-range
export and passed the offline tests recorded in the native-loading document.

## Save and load entry points

| Role | Entry | Confirmed contract |
| --- | --- | --- |
| Load named save | `0x004ea280` | ECX filename stem; constructs `Save\\` + stem + `.sav`; returns 0/1 |
| Write named save | `0x004ea5a0` | ECX filename stem, EDX overwrite admission; returns 2 for an existing readable destination when EDX is zero, 0 on selected failures, 1 after finalization |
| Load menu request | `0x004aa5d0` | Pending byte `+0x64` consumed; selected text at child `+0x45` passed to loader; success sets flags and fades/returns |
| Save menu request | `0x004abd00` | Trims leading ASCII spaces, rejects empty stem; passes `+0x5f` as overwrite admission; result 1 fades/returns |
| World update ingress | `0x0046afc0` | Selected load/save request flags dispatch near `0x0046b04a` / `0x0046b0c5`, before ordinary world work |

The world path uses stem `log` (`0x005d9fe4`) and passes overwrite admission 1
for saving. This is a concrete `Save\\log.sav` caller, not proof of an autosave
schedule or a replay format. Its triggering producers need separate tracing.
Native widgets do not invoke these recovered paths yet.

## File finalization and error boundaries

The final named file is `.sav`; `Save\\__temp.vas` is an intermediate stream.
The [save format notes](../formats/save-game.md) describe the two layers.

Writer order (high confidence within inspected branches):

1. If overwrite admission is zero, probe the final path with `rb`; an existing
   readable file produces result 2.
2. Open the final path with `wb`, then close it **before** opening the temporary
   path with `wb`. The destination can therefore be truncated before the save
   completes. This is not an atomic transaction.
3. Write the decoded header, campaign blocks and optional battle section to
   the temporary stream. Seek back and rewrite the header's accounting field.
4. Reopen/read the temporary stream, then open the final path and run the
   packing/obfuscation writer `0x004a17c0`.
5. If that wrapper reports success, close it and delete the temporary file.
   Otherwise close it and attempt `MoveFileA(temp, destination)`.
6. Return 1 from this final branch without checking the delete/move API result.

Consequences: result 1 does not prove durable successful persistence. Selected
earlier failures return 0; temporary cleanup on every failure is not established.
There is no verified crash recovery, flush guarantee, or safe overwrite policy.
A native implementation should expose its own explicit failure/durability policy,
while documenting any compatibility differences.

The loader reads the final file through `0x004a1820`, materializes a decoded
temporary file, then reads it with CRT calls. It requires a 12-byte header and
version `0x14`. Its inspected top-level path does not compare the `SAV` signature
or enforce the header accounting field. It then reads the blocks in order and
deletes the temporary file on both the selected success and failure paths.

Load is **not staged into an isolated world**: earlier globals/managers have
already been mutated when a later read fails. Failure leaves rollback semantics
unproven. `0x004a1820` also does not propagate all decoder failures as a failed
return: its decoder-zero branch copies input bytes. Do not treat this wrapper
as a safe parser for arbitrary files or infer strict checksum rejection at the
top-level loader from the underlying checksum routines.

## Persistent owners and restoration

| Block | Write / read | Scope and boundary |
| --- | --- | --- |
| Wizard records | `0x00594290` / `0x005942d0` | Loops over 80 records at manager `0x0065b250`; record serializers `0x00592e90` / `0x00593560` use fields plus nested serializers, not one raw struct dump |
| Realm state | `0x0054e750` / `0x0054e720` | One raw `0x13dc`-byte block at `0x00659e51`; returns byte count or -1 |
| Controller persistent arrays | `0x00577580` / `0x00577610` | Three groups at receiver `+0x84`, `+0xb4`, `+0x1b0`: 3, 63 and 25 DWORDs; total `0x16c`. Their full semantic mapping is open; this is not a claim that UI pointers are serialized |
| Script-related state | `0x0056d200` / `0x0056d270` | At `0x006f37f8`: 200 records of `0x40`, then `0x3c` and `0x2c` bytes; total `0x3268`. Read frees/reallocates record storage and initializes it before reading |
| Optional world | `0x004713a0` / `0x00471af0` | Receiver `0x006cbb78`; selected map/clock/pool/UI state plus nested serializers; complete field schema remains open |

The world writer begins with a `0x100`-byte receiver field at `+0xfc5a`,
four DWORDs from `0x006e980c`, nested `0x004a0ea0`, the world counter
`0x006c4830`, then `0x00501a60` and `0x004ffc50`. It converts selected queued
pointers with `0x00501940`; the loader reverses these with `0x00501970`.
That pair is explicit reference restoration evidence; its complete identity
domain is still unreviewed.

Further paired callees include `0x004e0fd0`, `0x004669c0`, `0x0052bcb0`,
`0x0049d990`, `0x00544ee0`, `0x0056cbe0` and `0x00464120` on write.
These are migration dependencies, not established contracts merely because
their caller was reviewed. The reader loads base assets/map context, rebuilds
lists and invokes initialization helpers. It explicitly reloads
`sprites\\lordking.spr` / `.ani` before restoration.

This connects persistence to [entity lifetimes](entity-lifetimes.md),
[world update ordering](world-tick-loop.md), map loading, and animation/audio
resource recreation. A native port needs owned identities and resource rebinding;
copying PE32 pointers or packed host objects is not a persistence design.

## Realm initialization and progression

`0x0054e230` reads `%s/realms/%s/realmview.cfg` using Windows profile lookups.
The realm state at `0x00659e51` has these statically supported fields:

| Relative offset | Meaning / evidence | Confidence |
| --- | --- | --- |
| `+0` | Current realm name, `currentRealmName`, up to `0x100` bytes | High |
| `+0x100` | `playerWizard1` index | High |
| `+0x104` | `wizardCount` | High |
| `+0x108` | `lastRegion`, default -1 | High |
| `+0x10c` | `nextRealm` path, up to `0x100` bytes | High |
| `+0x84c + i*0x10` | Wizard name storage initialized to `< wizard name >` | High |
| `+0xe8c + i*4` | `wizardIcon` | High |
| `+0xd4c + i*4` | Wizard eligibility/alive-like field initialized to 1 | Medium semantics |
| `+0xfcc + i*4`, `+0x110c + i*4` | Location / requested destination, both initialized by `wizardLocation` | High initialization; medium dynamic role |
| `+0x124c + i*4` | `wizardFlag` affiliation | High |
| `+0x138c + r*4` | `regionOwner_%02d` from `REGION_INFO` | High |

The array offsets above were corrected during loader implementation by
cross-checking the ECX receiver and DWORD-index arithmetic in `0x0054e230`:
the destination base is `+0x124c`, and index displacements -0x140, -0xf0,
-0xa0 and -0x50 are DWORD counts. Names occupy +0x84c..+0xd4b (80 * 16 bytes),
then the five 80-DWORD arrays occupy +0xd4c..+0x138b. The previous table
incorrectly subtracted some indices as byte offsets. Confidence: high from
instructions and nonoverlapping block arithmetic.

`regionCount` is a local input to initialization, not a newly identified
persistent count field. Some numeric inputs are clamped; absent keys do not
uniformly produce initialized values. These routines do not establish robust
bounds checking for arbitrary native configurations.

Installed configuration evidence:

| Realm | Wizards | Regions | Last region | Next realm |
| --- | ---: | ---: | ---: | --- |
| Celtic | 9 | 10 | 9 | `realms\\greek\\greek.cfg` |
| Greek | 13 | 15 | 14 | `realms\\medieval\\medieval.cfg` |
| Medieval | 17 | 17 | 16 | Empty |

The comments describe conquering the last region as the next-realm trigger.
This is confirmed intended data semantics; it is not live end-to-end campaign
evidence. Region/wizard indices should not be assumed interchangeable:
initialization uses zero-based wizard sections, while selected region loops
start at 1 and use a zero sentinel.

Main callback `0x004a75c0`, campaign branch at `0x004a7611`, sets realm string
`Celtic`, resets wizard/controller/script state and progression counters,
initializes realm state and requests Realm View. Exact behavioral adaptation
remains separate from this static evidence.

## Battle return, ownership and next realm

Realm View tick `0x005517a0` uses a small state machine at receiver `+0xa2b`:
selected states 0–4 handle normal view, admission/setup, battle-screen push,
realm change and launch. `0x005510d0` builds a realm/region configuration request
and pushes the world screen at `0x006cbb78`; when a world already exists it uses
the resume/push path instead of creating it again.

After a launched world returns, tick detects/clears `+0xa48` and calls
`0x00550e10`. Its result field `+0xa27 == 1` branch identifies a winner-like
wizard from the embedded realm snapshot and processes other eligible wizards
with matching affiliation and a zero movement-state field. It clears selected
eligibility fields and transfers their owned regions to that wizard, updates
flag presentation, and marks selected adjacency-related knowledge entries.
**High confidence** for these stores; **medium** for the winner/elimination
labels until the producers of `+0xa27` and the embedded snapshot are traced.
The other branch restores a requested destination from prior movement state
and calls `0x0054e7e0`. That helper checks location, adjacency and region occupancy
before starting a movement/presentation transition.

`0x0054e690` counts eligible wizards sharing a location; callers reject selected
admissions at count >= 4. `0x0054e6d0` checks another eligible wizard with the same
location while excluding one index. These are concrete campaign admission rules,
not native commander-order behavior.

The realm-change state (3) copies the nonempty `nextRealm` path from an embedded
snapshot into the saved path block at `0x006ddd58`, calls configuration loader
`0x004ca370`, reloads `RealmView.cfg`, and tears down the old view using
`0x00550370`. It then calls `0x00593ad0(8)` for Greek or `(0x14)` for Medieval.
That helper stores receiver `+0x100` and selects `+0xfc` by searching a table;
the exact reward/progression meaning is **unknown**, so these are not claimed
spell unlocks, experience awards or veterancy. The producer of realm-change
byte `+0xa3b`, complete victory/defeat result production and final ending remain
open.

The inspected battle-result callback `0x004747a0` only calls shared fade helper
`0x00557510` and sets return flag `+0x43 = 1`. Progression belongs to session/world
and realm-return code, not to that button or its native widget.

## Next bounded migration steps

1. Capture newly created campaign-only and battle saves in a disposable working
   installation, with original manifest checks before/after. Record input hash,
   map, actions, writer result and file API outcomes. No live save was created
   for this research; no `.sav` fixture was present in the inspected No-CD tree.
2. Decode both envelopes read-only and compare original decode output. Test
   truncation, bad version/checksums, overwrite refusal, interrupted writes,
   write/move failures and trailing bytes separately. Resolve header accounting.
3. Map wizard nested blocks and world serializer pairs, stable IDs, pool counts,
   script variables/triggers, RNG/timers and resource rebinding. Start with a
   campaign-only inspect/export round trip, then add one idle battle snapshot.
4. Observe battle launch/return and producers of `+0xa27`, `+0xa3b`, and the
   embedded realm snapshot. Compare ordinary victory/defeat, last-region conquest,
   realm transition and final ending, including cancellation and repeated returns.
5. Build native user-state storage separately from read-only assets. Define
   versioning, bounded validation, staged restoration and an explicit commit
   policy. Keep original-compatible import/export separate from a future native
   save schema; validate full state before replacing original loading.

No campaign, save, configuration, gameplay balance, animation or audio code was
changed. Coverage remains static research; see GP06/GP07 in the
[coverage ledger](coverage-ledger.md).
