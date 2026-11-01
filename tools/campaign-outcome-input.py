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
from campaign_spell import read_spell_rows, player_fireball_damage
from campaign_input_admission import spell_selection_points
from rendering_progression import CASES, require


def target_point(hero, target, body=False):
    dx, dy, dz = (target[k]-hero[k] for k in ('x', 'y', 'z'))
    sx, sy = 32*(dx-dy), 16*(dx+dy-dz)
    scale = max(1, abs(sx)/230, abs(sy)/130)
    return round(400+sx/scale), round(300+sy/scale-(22 if body else 0))


def combat_choice(case, hero, target, nearby_support, casts, since_spell):
    """Ordinary-play test policy, not recovered AI or spell admission rules."""
    distance=math.hypot(target['x']-hero['x'],target['y']-hero['y'])
    if case=='defeat':
        return 'idle' if distance<2 else 'move'
    if target['health']<=0:
        return 'portal'
    if nearby_support==0 and casts==0 and hero['mana']>=17*256:
        return 'initial-zombie'
    if distance<7:
        if nearby_support<2 and casts<6 and hero['mana']>=17*256 and since_spell>=6:
            return 'zombie'
        if hero['mana']>=6*256 and since_spell>=3:
            return 'fireball'
        return 'retreat' if hero['health']<80 and distance<4 else 'idle'
    return 'move'


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
    def save():
        # The Qt owner can terminate a failed helper before finally executes.
        # Preserve a bounded, atomic interchange snapshot throughout the run.
        temporary=a.output.with_suffix('.tmp')
        temporary.write_text(json.dumps(dict(result,seconds=time.monotonic()-start),indent=2)+'\n')
        temporary.replace(a.output)
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
            save()
        def state():return creatures(read_rows(a.experiment/'gameplay-events.bin'))
        def center():
            click(744,522);click(744,522);time.sleep(.4)
        def select(spell):
            for point in spell_selection_points(spell):click(*point);time.sleep(.2)
        last_sequence = 0; last_change = start; portal = None
        last_spell=start-10;pending=None;summons=0;fireballs=0
        result.update(casts=[],combat_policy='observed-support-and-fireball-v1')
        save()
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
            if pending:
                spells=read_spell_rows(a.experiment/'spell-events.bin')
                accepted=[r for r in spells if r[0]>pending['spell_sequence'] and r[1]==1
                    and r[4]==pending['id'] and r[5]==hero['slot'] and r[7]==hero['owner']
                    and r[15]==1 and r[8]>r[9]]
                if accepted:
                    pending['accepted']=list(accepted[0])
                    if pending['id']==71 or any(v['type']==14 and v['owner']==0 and v['active'] and v['health']>0
                        and v['slot'] not in pending['prior_support'] for v in actors.values()):
                        result['casts'].append(pending)
                        if pending['id']==14:summons+=1
                        else:fireballs+=1
                        pending=None;result['pending_cast']=None;save()
                if pending:
                    elapsed=time.monotonic()-pending['started']
                    if elapsed>30 and 'accepted' in pending:raise RuntimeError('Accepted summon lacks an observed living owned Zombie')
                    if elapsed>4 and not accepted:
                        result['casts'].append(dict(pending,refused_or_unobserved=True));pending=None;result['pending_cast']=None;save()
                    else:time.sleep(.25);continue
            enemies = [v for v in actors.values() if v['active'] and v['owner'] not in (0,0xffffffff) and v['health'] > 0]
            camulos = [v for v in actors.values() if v['owner']==1 and v['type']==24]
            if camulos and camulos[0]['health'] <= 0:portal=camulos[0]
            if a.case != 'defeat' and portal is not None:target=portal
            elif a.case != 'defeat' and camulos and camulos[0]['health'] > 0:target=camulos[0]
            elif enemies:target=min(enemies,key=lambda e:math.hypot(e['x']-hero['x'],e['y']-hero['y']))
            else:time.sleep(.3);continue
            # Center before reading fresh positions. Do not infer a camera
            # basis from obstacle-limited movement or walk an unsupported wizard
            # into a hostile army without using its ordinary starting spells.
            center();actors=state();hero=actors[hero['slot']]
            if not hero['active'] or hero['health']<=0:
                save();time.sleep(.2);continue
            target=actors.get(target['slot'],target) if portal is None else portal
            support=[v for v in actors.values() if v['type']==14 and v['owner']==0 and v['active'] and v['health']>0]
            nearby=sum(math.hypot(v['x']-hero['x'],v['y']-hero['y'])<10 for v in support)
            choice=combat_choice(a.case,hero,target,nearby,summons,time.monotonic()-last_spell)
            result['last_decision']=dict(choice=choice,hero=hero,target=target,nearby_support=nearby)
            if choice in ('initial-zombie','zombie','fireball'):
                require(fireballs<24,'Bounded physical Fireball budget exhausted')
                require(len(result['casts'])<40,'Bounded physical cast-attempt budget exhausted')
                spell='fireball' if choice=='fireball' else 'zombie'
                select(spell);actors=state();hero=actors[hero['slot']];target=actors.get(target['slot'],target)
                point=(440,310) if choice=='initial-zombie' else target_point(hero,target,spell=='fireball')
                if choice=='zombie':point=(min(650,point[0]+32),min(430,point[1]+16))
                spell_rows=read_spell_rows(a.experiment/'spell-events.bin')
                pending=dict(id=71 if spell=='fireball' else 14,spell_sequence=spell_rows[-1][0] if spell_rows else 0,
                    started=time.monotonic(),prior_support=[v['slot'] for v in support],point=list(point),hero=hero,target=target)
                result['pending_cast']=pending;save();click(*point,3);last_spell=time.monotonic()
            elif choice=='retreat':
                point=target_point(hero,dict(x=2*hero['x']-target['x'],y=2*hero['y']-target['y'],z=hero['z']))
                click(*point,3)
            elif choice!='idle':click(*target_point(hero,target),3)
            result['player_fireball_damage']=[list(r) for r in player_fireball_damage(read_spell_rows(a.experiment/'spell-events.bin'),0,hero['slot'])]
            save();time.sleep(.7 if choice!='idle' else 1)
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
        save()
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
