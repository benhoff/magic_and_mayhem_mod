# Indexed palette histories

## Confirmed implementation

The opt-in x86 history now supports indexed 8-bit surfaces alongside RGB.
`runtime/render/palette_history.h` observes the palette returned by a tracked
surface's saved GetPalette method, verifies 256-entry RGB capability, follows
canonical IUnknown identity and recognized IDirectDrawPalette aliases, and
reads initial entries through saved GetEntries. No observer reference is retained.
All observer Release calls bypass application hooks, including existing frame
and draw snapshots.

Each renderer surface retains native indices and its own palette texture. Shared
COM palettes are represented by emitting an update to every associated live
surface, not by converting indices to RGB. A successful SetEntries emits only
the affected RGB range after independently reading the original palette back.
Changed entries outside the requested range invalidate coverage. Successful
SetPalette reassignment emits a full new palette for that surface; former shared
users retain their own association. Failed setters emit no change.

Palette-only changes emit PRESENT for the last recorded copy destination when
it uses that palette. No additional draw or native-pixel upload is required.
PRESENT remains an inspection point, not a claim about primary-surface display
or flip timing. Full-surface indexed CPU writes use the existing writable-lock
path and retain exact index/color-key identity, including when two indices have
equal displayed RGB colors.

Palette flags are not alpha. Entry peFlags bytes are retained only in observer
snapshots; serialized palettes contain RGB triples and presentation alpha is 255.
Supported palette capability bits are 8BIT, INITIALIZE, PRIMARYSURFACE,
PRIMARYSURFACELEFT, ALLOW256 and VSYNC. 4BIT/1BIT/2BIT, 8BITENTRIES and ALPHA
modes are rejected. Readback failure, unsupported successful setter flags,
null palette detachment and successful reinitialization invalidate capture.
Failed unsupported calls continue unchanged and leave capture state intact.

## Color evidence and lifetime safety

Opcode 10 CHECK_RGBA compares OpenGL presentation with independently captured
original native pixels and palette entries converted by the bridge's established
RGBA conversion helper. Expected RGBA bytes never become rendering inputs.
The replay CLI reports `color_checks`; a color mismatch writes no native output
or preview. The tests independently calculate colors in Python as a second
oracle and deliberately poison RGBA evidence to ensure replay rejects it.

Initial color checks use the source/destination-before checkpoint, not live
post-draw destination memory. Later color checks take nonblocking original
read-only snapshots. Palette changes while an associated surface is application
locked invalidate capture instead of reacquiring that lock.

Palette observation slots retire when no tracked surface uses them, even if the
application still owns palette references. Rebinding starts a fresh observation
from current entries; an existing alias resolves canonical identity before its
next entry update or Release. This avoids retaining stale interface pointers
across an unobserved internal palette destruction. There are at most 16 palette
observation slots and 16 aliases per slot; slots are not reused in one session.
Surface Release also resolves canonical identity when an interface was obtained
outside the observed QI path, then retires the surface without dereferencing it
after the original final Release.

Hooks use x86 stdcall and the installed primary Wine `ddraw.h` signatures:
palette slots 0 QueryInterface, 2 Release, 3 GetCaps (observer), 4 GetEntries
(observer), 5 Initialize (coverage guard), 6 SetEntries; surface slot 20 GetPalette
(observer), 31 SetPalette. Palette tables contain seven methods and are never
patched as 33-method surface tables. Canonical interfaces must use a recognized
intercepted vtable; uncovered aliases invalidate capture. Hooks remain
process-lifetime instrumentation and must not be unloaded.

## Validation and remaining scope

```bash
./tools/test-render-palettes.py
./tools/test-render-history.py
./tools/test-render-bridge.py
```

The palette fixtures cover shared updates through another palette interface,
reassignment, rebinding through an existing alias, successful indexed writes,
failed setters, palette-only changes without movement, equal RGB colors with
separate native keys, unsupported flags/capabilities, null detachment, unobserved
color changes, readback failure, reinitialization, and poisoned RGBA evidence. Successful sessions
match original indices, an independent Python color oracle and Qt framebuffer
readback. Fake objects check balanced COM reference counts and original return
values/last-error behavior.

Evidence for this chunk:

- `working/tests/render-palettes/run-0mkv7fr3/report.json`: 11 x86 Wine fixtures;
  five successful independent native/RGBA/Qt comparisons and six expected GAP
  rejections, plus rejected poisoned RGBA evidence. DLL/executable hashes are
  recorded in the report.
- `working/tests/render-history/run-j_cqckge/report.json`: 14 RGB fixtures,
  including final Release through an unobserved interface alias.
- `working/tests/render/run-l6xnwa46/report.json`: default capture and Qt frame
  bridge regressions pass; both native copy paths still match original/CPU output.
- Production PE32 i386 bridge SHA-256:
  `49346542fafe39d05c6d9dd363b43171e8fbeea98f81a31ff2805ae9284aa796`.
- All six renderer CTests and seven Qt shell regression tests pass. Ordered
  command tests now compare RGBA across 8/16/24/32-bit formats and reject
  malformed/stale/poisoned color records.

Confidence: confirmed synthetic PE32 Wine and software Mesa/Qt behavior. Actual
game palette usage, physical GPUs, internal COM callbacks and concurrent game
calls remain unvalidated. Reentrant/overlapping observed calls invalidate the
history; these hooks do not establish complete DirectDraw coverage. Existing
16-operation, 240-record and 64 MiB history bounds still apply; successful palette
setters count toward the operation bound. Flips were pending in this palette chunk;
the subsequent [double-buffer extension](opengl-double-buffer-flips.md) adds guarded swaps.

The capture command remains ready, with no automatic game launch:

```bash
./tools/run-qt-shell.sh --capture-history
```

Click **Launch game** when ready. After exit, preview the complete history with
`./tools/run-qt-shell.sh --commands CAPTURE_DIRECTORY/history-0001.bin`.

Protocol: [surface commands](../formats/render-surface-commands.md).
