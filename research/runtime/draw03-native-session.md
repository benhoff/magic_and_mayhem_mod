# DRAW-03 native session orchestration

`compat/legacy/drawing_session.hpp` owns a thread-confined native lifecycle over
`DrawingScene`, separate from widgets, injected hooks and original acquisition.
Session IDs increase across restarts. Segment and presentation ordinals start at1
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

Default native budgets are64segments,8192published frames and1,048,576successfully
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

Fresh execution `NR.drawing-session.lifecycle-native-20261011` passes all12native
lifecycle cases and the existing drawing-scene regression with zero remaining
surfaces. The report pins prospective scope/scenario claims, source/binary hashes
and command logs. Original Quick Battle/campaign equivalence remains pending.
