# Bounded independent World pixel observation

2026-10-06. This extends the finite resource-cache campaign test into a 30-second
post-recovery guided idle World scene. Original drawing remains active; no renderer
replacement, synchronized animated-frame comparison or full-frame equivalence is
claimed. The original source preferences and 2,927-file immutable manifest remain
unchanged.

## Reproduction and finite bounds

```sh
cmake --build working/build/renderer --target live-render-route-probe -j2
xvfb-run -a -s '-screen 0 1800x1000x24' \
  python3 tools/test-live-render-routes.py working/build/renderer \
  --mode campaign --require-world-pixels --world-seconds 30
```

`--world-seconds` accepts 3 through 60 seconds. Requiring World pixels implies
required Active World publication/recovery and refuses a movie-only route. The
probe admits at most 160 ordered requests and retains its 180-second deadline;
this covers the maximum duration with up to three attempts for each comparison.
Each native snapshot is an explicit test framebuffer capture. Ordinary native and
RGBA readbacks and CPU viewport uploads remain zero. Production resource, pixel,
CPU storage and queue budgets are unchanged.

After original Main and Region Entry readiness, the test observes three original
World ticks, forces reader failure, and requires successful native recovery. It
compares the initial guidance rectangle `(160,225)-(640,375)`, clicks the original
client at `(400,300)` to dismiss it, and moves the pointer outside both views.
It samples stable forest terrain `(0,0)-(160,180)` and the commander portrait
`(700,500)-(778,561)` approximately every five seconds. Native framebuffer and
original X11 client screenshots are independent inputs. Every tested region must
match, permitting at most eight RGB channel values of error and three finite
attempts. Captures are unsynchronized: this is suitable for these stable regions,
not moving actors or effects. Original and native full screenshots are retained,
but pixels outside the named regions are not equivalence assertions.

## Observed result and remaining boundaries

The [fresh original-game record](opengl-world-observation-20261006.json) observes
642 native frames. After the forced World reader failure it publishes 331 further
frames over 32,398 ms in the same recovered session, remaining Active at every
sample. Five terrain and five portrait comparisons plus the initial guidance
comparison pass; the four menu-region checks also pass. All 15 regions match on the
first attempt with maximum RGB channel error 1. Final consumer resource retirement succeeds without ordinary readbacks or uploads.

The retained screenshots show that the opening guidance click advances to the
**Select Zombie Spell** tutorial prompt. The subsequent 30-second observation
therefore validates a guided idle World and stable terrain/portrait rendering,
not unrestricted active gameplay or spell selection/casting. Moving actors,
summons, combat, effects, other HUD states, scene transitions, movies, hardware
Windows drivers, synchronized full-World pixels and live replacement remain
pending. The original menu observer's bounded three World records do not prove
30 seconds of original simulation progress; continuing native publication and
independent stable pixel observations are the measured evidence.

The route CLI's invalid duration bounds and movie-only World-pixel request each
refuse before launching Wine. These three negative checks and strict native probe
rebuild passed. The reviewed committed range is `c84f26e..057f07d`; parent/current
source and behavior receipts match with no unresolved exact-accounting gaps.
Historical evidence fingerprints are retained. This test-only change does not
rerun all earlier synthetic producer matrices or promote replacement status.
