"""Four ordinary viewport rotation key presses for the explicit AC07 route."""
import ctypes as c
import os
import time


def exercise_views(display, x, xt, record):
    if os.environ.get('MNM_SCENE_EFFECTS_VIEWS') != '1':
        return
    x.XKeysymToKeycode.argtypes = [c.c_void_p, c.c_ulong]
    x.XKeysymToKeycode.restype = c.c_uint
    xt.XTestFakeKeyEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    key = x.XKeysymToKeycode(display, ord('.'))
    if not key:
        raise RuntimeError('No ordinary viewport rotation key')
    for ordinal in range(4):
        try:
            if not xt.XTestFakeKeyEvent(display, key, 1, 0):
                raise RuntimeError('Rotation key press refused')
            x.XFlush(display)
            time.sleep(.15)
        finally:
            xt.XTestFakeKeyEvent(display, key, 0, 0)
            x.XFlush(display)
        record('scene-view-key', ordinal=ordinal + 1)
        time.sleep(1)
