#!/usr/bin/env python3
"""Exercise real XTest modifiers, refusal and cancellation on an owned X11 child."""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


class KeyEvent(C.Structure):
    _fields_ = [('type', C.c_int), ('serial', C.c_ulong), ('send_event', C.c_int),
                ('display', C.c_void_p), ('window', C.c_ulong), ('root', C.c_ulong),
                ('subwindow', C.c_ulong), ('time', C.c_ulong), ('x', C.c_int),
                ('y', C.c_int), ('x_root', C.c_int), ('y_root', C.c_int),
                ('state', C.c_uint), ('detail', C.c_uint), ('same_screen', C.c_int)]


class Event(C.Union):
    _fields_ = [('key', KeyEvent), ('pad', C.c_long * 24)]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists():
        p.error('Refusing evidence overwrite')
    claims = json.loads(a.claims.read_text())
    sources = dict(claims['sources'])
    digest = lambda data: hashlib.sha256(data).hexdigest()
    sources['tools/test-drawing-documented-input.py'] = digest(Path(__file__).read_bytes())
    if any(digest((ROOT / n).read_bytes()) != h for n, h in sources.items()):
        raise ValueError('Prospective fixture sources changed')
    x = C.CDLL('libX11.so.6')

    def api(name, result, args):
        f = getattr(x, name)
        f.restype, f.argtypes = result, args
        return f

    ptr, win, integer = C.c_void_p, C.c_ulong, C.c_int
    open_display = api('XOpenDisplay', ptr, [C.c_char_p])
    root_window = api('XDefaultRootWindow', win, [ptr])
    create = api('XCreateSimpleWindow', win, [ptr, win, integer, integer, C.c_uint,
                                           C.c_uint, C.c_uint, win, win])
    select = api('XSelectInput', integer, [ptr, win, C.c_long])
    store = api('XStoreName', integer, [ptr, win, C.c_char_p])
    map_window = api('XMapWindow', integer, [ptr, win])
    sync = api('XSync', integer, [ptr, integer])
    pending = api('XPending', integer, [ptr])
    next_event = api('XNextEvent', integer, [ptr, C.POINTER(Event)])
    keycode = api('XKeysymToKeycode', C.c_ubyte, [ptr, win])
    query_keys = api('XQueryKeymap', integer, [ptr, C.c_char_p])
    query_pointer = api('XQueryPointer', integer, [ptr, win, C.POINTER(win),
                       C.POINTER(win), *([C.POINTER(integer)] * 4), C.POINTER(C.c_uint)])
    destroy = api('XDestroyWindow', integer, [ptr, win])
    close = api('XCloseDisplay', integer, [ptr])
    display = open_display(None)
    if not display:
        raise ValueError('Private X11 display required')
    parent = child = 0
    runs = []
    try:
        parent = create(display, root_window(display), 20, 20, 820, 620, 0, 0, 0)
        child = create(display, parent, 10, 10, 800, 600, 0, 0, 0)
        select(display, child, 1 | 2 | 4 | 8 | 64)
        store(display, parent, b'DrawingSession')
        map_window(display, child)
        map_window(display, parent)
        sync(display, 0)
        root = ROOT / 'working/tests/drawing-documented-input'
        root.mkdir(parents=True, exist_ok=True)
        run = Path(tempfile.mkdtemp(prefix='run-', dir=root))
        codes = {name: keycode(display, symbol) for name, symbol in
                 [('Shift', 0xffe1), ('space', 0x20), ('Insert', 0xff63), ('Delete', 0xffff)]}
        cases = [('positive', [dict(kind='click', x=100, y=100, button=1,
                                  modifiers=['Shift'], after_ms=40),
                              *[dict(kind='tap', key=name, after_ms=40)
                                for name in ('space', 'Insert', 'Delete')]], None)]
        for name, modifiers, kind in [('nonarray', 'Shift', 'click'),
                                      ('unknown', ['Control'], 'click'),
                                      ('duplicate', ['Shift', 'Shift'], 'click'),
                                      ('wrong-action', ['Shift'], 'wait')]:
            cases.append((name, [dict(kind=kind, x=100, y=100, button=1,
                                     modifiers=modifiers, after_ms=40)], None))
        cases.append(('cancel-held', [dict(kind='click', x=100, y=100, button=1,
                                          modifiers=['Shift'], after_ms=40)], signal.SIGTERM))
        for name, actions, cancel in cases:
            directory = run / name
            directory.mkdir()
            (directory / 'gameplay.bin').write_bytes(bytes(80))
            schedule, report = directory / 'schedule.json', directory / 'input.json'
            schedule.write_text(json.dumps(dict(actions=actions)))
            while pending(display):
                event = Event()
                next_event(display, C.byref(event))
            events, sent = [], False
            with (directory / 'input.log').open('x') as log:
                process = subprocess.Popen([str(a.executable.resolve()), str(directory),
                                            str(schedule), str(report)], stdout=log,
                                           stderr=subprocess.STDOUT,
                                           env=dict(os.environ, QT_QPA_PLATFORM='xcb'))
                deadline = time.monotonic() + 15
                while process.poll() is None:
                    while pending(display):
                        event = Event()
                        next_event(display, C.byref(event))
                        e = event.key
                        if e.type in (2, 3, 4, 5):
                            events.append(dict(type=e.type, detail=e.detail, state=e.state))
                            if cancel and e.type == 4 and not sent:
                                process.send_signal(cancel)
                                sent = True
                    if time.monotonic() >= deadline:
                        process.kill()
                        process.wait()
                        raise ValueError('Owned input fixture timed out')
                    time.sleep(.002)
                sync(display, 0)
                while pending(display):
                    event = Event()
                    next_event(display, C.byref(event))
                    e = event.key
                    if e.type in (2, 3, 4, 5):
                        events.append(dict(type=e.type, detail=e.detail, state=e.state))
            outcome = json.loads(report.read_text())
            keys = C.create_string_buffer(32)
            query_keys(display, keys)
            assert not any(keys.raw[code // 8] & (1 << (code % 8)) for code in codes.values())
            rw, cw, rx, ry, wx, wy, state = win(), win(), integer(), integer(), integer(), integer(), C.c_uint()
            assert query_pointer(display, child, C.byref(rw), C.byref(cw), C.byref(rx),
                                 C.byref(ry), C.byref(wx), C.byref(wy), C.byref(state))
            assert not state.value & (1 | 256 | 512 | 1024)
            if name == 'positive':
                assert process.returncode == 0 and outcome['success']
                buttons = [e for e in events if e['type'] in (4, 5)]
                assert [e['type'] for e in buttons] == [4, 5]
                assert all(e['detail'] == 1 and e['state'] & 1 for e in buttons)
                for code in codes.values():
                    assert [e['type'] for e in events if e['type'] in (2, 3) and e['detail'] == code] == [2, 3]
            elif cancel:
                assert sent and process.returncode == 1 and not outcome['success']
                assert outcome['error'] == 'Cancelled'
            else:
                assert process.returncode == 1 and not outcome['success']
                assert not any(e['type'] in (2, 4) for e in events)
            runs.append(dict(name=name, exit=process.returncode, outcome=outcome,
                             events=events, held_inputs_cleared=True))
        assert all(digest((ROOT / n).read_bytes()) == h for n, h in sources.items())
        result = dict(schema=1, success=True, sources=sources, sources_stable=True,
                      claims=claims['claims'], executable_sha256=digest(a.executable.read_bytes()),
                      artifacts=str(run.relative_to(ROOT)), cases=runs,
                      original_game_executed=False, selection_or_pause_semantics_proved=False,
                      scope='Real XTest on owned X11 child: Shift state on both button events, key pairs, four malformed modifier refusals and cancellation while held with no leaked inputs. No original engine or presentation parity.')
        with a.report.open('x') as out:
            json.dump(result, out, indent=2)
            out.write('\n')
        print(json.dumps(dict(success=True, cases=len(runs), artifacts=str(run))))
    finally:
        if parent:
            destroy(display, parent)
        close(display)


if __name__ == '__main__':
    main()
