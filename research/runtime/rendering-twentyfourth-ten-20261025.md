# Ten bounded native surface implementations (2026-10-25)

This batch starts at local master `d6618d09`. The next small implementation
boundaries are grouped around existing partial surface systems: five copy entry
paths, two fill entry paths, two bitmap resource/recovery services and owned
vertical-bar surface creation. These have documented suppliers, independently
capturable arguments and reusable owned backend operations. They are ten bounded
component tasks within five partial parents, not ten completed engine subsystems.

| Task | Registered component | Partial parent and supplied behavior |
| --- | --- | --- |
| 1 | RE.surface-wrapper.opaque-guarded | RE.surface-rectangle-forwarding; Opaque guarded surface copy |
| 2 | RE.surface-wrapper.opaque-recovering | RE.surface-rectangle-forwarding; Opaque surface copy geometry |
| 3 | RE.surface-wrapper.keyed-recovering | RE.surface-rectangle-forwarding; Keyed surface copy geometry |
| 4 | RE.surface-wrapper.keyed-rectangle | RE.surface-rectangle-forwarding; Keyed rectangle copy geometry |
| 5 | RE.surface-wrapper.keyed-explicit | RE.surface-rectangle-forwarding; Explicit keyed source/destination rectangles |
| 6 | RE.surface-wrapper.partial-fill | RE.surface-fill.rgb565; Owned partial-fill wrapper service |
| 7 | RE.surface-wrapper.full-fill | RE.surface-fill.rgb565; Owned full-fill wrapper service |
| 8 | NP.surface-bitmap.owned-cursor | RE.surface-reload.cursor-sentinel; Owned fixed-path cursor bitmap binding |
| 9 | NP.surface-bitmap.indexed-recovery | RE.surface-driver.indexed-restore; Shared-palette indexed bitmap recovery |
| 10 | NP.vertical-bar.owned-surface | RS.canvas-vertical-bar; Owned logical vertical-bar surfaces |

The copy planner is portable C++ and wraps signed32 coordinate arithmetic without
host overflow. It preserves source rectangles and mutates the explicit keyed
wrapper's destination rectangle before native driver admission. Window context
and the resolved client offset are supplied values; this does not discover a
window, call Win32 or retain process pointers. Missing interfaces bypass only
the two guarded opaque paths. Native keyed missing handles are explicit refusals.
The callable executor performs one owned API call, preserving the boundary
between recovered forwarding and native clipping/stretching/ownership admission.
Original pointer/ABI identity and failure/retry semantics of these copy entries
remain outside their successful argument comparison.

The two fill services compose the existing recovered retry scheduler and owned
adapter. They preserve low-WORD color construction and distinct partial/full
loss/report/redraw order. A finite budget stop is incomplete, never an invented
successful original return. Explicit callbacks, cursor targets, display context
and key restoration are supplied bindings; no original resource discovery is
inferred. Native palette recovery and bar allocation are intentional native
composition policies and retain comparison none.

The immutable bitmap service validates the existing sequential BMP subset before
any backend mutation. The fixed cursor reader returns owned bytes or a missing
result. Captured callbacks retain immutable inputs after the resource object is
released. Reloads acquire/release an owned DC. Indexed recovery first checks the
expected shared palette identity, retains its entries and defines only the
written bitmap extent after Restore. Unwritten restored storage stays unknown.
Bar creation consumes logical dimensions/fill/format, never original output pixels.
Caller-owned surface references must be released through their creating backend.

Validation declares exact contracts and complete source inputs before execution.
Release-active controls and ASan/UBSan exercise the callable geometry, native
pixels, null guards, aliases, budget/loss ordering, DC cleanup, immutable callback
lifetime, palette identity/updates, partial validity and bar allocation refusals.
Unchanged original copy entries are compared through isolated fake successful COM
endpoints with manifest bookends; the original bar generator is a separate
supplier comparison. These do not execute a real driver or game session.

Historical evidence hashes and negative results are preserved. Source changes,
including renderer build metadata, can stale older evidence; this batch does not
renew broader lifecycle, scene, combat, hardware or performance acceptance.
Original allocation/reuse, arbitrary failures, complete owners/writers and live
replacement remain open. Parent understanding/implementation/comparison axes
are retained independently of the new child components.

Generated results use SQLite references under
`rendering-twentyfourth-ten-results-20261025/`. Verified local backups and exact
coverage reviews accompany the normal commit. The Videos drive holds this
isolated worktree, bounded builds, raw fixture outputs and compiler temporaries;
RAM staging remains unavailable due the observed tmpfs quota failure. The bounded
GL control launcher uses an authenticated private TCP Xvfb display, explicit
readiness and owned-process cleanup to avoid tmpfs Unix socket/lock creation;
authorization cookies are temporary and never enter evidence. Durable
SQLite evidence, raw original inputs/outputs and unique failures are retained;
CMake execution directories are temporary reusable outputs. The normal90GiB
main-drive reserve and100GiB cold-archive reserve remain in force. Concurrent
uncommitted launcher/bookkeeping work in master is excluded.
