# Palette and property callback admission

Intentional native scheduling policy extends continuous-only
`MNM_RENDER_ORDERED_COPIES=1` to the following application callbacks. This does
not recover original DirectDraw driver ordering or enable live replacement.

| Hook | Original slot | Admitted producer state |
| --- | --- | --- |
| palette_caps | Palette GetCaps 3 | observed capabilities and completeness |
| palette_get_entries | Palette GetEntries 4 | successful output colors |
| surface_get_palette | Surface GetPalette 20 | observed binding and palette interface installation |
| surface_palette | Surface SetPalette 31 | successful binding and interface installation |
| palette_entries | Palette SetEntries 6 | owned input colors committed after original success |
| palette_initialize | Palette Initialize 5 | successful completeness invalidation |
| surface_color_key | Surface SetColorKey 29 | successful observed source-key metadata |
| surface_property / surface_clipper | Surface SetClipper 28 | successful observed clipping metadata |

Read callbacks belong here because they update authoritative producer metadata.
The callback lease spans admission, the saved original and existing successful
commit/history processing. No tracker is held while waiting or calling original
code. Existing50ms tick admission and8ms tracker budgets, same-thread reentry,
finite storage/queue limits, original arguments/results/LastError and ordinary
failure cleanup are retained. No new COM references, observer methods, wire
fields, workers or budget extensions are introduced.

An admission timeout forwards the saved original exactly once, skipping producer
observation. Metadata/pixel epochs are invalidated before and after forwarding,
in addition to the existing missed-target and legacy-history incomplete guards.
A palette pointer is not a surface pointer; a missed-target token alone cannot
invalidate all bindings/dependent colors. Whole metadata invalidation is therefore
intentional even for a failed or read-only timed-out callback. The interrupted
history must refuse rather than replay guessed palette/property state.
Successful admitted operations retain the existing per-resource commit guards;
failed admitted originals never become successful producer mutations.

## Independent validation

The retained [native matrix](opengl-palette-property-order-native-20261006.json)
checks14 cases and554 complete frames: nine valid and five explicitly refused
sessions, with zero terminal consumer storage and ordinary native/RGBA readbacks
or viewport uploads. All source fingerprints remain stable during execution.

- `cross-key` and `cross-clipper` workers wait behind an original copy. Each
  original setter fails once, then succeeds; exact argument/result/LastError and
  call counts are checked. These fixtures set an already-known zero key or null
  clipper and validate ordering/failure handling, not changed clipping geometry.
- `key-timeout` and `clipper-timeout` force the outer original to wait for the
  worker. Both failed and successful original attempts forward; the interrupted
  native session publishes only its initial frame and ends with GAP.
- `cross-palette` uses indexed front/back surfaces sharing an explicit palette.
  An independently computed two-entry RGB update, with original failure/retry
  and poisoned input afterward, follows a pending copy. Three independently
  distinguishable full GPU frames match: initial, copied indices, updated colors.
- `palette-timeout` independently records palette-first then copy output while
  originals continue; only the initial native frame is admitted before GAP.
- `source-key` performs a same-thread setter inside original copy execution.
  It reaches the original without waiting; source generation/origin6 changes
  invalidate the pending copy while the target/epoch remain unchanged.
- Gated indexed lifetime/recreation, mixed RGB16 mutations, RGB32 DIB handoffs,
  concurrent DC/flip/CPU callbacks and same-thread source writes retain their
  independent pixel/refusal results. Indexed recreation still checks41 exact
  palette CREATE/DELETE identities and ordered partial updates. This regression
  does not establish cross-thread release safety outside the callback gate.

Reproduce with the existing optimized probes and Wine/Xvfb prerequisites:

```sh
xvfb-run -a python3 tools/test-render-mutations.py working/build/renderer-live --palette-resources --case cross-key --case key-timeout --case cross-clipper --case clipper-timeout --case cross-palette --case palette-timeout --case source-key --case indexed --case mx16 --case dc32 --case cross-dc --case cross-flip --case cross-lock --case source-write
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --ordered-copies --require-world-summon --world-seconds 30
```

Pending: dedicated concurrent GetCaps/GetEntries/GetPalette/SetPalette/Initialize
fixtures and nested palette-update scenarios; QueryInterface, CreateSurface,
CreatePalette and final Release; surface description/attachment/Restore/BltBatch
callbacks; borrowed CPU/DC intervals and abnormal owner termination; legacy
archive-timeout replay, repeated scenes/actions and animation/full-frame/driver
comparison/default enablement. This chunk admits palette/property callbacks;
it does not complete lifetime scheduling or gameplay modernization.

Committed history `b9ed3a9..e794fd2` was reviewed for exact parent/current file
and behavior receipts with no unresolved entries. The retained history report
does not infer original semantic coverage or assert fresh validation of that
older code.

The [final original tutorial observation](opengl-palette-property-order-live-20261006.json)
passes with783 native frames,487 additional frames after forced
World recovery over40721ms,17 matched independent stable regions and zero
terminal resources. Original Zombie selection/summoning is independently read
from tutorial prompts and the0/15 to1/15 control count. Source fingerprints and
unchanged installation preferences pass;2,927 immutable inputs verify before
and after. These are bounded unsynchronized stable-region observations, not
animated/full-frame/driver equivalence or default enablement.
