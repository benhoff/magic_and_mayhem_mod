# DRAW-03 native session orchestration

`compat/legacy/drawing_session.hpp` owns a thread-confined native lifecycle over
`DrawingScene`, separate from widgets, injected hooks and original acquisition.
Session IDs increase across restarts. Segment and presentation ordinals start at 1
and are contiguous within a session; producer sequence and World queue identities
restart only when a fresh segment creates a new scene. Frame identities include
the session, segment, presentation ordinal, native stage and complete source
boundary. Stale frame queries refuse without poisoning a newer valid frame.

Native stage labels are Scene/Menu/Loading/World/Results. They do not recover
original screen numbers or execute simulation actions. An explicit transition
discards the previous history, including an interrupted World queue, and starts
undefined native storage. Reused canvas IDs and changed extents cannot inherit
pixels across this boundary. Continuity within a segment remains `DrawingScene`'s
responsibility. Restoring original game state or choosing when the original game
transitions is outside this controller.

Only complete explicitly declared boundaries publish. A new producer invalidates
the current frame; a final incomplete boundary cannot finish a session. Finished
sessions retain the complete terminal frame until stop/restart. Invalid running
mutations, missing source assets, sequence gaps and exhausted budgets discard all
owned scene resources and enter Failed. Recovery requires a greater session ID
and a new source history. Active begin and stale read/presentation queries refuse
without replacing current state. Stop is idempotent.

Default native budgets are 64 segments, 8192 published frames and 1,048,576 successfully
applied producer records per session; callers may lower them. These are native
admission policies, not recovered original limits. Counters span segments and
reset only at a new session. External immutable GPU leases may outlive transition
or stop. Session access is invalidated immediately; storage retires when the last
external lease is released. Reported zero-storage cleanup requires releasing
those leases, not claiming they disappeared at stop.

The executable native fixture tests ordered menu/loading/World/results/menu
lifecycles, actual asset-backed World completion, canvas recreation and extent
changes, stale identities, interrupted World history, missing assets, incomplete
publication, segment/frame/record budgets, fresh recovery and GPU lease cleanup.
Its synthetic source inputs establish this native policy only. The existing
scene regression exercises the underlying renderer independently.

```sh
python3 tools/draft-coverage-claims.py --behavior NR.drawing-session \
  --scenario DRAW03.native-session-lifecycle --output <new-claims.json>
python3 tools/test-native-drawing-session.py --claims <new-claims.json> \
  --report <new-lifecycle-report.json>
```

Full Quick Battle/campaign presentation, native picking/caller-state contracts,
movie composition, original transition/failure equivalence and live replacement
remain pending. DRAW-01 owns recovered contracts; DRAW-02 owns original capture
and independent reference admission. This controller changes neither collector
nor the fourteen-journey/twenty-one-operation acceptance denominator.

Fresh execution `NR.drawing-session.lifecycle-native-20261011` passed all 12 native
lifecycle cases and the existing drawing-scene regression with zero remaining
surfaces. The report pins prospective scope/scenario claims, source/binary hashes
and command logs. Original Quick Battle/campaign equivalence remains pending.

The owned-input runner `mnm-drawing-session-replay` makes this orchestration
reproducible end to end: bounded contained source admission, immutable producer
and asset hashes, ordered stage execution, complete RGB565 frame files, identity
reports and cleanup. Its [v1 declaration](../formats/drawing-native-session-v1.md)
is a native policy format. It does not implement the distinct DRAW-02
presented-session ABI or original action/caller-state contract.

The combined check also exercises six independently expected complete native
planes across six segments, a real synthetic World raster, repeated publication,
interrupted World cancellation, and 29 CLI refusals for input binding, schema,
identity, boundary, path containment, original result contamination and storage
cleanup. The first lifecycle result remains historical after the runner/build
changes; a new exact-source execution result is registered separately.
