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
import sys
import time
from campaign_gameplay import read_rows, creatures, signed
from campaign_dialogue import ActorQuietWindow
from campaign_spell import read_spell_rows, player_fireball_damage
from campaign_input_admission import spell_selection_points
from rendering_progression import CASES, require
from campaign_report import report_callback
from campaign_return import read_realm_return_state
from campaign_route import frame_size, hud_point, approach_point, aim_point, follow_point, forest_waypoints, validate_opening_anchors, bridge_entry_ready
from campaign_input_observation import decode as input_observations
from campaign_input_admission import fresh_hover
from campaign_follow import read_rows as read_follow_rows, followed_wizard
from campaign_order_observation import decode as order_observations
from campaign_target_admission import candidate_after_hover,hover_nudge,support_attack_submission,reserved_boss_target
from campaign_outcome_record import AtomicSnapshotWriter


def target_point(hero, target, body=False):
    dx, dy, dz = (target[k]-hero[k] for k in ('x', 'y', 'z'))
    sx, sy = 32*(dx-dy), 16*(dx+dy-dz)
    scale = max(1, abs(sx)/230, abs(sy)/130)
    return round(400+sx/scale), round(300+sy/scale-(22 if body else 0))


def combat_choice(case, hero, target, nearby_support, casts, since_spell, support_ordered=False):
    """Ordinary-play test policy, not recovered AI or spell admission rules."""
    distance=math.hypot(target['x']-hero['x'],target['y']-hero['y'])
    if case=='defeat':
        return 'idle' if distance<2 else 'move'
    if target['health']<=0:
        return 'portal'
    if support_ordered and target.get('type')==24 and hero['mana']<6*256:
        return 'retreat' if distance<8 else 'idle'
    if distance<7:
        if hero['mana']>=6*256 and since_spell>=3:
            return 'fireball'
        return 'retreat' if hero['mana']<6*256 or hero['health']<80 else 'idle'
    return 'move'


def recent_melee_attackers(rows, hero, actors, follow_slot):
    """Bounded input-policy witness when an actor's target field clears."""
    if not rows or not hero['active'] or hero['health']<=0:return set()
    protected={(hero['slot'],hero['type'],hero['owner'])}
    follower=actors.get(follow_slot)
    if follower and follower['active'] and follower['health']>0 and follower['owner']==0 and follower['type']==14:
        protected.add((follow_slot,14,0))
    now=rows[-1][3]
    return {tuple(r[4:7]) for r in rows if r[1]==2 and tuple(r[8:11]) in protected
            and r[6] not in (0,0xffffffff) and signed(r[7])>0 and r[13]
            and signed(r[11])>signed(r[12]) and ((now-r[3])&0xffffffff)<=3000}


def engaged_threats(hero, actors, follow_slot, radius=10, recent=()):
    """Forest input policy: protect living owned actors from nearby attackers.

    Enemy ownership/type is observed, not inferred from the initial bridge
    Redcap. Camulos keeps the separate boss casting/portal path.
    """
    if not hero['active'] or hero['health']<=0 or hero['owner']!=0 or hero['type']!=0:return []
    protected={hero['slot']}
    follower=actors.get(follow_slot)
    if follower and follower['active'] and follower['health']>0 and follower['owner']==0 and follower['type']==14:
        protected.add(follow_slot)
    return [v for v in actors.values() if v['active'] and v['health']>0
            and v['owner'] not in (0,0xffffffff)
            and (v['target'] in protected or (v['slot'],v['type'],v['owner']) in recent)
            and not (v['type']==24 and v['owner']==1)
            and math.hypot(v['x']-hero['x'],v['y']-hero['y'])<=radius]


def route_combat_needed(phase, rearmed, hero, actors, follow_slot, recent=()):
    """Suspend resupply or post-refill staging for affordable nearby combat."""
    return (phase=='resupply' or phase=='bridge' and rearmed) and hero['mana']>=6*256 and bool(engaged_threats(hero,actors,follow_slot,7,recent))


def firing_line_allies(hero, target, actors, radius=.8):
    return firing_line_blockers(hero,target,actors,radius,friendly_only=True)


def firing_line_blockers(hero, target, actors, radius=.8, friendly_only=False):
    """Conservative physical-input guard; no claim about recovered collision."""
    dx,dy=target['x']-hero['x'],target['y']-hero['y']
    length=dx*dx+dy*dy
    if not length:return []
    blocked=[]
    for ally in actors.values():
        if ally['slot'] in (hero['slot'],target['slot']) or not ally['active'] or ally['health']<=0:continue
        if friendly_only and ally['owner']!=hero['owner']:continue
        ax,ay=ally['x']-hero['x'],ally['y']-hero['y']
        t=(ax*dx+ay*dy)/length
        if .05<t<.98 and math.hypot(ax-t*dx,ay-t*dy)<radius:blocked.append(ally['slot'])
    return blocked


def fireball_point(hero, target, size):
    point=aim_point(hero,target,size,body=True)
    if point is not None and target.get('type')==24:
        point=(point[0],point[1]-18) if point[1]>=82 else None
    return point


def camulos_pause_point(hero, target, actors, size):
    """Authored visible staging; not a recovered collision/range rule."""
    if not hero.get('active') or hero.get('health',0)<=0 or hero.get('owner')!=0:return None
    if not target.get('active') or target.get('health',0)<=0 or target.get('type')!=24 or target.get('owner')!=1:return None
    if math.hypot(target['x']-hero['x'],target['y']-hero['y'])>5.5:return None
    if firing_line_blockers(hero,target,actors):return None
    return fireball_point(hero,target,size)


