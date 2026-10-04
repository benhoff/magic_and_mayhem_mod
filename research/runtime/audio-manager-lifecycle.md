# Aggregate audio manager lifecycle

Selected static reconstruction plus synthetic offline integration. The pinned
NoCD SHA-256 is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high for listed control flow, call order and heap ownership;
original live equivalence has not been measured. No game or sound output device
was launched.

Implementation: [manager_lifecycle.hpp](../../reconstruction/audio/manager_lifecycle.hpp)
and [manager_lifecycle.cpp](../../reconstruction/audio/manager_lifecycle.cpp).
The controller composes the existing catalog, primary setup/control, source pool
and schedule models. Backend interfaces remain independent of widgets; neither
native asset storage nor native audio depends on this build-specific controller.

## Evidence

`./tools/export-audio-manager-config.py` now exports destructor `0x56de50` as
well as initialization `0x56df60`, shutdown `0x56e640`, primary setup `0x56fe80`
and configuration/pool support. The guarded run produced
`working/decompiled/audio-support-2o7a86ga/manifest.json`, with assembly/strings
artifact hashes. Immutable-input verification passes before and after export;
the working executable hash is checked and no binary is changed. Cached
`working/decompiled/audio-map-jgd64h_b/program.asm` also shows CRT array
teardown `0x59bf07` visiting group descriptors in reverse index order.

| Block | Confirmed behavior |
|---|---|
| `0x56df60..0x56e48b` | Store application/window/root/profile; access check; catalog loading |
| `0x56e490..0x56e4d8` | Default, nonaggregated device creation; initialized flag before level-2 cooperative call |
| `0x56e4da..0x56e532` | Reset primary/size, setup, capture saved volume, looping Play, active only on success |
| `0x56e534..0x56e567` | Shutdown on post-device failure, then unconditional outer class/ID frees |
| `0x56e569..0x56e5a1` | Publish global device, read/store simultaneous limit, allocate scheduler |
| `0x56e640..0x56e7f0` | Volume restore, active schedule sweep/Stop, sources/globals/device/tables teardown |
| `0x56de50..0x56defa` | Disabled wrapper destruction before shutdown, scheduler allocation free afterwards |

## Startup and failure ownership

`initializeManager` is supported for constructor state or a fully shut-down
manager. Application/window and backend tokens are host annotations, not process
pointers. The profile adapter binds subsequent decoded reads to `root\Sounds.ini`.
Catalog allocation observers distinguish original allocated table ownership from
empty host vectors. Original class contents are unspecified until map pool setup.

Access or source-table validation failure returns E_FAIL without device work.
A later group-section failure can leave source/class tables allocated while the
manager remains uninitialized. No general transaction rollback is invented.
Device-create failure frees only classes and source IDs, returns the exact
nonzero HRESULT, and retains group tables. The uninitialized shutdown gate skips
those groups even when called by the destructor. Host RAII eventually reclaims
its annotations; that does not establish an original heap cleanup.

Post-device setup/control failures call shutdown, then outer `free(NULL)` for
class/source pointers already cleared by shutdown. Logical free callbacks record
that order without double-freeing C++ storage. Caps/format/compact/GetVolume/Play
failures retain the primary token and return the triggering HRESULT when the
shutdown dereference domain is valid. Restore/Stop errors never replace that
startup error.

The original shutdown unconditionally dereferences primary `+0xc` once
initialized. A failed cooperative-level call with no valid prior primary, or a
failed primary create whose output remains null, therefore has no confirmed safe
unwind. The model raises `domain_error` at that boundary and preserves preceding
state rather than claiming a clean return/release. A valid prior primary enables
the cooperative failure cleanup path. The original constructor does not initialize
all device/primary/saved-volume fields; tests explicitly supply their initial
host state and do not infer those original bytes were zero.

The primary caps query HRESULT is ignored. If its size word is unknown, the host
records `primaryBytes = nullopt`; it does not invent a zero size. Compact errors
still propagate. Global device publication precedes limit parsing/allocation;
unsupported count-0/1 scheduler domains preserve that partial publication.
The constructor's global manager registration is caller-supplied. Initialization
does not republish it after shutdown, matching the selected instructions.

## Shutdown and destruction

An uninitialized manager does no shutdown work. Otherwise:

1. Restore saved primary volume; ignore the HRESULT.
2. When active, traverse from the current schedule head. A nonzero voice is
   retired if its unsigned deadline is future or `0xffffffff` (looping).
   Expired voices are skipped for retirement. Their nonzero output slots are
   nevertheless zeroed. A voice-less stray output slot is not dereferenced.
   Every record resets deadline/volume/voice/output/x/y while retaining links.
3. Clear active and global last-Stop status before primary Stop; retain its
   returned status in the host/global annotation. Continue even on error.
4. Walk the source ring backward from the tail, destroying duplicate descendants
   before root buffer/free, then clear ring ownership.
5. Clear global device/manager tokens, release a nonzero device, free classes,
   source IDs, group IDs, then reverse-order member arrays and descriptor storage.
   Clear initialized last. Table counts remain; freed pointer ownership clears.

Inactive shutdown skips the schedule sweep and primary Stop but still restores
volume and destroys owned sources/device/tables. Shutdown does **not** release
or zero primary `+0xc` and does **not** free scheduler allocation `+0x25c`.
The model records these retained resources instead of assigning guessed release
calls. Ownership outside these recovered functions needs separate evidence.

`destroyManager` first destroys the disabled wrapper/descendants, calls shutdown,
then frees scheduler allocation. Host destroyed/sentinel flags prevent repeated
C++ disposal; that idempotence guard is a host safety policy, not an original
repeated-destructor guarantee. Retained group allocations after uninitialized
failure stay visible. Reinitializing partially failed ownership is rejected;
the original overwrite/leak behavior on such retries is outside this model.

## Validation and remaining work

Run `./tools/test-audio-manager-lifecycle.py`. Final evidence:
`working/tests/audio-manager-lifecycle/run-h7qqfigj/report.json` and
`sanitizer.log`. All 26 current audio/assets CTests pass; address, undefined
behavior and leak sanitizer checks pass. Fixtures exercise successful startup,
positive/negative exact failures at six stages, unsafe absent-primary domains,
valid prior-primary cooperative cleanup, empty groups, early access/group exits,
ignored caps-query errors, unsupported scheduler limits, per-map pool integration,
shutdown deadlines/stray slots/inactive behavior, duplicate-before-root release,
restart and destructor ordering. These tests use decoded synthetic profile results
and backend call traces, not original-generated lifecycle captures.

Windows profile/CRT file semantics, real allocator/SEH faults, COM reference counts,
original saved-volume initialization and external primary ownership remain
unverified. Retirement delegates to the recovered voice/lifetime backend; no raw
process pointers are dereferenced. Device/thread/timing, audible transitions and
live replacement remain separate milestones. Next offline work is a concrete
native backend adapter connecting Qt asset/profile reads and native audio services
to this controller, with end-to-end synthetic PCM evidence.
