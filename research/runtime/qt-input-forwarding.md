# Input from the OpenGL Qt viewport

The shell now forwards input from the displayed image to an application client
under the new Wine desktop. This requires Qt's `xcb` platform on X11/XWayland.
No input is sent to a window that existed before this launch. Discovery binds
only a unique deepest mapped input/output child with key/button/motion event
subscriptions matching the frame dimensions
under exactly one new `MagicMayhem` desktop. Ambiguous clients, missing frames
and destroyed targets disable forwarding.

Click the OpenGL image to focus it. Keyboard scan codes, mouse motion, three
buttons and vertical wheel notches are sent as targeted X11 events. Desktop
focus and the physical cursor are not globally moved. Coordinates use the same
rounded/device-scaled image rectangle as OpenGL presentation. Bar clicks are
ignored; held-button drags and releases outside the image clamp to its edge.
Auto-repeat keeps the key down. Focus loss, hiding, unbinding and launcher exit
release held inputs. Target errors clear retained state. Wheel sub-notch deltas
accumulate; dispatch is bounded to 16 notches per event.

## Windows polling

Static import inspection of the pinned working `Chaos.exe` using
`llvm-readobj --coff-imports` confirms GetAsyncKeyState, GetKeyState,
GetCursorPos and ClientToScreen imports. It does not establish every caller's
behavior or prove that Windows messages alone are sufficient.

The shell also writes a [shared input-state snapshot](../formats/render-input-state.md).
For the pinned image, the PE32 bridge optionally intercepts these USER32 IAT
slots:

| API | IAT address |
|---|---|
| GetAsyncKeyState | `0x005c5204` |
| GetCursorPos | `0x005c5210` |
| GetKeyState | `0x005c5290` |

Staging still checks the full executable hash and the original DirectDraw
thunks. Before writing these slots, the input installer verifies all three
current pointers against the original USER32 exports. A mismatch leaves all
three slots untouched. Native Wayland and dynamic API lookup bypassing these
slots are outside this chunk.

With the optional channel present, DirectDraw vtable slot 20
(SetCooperativeLevel) is observed and forwarded unchanged. Only a successful
call records its HWND for ClientToScreen. Snapshot polling preserves LastError
on success; fallback preserves the original API's behavior and errors. The
original drawing, window/message loop and audio APIs remain active. No
DirectInput replacement, IME/text composition, side-specific modifier polling,
horizontal/pixel wheel or game activation policy is implemented here.

## Offline evidence and remaining validation

`tests/qt-input-test.cpp` creates an independent X11 client and checks actual
received targeted events, resolution matching, ambiguous clients, mapping,
dragging, modifiers, repeats, wheel accumulation, focus cleanup, resizing and
target destruction. CTest runs it at normal DPI and scale factor 1.5.

```bash
ctest --test-dir working/build/qt-shell --output-on-failure
./tools/test-render-input.py
```

The integration tool exports bytes produced by the actual Qt state writer,
then reads them through synthetic PE32 Wine hooks. It checks polling semantics,
client-to-screen conversion, LastError, inactive/odd/invalid/stale fallback,
guarded IAT installation and successful/failed cooperative-window forwarding.
Evidence is recorded under `working/tests/render-input/` with DLL and snapshot
hashes. This test consumes no original game artifact and never launches Chaos.

Confidence: confirmed synthetic Qt/X11 event delivery and Qt-to-x86 polling.
Actual Wine translation of forwarded messages, game focus/activation behavior,
menu input and movement orders still require live validation. The separate Wine
window remains available. Input forwarding does not make the live drawing path
independent of DirectDraw.

Evidence: Qt-to-x86 polling passed in
`working/tests/render-input/run-9dpbs9jr/report.json`. All 16 Qt/renderer CTests
passed; the normal/high-DPI input tests were rerun after tightening client
discovery. The existing synthetic DirectDraw bridge suite also passed in
`working/tests/render/run-ybiy3puv/report.json`. The production bridge builds
as PE32 Intel i386. No game was launched for this chunk.
