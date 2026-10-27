# Original alternate allocation compatibility repair

The supplied NoCD initializer leaves the alternate pointer pair untouched. The
historical allocator/publisher remains unidentified. This opt-in repair owns
storage inside the original process and lets the original World dispatcher,
rasters and integer copy consume it. Allocation, publication and retirement are
an intentional native compatibility policy (`NP.original-alternate-allocation-repair`),
separate from recovered baseline behavior. No original media are modified.

## Storage and original execution

The original debug option sets horizontal padding to 96 and vertical padding to 0.
Its offscreen constructor requests visible width plus 96, while alternate binding
uses visible width plus 192. The integer copy advances its CPU source by ordinary
pitch plus 192 WORDs. Giving that constructor a padded DirectDraw surface would
also expand the ordinary capture/destination footprint. The repair admits only
the pinned 16-bit nominal-pitch format, changes that one initialization descriptor
to nominal 640x480 or 800x600, and supplies CPU storage 832x480 or 992x600.

The CPU reservation has inaccessible outer pages and 0xa5 canaries around a
zero-filled payload. The centered copy source is `base + 96` WORDs. The module
publishes source (`006e1f6c`) before rendering base (`006e1f68`), which selects the
original alternate branch. Every rebuild retires the prior generation before
original initialization. Surface retirement and shutdown close publication and
invalidate active bindings into the CPU span before releasing it. Disabling the
option after an owned generation resets its owned padding. Foreign pointers,
unexpected format/geometry, wrong-thread World binding and damaged guards refuse.
There is no independent pixel oracle in this rendering path.

Six byte-pinned entry hooks preserve displaced instructions through trampolines:
initializer `004e40d0`, offscreen constructor `0058ad90`, surface retirement `0058b3e0`,
shutdown `0058acf0`, original copy `00594508` and active binding `0057dc20`. Flags,
general registers, floating/SIMD state and LastError are preserved around hook
callbacks. The owner binds World only at return site `004fd480`. Other build
addresses are not admitted. The immutable executable SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Current acceptance

The final prospectively declared behavior/scenario contracts and complete source
closure match these immutable SQLite-backed reports:

| Evidence | Result | Boundary |
| --- | --- | --- |
| [controls09](original-alternate-allocation-results-20261026/controls09.json) | Nine controls; five balanced generations and 10 original copies in the positive case | Original bind/copy bytes execute; lifecycle providers are authored |
| [battles05](original-alternate-allocation-results-20261026/battles05.json) | Two normal processes, both initial resolutions, two Battles each; 224 copies and two retired generations | Windowed Wine, original World and copy |
| [campaign05](original-alternate-allocation-results-20261026/campaign05.json) | Two normal processes, both initial resolutions; Save, Mini Load, Main Loads and two display rebuilds each; 491 copies and six retired generations | Normal Quit, no forced termination credited |
| [staging02](original-alternate-allocation-results-20261026/staging02.json) | Standalone installation staging succeeds; assets unchanged, configuration round trip and original stored sections verified | Preparation only |
| [source-check02](original-alternate-allocation-results-20261026/source-check02.json) | Current source and prospective contract checks | Does not add execution credit |

The accepted live total is 715 original World copies, eight balanced
allocation/publication/retirement generations, and 15,744,000 WORD comparisons of
the CPU source with its original-copy destination before UI overlays. Checks
sample the first eight copies and every 64th subsequent copy; guards are checked
around every copy and retirement. All four processes exit through normal Quit
with zero owner errors and both published slots cleared. This establishes the
bounded original-process repair, not independent final-scene equivalence.

Controls also cover absent logging, saturated diagnostics, entry-signature and
preexisting-owner rejection, foreign-thread binding, canary damage, foreign
source replacement and allocation failure. Logging saturation stops new event
rows while the repair continues. Production excludes test-only exports and
failure injection. The [binary diagnostic format](../formats/original-alternate-owner-log.md)
is bounded and does not share host pointers.

## Reproduction and opt-in use

Stage a fresh owned installation from an existing prepared asset installation:

```sh
python3 tools/stage-original-alternate-owner.py --installation \
  working/game-nocd working/experiments/alternate-owner \
  --report research/runtime/alternate-owner-staging-<new-id>.json
```

Use the storage guard described in AGENTS.local.md for staging and execution.
The script verifies the immutable-input manifest before and after, clones the
assets, builds `MnmAlt.dll`, adds an import to a copy of the pinned executable,
and enables only `DEBUG/UseMallocedBackBuffer` in the owned configurations. It
checks the original stored sections and entry point before staging. Use a
separate Wine prefix and run `Chaos.exe -win` from the new `game` directory with
`MNM_ORIGINAL_ALTERNATE_OWNER=1`. An optional
`MNM_ORIGINAL_ALTERNATE_LOG=<fresh Windows path>` records diagnostics; an existing
log path refuses installation. Without the owner environment opt-in the imported
DLL does not install its hooks. This does not change the standard launcher.

`tools/test-original-alternate-owner.py` provides isolated controls. For live
journeys, `tools/test-alternate-ownership-session.py --allocation-repair` requires
`--debug-choice enabled`, accepts only shutdown-watch mode and prospectively
binds the declared OA1 Battle/Campaign scenario before execution. Its repair
publications are explicitly labeled authored; natural publication remains false.
Use the existing tool help for the installation, prefix, output and report paths.

## Negative evidence and limits

All earlier attempts and exact source versions are retained. Initial controls
failed when Wine occupied the fixture's intended address range; the authored
fixture now reserves that range in its PE image. The first Battle attempt copied
112 frames and retired its generation, but failed overall because the driver
looked in the wrong log location. That partial diagnostic has no full-pass credit.
The owner now receives an explicit case log path.

[campaign02](original-alternate-allocation-results-20261026/campaign02.json)
stalled after Mini Load with no owner error, 116 copies and one still-live
generation before forced cleanup. Its screenshot and diagnostics are retained;
the cause remains unestablished. Unchanged repeat journeys and the final
scenario-bound campaign05 pass, without erasing that failure. Earlier passing
reports retain their earlier source/contract fingerprints and are not relabeled
as current validation.

Historical original ownership, other executable builds, physical/exclusive
fullscreen drivers, arbitrary device-loss recovery, long-running counter wrap,
performance and whole-session raster/simulation equivalence remain outside this
acceptance. The independent reconstruction continues to borrow its supplied
storage; this repair gives a staged original process an explicit owner rather
than claiming that borrowed model recovered the missing historical allocator.

The current results refresh the complete declared launcher dependency after the
committed CPU-copy/input integration. Earlier controls08/battles04/campaign04
remain immutable historical reports. The planning fixture acknowledges the
repair as unrated; weights are unchanged. Bounded broad planning-suite attempts
had failures and were terminated; they receive no full-suite pass credit.
Worktree-owned SQLite evidence and review backups are retained locally. The
failed full shared-store backup remains explicit storage evidence; it is not a
successful dual-store backup or an engine result.
