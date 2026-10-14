# Multiplayer session selection and lobby recovery

Seeded pinned No-CD40209ca7 session selector6de838 (ID21/vtable5c65d4),
constructor4afb60. Read-only exportmultiplayer-controls-hvicfif6 retains the
constructor, entry and refresh-related functions. This is discovery only;
network/list identities, action admission and caller returns are pending.

Confirmed: selector standard tick5595d0, deferred slot+2c4afd90. Entry4afc90
starts enumeration (5359c0), marks first enumeration+65, defers original input
when provider requests it; refresh4b0790 runs enumeration535fa0 every500ms,
rebuilds ListBox rows and rematches the previous selected name. First failure can
return to Quick via+37=6e0020/+43=1. Automatic6e1f0c is outside normal admission.

ListBox+53/vtable5c6db8 has row pointer+45, count+49, selected+5b and257-byte
rows (256-byte name plus marked byte). Original selector4d1060(-1) clears all
marks before choosing one row, so V16 uses those original setters. Snapshot
bounds are100 nonempty names shorter than128 bytes; longer/larger lists use
original controls. Session IDs in Qt are row ordinals; original lookup uses the
displayed game name, so no GUID or unambiguous peer identity is claimed.

Two callback pointers+4f type5c6604: button4b0a00 and list4b0a30. Button IDs
0OK/1Cancel. OK sets+64=1; it neither joins directly nor fades. Cancel fades
and requests Quick via+37 without setting+43. List repeated-selection can set
+64, with remembered selected row+60. Deferred4afd90 consumes+64, copies the
original selected name into6de59c, calls original5360e0/537da0 and on success
requests the Join lobby6c477c. Host lobby6a4b1c is a different receiver. These
network calls remain original; their static destinations are not peer validation.

V16 admission requires Main/Quick/Join/selector depth5, exact receivers/vtables,
Join resources released (initialized0), mode1/context2, provider1..3 and no
automatic launch. Commands reject first enumeration/deferred/modal/fade and
pending/return transitions. Original enumeration continues while Qt owns the
list; identical snapshots do not rebuild the user's native selection. Join
hands presentation back to original networking/lobby; Cancel/window close use
original callback and Quick/Main Quit. Live validation remains pending.

UI44.original-session-fixture executes15 original callback/adapter cases and35
mutation-free rejection checks in private PE32. Callback ABI is checked; fade
and row setter are stubbed. It does not execute enumeration, networking or
lobby work. The callback's explicit byte range is backed by pinned entry anchors.

UI44.sessions-live2026-10-12 records original TCP Join submission to native
empty list twice, Cancel and window-close to original Quick/Main Quit: nine ready
states/eleven ACKs, same engine thread, two original viewport networking handoffs.
`working/tests/live-menus/run-css82x15` / experiment `run-gxt_8cwh` retain the
prospective/final claim/source bindings and captures. All2927 original inputs
verify before/after. Three targeted Qt bridge/widget checks pass. This promotes
only the bounded native selector integration; populated list behavior is private
fixture validation, not peer evidence. Original networking/rendering/audio and
lifecycle remain active. No lobby or gameplay status is promoted.
