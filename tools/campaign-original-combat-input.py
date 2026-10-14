#!/usr/bin/env python3
"""Bounded physical-input first-mission driver; reads passive records only.

Projection and flank selection are test heuristics, not recovered simulation.
Never writes game memory, health, mana, experience or campaign flags.
Stops at a defeated Camulos for inspected original portal/report handling.
"""
import argparse
import ctypes as c
import json
import math
import struct
import time
from pathlib import Path
from PIL import ImageGrab
from campaign_gameplay import read_rows, creatures

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--session', type=Path, required=True)
    ap.add_argument('--seconds', type=int, default=360)
    a = ap.parse_args()
    if not 30 <= a.seconds <= 600: ap.error('seconds must be 30..600')
    session = json.loads(a.session.read_text()); out = a.session.parent
    if session.get('original_display') is not True: raise ValueError('Requires original display')
    trace = Path(session['experiment']) / 'gameplay-events.bin'
    x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
    x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
    x.XKeysymToKeycode.argtypes = [c.c_void_p,c.c_ulong]; x.XKeysymToKeycode.restype = c.c_uint
    xt.XTestFakeMotionEvent.argtypes = [c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes = [c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeKeyEvent.argtypes = [c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    d = x.XOpenDisplay(session['display'].encode())
    if not d: raise RuntimeError('Private display unavailable')
    actions = []; steps = []; success = False; error = ''
    def record(kind, **fields):
        row = dict(time=time.time(), kind=kind, **fields); actions.append(row)
        with (out / 'combat-actions.jsonl').open('a') as f: f.write(json.dumps(row)+'\n')
    def click(px, py, button=1):
        px, py = round(px), round(py)
        if not 0 <= px < 800 or not 0 <= py < 600: raise ValueError('Input outside original viewport')
        xt.XTestFakeMotionEvent(d,-1,px,py,0); x.XFlush(d); time.sleep(.03)
        for down in (1,0):
            xt.XTestFakeButtonEvent(d,button,down,0); x.XFlush(d); time.sleep(.07)
        record('click', x=px,y=py,button=button)
    def key(sym):
        code=x.XKeysymToKeycode(d,sym)
        for down in (1,0):
            xt.XTestFakeKeyEvent(d,code,down,0); x.XFlush(d); time.sleep(.15)
        record('key', keysym=sym)
    def state(): return creatures(read_rows(trace))
    def hero(rows):
        values=[p for p in rows.values() if p['active'] and p['type']==0 and p['owner']==0 and p['health']>0]
        if len(values)!=1: raise RuntimeError('Expected one living original player wizard')
        return values[0]
    def enemy(rows):
        values=[p for p in rows.values() if p['active'] and p['type']==24 and p['owner']==1]
        if len(values)!=1: raise RuntimeError('Expected the fixed first-mission Camulos role')
        return values[0]
    def capture(name): ImageGrab.grab(xdisplay=session['display']).save(out / (name+'.png'))
    def center():
        click(744,522); click(744,522); time.sleep(.4)
    def projection(h,p,body=True):
        dx,dy,dz=p['x']-h['x'],p['y']-h['y'],p['z']-h['z']
        return 400+32*(dx-dy),300+16*(dx+dy-dz)-(22 if body else 0)
    def blocked(h,e,rows):
        dx,dy=e['x']-h['x'],e['y']-h['y']; length=dx*dx+dy*dy
        if not length:return False
        for p in rows.values():
            if p['owner']!=0 or not p['active'] or p['health']<=0 or p['slot']==h['slot']:continue
            t=((p['x']-h['x'])*dx+(p['y']-h['y'])*dy)/length
            if .05<t<.98 and math.hypot(p['x']-h['x']-t*dx,p['y']-h['y']-t*dy)<.65:return True
        return False
    try:
        # One unchanged checkpoint is staged; original menu/writer handles loading.
        deadline=time.monotonic()+90
        events=Path(session['experiment'])/'events.bin'
        while time.monotonic()<deadline:
            if events.exists():
                b=events.read_bytes();end=16+(len(b)-16)//64*64
                records=list(struct.iter_unpack('<16I',b[16:end])) if len(b)>=16 else []
                if any(r[3]==3 and r[6]==1 and not r[8] and not r[14] for r in records):break
            time.sleep(.2)
        else:raise RuntimeError('Original Main readiness timed out')
        time.sleep(.5);click(400,380);time.sleep(1.5);capture('auto-load')
        click(145,555);time.sleep(3)
        start=time.monotonic(); talked=False; previous=None; stalled=0
        while time.monotonic()-start<a.seconds:
            rows=state();h=hero(rows);e=enemy(rows)
            if e['health']<=0:
                success=True;capture('auto-camulos-defeated');key(65307);break
            if h['health']<45:raise RuntimeError('Wizard health below bounded retreat threshold')
            if previous==h['sequence']:stalled+=1
            else:stalled=0
            previous=h['sequence']
            if stalled>=2:
                # Original conversations stop World sampling. Ordinary clicks
                # advance the fixed speech area; do not alter script completion.
                capture('auto-dialogue')
                for _ in range(7):click(310,150);time.sleep(.25)
                talked=True;stalled=0
                continue
            center();rows=state();h=hero(rows);e=enemy(rows)
            distance=math.hypot(e['x']-h['x'],e['y']-h['y'])
            record('roles', wizard=h,camulos=e,distance=distance)
            steps.append(dict(wizard=h,camulos=e))
            if distance>9:
                px,py=projection(h,e,False);sx,sy=px-400,py-300
                scale=max(1,abs(sx)/240,abs(sy)/110)
                click(400+sx/scale,300+sy/scale,3);time.sleep(2)
                continue
            if not talked:
                # Allow the original range trigger to speak before combat input.
                time.sleep(1);new=state()
                if new[h['slot']]['sequence']==h['sequence']:continue
                talked=True
            px,py=projection(h,e)
            if blocked(h,e,rows) or not (90<=px<=710 and 100<=py<=470):
                candidates=[]
                for dx,dy in [(4,0),(0,4),(-4,0),(0,-4)]:
                    p=dict(h,x=e['x']+dx,y=e['y']+dy,z=e['z'])
                    candidates.append((blocked(p,e,rows),math.hypot(p['x']-h['x'],p['y']-h['y']),p))
                _,_,p=min(candidates,key=lambda item:item[:2])
                px,py=projection(h,p,False);sx,sy=px-400,py-300
                scale=max(1,abs(sx)/230,abs(sy)/110)
                click(400+sx/scale,300+sy/scale,3);time.sleep(1.5)
                continue
            if h['mana']<6*256:time.sleep(1);continue
            # Switch through another spell so an already selected Fireball
            # cannot be inadvertently toggled off by a repeated icon click.
            click(524,575);click(477,575)
            rows=state();h=hero(rows);e=enemy(rows);px,py=projection(h,e)
            if 90<=px<=710 and 100<=py<=470:
                click(px,py,3);time.sleep(1.5)
            else:time.sleep(.4)
        if not success:raise RuntimeError('Bounded Camulos pursuit did not complete')
    except Exception as exc:
        error=str(exc)
        try:capture('auto-bounded-failure');key(65307)
        except Exception:pass
    finally:
        x.XCloseDisplay(d)
    (out/'combat-input-report.json').write_text(json.dumps(dict(success=success,error=error,steps=steps,
        scope='Original physical inputs and passive role checks; projection/flank heuristics, no simulation or progress writes'),indent=2)+'\n')
    return 0 if success else 1

if __name__=='__main__':raise SystemExit(main())
