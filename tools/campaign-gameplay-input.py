#!/usr/bin/env python3
"""Bounded physical XTest actions against an explicitly supplied native viewport."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rect', nargs=4, type=int, required=True, metavar=('X', 'Y', 'W', 'H'))
    parser.add_argument('--seconds', type=int, choices=range(10, 121), required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--camera-only',action='store_true',help='Resumed post-combat stress: camera/rotation/hover/HUD/wheel without new ground orders')
    args = parser.parse_args()
    x0, y0, width, height = args.rect
    if width < 320 or height < 240:
        parser.error('Native viewport is too small')
    x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
    x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
    x.XKeysymToKeycode.argtypes = [c.c_void_p, c.c_ulong]; x.XKeysymToKeycode.restype = c.c_uint
    xt.XTestFakeKeyEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_int, c.c_ulong]
    display = x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not display:raise RuntimeError('No input display')
    start = time.monotonic(); log = []; held = set(); completed = False
    def record(action, **values):log.append(dict(action=action, seconds=time.monotonic()-start, **values))
    def motion(fx, fy):
        px, py = x0+int(width*fx), y0+int(height*fy)
        if not xt.XTestFakeMotionEvent(display, -1, px, py, 0):raise RuntimeError('Motion injection failed')
        x.XFlush(display); record('motion', x=px, y=py)
    def button(b):
        for down in [1, 0]:
            if not xt.XTestFakeButtonEvent(display, b, down, 0):raise RuntimeError('Button injection failed')
            x.XFlush(display); time.sleep(.08)
        record('click', button=b)
    def key(sym, seconds):
        code = x.XKeysymToKeycode(display, sym)
        if not code or not xt.XTestFakeKeyEvent(display, code, 1, 0):raise RuntimeError('Key injection failed')
        held.add(code); x.XFlush(display); time.sleep(seconds)
        xt.XTestFakeKeyEvent(display, code, 0, 0); held.remove(code); x.XFlush(display)
        record('key', keysym=sym, held_seconds=seconds); time.sleep(.1)
    try:
        motion(.93, .87); button(1) # ordinary wizard HUD selection
        if not args.camera_only:motion(.5, .45); button(1)
        while time.monotonic()-start < args.seconds:
            for sym in [0xff53, 0xff54, 0xff51, 0xff52]:key(sym, .45)
            for sym in [ord('.'), ord('.'), ord('.'), ord('.'), ord(','), ord(',')]:key(sym, .12)
            for fx, fy, b in [(.46,.44,1),(.58,.49,3),(.65,.35,3),(.48,.6,1)]:
                motion(fx,fy)
                if not args.camera_only:button(b)
                time.sleep(.15)
            motion(.86,.16)
            if not args.camera_only:button(1) # HUD/minimap edge interaction
            record('spell-hud-and-target-attempt')
            motion(.655,.945);button(1)
            motion(.53,.51)
            if not args.camera_only:button(3)
            time.sleep(.3)
            record('creature-hud-and-source-order-attempt')
            motion(.82,.945);button(1)
            motion(.63,.25)
            if not args.camera_only:button(3)
            time.sleep(.3)
            motion(.93,.87);button(1)
            motion(.5,.45);button(4);button(5)
            time.sleep(.5)
        completed = True
    finally:
        for code in held:xt.XTestFakeKeyEvent(display,code,0,0)
        for b in [1,2,3]:xt.XTestFakeButtonEvent(display,b,0,0)
        x.XFlush(display);x.XCloseDisplay(display)
        args.output.write_text(json.dumps(dict(success=completed, seconds=time.monotonic()-start, actions=log,
            camera_only=args.camera_only,scope='Physical camera keys, rotation, scene/HUD clicks, spell/creature selection and target/source-order attempts, and wheel input. Movement/casting/combat outcomes require separate observation.'),indent=2)+'\n')


if __name__ == '__main__':main()
