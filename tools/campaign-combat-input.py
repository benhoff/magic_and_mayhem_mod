#!/usr/bin/env python3
"""Physical native viewport summon/combat journey with passive original evidence."""
import argparse
import ctypes as c
import json
import math
import os
from pathlib import Path
import subprocess
import time

from campaign_gameplay import read_rows, creatures, damage_rows


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect', nargs=4, type=int, required=True)
    p.add_argument('--seconds', type=int, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--experiment', type=Path, required=True)
    args=p.parse_args(); x0,y0,w,h=args.rect
    if w<320 or h<240:p.error('Native viewport is too small')
    out=args.output.parent; trace=args.experiment/'gameplay-events.bin'
    x=c.CDLL('libX11.so.6'); xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
    x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    display=x.XOpenDisplay(os.environ['DISPLAY'].encode())
    if not display:raise RuntimeError('No native input display')
    start=time.monotonic(); actions=[]; captures=[]; result=dict(success=False)
    def record(action,**values):
        actions.append(dict(action=action,seconds=time.monotonic()-start,**values))
        args.output.write_text(json.dumps(dict(result,seconds=time.monotonic()-start,actions=actions,captures=captures),indent=2)+'\n')
    def click(px,py,b=1):
        if not xt.XTestFakeMotionEvent(display,-1,x0+int(w*px/800),y0+int(h*py/600),0):raise RuntimeError('Motion injection failed')
        x.XFlush(display);record('motion',logical_x=px,logical_y=py);time.sleep(.1)
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,b,down,0):raise RuntimeError('Button injection failed')
            x.XFlush(display);time.sleep(.12)
        record('click',button=b)
    def state():return creatures(read_rows(trace))
    def capture(label, count=False):
        name=f'combat-{len(captures):02d}-{label}'
        request=out/'capture-request.json';temp=out/'capture-request.tmp'
        temp.write_text(json.dumps(dict(name=name)));temp.replace(request)
        deadline=time.monotonic()+10
        while not (out/(name+'.json')).exists():
            if time.monotonic()>deadline:raise RuntimeError('Native capture request timed out')
            time.sleep(.1)
        meta=json.loads((out/(name+'.json')).read_text())
        if not meta['native_saved'] or not meta['original_saved'] or meta['fallback']:raise RuntimeError('Independent native/original capture failed')
        item=dict(name=name,metadata=meta,sequence=read_rows(trace)[-1][0],ocr={})
        if count:
            from PIL import Image
            for route in ('native','original'):
                crop=out/(name+'-'+route+'-count.png')
                Image.open(out/(name+'-'+route+'.png')).crop((716,444,759,466)).resize((272,120)).save(crop)
                text=subprocess.run(['tesseract',str(crop),'stdout','--psm','7','-c','tessedit_char_whitelist=0123456789/'],capture_output=True,text=True,check=True,timeout=10).stdout.strip().replace(' ','')
                item['ocr'][route]=text
        captures.append(item);record('capture',name=name)
        return item
    def center():
        click(744,522);click(744,522);time.sleep(.5);click(400,270);time.sleep(.25)
    try:
        deadline=time.monotonic()+12
        while not trace.exists() or not state():
            if time.monotonic()>deadline:raise RuntimeError('No creature snapshots from original World')
            time.sleep(.2)
        initial=state();wizards=[a for a in initial.values() if a['type']==0 and a['active'] and a['health']>0]
        if len(wizards)!=1:raise RuntimeError('Expected unique living player wizard: '+str(wizards))
        wizard=wizards[0];owner=wizard['owner'];result.update(player_owner=owner,wizard_slot=wizard['slot'],initial_creatures=initial)
        click(400,300);time.sleep(.5) # dismiss ordinary Hermes dialogue
        before=capture('before-summon',True)
        if before['ocr']!={'native':'0/15','original':'0/15'}:raise RuntimeError('Missing initial native/original 0/15 controlled-creature count: '+str(before['ocr']))
        click(523,575);time.sleep(.5);cast_before=read_rows(trace)[-1][0]
        click(400,280,3);record('summon-order',before_sequence=cast_before)
        deadline=time.monotonic()+12; summoned=[]
        while time.monotonic()<deadline:
            summoned=[a for a in state().values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and a['slot'] not in initial]
            if summoned:break
            time.sleep(.25)
        if not summoned:raise RuntimeError('Native right-click did not create a living player Zombie')
        time.sleep(1);after=capture('after-summon',True)
        if after['ocr']!={'native':'1/15','original':'1/15'}:raise RuntimeError('Missing independent native/original 1/15 summon count: '+str(after['ocr']))
        result.update(summon=dict(before_sequence=cast_before,after_sequence=after['sequence'],slot=summoned[0]['slot'],before_capture=before['name'],after_capture=after['name']))
        record('summon-confirmed',slot=summoned[0]['slot'])
        # Permit nearby enemies to engage the actual summoned Zombie first.
        extra_slots=[summoned[0]['slot']]
        for _ in range(15):
            hits=damage_rows(read_rows(trace),owner,extra_slots,cast_before)
            if hits:
                result['combat']=dict(first_hit=list(hits[0]),hit_count=len(hits));capture('combat-damage');break
            time.sleep(1)
        # Public wizard selection/centering and ground movement, calibrated from
        # actual observed tile movement. No writes to positions, AI or health.
        calibration=[]
        if 'combat' not in result:center()
        for dx,dy in (() if 'combat' in result else ((140,-50),(150,0))):
            a=state()[wizard['slot']];click(400+dx,280+dy,3);time.sleep(3)
            b=state()[wizard['slot']];calibration.append((b['x']-a['x'],b['y']-a['y']));center()
        record('movement-calibration',deltas=calibration)
        capture('navigation-start')
        combat_deadline=time.monotonic()+180; extra_slots=[summoned[0]['slot']]
        while 'combat' not in result and time.monotonic()<combat_deadline:
            rows=read_rows(trace);hits=damage_rows(rows,owner,extra_slots,cast_before)
            if hits:
                result['combat']=dict(first_hit=list(hits[0]),hit_count=len(hits));capture('combat-damage');break
            all_creatures=state();actor=all_creatures[wizard['slot']]
            if actor['health']<=0:raise RuntimeError('Player wizard died before verified combat')
            enemies=[a for a in all_creatures.values() if a['active'] and a['health']>0 and a['type']==10 and a['owner']!=owner]
            if not enemies:raise RuntimeError('No living enemy Redcap for combat')
            enemy=min(enemies,key=lambda a:math.hypot(a['x']-actor['x'],a['y']-actor['y']))
            dx,dy=enemy['x']-actor['x'],enemy['y']-actor['y']
            (ax,ay),(bx,by)=calibration;det=ax*by-ay*bx
            if not det:raise RuntimeError('Wizard movement calibration did not move in two directions')
            u=(by*dx-bx*dy)/det;v=(ax*dy-ay*dx)/det
            sx=140*u+150*v;sy=-50*u
            scale=max(1,abs(sx)/170,abs(sy)/110);sx/=scale;sy/=scale
            record('approach-enemy',wizard=actor,enemy=enemy,screen_delta=[sx,sy])
            center();click(400+sx,280+sy,3);time.sleep(3)
            center()
            if math.hypot(dx,dy)<7:
                # Summon beside the enemy, through the same native spell HUD.
                click(523,575);click(400+sx,280+sy,3);time.sleep(2)
                extra_slots=[a['slot'] for a in state().values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0]
                capture('combat-approach')
                time.sleep(3)
        if 'combat' not in result:raise RuntimeError('No player Zombie melee damage before bounded combat deadline')
        while time.monotonic()-start<args.seconds:
            center();time.sleep(1)
        result['success']=True;record('casting-combat-complete')
    except Exception as error:
        result['error']=str(error)
        try:capture('failure')
        except Exception:pass
        raise
    finally:
        for b in (1,2,3):xt.XTestFakeButtonEvent(display,b,0,0)
        x.XFlush(display);x.XCloseDisplay(display)
        record('input-released')


if __name__=='__main__':main()
