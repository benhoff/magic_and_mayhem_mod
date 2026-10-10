#!/usr/bin/env python3
"""Ordinary native Quick Battle input with passive original Cure proof."""
import argparse
import ctypes as c
import hashlib
import json
import math
import os
from pathlib import Path
import time
from campaign_gameplay import creatures, read_rows
from campaign_spell import read_spell_rows, player_cure_healing


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect',nargs=4,type=int,required=True);p.add_argument('--experiment',type=Path,required=True)
    p.add_argument('--owner',type=int,required=True);p.add_argument('--seconds',type=int,required=True);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--crowded',action='store_true')
    a=p.parse_args();x0,y0,w,h=a.rect
    if w<320 or h<240:p.error('Native viewport too small')
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p;x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong];xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    display=x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not display:raise RuntimeError('No native healing display')
    start=time.monotonic();actions=[];captures=[];result=dict(success=False,owner=a.owner)
    def record(action,**values):
        actions.append(dict(action=action,seconds=time.monotonic()-start,**values));a.output.write_text(json.dumps(dict(result,actions=actions,captures=captures,seconds=time.monotonic()-start),indent=2)+'\n')
    def state():return creatures(read_rows(a.experiment/'gameplay-events.bin'))
    def click(px,py,b=1):
        if not xt.XTestFakeMotionEvent(display,-1,x0+round(w*px/800),y0+round(h*py/600),0):raise RuntimeError('Native healing motion failed')
        x.XFlush(display);time.sleep(.1)
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,b,down,0):raise RuntimeError('Native healing click failed')
            x.XFlush(display);time.sleep(.12)
        record('click',logical_point=[px,py],button=b)
    def center():click(744,522);click(744,522);time.sleep(.75)
    def capture(label):
        name=f'healing-{len(captures):02d}-{label}';request=a.output.parent/'capture-request.json';temp=request.with_suffix('.tmp');temp.write_text(json.dumps(dict(name=name)));temp.replace(request)
        meta_path=a.output.parent/(name+'.json');deadline=time.monotonic()+10
        while not meta_path.exists():
            if time.monotonic()>deadline:raise RuntimeError('Native healing capture timed out')
            time.sleep(.1)
        meta=json.loads(meta_path.read_text())
        if meta['fallback'] or not meta['native_saved'] or not meta['original_saved']:raise RuntimeError('Independent healing capture failed')
        item=dict(name=name,metadata=meta,sequence=read_rows(a.experiment/'gameplay-events.bin')[-1][0],image_sha256={r:hashlib.sha256((a.output.parent/(name+'-'+r+'.png')).read_bytes()).hexdigest() for r in ('native','original')});captures.append(item);record('capture',name=name);return item
    try:
        initial=state();actors=[v for v in initial.values() if v['type']==0 and v['owner']==a.owner and v['active'] and v['health']>0]
        if len(actors)!=1:raise RuntimeError('No unique original human wizard')
        wizard=actors[0];slot=wizard['slot'];result.update(wizard_slot=slot,initial_wizard=wizard,initial_creatures=initial);center();capture('before-orders')
        # The native loadout holds Cure as its first Law item and Zombie as its
        # only Chaos item. This two-item HUD centers Zombie523 and Cure573. Original cast IDs, not artwork, determine outcomes.
        summon_attempts=8 if a.crowded else 1
        for attempt in range(summon_attempts):
            before=state();begin=read_rows(a.experiment/'gameplay-events.bin')[-1][0]
            click(523,575);click(440+(attempt%3)*15,310+(attempt//3)*15,3);time.sleep(2);center()
            after=state();created=[v for k,v in after.items() if v['type']==14 and v['owner']==a.owner and v['active'] and v['health']>0 and (k not in before or not before[k]['active'] or before[k]['health']<=0)]
            record('summon-attempt',before_sequence=begin,after_sequence=read_rows(a.experiment/'gameplay-events.bin')[-1][0],created_slots=[v['slot'] for v in created])
        basis=[];probes=[]
        for sx,sy in [(140,-50),(150,0),(-140,50),(-140,-60),(0,110),(0,-110),(100,90),(-100,90)]:
            actor=state()[slot]
            if actor['health']<wizard['health']:break
            begin=read_rows(a.experiment/'gameplay-events.bin')[-1][0];click(400+sx,280+sy,3);time.sleep(3);after=state()[slot]
            delta=(after['x']-actor['x'],after['y']-actor['y']);probes.append(dict(before_sequence=begin,after_sequence=after['sequence'],screen_delta=[sx,sy],tile_delta=list(delta)));record('movement-probe',**probes[-1]);center()
            if delta!=(0,0):
                if not basis:basis.append((delta,(sx,sy)))
                elif basis[0][0][0]*delta[1]-basis[0][0][1]*delta[0]:basis.append((delta,(sx,sy)));break
        result['movement_probes']=probes;deadline=time.monotonic()+180
        while time.monotonic()<deadline:
            current=state();actor=current[slot]
            if not actor['active'] or actor['health']<=0:raise RuntimeError('Original human wizard died before Cure')
            if actor['health']<wizard['health']:
                center();capture('before-cure');record('cure-injured-wizard',wizard=actor)
                injury_begin=actor['sequence']
                for point in [(400,275),(400,295),(370,275),(430,275)]:
                    begin=read_rows(a.experiment/'gameplay-events.bin')[-1][0];click(573,575);click(*point,3);time.sleep(3)
                    heals=player_cure_healing(read_spell_rows(a.experiment/'spell-events.bin'),a.owner,slot)
                    record('cure-order',before_sequence=begin,logical_point=list(point))
                    if heals:
                        result['healing']=dict(first_health_change=list(heals[0]),before_sequence=injury_begin,after_sequence=read_rows(a.experiment/'gameplay-events.bin')[-1][0]);capture('after-cure');break
                    center()
                if result.get('healing'):break
                raise RuntimeError('No original player Cure health increase after bounded native targets')
            enemies=[v for v in current.values() if v['active'] and v['health']>0 and v['owner'] not in (a.owner,0xffffffff) and (a.crowded or v['type'] in (0,10,14,1,11))]
            if not enemies:raise RuntimeError('No original opponent to exercise injury/healing')
            if len(basis)!=2:raise RuntimeError('No independent native movement basis for healing route')
            enemy=min(enemies,key=lambda v:math.hypot(v['x']-actor['x'],v['y']-actor['y']));dx=enemy['x']-actor['x'];dy=enemy['y']-actor['y']
            ((ax,ay),(asx,asy)),((bx,by),(bsx,bsy))=basis;det=ax*by-ay*bx;u=(by*dx-bx*dy)/det;v=(ax*dy-ay*dx)/det;sx=asx*u+bsx*v;sy=asy*u+bsy*v;scale=max(1,abs(sx)/170,abs(sy)/110)
            center();click(400+sx/scale,280+sy/scale,3);record('approach-opponent',wizard=actor,enemy=enemy);time.sleep(3);center()
        if not result.get('healing'):raise RuntimeError('No successful native Cure before bounded deadline')
        while time.monotonic()-start<a.seconds:
            if a.crowded:
                actor=state()[slot]
                if not actor['active'] or actor['health']<=0:raise RuntimeError('Original human wizard died during crowded stress')
                center()
                if actor['health'] < wizard['health']-50 and actor['mana'] >= 20*256:
                    click(573,575);click(400,275,3);record('stress-cure',wizard=actor)
                else:
                    # Physical camera scroll, rotation and HUD selection. Restore
                    # the wizard view so its summoned squad remains in view.
                    xt.XTestFakeButtonEvent(display,4,1,0);xt.XTestFakeButtonEvent(display,4,0,0);x.XFlush(display)
                    record('stress-camera-and-portrait',wizard=actor)
                time.sleep(1)
            else:time.sleep(.2)
        if a.crowded:capture('crowded-end')
        result['success']=True
    except Exception as error:
        result['error']=str(error)
        try:capture('failure')
        except Exception:pass
        raise
    finally:
        for b in (1,2,3):xt.XTestFakeButtonEvent(display,b,0,0)
        x.XFlush(display);x.XCloseDisplay(display);record('input-released')

if __name__=='__main__':main()