def support_for_attack(hero,target,actors):
    """Only an unambiguous living original-friendly selection can be ordered."""
    if not hero.get('active') or hero.get('health',0)<=0 or hero.get('owner')!=0:return None
    if not target.get('active') or target.get('health',0)<=0 or target.get('owner')!=1 or target.get('type')!=24:return None
    friendly=[v for v in actors.values() if v['slot']!=hero['slot'] and v['owner']==0 and v['active'] and v['health']>0]
    return friendly[0] if len(friendly)==1 and friendly[0]['type']==14 else None


def fireball_burst_limit(hero, target, paused):
    if target.get('type')==11 and target.get('owner')==1:
        # The retained escort takes two hits while a third cast is admitted.
        # Observe damage after two; surviving targets can get another burst.
        return min(2,max(0,hero['mana']//(6*256)))
    if target.get('type')!=24:return 7
    available=max(0,hero['mana']//(6*256))
    return min(6,available) if paused else min(2,available)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rect', nargs=4, type=int, required=True)
    p.add_argument('--case', choices=CASES, required=True)
    p.add_argument('--frame-size',nargs=2,type=int,default=[800,600])
    p.add_argument('--experiment', type=Path, required=True)
    p.add_argument('--seconds', type=int, default=540)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--stop-at-rearm',action='store_true',help='Diagnostic only: stop at the observed full-mana power point, before saving')
    p.add_argument('--resume-checkpoint',action='store_true',help='Diagnostic only: verified original CF52 power checkpoint')
    p.add_argument('--stop-after-support-attack',action='store_true',help='Diagnostic only: stop after original opcode7 submission, before the boss volley')
    a = p.parse_args()
    require(not a.stop_after_support_attack or a.resume_checkpoint,'Support stop requires checkpoint diagnostic')
    require(not a.resume_checkpoint or a.case=='victory' and os.environ.get('MNM_CAMPAIGN_CHECKPOINT')=='restore' and os.environ.get('MNM_CAMPAIGN_CHECKPOINT_PROBE')=='combat' and os.environ.get('MNM_CAMPAIGN_ORDER_ADMISSION')=='1','Verified checkpoint combat diagnostic required')
    require(not a.stop_at_rearm or a.case!='defeat','Checkpoint capture requires the ordinary victory route')
    width,height = frame_size(a.frame_size)
    require(30 <= a.seconds <= 600 and a.rect[2] >= 320 and a.rect[3] >= 240, 'Invalid bounded viewport/deadline')
    result = dict(success=False, completed=False,resume_checkpoint=a.resume_checkpoint, case=a.case, frame_size=a.frame_size, actions=[], health_observations=[],
                  scope=__doc__, simulation_writes=False, semantic_outcome_accepted=False)
    display = None;combat_paused=False;combat_pause_since=0
    x = c.CDLL('libX11.so.6'); xt = c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes = [c.c_char_p]; x.XOpenDisplay.restype = c.c_void_p
    x.XFlush.argtypes = x.XCloseDisplay.argtypes = [c.c_void_p]
    x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes = [c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes = [c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    start = time.monotonic(); held = set();held_keys=set()
    snapshots=AtomicSnapshotWriter(a.output)
    def save():
        # The Qt owner can terminate a failed helper before finally executes.
        # Retain complete cumulative histories in immutable snapshots. One
        # pending snapshot replaces an older pending image; original passive
        # events remain authoritative if cancellation interrupts persistence.
        result['snapshot_persistence']=snapshots.statistics()
        snapshots.submit((json.dumps(dict(result,seconds=time.monotonic()-start),indent=2)+'\n').encode())
    try:
        display = x.XOpenDisplay(os.environ['DISPLAY'].encode())
        require(display, 'Missing owned native display')
        def click(px, py, button=1, cast=False,target_actor=None):
            require(0 <= px < width and 0 <= py < height, 'Projection outside actual viewport')
            observe=(button==3 or target_actor is not None) and os.environ.get('MNM_CAMPAIGN_INPUT_ADMISSION')=='1'
            path=a.experiment/'input-admission.bin'
            prior=input_observations(path.read_bytes())['rows'] if observe else []
            after=prior[-1]['sequence'] if prior else 0
            already=bool(prior and prior[-1]['input_active']==1 and abs(prior[-1]['x']-px)<=1 and abs(prior[-1]['y']-py)<=1)
            x0,y0,w,h = a.rect
            require(xt.XTestFakeMotionEvent(display, -1, x0+round(px*w/width), y0+round(py*h/height), 0), 'Motion failed')
            x.XFlush(display); time.sleep(.1)
            if observe:
                deadline=time.monotonic()+(1.2 if target_actor is not None else 5); admitted=None;nudged=False;motion_started=time.monotonic()
                while time.monotonic()<deadline:
                    admitted=fresh_hover(input_observations(path.read_bytes())['rows'],(px,py),after,already,cast)
                    if admitted is not None and target_actor is not None:
                        candidate=candidate_after_hover(order_observations((a.experiment/'order-admission.bin').read_bytes())['rows'],admitted,target_actor,0)
                        if candidate is None:admitted=None
                    if admitted is not None:break
                    if not nudged and time.monotonic()-motion_started>.3:
                        nx,ny=hover_nudge((px,py),a.frame_size)
                        for dx,dy in ((nx,ny),(px,py)):
                            require(xt.XTestFakeMotionEvent(display,-1,x0+round(dx*w/width),y0+round(dy*h/height),0),'Hover nudge failed')
                            x.XFlush(display);time.sleep(.12)
                        result.setdefault('hover_nudges',[]).append(dict(point=[px,py],via=[nx,ny],after=after));nudged=True
                    time.sleep(.05)
                if admitted is None or not cast and admitted['mode'] not in (0,1):
                    refusals=result.setdefault('hover_refusals',[])
                    refusals.append(dict(after=after,point=[px,py],cast=cast,target_actor=target_actor,button_sent=False,mode=admitted['mode'] if admitted else None))
                    save();require(len(refusals)<=24,'Bounded original-hover refusal budget exhausted')
                    return False
                result.setdefault('hover_admissions',[]).append(dict(after=after,ready=admitted['sequence'],point=[px,py],cast=cast,mode=admitted['mode'],ground=admitted['ground'],hit_class=admitted['hit_class'],hit_index=admitted['hit_index']))
            for down in (1,0):
                require(xt.XTestFakeButtonEvent(display, button, down, 0), 'Click failed')
                if down:held.add(button)
                else:held.discard(button)
                x.XFlush(display);time.sleep(.12)
            result['actions'].append(dict(seconds=time.monotonic()-start, point=[px,py], button=button))
            save();return True
        def key(sym,persist=True):
            code=x.XKeysymToKeycode(display,sym);require(code,'Missing original-control key mapping')
            for down in (1,0):
                require(xt.XTestFakeKeyEvent(display,code,down,0),'Key input failed')
                if down:held_keys.add(code)
                else:held_keys.discard(code)
                x.XFlush(display);time.sleep(.12)
            result['actions'].append(dict(seconds=time.monotonic()-start,kind='key',keysym=sym))
            if persist:save()
        def modified_click(sym,point):
            code=x.XKeysymToKeycode(display,sym);require(code,'Missing follow modifier mapping')
            require(xt.XTestFakeKeyEvent(display,code,1,0),'Follow modifier press failed');held_keys.add(code)
            x.XFlush(display);time.sleep(.12)
            result['actions'].append(dict(seconds=time.monotonic()-start,kind='modifier',keysym=sym,down=True));save()
            try:return click(*point,3)
            finally:
                require(xt.XTestFakeKeyEvent(display,code,0,0),'Follow modifier release failed');held_keys.discard(code)
                x.XFlush(display);time.sleep(.12)
                result['actions'].append(dict(seconds=time.monotonic()-start,kind='modifier',keysym=sym,down=False));save()
        def order_rows():return order_observations((a.experiment/'order-admission.bin').read_bytes())['rows']
        def state():return creatures(read_rows(a.experiment/'gameplay-events.bin'))
        def center():
            # Publisher manual p15: W selects the wizard; a second W centers it.
            key(ord('w'));key(ord('w'));time.sleep(.4)
        def select(spell):
            for point in spell_selection_points(spell):click(*hud_point(point,a.frame_size));time.sleep(.2)
        last_sequence = 0; last_change = start; portal = None;quiet=ActorQuietWindow()
        last_spell=start-10;pending=None;summons=0;fireballs=0
        route=None;phase="power-approach";power_since=power_before=power_after=0
        if a.resume_checkpoint:phase='bridge'
        follow_slot=None;follow_start=None;follow_wait_since=0
        follow_stage_target=None;follow_quiet_position=None;follow_quiet_since=0
        firing_position=None;firing_since=0;bridge_attacker=None;rearm_since=0;rearm_onpoint_since=0;rearmed=a.resume_checkpoint
        interrupted_phase=None
        portal_clear_since=0;portal_clear_goal=None;portal_clear_sent=False;portal_cleared=False
        camulos_approach_since=0;bridge_arrival_since=0
        result.update(casts=[],combat_policy='observed-target-boss-reserve-v34' if a.case!='defeat' else 'ordinary-defeat-v1',route_stage=phase)
        save()
        while time.monotonic()-start < a.seconds:
            require(not combat_paused or time.monotonic()-combat_pause_since<12,'Bounded ordinary combat pause exceeded12seconds')
            # The passive menu observer owns these records. UI report OK may
            # already have run; inspect the whole prefix, not only its last row.
            menu = (a.experiment/'events.bin').read_bytes()
            require(menu[:16] == b'MNMMENU1'+struct.pack('<II',1,64), 'Invalid original event header')
            end = 16+(len(menu)-16)//64*64
            events = list(struct.iter_unpack('<16I',menu[16:end]))
            dispatch=report_callback(events,events[0][2] if events else None)
            if dispatch is not None:
                require(not combat_paused,'Original report arrived before ordinary pause release')
                result['original_report_dispatch_observed']=dispatch
                if a.case=='defeat':
                    result.update(success=True);break
                returned=read_realm_return_state(a.experiment/'channel.bin',events,dispatch)
                result['realm_return_state']=returned
                if returned=='ready':
                    result.update(success=True);break
                if returned=='dismiss':
                    count=result.get('realm_tutorial_clicks',0)
                    require(count<12,'Bounded first Realm tutorial dismissal exhausted')
                    click(width//2,height//2)
                    result['realm_tutorial_clicks']=count+1;save();time.sleep(.8)
                else:time.sleep(.2)
                continue
            trace = a.experiment/'gameplay-events.bin'
            if not trace.exists():time.sleep(.2);continue
            rows = read_rows(trace); actors = creatures(rows)
            if not rows:time.sleep(.2);continue
            if rows[-1][0] != last_sequence:last_sequence=rows[-1][0];last_change=time.monotonic()
            heroes = [v for v in actors.values() if v['type']==0 and v['owner']==0 and v['active']]
            require(len(heroes) <= 1, 'Ambiguous player wizard')
            if not heroes:
                dead=[v for v in actors.values() if v['type']==0 and v['owner']==0 and v['health']<=0]
                if len(dead)==1 and not result.get('terminal_hero'):
                    result['terminal_hero']=dead[0];save()
                time.sleep(.2);continue
            hero = heroes[0]
            attack=result.get('support_attack')
            if attack and attack.get('submitted') and not attack.get('observed'):
                current=actors.get(attack['before']['slot']);victim=actors.get(attack['target']['slot'])
                if current and current['active'] and current['health']>0 and current['type']==14 and current['owner']==0 and victim and victim['type']==24 and victim['owner']==1 and current['target']==victim['slot']:
                    attack.update(observed=True,after=current)
            if len(result['health_observations']) < 1024:
                result['health_observations'].append(dict(sequence=hero['sequence'], health=hero['health'], slot=hero['slot']))
            # Dismiss stalled original conversations through ordinary input.
            if time.monotonic()-last_change > 2:
                key(0xff0d);time.sleep(.4);continue
            if hero['health'] <= 0:
                require(a.case == 'defeat', 'Player died before the candidate victory')
                time.sleep(.3);continue
            if not combat_paused and quiet.observe(actors,time.monotonic()):
                result.setdefault('quiet_actor_dialogue_candidates',[]).append(dict(sequence=rows[-1][0],hero=hero,seconds=time.monotonic()-start))
                key(0xff0d);time.sleep(.4);continue
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
                        if pending.get('purpose')=='power-capture':
                            phase='power-refill';power_since=time.monotonic();power_before=accepted[0][8];power_after=accepted[0][9]
                            result['power_mana_debit']=list(accepted[0]);result['route_stage']=phase
                        if pending['id']==14:summons+=1
                        else:fireballs+=1
                        pending=None;result['pending_cast']=None;save()
                if pending:
                    elapsed=time.monotonic()-pending['started']
                    if elapsed>30 and 'accepted' in pending:raise RuntimeError('Accepted summon lacks an observed living owned Zombie')
                    if elapsed>4 and not accepted:
                        result['casts'].append(dict(pending,refused_or_unobserved=True));pending=None;result['pending_cast']=None;save()
                    else:time.sleep(.25);continue
            if a.case!='defeat' and route is None:
                route=forest_waypoints(a.experiment/'game')
                result['route']=route
                if a.resume_checkpoint:
                    power=route['points']['CF52 PoP']
                    require(all(hero[k]==power[k] for k in ('x','y','z')) and hero['mana']>=36*256,'Restored checkpoint is not at the observed ready power point')
                    result['checkpoint_start']=dict(wizard=hero,actors=actors,scope='Original verified save input only; no opening-route or nominal progression claim')
                else:result['opening_anchors']=validate_opening_anchors(route,rows)
                save()
            recent=recent_melee_attackers(rows,hero,actors,follow_slot)
            if a.case!='defeat' and phase=='combat' and bridge_attacker is not None and not rearmed and not rearm_since:
                actor=actors.get(bridge_attacker)
                dead=actor and actor['type']==10 and actor['owner']==2 and actor['health']<=0
                lethal=[r for r in rows if r[1]==2 and r[8:11]==(bridge_attacker,10,2) and r[13] and signed(r[11])>0 and signed(r[12])<=0]
                spell_lethal=[r for r in player_fireball_damage(read_spell_rows(a.experiment/'spell-events.bin'),0,hero['slot']) if r[4:7]==(bridge_attacker,10,2) and signed(r[8])<=0]
                if dead or lethal or spell_lethal:
                    phase='resupply';rearm_since=time.monotonic();result['route_stage']=phase
                    result['rearm']=dict(attacker=bridge_attacker,before=hero,lethal=list((lethal or spell_lethal)[-1]) if lethal or spell_lethal else None);save()
            if a.case!='defeat' and route_combat_needed(phase,rearmed,hero,actors,follow_slot,recent):
                interruptions=result.setdefault('route_threat_interruptions',[])
                require(len(interruptions)<4,'Bounded route combat interruptions exhausted')
                interruptions.append(dict(phase=phase,wizard=dict(hero),threats=engaged_threats(hero,actors,follow_slot,7,recent),recent_melee=[list(v) for v in sorted(recent)],seconds=time.monotonic()-start))
                interrupted_phase=phase;phase='combat';result['route_stage']=phase;save()
            if a.case!='defeat' and phase!='combat':
                center();actors=state();hero=actors[hero['slot']]
                if not hero['active'] or hero['health']<=0:continue
                power=route['points']['CF52 PoP']
                if phase=='resupply':
                    result['rearm']['latest']=hero;save()
                    onpoint=all(hero[k]==power[k] for k in ('x','y','z'))
                    if not rearm_onpoint_since:
                        require(time.monotonic()-rearm_since<60,'Bounded rearm travel did not reach the observed power point')
                        if onpoint:
                            rearm_onpoint_since=time.monotonic();result['rearm']['arrival']=hero
                            result['rearm']['arrival_seconds']=time.monotonic()-start;save()
                    if onpoint and hero['mana']>=36*256:
                        rearmed=True;follow_wait_since=0;phase='bridge';result['route_stage']=phase
                        result['rearm']['complete']=hero;save()
                        if a.stop_at_rearm:
                            require(hero['health']>=60,'Checkpoint wizard has less than60HP')
                            result.update(success=True,checkpoint_ready=True,checkpoint_actors=actors,
                                checkpoint_scope='Ordinary route through bridge clearing and full-mana resupply; no outcome or progression acceptance')
                            save();break
                    else:
                        if rearm_onpoint_since:require(time.monotonic()-rearm_onpoint_since<210,'Bounded on-point rearm did not reach36mana')
                        click(*approach_point(hero,power,a.frame_size),3);time.sleep(1);continue
                if phase=='power-approach':
                    if math.hypot(hero['x']-power['x'],hero['y']-power['y'])>2.25:
                        click(*approach_point(hero,power,a.frame_size),3);time.sleep(.7);continue
                    require(hero['mana']>=17*256,'Insufficient ordinary mana for power capture candidate')
                    select('zombie');actors=state();hero=actors[hero['slot']]
                    point=aim_point(hero,power,a.frame_size)
                    require(point is not None,'Power candidate is outside visible ground')
                    spell_rows=read_spell_rows(a.experiment/'spell-events.bin')
                    pending=dict(id=14,purpose='power-capture',spell_sequence=spell_rows[-1][0] if spell_rows else 0,
                        started=time.monotonic(),prior_support=[v['slot'] for v in actors.values() if v['type']==14 and v['owner']==0 and v['active'] and v['health']>0],point=list(point),hero=hero,target=power)
                    result['pending_cast']=pending;save()
                    if not click(*point,3,cast=True):
                        pending=None;result['pending_cast']=None;save();continue
                    last_spell=time.monotonic();continue
                if phase=='power-refill':
                    result.setdefault('mana_recovery_observations',[]).append(dict(sequence=hero['sequence'],mana=hero['mana'],position=[hero[k] for k in ('x','y','z')]))
                    if hero['mana']>=power_before and hero['mana']>power_after:
                        result['power_mana_recovered']=dict(sequence=hero['sequence'],before=power_before,after_debit=power_after,recovered=hero['mana'])
                        phase='bridge';result['route_stage']=phase;save()
                    else:
                        require(time.monotonic()-power_since<180,'Power candidate did not restore the observed mana debit')
                        click(*approach_point(hero,power,a.frame_size),3);save();time.sleep(1);continue
                bridge=route['points']['CF52 Bridge']
                if follow_slot is None:
                    friendly=[v for v in actors.values() if v['slot']!=hero['slot'] and v['owner']==hero['owner'] and v['active'] and v['health']>0]
                    require(len(friendly)==1 and friendly[0]['type']==14,'Follow selection requires exactly the observed living owned Zombie')
                    follower=friendly[0]
                    if not follow_wait_since:follow_wait_since=time.monotonic()
                    require(time.monotonic()-follow_wait_since<20,'Bounded follow staging exhausted')
                    projected=follow_point(follower,hero,a.frame_size)
                    if projected is None or not (112<=projected[0]<width-104 and 96<=projected[1]<height-130):
                        result.setdefault('follow_visibility_staging',[]).append(dict(wizard=hero,source=follower,projected=projected,target=bridge))
                        click(*approach_point(hero,bridge,a.frame_size),3);save();time.sleep(.3);continue
                    if math.hypot(hero['x']-follower['x'],hero['y']-follower['y'])<3:
                        if follow_stage_target is None:
                            dx,dy=bridge['x']-hero['x'],bridge['y']-hero['y'];distance=max(1,math.hypot(dx,dy))
                            follow_stage_target=dict(x=hero['x']+dx*min(1,4/distance),y=hero['y']+dy*min(1,4/distance),z=bridge['z'])
                        result.setdefault('follow_staging',[]).append(dict(wizard=hero,source=follower,target=follow_stage_target));save()
                        follow_quiet_position=None
                        click(*approach_point(hero,follow_stage_target,a.frame_size),3);time.sleep(.7);continue
                    position=tuple(hero[k] for k in ('x','y','z'))
                    if follow_quiet_position!=position:
                        follow_quiet_position=position;follow_quiet_since=time.monotonic()
                    if time.monotonic()-follow_quiet_since<1:
                        time.sleep(.2);continue
                    follow_slot=follower['slot'];follow_start=[follower[k] for k in ('x','y','z')]
                    # Manual p15: V centres the next selected friendly. With
                    # exactly one selected follower this makes its origin explicit.
                    key(ord('x'));path=a.experiment/'game/native-creature-follow.bin'
                    deadline=time.monotonic()+5;observed=None;attempts=[]
                    for offset in (40,22,54):
                        if time.monotonic()>=deadline:break
                        key(ord('v'));time.sleep(.4)
                        current=state();follower=current[follow_slot];hero=current[hero['slot']]
                        prior=read_follow_rows(path);after=prior[-1][0] if prior else 0
                        point=follow_point(follower,hero,a.frame_size,body_offset=offset)
                        require(point is not None,'Wizard follow target is not visible')
                        attempt=dict(source=follower,wizard=hero,point=list(point),body_offset=offset,after=after)
                        attempts.append(attempt)
                        result['support_follow']=dict(attempt,attempts=attempts,manual='Publisher manual printed14/15: X selects,V centres selected friendly; Alt-right-click guards/follows')
                        require(modified_click(0xffe9,point),'Follow click was not sent')
                        until=min(deadline,time.monotonic()+.6)
                        while time.monotonic()<until:
                            current=state();follower=current[follow_slot];hero=current[hero['slot']]
                            observed=followed_wizard(read_follow_rows(path),follower,hero,after)
                            if observed:break
                            time.sleep(.1)
                        if observed:break
                    result['support_follow']['reference_observed']=observed;save()
                    require(observed is not None,'Ordinary guard/follow did not establish a fresh living wizard reference')
                    center();actors=state();hero=actors[hero['slot']]
                    follow_wait_since=0
                result['bridge_candidate']=bridge
                if not bridge_entry_ready(hero,bridge,rearmed):
                    if rearmed:
                        if not bridge_arrival_since:bridge_arrival_since=time.monotonic()
                        require(time.monotonic()-bridge_arrival_since<60,'Cleared bridge waypoint was not reached within60seconds')
                        result.setdefault('cleared_bridge_staging',[]).append(dict(wizard=hero,bridge=bridge,seconds=time.monotonic()-start))
                    click(*approach_point(hero,bridge,a.frame_size),3);time.sleep(.7);continue
                if rearmed:
                    result.setdefault('cleared_bridge_arrival',dict(wizard=hero,bridge=bridge,seconds=time.monotonic()-start))
                    bridge_arrival_since=0
                follower=actors[follow_slot];distance=math.hypot(follower['x']-hero['x'],follower['y']-hero['y'])
                if not follow_wait_since:
                    follow_wait_since=time.monotonic()
                    result.setdefault('bridge_catchup_windows',[]).append(dict(start_seconds=follow_wait_since-start,rearmed=rearmed,wizard=hero,source=follower))
                moved=[follower[k] for k in ('x','y','z')]!=follow_start
                result['support_follow']['bridge_observation']=dict(source=follower,wizard=hero,distance=distance,moved=moved);save()
                require(follower['active'] and follower['health']>0,'Follower died before the combat route')
                if not moved or distance>8:
                    require(time.monotonic()-follow_wait_since<20,'Observed follower did not travel with the wizard to the bridge')
                    time.sleep(.5);continue
                phase='combat';result['route_stage']=phase;save()
            enemies = [v for v in actors.values() if v['active'] and v['owner'] not in (0,0xffffffff) and v['health'] > 0]
            camulos = [v for v in actors.values() if v['owner']==1 and v['type']==24]
            threats=engaged_threats(hero,actors,follow_slot,recent=recent)
            if camulos and camulos[0]['health'] <= 0:portal=camulos[0]
            if camulos and portal is None:
                lethal=[r for r in player_fireball_damage(read_spell_rows(a.experiment/'spell-events.bin'),0,hero['slot']) if r[4]==camulos[0]['slot'] and r[5]==24 and r[6]==1 and signed(r[8])<=0]
                if lethal:
                    portal=dict(camulos[0],health=0);result['portal_candidate_from_original_lethal']=dict(event=list(lethal[-1]),last_position=camulos[0])
            if interrupted_phase and (not threats or hero['mana']<6*256) and portal is None:
                # Resume the same required waypoint/follower checks. Existing
                # stage clocks keep running; combat grants no extra deadline.
                phase=interrupted_phase;interrupted_phase=None;firing_position=None
                result['route_stage']=phase
                result.setdefault('route_combat_resumptions',[]).append(dict(phase=phase,wizard=dict(hero),seconds=time.monotonic()-start))
                save();continue
            if a.case!='defeat' and portal is not None and not portal_cleared:
                follower=actors.get(follow_slot)
                blocked=follower and follower['active'] and follower['health']>0 and math.hypot(follower['x']-portal['x'],follower['y']-portal['y'])<2
                if blocked:
                    if not portal_clear_since:
                        candidates=[hero]+[c['hero'] for c in reversed(result['casts']) if c.get('id')==71 and c.get('target',{}).get('type')==24]
                        portal_clear_goal=next((p for p in candidates if math.hypot(p['x']-portal['x'],p['y']-portal['y'])>=3),None)
                        require(portal_clear_goal is not None,'No observed walkable portal-clear destination')
                        portal_clear_since=time.monotonic();result['portal_clear']=dict(portal=portal,before=follower,goal=portal_clear_goal)
                    require(time.monotonic()-portal_clear_since<15,'Observed follower did not clear the portal within15seconds')
                    if math.hypot(hero['x']-portal['x'],hero['y']-portal['y'])<3 and not portal_clear_sent:
                        center();actors=state();hero=actors[hero['slot']]
                        click(*approach_point(hero,portal_clear_goal,a.frame_size),3);save();time.sleep(.3);continue
                    if not portal_clear_sent:
                        friendly=[v for v in actors.values() if v['slot']!=hero['slot'] and v['owner']==0 and v['active'] and v['health']>0]
                        require(len(friendly)==1 and friendly[0]['slot']==follow_slot and friendly[0]['type']==14,'Ambiguous living portal-clear follower')
                        key(ord('x'));key(ord('v'));time.sleep(.4);follower=state()[follow_slot]
                        point=approach_point(follower,portal_clear_goal,a.frame_size)
                        result['portal_clear']['point']=list(point);require(click(*point,3),'Portal-clear ground order was not sent')
                        portal_clear_sent=True;save()
                    time.sleep(.2);continue
                portal_cleared=True
                if portal_clear_since:result['portal_clear']['after']=follower;save()
            reserved=reserved_boss_target(hero,actors,rearmed,interrupted_phase is not None) if a.case!='defeat' and portal is None else None
            if a.case != 'defeat' and portal is not None:target=portal
            elif reserved is not None:
                target=reserved
                result.setdefault('boss_reserve_priority',dict(wizard=dict(hero),target=dict(target),
                    estimated_casts=(target['health']+19)//20,observed_threats=threats,
                    scope='Native physical test priority only; actual original cast/debit/damage and outcome checks remain required'))
            elif a.case!='defeat' and threats:
                target=min(threats,key=lambda e:math.hypot(e['x']-hero['x'],e['y']-hero['y']))
                if bridge_attacker is None and target['type']==10 and target['owner']==2:
                    bridge_attacker=target['slot']
                selections=result.setdefault('engaged_threat_selections',[])
                if not any((v['target']['slot'],v['target']['type'],v['target']['owner'])==
                           (target['slot'],target['type'],target['owner']) for v in selections):
                    require(len(selections)<16,'Bounded engaged-threat identities exhausted')
                    selections.append(dict(target=dict(target),wizard=dict(hero),
                        follower=dict(actors.get(follow_slot,{})),seconds=time.monotonic()-start))
            elif a.case != 'defeat' and camulos and camulos[0]['health'] > 0:
                target=camulos[0]
            elif enemies:target=min(enemies,key=lambda e:math.hypot(e['x']-hero['x'],e['y']-hero['y']))
            else:time.sleep(.3);continue
            # Center before reading fresh positions. Do not infer a camera
            # basis from obstacle-limited movement or walk an unsupported wizard
            # into a hostile army without using its ordinary starting spells.
            center();actors=state();hero=actors[hero['slot']]
            if not hero['active'] or hero['health']<=0:
                save();time.sleep(.2);continue
            target=actors.get(target['slot'],target) if portal is None else portal
            if portal is None and (not target['active'] or target['health']<=0):continue
            support=[v for v in actors.values() if v['type']==14 and v['owner']==0 and v['active'] and v['health']>0]
            nearby=sum(math.hypot(v['x']-hero['x'],v['y']-hero['y'])<10 for v in support)
            choice=combat_choice(a.case,hero,target,nearby,summons,time.monotonic()-last_spell,
                support_ordered=result.get('support_attack',{}).get('submitted',False))
            result['last_decision']=dict(choice=choice,hero=hero,target=target,nearby_support=nearby)
            if choice!='fireball':firing_position=None
            if choice=='fireball':
                if target['type']==24 and 'combat_pause' not in result:
                    point=camulos_pause_point(hero,target,actors,a.frame_size)
                    if point is None:
                        if not camulos_approach_since:camulos_approach_since=time.monotonic()
                        require(time.monotonic()-camulos_approach_since<25,'No visible clear close firing position within25seconds')
                        blocked=firing_line_blockers(hero,target,actors)
                        goal=target
                        if blocked:
                            dx,dy=target['x']-hero['x'],target['y']-hero['y'];length=max(1,math.hypot(dx,dy))
                            goal=dict(x=hero['x']-3*dy/length,y=hero['y']+3*dx/length,z=hero['z'])
                        move=approach_point(hero,goal,a.frame_size)
                        result.setdefault('camulos_approach',[]).append(dict(hero=hero,target=target,allies=firing_line_allies(hero,target,actors),blockers=blocked,point=list(move)))
                        click(*move,3);save();time.sleep(.3);continue
                    # Publisher manual p14/15 permits single-player orders and
                    # casting during Space pause. Use one bounded pair; the
                    # original owns pause,cast admission and resumed simulation.
                    result['combat_pause']=dict(before=hero,target=target,ready_point=list(point),casts_before=len(result['casts']),released=False)
                    key(0x20);combat_paused=True;combat_pause_since=time.monotonic()
                    source=support_for_attack(hero,target,actors)
                    require(source is not None,'No unambiguous living owned Zombie for the bounded attack order')
                    result['support_attack']=dict(before=source,target=target,camera_wizard=hero,observed=False,submitted=False)
                    # Select through an actual scene candidate before arming a
                    # spell. Re-clicking the Fireball icon leaves mode2 active
                    # in the original; it is not a cancellation acknowledgement.
                    selection_after=order_rows()[-1]['sequence'];selected=None;selection_attempts=[]
                    for body_offset in (22,40,54):
                        projected=aim_point(hero,source,a.frame_size,body=True)
                        if projected is None:break
                        select_point=(projected[0],projected[1]+22-body_offset)
                        if not (48<=select_point[0]<width-40 and 64<=select_point[1]<height-90):continue
                        sent=click(*select_point,1,target_actor=source['slot'])
                        selection_attempts.append(dict(point=list(select_point),body_offset=body_offset,button_sent=sent))
                        if not sent:continue
                        until=time.monotonic()+.7
                        while time.monotonic()<until:
                            selected=next((v for v in reversed(order_rows()) if v['kind']==1 and v['sequence']>selection_after and v['flags']&1 and v['primary']==[source['slot']]),None)
                            if selected:break
                            time.sleep(.05)
                        if selected:break
                    result['support_attack']['selection_attempts']=selection_attempts
                    require(selected is not None,'Original paused mouse selection did not identify the sole owned Zombie')
                    result['support_attack']['selection']=selected
                    submitted=None;attempts=[]
                    for body_offset in (22,40,54):
                        projected=aim_point(hero,target,a.frame_size,body=True)
                        if projected is None:break
                        attack_point=(projected[0],projected[1]+22-body_offset)
                        if not (48<=attack_point[0]<width-40 and 64<=attack_point[1]<height-90):continue
                        after=order_rows()[-1]['sequence']
                        sent=click(*attack_point,3,target_actor=target['slot'])
                        attempts.append(dict(point=list(attack_point),body_offset=body_offset,after=after,button_sent=sent))
                        if not sent:continue
                        until=time.monotonic()+1
                        while time.monotonic()<until:
                            submitted=support_attack_submission(order_rows(),source,target,after)
                            if submitted:break
                            time.sleep(.05)
                        if submitted:break
                    result['support_attack'].update(attempts=attempts,submission=submitted,submitted=submitted is not None)
                    save();require(submitted,'Original opcode7 Zombie-to-Camulos submission was not observed')
                    if a.stop_after_support_attack:
                        result.update(success=True,checkpoint_support_order=True)
                        save();break
                    firing_position=tuple(hero[k] for k in ('x','y','z'));firing_since=time.monotonic()-1;save()
                position=tuple(hero[k] for k in ('x','y','z'))
                if position!=firing_position:
                    firing_position=position;firing_since=time.monotonic()
                if time.monotonic()-firing_since<1:
                    save();time.sleep(.2);continue
                select('fireball');actors=state();hero=actors[hero['slot']]
                origin=dict(hero);burst=0;target_before=dict(target)
                damage_before=read_spell_rows(a.experiment/'spell-events.bin')
                damage_sequence=damage_before[-1][0] if damage_before else 0
                burst_limit=fireball_burst_limit(hero,target,combat_paused)
                for _ in range(burst_limit):
                    if combat_paused and burst and time.monotonic()-combat_pause_since>9:break
                    require(not combat_paused or time.monotonic()-combat_pause_since<12,'Bounded ordinary combat pause exceeded12seconds')
                    actors=state();hero=actors[hero['slot']];target=actors.get(target['slot'],target)
                    if not hero['active'] or hero['health']<=0 or not target['active'] or target['health']<=0 or hero['mana']<6*256:break
                    if any(hero[k]!=origin[k] for k in ('x','y','z')):break
                    blocked=firing_line_blockers(hero,target,actors) if target['type']==24 else firing_line_allies(hero,target,actors)
                    if blocked:
                        result.setdefault('firing_line_refusals',[]).append(dict(hero=hero,target=target,allies=firing_line_allies(hero,target,actors),blockers=blocked))
                        dx,dy=target['x']-hero['x'],target['y']-hero['y'];length=max(1,math.hypot(dx,dy))
                        flank=dict(x=hero['x']-3*dy/length,y=hero['y']+3*dx/length,z=hero['z'])
                        click(*approach_point(hero,flank,a.frame_size),3);break
                    point=fireball_point(origin,target,a.frame_size)
                    if combat_paused and target['type']==24 and result.get('support_attack',{}).get('submitted'):
                        point=tuple(result['support_attack']['attempts'][-1]['point'])
                    if point is None:break
                    require(fireballs<24 and len(result['casts'])<40,'Bounded physical cast budget exhausted')
                    spell_rows=read_spell_rows(a.experiment/'spell-events.bin')
                    pending=dict(id=71,purpose='bounded-fireball-burst',spell_sequence=spell_rows[-1][0] if spell_rows else 0,
                        started=time.monotonic(),prior_support=[v['slot'] for v in support],point=list(point),hero=hero,target=target)
                    result['pending_cast']=pending;save()
                    if not click(*point,3,cast=True):
                        pending=None;result['pending_cast']=None;save();break
                    last_spell=time.monotonic();deadline=last_spell+1
                    accepted=[]
                    while time.monotonic()<deadline:
                        accepted=[r for r in read_spell_rows(a.experiment/'spell-events.bin') if r[0]>pending['spell_sequence'] and r[1]==1 and r[4]==71 and r[5]==hero['slot'] and r[7]==hero['owner'] and r[15]==1 and r[8]>r[9]]
                        if accepted:break
                        time.sleep(.05)
                    if not accepted:break
                    pending['accepted']=list(accepted[0]);pending['target_matched']=accepted[0][10]==target['slot'];result['casts'].append(pending)
                    matched=pending['target_matched']
                    pending=None;result['pending_cast']=None;fireballs+=1;burst+=1;save();time.sleep(.05)
                    if not matched:break
                if combat_paused:
                    require(time.monotonic()-combat_pause_since<12,'Bounded ordinary combat pause exceeded12seconds')
                    key(0x20);combat_paused=False
                    result['combat_pause'].update(released=True,seconds=time.monotonic()-combat_pause_since,accepted_in_burst=burst,after=state()[hero['slot']]);save()
                if burst:
                    save();time.sleep(3)
                    # A time window can include earlier projectiles. Cast counts
                    # and this observation never grant semantic victory.
                    damage=player_fireball_damage(read_spell_rows(a.experiment/'spell-events.bin'),0,hero['slot'])
                    result.setdefault('fireball_bursts',[]).append(dict(accepted=burst,limit=burst_limit,origin=origin,
                        target_before=target_before,target_after=state().get(target['slot']),
                        observed_damage_window=[list(r) for r in damage if r[0]>damage_sequence and r[4]==target['slot']]))
            elif choice=='retreat':
                point=approach_point(hero,dict(x=2*hero['x']-target['x'],y=2*hero['y']-target['y'],z=hero['z']),a.frame_size)
                click(*point,3)
            elif choice!='idle':click(*approach_point(hero,target,a.frame_size),3)
            result['player_fireball_damage']=[list(r) for r in player_fireball_damage(read_spell_rows(a.experiment/'spell-events.bin'),0,hero['slot'])]
            result['camulos_fireball_damage']=[r for r in result['player_fireball_damage'] if r[5]==24 and r[6]==1]
            save();time.sleep(.7 if choice!='idle' else 1)
        require(result['success'], 'Natural report not reached within bounded ordinary-input attempt')
    except Exception as error:
        result.update(success=False,error=str(error))
    finally:
        if display:
            if combat_paused:
                try:
                    key(0x20,persist=False);combat_paused=False
                    result['combat_pause'].update(released=True,released_on_failure=True,seconds=time.monotonic()-combat_pause_since)
                except Exception as error:
                    result.update(success=False,error=str(error))
            for button in held:xt.XTestFakeButtonEvent(display,button,0,0)
            for code in held_keys:xt.XTestFakeKeyEvent(display,code,0,0)
            x.XFlush(display);x.XCloseDisplay(display)
        result['completed']=True
        result['seconds'] = time.monotonic()-start
        # Bounded physical interchange; the acquisition collector imports this
        # exact file to SQLite, including failed attempts.
        try:
            save()
        except Exception as error:
            result.update(success=False,error=str(error))
        try:
            snapshots.close()
        except Exception as error:
            result.update(success=False,error=str(error))
            print(result['error'],file=sys.stderr)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
