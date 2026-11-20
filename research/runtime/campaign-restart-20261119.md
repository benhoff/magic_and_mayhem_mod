# Campaign Restart Level

The native campaign Mini menu offers **Restart Level** after entering a level
through native Region Entry in the current session. Confirmation defaults to No;
Escape cancels it. Yes restores the engine-written level-start checkpoint.
Previous campaign progress and the original generated map are part of that save.
This is an intentional native policy, not a recovered original Restart command.

The checkpoint lives at `Save/__mnm_restart/level.sav` inside the disposable
installation. It is separate from named saves and is not copied into the user's
persistent save store. A new level replaces it. Loading another save, returning
to Main or exiting the World disables it. Older saves and levels entered through
unsupported original Region controls have no checkpoint; Restart stays disabled.
There is no attempt to reconstruct missing starting state from a midlevel save.

The original No-CD build remains pinned to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Native Region Enter arms capture. The first initialized World tick calls original
writer `0x004ea5a0` on the engine thread, before its original update. The adapter
checks the writer instruction anchor, result and private file extent. Restart
uses original Mini Load callback `0x004b23f0`, original edit setter `0x004cd0e0`
and original Load callback `0x004aa800`; the original loader, teardown, resource
restoration and rendering transitions remain authoritative. No simulation pool
is patched and the checkpoint is not recaptured on a restart or user Load.

`menu_v23.h` defines a versioned wrapper around an explicitly selected V13–V22
base. Its immutable base word preserves the existing feature gates; choosing V23
does not enable experimental progressed-Realm, character, spell or book paths.
Mini action bit32 offers Restart only with an available checkpoint. Action52 has
no argument, path or host pointer. Existing request generation, ACK, readiness,
lease and once-only dispatch guards apply. The widget emits semantic intent;
the controller owns confirmation, the session owns wire admission and the
build-specific adapter owns checkpoint creation and original Load dispatch.

`menu-campaign-restart-test` checks V23/base identity, optional checkpoint masks,
No/Yes, duplicate requests, stale confirmations, return ownership and Battle
exclusion. Existing Mini, Quit and Save/Load controls remain focused regressions.
`tools/test-campaign-restart.py` provides a bounded private-display live scenario:
fresh Celtic Adept entry, two No/Yes confirmations, original World-clock resets,
active creature slot/type/owner/HP/position/mana comparisons and fresh resident
presentation after each Load. The [live run](campaign-restart-20261119/live-06.json) passed both restarts: the
original clock restored to zero twice, and all recorded fields of the one
initially active wizard matched at both original Load boundaries. The same
checkpoint SHA-256 was retained. [Four focused Qt checks](campaign-restart-20261119/qt-02.json)
passed. Earlier attempts and startup/test-helper failures remain retained in
`campaign-restart-20261119/live-01.json` through `live-05.json`. Fresh Wine prefix
bootstrap rejected its changed DirectPlay builtins; the successful run used a
private clone of a stopped, hash-verified admitted prefix. Broader campaign maps, complete world/RNG/AI/
script state equivalence, user hardware, save failure recovery and normal Quit
are independent boundaries.
