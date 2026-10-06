# Bounded ordered startup through first primary presentation

2026-10-05. Native policy `NR.owned-session-startup` addresses the 16-operation
cutoff found by [packed-unlock startup observation](opengl-owned-session-unlock-packed.md).
This remains an opt-in bounded shadow session with original drawing active.

## Completion policy

The ordinary sample still completes after 16 successfully routed operations
when at least one eligible primary PRESENT exists. An entirely offscreen startup
can continue beyond operation 16, up to a separate ceiling of 64 successful
operations. The first eligible PRESENT at operation 16 or later completes the
sample immediately. A first PRESENT at operation 64 is admitted; no PRESENT by
operation 64 ends with the existing missing-presentation GAP reason 6.

A failed original draw does not consume a successful-operation slot. Producer
exit before presentation also refuses with GAP 6. Epoch/identity invalidation,
record/byte/surface/pixel limits and incomplete lock/DC ownership retain their
existing refusals. The new startup operation ceiling does not raise those limits,
change command input, CHECK semantics or presentation conversion, or reopen a
completed stream. No wire version or opcode changes.

## Validation protocol

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-startup.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-channel.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-unlocks.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel
```

The independent PE32 fixture performs repeated nonprimary full locks, then an
opaque primary copy. It separately tests first presentation at operations 16,
18 and 64; a failed original primary copy before success at the ceiling; missing
presentation at the ceiling, early exit, first primary copy too late, and
invalidation before presentation. The delayed case performs more original work
after completion to prove the ordered stream stays ended.

Native default presentation is compared with independent complete engine pixels,
and explicit CPU/native-byte CHECK replay compares final native storage. Original
Unlock poisons exposed backing memory; HRESULT, LastError and original call counts
are checked. Refused streams retain no native presentation or GPU resources.
Diagnostic frame/native reads are explicit; ordinary native/RGBA readbacks and
viewport image uploads stay zero.

## Execution evidence

[Eight startup fixtures](opengl-owned-session-startup.json) pass: four complete
sessions with independent complete frame comparisons, and four deliberate
refusals. [Seven live transport regressions](opengl-live-command-startup-regression.json)
and [26 packed unlock regressions](opengl-owned-session-unlock-startup-regression.json)
pass. The native incremental-consumer and live-channel CTests pass. Normal native
and RGBA readbacks and viewport image uploads remain zero; terminal native
surfaces are released.

[Original-game startup](opengl-real-game-shadow-startup.json) now reaches one
native GPU PRESENT before producer exit and ends cleanly. The producer completes
at 23 successful operations: 69 records (7 CREATE, 18 UPDATE, 4 BLIT, 31 CHECK,
1 PRESENT, 7 DELETE, 1 END), 39,226,036 published bytes. The mapped channel and
mirror match; native surfaces are released, with zero ordinary frame readbacks
and viewport uploads. The live frame QRgb SHA-256 is
`f4e2db9a02fa70eac1e8f0ffa0868ca5dfb158575ff000903d69a6070af382bf`.
The harness verifies all 2,927 immutable original files before and after launch.

Explicit GPU verification of the emitted stream passes all 31 owned CPU CHECK
snapshots, with four copies, 25 uploads and one presentation. This diagnostic
comparison is against hook-owned snapshots, not independent original-driver
pixels. The native-byte output SHA-256 is
`8e05ca65dd99427d152eab58518c4d4f2a054604cb26bd376c9233eac33d1941`;
command SHA-256 is
`9149fb466f723cc621d64a3dcb0810cd46780ef120dda58cf3f26ddbcfcb44dc`.

Historical records retain their fingerprints; shared source edits leave older
evidence stale. Bounded first presentation remains separate from independent
original-driver pixel equivalence, continuous original-game presentation and
rendering replacement. The bounded sample is not a complete menu or gameplay
session. No gameplay balance changes are introduced.
