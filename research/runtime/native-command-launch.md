# Native command launch mode

2026-10-07 default selection: ordinary Qt OpenGL game launches now select native
command presentation without requiring `--native-commands`. The same continuous
v2, implied lock capture and recovery/fallback policy applies. `--frame-readback`
selects the earlier original-renderer frame-copy path; combining it with
`--native-commands` is rejected. Explicit draw/history/lock/no-readback diagnostic
flags retain their earlier path unless native commands are explicitly requested.
Wine-window embedding and live menus retain their existing selection. This is
application policy; it does not force a GPU driver or bypass original drawing.
The existing Launch-button harness now omits `--native-commands` in its default
normal/high-DPI cases and retains it for explicit continuous/bounded cases.
User-desktop NVIDIA launch, movies and sustained gameplay remain pending.
The subsequent surface-contention fix also makes `--frame-readback` imply
application-held capture, avoiding observer-created DirectDraw locks in both
ordinary presentation modes. Legacy draw/history readback remains an explicit
diagnostic. See [ownership evidence and remaining scope](render-surface-contention.md).
Fresh evidence `native-render-selection-default-20261007.json` passes all four
Launch-button cases (including default normal/fullscreen fractional-DPI frames
and explicit continuous/bounded modes). Build and CLI smoke/conflict checks
also pass. The first sandboxed Xvfb attempt could not open its display; the
successful isolated execution ran outside the sandbox without original inputs.

2026-10-06; intentional Qt application policy `HOST.native-command-launch`.
No recovered original address or simulation behavior changes.

The shell's `--native-commands` previously created a v1 bounded channel unless
`MNM_RENDER_CONTINUOUS=1` was explicitly inherited. The launch instructions given
for the fullscreen/scaling feature omitted that setting. The former successful
`run-rjlplm7o` experiment records continuous mode; the later `run-9nmjfe1z`,
`run-3eutqrpi` and `run-ab06eqne` manifests record bounded mode.

Confirmed within those completed captures: each bounded archive contains one
PRESENT at sequence 7 followed eventually by DELETE/END. Independent prefix
reconstruction from CREATE/UPDATE yields an 800×600 RGB565 image with every
native byte zero. The three archives are byte-identical. This explains a black
native command view even before fullscreen; native command mode consumes that
short command sample instead of later CPU mirror frames. This is inspection of
prior startup artifacts, not new live equivalence or a graphics-driver finding.

`--native-commands` now defaults to continuous v2. The shell chooses the mode once
from its launch environment and passes the same value to the child, command
channel version, and recovery/control-channel setup. Explicit
`MNM_RENDER_CONTINUOUS=0` retains the bounded diagnostic preview. Direct experiment
and PE32 producer defaults and finite admission/refusal budgets stay as recorded
in the continuous producer contract.

`tools/test-native-command-launch.py` clicks the real Qt Launch button in a fake
repository with a synthetic producer; no Wine or original game is started. The
producer first publishes black, then visible frames through 4,202 commands,
exceeding the bounded decoder's 4,096-record limit. The harness checks matching
environment/channel/control identity, all command bytes acknowledged, retained
composited window pixels, ordinary and fullscreen windows, fractional high DPI,
explicit continuous selection, and explicit bounded selection. Optional
`--captured-run working/experiments/opengl-render/RUN` independently reconstructs
only CREATE/UPDATE/check prefixes before the first PRESENT; additional prefix
operations are explicitly refused rather than silently inferred.

Run on a sufficiently large isolated display:

```sh
xvfb-run -a -s '-screen 0 1920x1440x24' \
  python3 tools/test-native-command-launch.py working/build/qt-shell
```

Fresh evidence: `native-command-default-20261006.json`, including the unchanged
display scaling/input regression matrix. Actual-game retry on the user's driver,
movies, sustained gameplay and full native replacement remain separate boundaries.

The subsequent continuous user run `run-b_9tbgld` instead refused with producer
GAP after tracker contention and incomplete surface history; changing the launch
default does not fix this separate producer failure. See
[the diagnosis](native-command-refusal.md). The existing opt-in drawing-order
policy is exposed through `MNM_RENDER_ORDERED_COPIES=1`, independently of
display scaling and the continuous launch default.
