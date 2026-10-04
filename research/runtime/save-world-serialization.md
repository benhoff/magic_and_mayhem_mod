# Save-world serializer evidence

Reviewed 2026-10-04. Static paired-reader/writer evidence for No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred PE image base `0x00400000`. [Physical grammar and native interface](../formats/save-world-native-loading.md).
Addresses below are build-specific. They are not native service dependencies.

## Functions and reference conversion

| Area | Writer | Reader |
| --- | --- | --- |
| World root | `0x004713a0` | `0x00471af0` |
| Registry | `0x004a0ea0` | `0x004a0f70` |
| Map | `0x00501a60` | `0x00501e00` |
| Secondary map | `0x004ffc50` | `0x004ffcb0` |
| Records-40 | `0x004e0fd0` | `0x004e1050` |
| Eight state slots | `0x004669c0` | `0x00466a30` |
| Individual state slot | `0x00465870` | `0x00465a50` |
| Creatures | `0x0052bcb0` | `0x0052c160` |
| Individual creature | `0x00524df0` | `0x00527310` |
| Creature command | `0x0052c9d0` (base `0x0052c620`) | `0x0052ca70` |
| Creature word lists | `0x0054de20` | `0x0054dfc0` |
| Missiles | `0x0049d990` | `0x0049dcf0` |
| Individual missile | `0x0049aa90` | `0x0049b3d0` |
| Effects | `0x00544ee0` | `0x00544fb0` |
| Individual effect | `0x00544020` | `0x00544350` |
| 104 path slots | `0x0057b670` | `0x0057b6c0` |
| Optional records-660 | `0x00542cd0` | `0x00542f60` |
| Optional records-6 | `0x005346f0` | `0x00534850` |
| Records-1148 | `0x0056cbe0` | `0x0056cd40` |
| Resource slots | `0x00464120` | `0x004641b0` |
| Individual resource | `0x00463a10` | `0x00463b40` |
| Animation state | `0x00465180` | `0x004652a0` |
| Map lists-40 | `0x005334b0` | `0x00533550` |
| Map list-72 | `0x0049f3f0` | `0x0049f490` |

`0x00501940` / `0x00501970` convert map cell pointers and ordinal values;
null becomes -1. Entity references often serialize a pointed-to first DWORD
ID, with -1 for null. Other references are resource-relative indices rather
than entity IDs. Native ranges retain original integers without validating
referents. Rebuilding these references requires resource/pool reconstruction.

Creature name predicate `0x00411e50` tests types 0/24/25/26 via the type object.
Its reader uses serialized type and rebuilds the type pointer. Animation reader
requires loaded ANI resources. These host dependencies are not implemented by
structural parsing.

The 104 path-slot count follows `(0x006c3750 - 0x006b1268) / 0x2d1` in the
original loop; disk slots are 256 bytes, not the host stride. Map writer return
accounting uses a large constant that differs from its physical write sizes.
Creature command writer return values are not accumulated by the parent.
Do not use accumulated return accounting to locate following records.

## Isolated original writer experiment

`tools/test-save-world-writers.py` hash-checks the executable, compiles
`tests/save-world-writer-reference.cpp` with `-m32 -fno-pie -no-pie`, then
script-patches only a disposable `working/tests/save-world/run-*/capture.exe`.
The five bytes at fwrite entry `0x0059c24f` become a relative jump to a bounded
host capture callback. The original bytes, patch bytes, callback address,
source/copy/host hashes and each captured stream hash are recorded. Original
serializer bodies are retained; original input files are never modified.
Manifest verification passes before and after. The harness maps the PE in a
separate Linux process, supplies synthetic receiver storage and does not launch
a game session or call the complete world writer.

Recorded run: `working/tests/save-world/run-knijjno2/report.json`:

| Serializer cases | Captured physical bytes |
| --- | --- |
| Creature inactive / active ordinary / active named-type with conditional state | 8 / 2741 / 2965 |
| Missile inactive / active / active attached | 8 / 506 / 514 |
| Effect inactive / ordinary active / type 50 / type 61 | 8 / 74 / 70 / 70 |

Creature conditional case includes a zero-length name, mode nonzero, byte-gated
animation, three DWORD-gated animations, nested word lists and a 204-byte
extension. Animations in captured cases are null; non-null animation states,
nonempty names, commands and kind-dependent extra references have synthetic
and static evidence only. Missile resource search count is set to zero to
exercise the missing-resource ordinal branch safely. Effects 50/61 omit animation.

All ten captures pass native extent parsing and packed/unpacked synthetic-save
integration (`native-comparison.json`). This checks captured records as input,
not equivalence of native serialization or restoration. The sandbox rejects
32-bit host execution with SIGSYS; the successful run used the authorized
isolated execution outside that restriction. No live observation or replacement
is claimed.

## Remaining boundary

Original campaign and active-battle saves were not available in the installation.
Complete world writer execution, full original reader equivalence, missing or
changed resource handling, pointer/ordinal validity, simulation restoration and
save round trips remain unverified. Fixed script-state bytes are retained by
the campaign reader; script/trigger semantics remain a separate recovery task.
