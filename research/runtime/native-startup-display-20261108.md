# SX08: authenticated startup with RAM-backed diagnostic paths

The RT08 pre-menu failure has a confirmed private-display cause. Its launcher
stderr, outside the original acquisition's artifact list, repeatedly reports
X11 authorization failures. The verified authority file is beneath
`working/recovery-closeout08`, which aliases `/tmp`. The child replaces `/tmp`
with private tmpfs. The original boundary validates the host file but does not
bind it into the child, so the child's inherited `XAUTHORITY` path is absent.
The accompanying shader-cache path warning has the same masked-path mechanism;
this work changes only the required authority-file mapping.

`native_output_owner.command` now binds that validated file read-only to
`/tmp/mnm-private-xauthority` and selects that child-local path. It retains the
separate private socket, user/PID/network/IPC namespaces, synthetic devices and
host display/credential environment exclusions. No broader directory is exposed.
No native renderer, guest DLL, binary patch, gameplay or source asset changes.

The existing no-game X11 child reproduces the old defect: disk credentials work,
while credentials under masked RAM do not authenticate. With the fix, both disk
and RAM credentials permit private drawing; a wrong cookie remains rejected.
Host pixels remain unchanged. The child also verifies host sockets and graphics
devices remain inaccessible. These three current namespace cases and 25 Python
admission/readiness/confirmation/pointer guards pass. The exact old boundary,
before/after results and previously omitted launch logs are SQLite references in
[the result directory](native-startup-display-results-20261108/after-controls-01.json).

A new prospective plan admits only one original startup, with a 300-second
deadline and no retry. It preserves both stopped RT07/RT08 recovery budgets.
The exact retained player and its 406 compiler inputs remain unchanged; the
current guest source phase has 1021 paths. The check reuses ordinary public Low
Campaign/Adept entry and physical input, with full viewport/source capture off.
Readiness uses original return records and active native input; it grants no
paint or independently compared pixel credit.

That run reaches raw original Main on owner thread 312 and then native World
completion 1280, nonce `10179220446829153604`, without renderer error. Its raw
SQLite samples bind to the complete frame/input stream: original returns and
generation advance from 62 to 186, and the released Up counter changes from 0 to
1 at actual 640×480. The new launcher log has no authorization errors. This
confirms the repaired startup path without another original run or API trace.
The current player reports llvmpipe; no universal provider claim follows.

The extra ordinary-Quit check does **not** finish. The test reaches the original
confirmation, then the collector's finalizer raises while returning its prefix.
That exception prevents the original acquisition summary and primary error from
being saved. The primary exception is therefore recorded as unavailable.
Independent code review identifies an additional diagnostic defect: the new
capture-free progress adapter lacks the captured-paint fields expected by the
inherited pointer method. A capture-free pointer override and regression guard
correct that interface; error checkpointing and guarded prefix-return handling
preserve future primary failures. These collector corrections have unit execution
only. No additional game is launched to refresh them.

[The postmortem](native-startup-display-results-20261108/startup-postmortem-01.json)
verifies and retains the original Main wires, strict frame/input samples, raw
reports, screenshots, exact tested executables and exact tested driver preimage.
Its success means the retained startup observations verify. It explicitly records
an incomplete collector and unverified normal Quit, with no invented primary
exception or successful cleanup. The held private prefix is subsequently checked
idle and returned through the repository's guarded recovery API; its original
receipt and separate recovered delta remain preserved.

The authority-mount policy has fresh scoped/headless control evidence. The full
startup/normal-Quit collector receives no implementation or integration milestone.
Earlier RT07 accessibility failures had different diagnostics and are not
retroactively relabeled. Actual fault/restart intervals, both Low recovery
domains, all four requested operation rows and both final rendering decisions
remain separate. Incoming natural-defeat acceptance remains selected at
102/156 passed, 54 pending; this work adds no acceptance rows and resets none.

New generated observations and bookkeeping use SQLite. Shared immutable game and
prefix inputs, tested source phases and all negative observations are preserved.
RAM staging and resettable environments remain temporary; no full installation
or prefix snapshot is added merely for the setup failure. No source push,
evidence publication, private transfer or generic cleanup is performed.

Accounting preserves the incoming font/DC contract recovery at `cddf1bd6b`.
Exact history reviews cover 104 earlier incoming transitions and 88 newly
integrated transitions, including 66 retrospective opaque-attachment custody
receipts. These are custody reviews, not historical gate or validation claims.
The first audit refuses an empty scenario-evidence link; the corrected record
links the incomplete actual observation without promoting its full scenario.
[The reviewed audit](native-startup-display-results-20261108/audit-02.json) has
zero errors. Broad historical ownership/recovery evidence remains explicitly
pending where its source or scope differs; only the narrow authority controls
have current implementation and headless integration proof.
