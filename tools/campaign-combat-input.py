#!/usr/bin/env python3
"""Physical native viewport summon/combat journey with passive original evidence."""
import argparse
import ctypes as c
import json
import hashlib
import math
import os
from pathlib import Path
import subprocess
import time

from campaign_gameplay import read_rows, creatures, damage_rows, player_lethal_rows
from campaign_spell import read_spell_rows, player_fireball_damage


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect', nargs=4, type=int, required=True)
    p.add_argument('--seconds', type=int, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--experiment', type=Path, required=True)
    p.add_argument('--portrait-stress-seconds', type=int, default=0)
    p.add_argument('--spell-cases', action='store_true')
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
    start=time.monotonic(); actions=[]; captures=[]; result=dict(success=False,monotonic_start=start)
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
        item=dict(name=name,metadata=meta,sequence=read_rows(trace)[-1][0],ocr={},image_sha256={route:hashlib.sha256((out/(name+'-'+route+'.png')).read_bytes()).hexdigest() for route in ('native','original')})
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
        click(744,522);click(744,522);time.sleep(.5);click(400,310);time.sleep(.25)
    def blocked_cast(label, owner, wizard_slot):
        prior_rows=read_rows(trace);begin=prior_rows[-1][0];prior=creatures(prior_rows)
        click(523,575);time.sleep(.25);click(90 if label=='invalid-target' else 440,90 if label=='invalid-target' else 310,3)
        time.sleep(2);current_rows=read_rows(trace);current=creatures(current_rows)
        created=[a['slot'] for a in current.values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and
                 (a['slot'] not in prior or not prior[a['slot']]['active'] or prior[a['slot']]['type']!=14 or prior[a['slot']]['owner']!=owner)]
        if created or current[wizard_slot]['mana']<prior[wizard_slot]['mana']:raise RuntimeError('Expected blocked cast created a Zombie or spent mana')
        item=dict(kind=label,before_sequence=begin,after_sequence=current_rows[-1][0],mana_before=prior[wizard_slot]['mana'],mana_after=current[wizard_slot]['mana'],created_slots=created)
        capture(label,True);result.setdefault('blocked_casts',[]).append(item);record('blocked-cast',**item)
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
        if args.spell_cases:
            center();blocked_cast('invalid-target',owner,wizard['slot'])
        click(523,575);time.sleep(.5);cast_before=read_rows(trace)[-1][0];cast_prior=state()
        click(440,310,3);record('summon-order',before_sequence=cast_before)
        deadline=time.monotonic()+12; summoned=[]
        while time.monotonic()<deadline:
            summoned=[a for a in state().values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and
                      (a['slot'] not in cast_prior or not cast_prior[a['slot']]['active'] or cast_prior[a['slot']]['type']!=14 or cast_prior[a['slot']]['owner']!=owner)]
            if summoned:break
            time.sleep(.25)
        if not summoned:raise RuntimeError('Native right-click did not create a living player Zombie')
        time.sleep(1);after=capture('after-summon',True)
        if after['ocr']!={'native':'1/15','original':'1/15'}:raise RuntimeError('Missing independent native/original 1/15 summon count: '+str(after['ocr']))
        result.update(summon=dict(before_sequence=cast_before,after_sequence=after['sequence'],slot=summoned[0]['slot'],before_capture=before['name'],after_capture=after['name']),additional_summons=[])
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
        calibration=[]; probes=[]
        if 'combat' not in result:center()
        # Ground can be occupied or unreachable; two idle clicks are not an
        # input failure. Try bounded directions and retain the actual basis.
        for dx,dy in (() if 'combat' in result else ((140,-50),(150,0),(-140,50),(-140,-60),(0,110),(0,-110),(100,90),(-100,90))):
            a=state()[wizard['slot']];begin=read_rows(trace)[-1][0]
            click(400+dx,280+dy,3);time.sleep(3)
            b=state()[wizard['slot']]
            delta=(b['x']-a['x'],b['y']-a['y'])
            probes.append(dict(before_sequence=begin,after_sequence=b['sequence'],screen_delta=[dx,dy],tile_delta=list(delta)))
            record('movement-probe',**probes[-1]);center()
            if delta!=(0,0):
                if not calibration:calibration.append((delta,(dx,dy)))
                elif calibration[0][0][0]*delta[1]-calibration[0][0][1]*delta[0]:
                    calibration.append((delta,(dx,dy)));break
        result['movement_probes']=probes
        record('movement-calibration',basis=calibration)
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
            if len(calibration)!=2:raise RuntimeError('No independent movement after bounded ground probes')
            ((ax,ay),(asx,asy)),((bx,by),(bsx,bsy))=calibration;det=ax*by-ay*bx
            if not det:raise RuntimeError('Wizard movement calibration did not move in two directions')
            u=(by*dx-bx*dy)/det;v=(ax*dy-ay*dx)/det
            sx=asx*u+bsx*v;sy=asy*u+bsy*v
            scale=max(1,abs(sx)/170,abs(sy)/110);sx/=scale;sy/=scale
            record('approach-enemy',wizard=actor,enemy=enemy,screen_delta=[sx,sy])
            center();click(400+sx,280+sy,3);time.sleep(3)
            center()
            if math.hypot(dx,dy)<7:
                if args.spell_cases and not result.get('ranged'):
                    # The starting green spell is Fireball71. Cast at nearby
                    # enemy terrain and require damage in its original effect.
                    for px,py in ((400,270),(400,240)):
                        begin=read_rows(trace)[-1][0];click(477,575);time.sleep(.25);click(px,py,3);time.sleep(1)
                        effects=player_fireball_damage(read_spell_rows(args.experiment/'spell-events.bin'),owner,wizard['slot'])
                        record('fireball-order',before_sequence=begin,logical_x=px,logical_y=py)
                        if effects:result['ranged']=dict(first_damage=list(effects[0]));capture('fireball-damage');break
                    if not result.get('ranged'):raise RuntimeError('No player Fireball enemy damage after bounded shots')
                # Summon beside the enemy, through the same native spell HUD.
                click(523,575);cast_sequence=read_rows(trace)[-1][0];prior=state()
                click(400+sx,280+sy,3);time.sleep(2)
                current=state()
                for a in current.values():
                    old=prior.get(a['slot'])
                    if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and (not old or not old['active'] or old['type']!=14 or old['owner']!=owner):
                        result['additional_summons'].append(dict(before_sequence=cast_sequence,after_sequence=a['sequence'],slot=a['slot']))
                extra_slots=[a['slot'] for a in state().values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0]
                capture('combat-approach')
                time.sleep(3)
        if 'combat' not in result:raise RuntimeError('No player Zombie melee damage before bounded combat deadline')
        finish_deadline=time.monotonic()+60
        while True:
            rows=read_rows(trace);hits=damage_rows(rows,owner,extra_slots,cast_before)
            lethal=player_lethal_rows(rows,owner,wizard['slot'],hits)
            if lethal and time.monotonic()-start>=args.seconds:
                result['combat'].update(lethal_hit=list(lethal[0]),hit_count=len(hits));break
            if state()[wizard['slot']]['health']<=0:raise RuntimeError('Player wizard died before required combat completion')
            if time.monotonic()>finish_deadline:raise RuntimeError('No player party lethal health depletion before combat deadline')
            if args.portrait_stress_seconds:
                center();record('combat-portrait-recenter');time.sleep(1);continue
            # Continue observing the fight without repeatedly reselecting and
            # recentering the wizard. Hover ordinary empty terrain through the
            # native viewport; retained negative runs cover portrait stress.
            px,py=((180,380),(600,350))[len(actions)%2]
            if not xt.XTestFakeMotionEvent(display,-1,x0+int(w*px/800),y0+int(h*py/600),0):raise RuntimeError('Motion injection failed')
            x.XFlush(display);record('combat-terrain-hover',logical_x=px,logical_y=py);time.sleep(.5)
        record('casting-combat-complete')
        if args.portrait_stress_seconds or args.spell_cases:
            # Use ordinary ground orders to survive later enemy
            # waves while exercising rendering; no health/position writes.
            center()
            for attempt in range(4):
                actor=state()[wizard['slot']];dx=wizard['x']-actor['x'];dy=wizard['y']-actor['y']
                if math.hypot(dx,dy)<5:break
                if len(calibration)==2:
                    ((ax,ay),(asx,asy)),((bx,by),(bsx,bsy))=calibration;det=ax*by-ay*bx
                    u=(by*dx-bx*dy)/det;v=(ax*dy-ay*dx)/det;sx=asx*u+bsx*v;sy=asy*u+bsy*v
                    scale=max(1,abs(sx)/170,abs(sy)/110);sx/=scale;sy/=scale
                else:sx,sy=-140,50
                center();click(400+sx,280+sy,3);time.sleep(3)
                record('post-combat-retreat',attempt=attempt+1,wizard=state()[wizard['slot']])
            center()
        if args.spell_cases:
            if state()[wizard['slot']]['mana']>=17*256:raise RuntimeError('Insufficient-mana case was not reached through normal casts')
            blocked_cast('insufficient-mana',owner,wizard['slot'])
        if args.portrait_stress_seconds:
            portrait_start=time.monotonic();cycles=0
            record('portrait-stress-start')
            while time.monotonic()-portrait_start<args.portrait_stress_seconds:
                if state()[wizard['slot']]['health']<=0:raise RuntimeError('Player wizard died during portrait stress')
                center();cycles+=1;record('portrait-recenter',cycle=cycles);time.sleep(1)
            result['portrait_stress']=dict(seconds=time.monotonic()-portrait_start,cycles=cycles)
            capture('portrait-stress');record('portrait-stress-complete')
        result['success']=True
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
