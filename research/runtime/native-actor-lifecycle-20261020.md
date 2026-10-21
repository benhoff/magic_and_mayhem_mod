# Native actor visual lifecycle

ALIFE1, No-CD build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original instruction addresses and packed stores have high confidence within the
scope below. These are native entry implementations with explicit external service
boundaries, not a complete native simulation or replacement of every called manager.

`reconstruction/rendering/actor_visual_lifecycle.hpp` owns the recovered lifecycle flow.
Its backend supplies PE32 address-valued memory, allocation, and typed calls to
existing services. The runtime backend is independent of Qt and renderer widgets.

| Entry | Native behavior |
| --- | --- |
| `505db0`, `505f70` | Packed `e4b` record construction and destruction, callback/list ownership and histogram array release |
| `5060a0`, `52a5c0` | One/all-slot reset, callback alias at `708/cf3`, list sentinel recreation, manager-list erasure |
| `473100`, `4bfef0`, `4c00d0` | Pool preparation/construction, count-prefixed backing allocation, reverse element destruction and manager-list reclamation |
| `52a640` | First inactive slot, without reservation or compaction |
| `506290`, `5065e0` | Activation, map attachment, high-water publication, baseline state, draw history, facing, attached-list retirement and registrations |
| `51f5e0` | Cleanup, peer/secondary notifications, selected attachments and voice handles |
| `506d30` | Logical release, current/saved occupancy detachment, tail-bound shrink, shared type resources and recursive release-then-cleanup cascade |
| `52baf0` | Presentation retirement across capacity, including inactive records, audio virtual calls and voice stops |

Cleanup does not clear the pool-active flag. Release does not destroy slot storage.
Neither operation is an idempotent deletion API. Pool destruction does not first
perform gameplay cleanup/release for every record. The original ordering remains
visible to external services; later fields, pointers and bounds are reloaded after
callbacks where the recovered code reloads them.

Queue rows and particles retain borrowed actor addresses. Slot reset/reuse keeps
the same backing address and index; no generation repair is introduced. Type
records and animation handles are borrowed, while the classified wizard release
branch owns selected type arrays and four sprite references. It reclaims those
resources before cascading through other active actors of the same type. Queue
destruction must not destroy the row's actor. Arbitrary terrain loading and every
external resource owner are separate contracts.

Two original details matter for repeat sessions:

* Preparation resets selection/scheduler/iterator fields but does not reset the
  manager's `+228` high-water bound. The enclosing session establishes that state.
* Reset clears the histogram pointer without invoking its destructor. The owned
  fixture records the resulting retained generations. Native baseline recovery
  preserves this original behavior; it does not silently add a new reclamation
  policy. Ordinary destruction releases a still-referenced histogram array.

`runtime/actor-lifecycle/adapter.cpp` installs thirteen native entry stubs only
after matching all 7,138 original body bytes and preparing every page protection.
A protection refusal occurs before instruction mutation. Invalid memory ownership
terminates explicitly. Zero map divisors terminate instead of entering original
infinite subtraction loops. The original default wizard resource-index branch
uses an uninitialized stack word; the native implementation refuses an unclassified
wizard type. These refusals are native policies, recorded separately from recovered
valid-context behavior.

External map occupancy, animation, AI/owner/session, secondary-effect, histogram,
allocator, audio and COM services retain their existing entry boundaries in the
runtime adapter. This change supplies lifecycle orchestration and its owned stores;
it does not claim that those service bodies have all become native. Exception
unwinding and original allocation-failure behavior are outside the compared domain.

Use both explicit options to combine the earlier drawing helpers with this work:

```sh
python3 tools/capture-scene-game.py --native-creature-drawing --native-actor-lifecycle
```

Both staging and capture support `--native-actor-lifecycle`; defaults remain opt-in.
The generated DLL source manifest is checked against capture's prospective source
claim. Import staging preserves all original section bytes. No original file is
patched in place. Reproduce guards with `tools/update-actor-lifecycle-guards.py`.

The comparison runner is `tools/test-actor-lifecycle.py`. Run it through the main
checkout's storage guard, with a new output under this checkout's `working/tests`.
The latest durable result is `native-actor-lifecycle-master-capture-results-20261020.json`;
the initial `native-actor-lifecycle-results-20261020.json` remains unchanged.
Compressed
full traces, compiler output and failed diagnostic attempts remain under `working/`.
It records 192 composed cases across six types, sixteen external-provider scenarios
and both cleanup modes, two-node list retirement at activation and destruction,
resource cascades and same-address reuse. Every external service boundary snapshots
complete owned allocations and actor state. Original, native core and installed
runtime entries must produce identical ordered traces. Native core runs poison
the original lifecycle bodies; there is no original-body fallback.

The fixture's external services and Windows APIs are explicit owned providers.
It supplies scoped original comparisons and runtime entry execution, not physical
Windows-provider validation, actual-game admission, whole-scene equivalence or a
live replacement milestone. Stable behavior IDs are `RS.actor-pool-storage`,
`RS.actor-activation`, `RS.actor-release-cascade` and
`NP.native-actor-lifecycle-adapter`. Earlier creature evidence remains historical
when shared launch-tool fingerprints change.

Master integration retains both earlier results unchanged. The distinct visual
header avoids replacing the separately developed ownership model and its API.
The combined run repeats all 192 cases and 7,138 guard mutation controls.
The original include path remains as a forwarding compatibility header, preserving
historical evidence path readability. The fixture compiles both include paths.
Capture declares the renamed visual header. The comparison runner evaluates the
actual capture source declaration and compares it with the generated PE32 builder
manifest, rejecting any omitted compiler input or prospective hash mismatch.
