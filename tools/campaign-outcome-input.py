#!/usr/bin/env python3
"""Candidate Forest of Pain physical-input driver; no simulation memory writes.

Movement/projection and dialogue dismissal are bounded test heuristics. A timeout
or combat failure is negative evidence, never a manufactured win or defeat.
"""
import argparse
import ctypes as c
import json
import math
import os
from pathlib import Path
import struct
import time
from campaign_gameplay import read_rows, creatures
from rendering_progression import CASES, require


def target_point(hero, target, body=False):
    dx, dy, dz = (target[k]-hero[k] for k in ('x', 'y', 'z'))
    sx, sy = 32*(dx-dy), 16*(dx+dy-dz)
    scale = max(1, abs(sx)/230, abs(sy)/130)
    return round(400+sx/scale), round(300+sy/scale-(22 if body else 0))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect', nargs=4, type=int, required=True)
    p.add_argument('--case', choices=CASES, required=True)
    p.add_argument('--experiment', type=Path, required=True)
    p.add_argument('--seconds', type=int, default=540)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    require(30 <= a.seconds <= 600 and a.rect[2] >= 320 and a.rect[3] >= 240, 'Invalid bounded viewport/deadline')
    result = dict(success=False, case=a.case, actions=[], health_observations=[],
                  scope=__doc__, simulation_writes=False, semantic_outcome_accepted=False)
    display = None
    x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
    x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
    xt.XTestFakeMotionEvent.argtypes = [c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes = [c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    start = time.monotonic(); held = set()
    try:
        display = x.XOpenDisplay(os.environ['DISPLAY'].encode())
        require(display, 'Missing owned native display')
        def click(px, py, button=1):
            require(0 <= px < 800 and 0 <= py < 600, 'Projection outside logical viewport')
            x0,y0,w,h = a.rect
            require(xt.XTestFakeMotionEvent(display, -1, x0+round(px*w/800), y0+round(py*h/600), 0), 'Motion failed')
            x.XFlush(display); time.sleep(.1)
            for down in (1,0):
                require(xt.XTestFakeButtonEvent(display, button, down, 0), 'Click failed')
                if down:held.add(button)
                else:held.discard(button)
                x.XFlush(display);time.sleep(.12)
            result['actions'].append(dict(seconds=time.monotonic()-start, point=[px,py], button=button))
        last_sequence = 0; last_change = start; portal = None
        while time.monotonic()-start < a.seconds:
            # The passive menu observer owns these records. UI report OK may
            # already have run; inspect the whole prefix, not only its last row.
            menu = (a.experiment/'events.bin').read_bytes()
            require(menu[:16] == b'MNMMENU1'+struct.pack('<II',1,64), 'Invalid original event header')
            end = 16+(len(menu)-16)//64*64
            events = list(struct.iter_unpack('<16I',menu[16:end]))
            if any(r[1] == 28 and r[11] == 0x6db967 for r in events):
                result.update(success=True, original_report_observed=True)
                break
            trace = a.experiment/'gameplay-events.bin'
            if not trace.exists():time.sleep(.2);continue
            rows = read_rows(trace); actors = creatures(rows)
            if not rows:time.sleep(.2);continue
            if rows[-1][0] != last_sequence:last_sequence=rows[-1][0];last_change=time.monotonic()
            heroes = [v for v in actors.values() if v['type']==0 and v['owner']==0 and v['active']]
            require(len(heroes) <= 1, 'Ambiguous player wizard')
            if not heroes:time.sleep(.2);continue
            hero = heroes[0]
            if len(result['health_observations']) < 1024:
                result['health_observations'].append(dict(sequence=hero['sequence'], health=hero['health'], slot=hero['slot']))
            # Dismiss stalled original conversations through ordinary input.
            if time.monotonic()-last_change > 2:
                click(400,300);time.sleep(.4);continue
            if hero['health'] <= 0:
                require(a.case == 'defeat', 'Player died before the candidate victory')
                time.sleep(.3);continue
            enemies = [v for v in actors.values() if v['active'] and v['owner'] not in (0,0xffffffff) and v['health'] > 0]
            camulos = [v for v in actors.values() if v['owner']==1 and v['type']==24]
            if camulos and camulos[0]['health'] <= 0:portal=camulos[0]
            if a.case != 'defeat' and portal is not None:target=portal
            elif a.case != 'defeat' and camulos and camulos[0]['health'] > 0:target=camulos[0]
            elif enemies:target=min(enemies,key=lambda e:math.hypot(e['x']-hero['x'],e['y']-hero['y']))
            else:time.sleep(.3);continue
            # Ordinary HUD focus, then ground movement or attack. No selection
            # of enemy health, win flags, portal state or campaign destinations.
            click(744,522);click(744,522);time.sleep(.3)
            distance=math.hypot(target['x']-hero['x'],target['y']-hero['y'])
            attack = a.case != 'defeat' and portal is None and distance < 7
            point=target_point(hero,target,attack)
            if a.case == 'defeat' and distance < 2:
                # Ground movement brings the player into hostile melee range;
                # idle instead of attacking or using an abandon command.
                time.sleep(1)
            else:click(*point,3);time.sleep(.7)
        require(result['success'], 'Natural report not reached within bounded ordinary-input attempt')
    except Exception as error:
        result['error'] = str(error)
    finally:
        if display:
            for button in held:xt.XTestFakeButtonEvent(display,button,0,0)
            x.XFlush(display);x.XCloseDisplay(display)
        result['seconds'] = time.monotonic()-start
        # Bounded physical interchange; the acquisition collector imports this
        # exact file to SQLite, including failed attempts.
        a.output.write_text(json.dumps(result,indent=2)+'\n')
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
