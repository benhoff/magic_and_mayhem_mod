# Campaign Mini Preferences bridge (UI34)

V12 connects the fresh campaign Mini Menu's Preferences button to the original
engine. The channel retains V6 Preferences fields and actions with a new version
and size (106496 bytes); older versions retain their existing capabilities.
This chunk has isolated original-bytecode and headless Qt validation. Full V12
campaign lifecycle execution remains pending; no manual testing is required by
these checks and no live equivalence or engine replacement is claimed.

## Recovered contract

For pinned NoCD build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
Mini callback `0x4b23f0`, local index 2, queues Preferences singleton `0x6a4948`
in Mini's pending pointer (+0x33), sets Mini's return flag (+0x43), and returns
zero. Common tick `0x5595d0` prioritizes this push before the return flag.
Mini suspend `0x4b1f30` destroys its controls through leave `0x4b1ed0` and
`0x4b2010`, clears the pending pointer, and retains the return flag.

The supported Preferences caller is therefore Mini ID17 at depth6, above the
exact fresh Main/Realm/World/Mini stack, mode2, context5 and campaign layout0.
The adapter checks those identities plus the original Preferences arrays,
callback receiver, slider ranges, radio groups and per-control availability.
Other callers retain original ownership. Main's existing V6 path remains valid.

Preview uses original slider setter `0x4cdf40` and callback `0x4a9700`.
Preferences Cancel `0x4a9840`, local1, restores entry audio (+0x68/+0x6c), starts
the original fade and returns without writing preferences. OK, local0, applies
original groups/sliders and invokes the original writer `0x54c890` before
starting the return. Qt saves accepted settings only after the callback ACK and
only if the engine-written configuration matches those settings.

Static recovery of leave `0x4a8b60` identifies World setup/teardown dependencies
`0x4711d0`, optional resolution rebuild `0x4a88f0`, `0x470ee0`, `0x4d37c0` and
`0x4d3b40`; their bodies remain original and opaque. Common pop resumes Mini
through `0x4b1f20`/`0x4b1f40`, which retains its return flag. Mini then pops to
World. The adapter excludes that transient Mini until the original fresh
Escape initialization `0x4b1eb0` resets it. Confidence is high for these selected
static branches; their complete live timing and dependency effects are pending.

## Validation and boundaries

[Machine-readable evidence](campaign-preferences-engine-bridge.json) retains
fresh source fingerprints and the disposable fixture directory. Run
`python3 tools/test-campaign-preferences-bridge.py` for pinned original Mini
entry, preview, Cancel rollback, OK/group changes, synthetic writer count,
once-only dispatch, wrong context/layout/stack/depth/mode, Main compatibility,
and stale/invalid request rejection. Original setter/group/callback bytecode
runs at its expected virtual addresses in private PE32; audio, writer, display,
fade and outer engine dependencies are synthetic. Full hook installation and
the outer lifecycle are not exercised by this fixture.

Six targeted Qt checks include `qt-menu-campaign-preferences`, the existing V6
bridge/controller, preference store, V10 Mini Quit and V11 defeat. The new test
uses a synthetic wire peer to check Mini capability, same-thread fresh admission,
preview acknowledgement, Cancel return ownership, rejection of transient Mini,
fresh Mini readmission, original availability and persistence after OK ACK.
The complete shell builds successfully. No game process runs in the Qt tests.

Resolution rebuild, game-speed timer/pause cadence, loaded campaigns, Realm and
Quick Battle callers, full original gameplay return, and native replacement
remain pending. Historical evidence retains its original hashes; shared source
changes make earlier results stale without renewing their validation claims.

## Later live validation (UI35)

The selected effects/dialogue Preferences lifecycle is now observed in the [automated native campaign round trip](campaign-preferences-live-engine-bridge.md): Cancel rollback, OK persistence, reopened values, three original World resumes and normal Quit. This separate result retains UI34 evidence and does not renew its old fingerprints or establish timer/resolution/other-caller equivalence.
