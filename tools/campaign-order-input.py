#!/usr/bin/env python3
"""Bounded ordinary-input checkpoint selection/move probe; no simulation writes."""
import argparse
import ctypes as c
import json
import os
from pathlib import Path
import time
from campaign_gameplay import read_rows,creatures
from campaign_spell import read_spell_rows
from campaign_input_admission import spell_selection_points
from campaign_route import hud_point
from campaign_order_observation import decode
from campaign_input_observation import decode as inputs
from campaign_input_admission import fresh_hover
from campaign_target_admission import candidate_after_hover,empty_candidate_after_hover


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--experiment',type=Path,required=True)
    p.add_argument('--rect',type=int,nargs=4,required=True)
    p.add_argument('--frame-size',type=int,nargs=2,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();mode=os.environ.get('MNM_CAMPAIGN_CHECKPOINT_PROBE','orders')
    result=dict(success=False,completed=False,simulation_writes=False,actions=[],probe_mode=mode,scope=__doc__)
    x=c.CDLL('libX11.so.6');xt=c.CDLL('libXtst.so.6')
    x.XOpenDisplay.argtypes=[c.c_char_p];x.XOpenDisplay.restype=c.c_void_p
    x.XFlush.argtypes=x.XCloseDisplay.argtypes=[c.c_void_p]
    x.XKeysymToKeycode.argtypes=[c.c_void_p,c.c_ulong];x.XKeysymToKeycode.restype=c.c_uint
    xt.XTestFakeKeyEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    xt.XTestFakeMotionEvent.argtypes=[c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_ulong]
    xt.XTestFakeButtonEvent.argtypes=[c.c_void_p,c.c_uint,c.c_int,c.c_ulong]
    display=None;held=[];paused=False
    def orders():return decode((a.experiment/'order-admission.bin').read_bytes())['rows']
    def actors():return creatures(read_rows(a.experiment/'gameplay-events.bin'))
    def wait(predicate,seconds=5):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            value=predicate()
            if value:return value
            time.sleep(.05)
        return None
    def require(value,message):
        if not value:raise ValueError(message)
        return value
    def key(sym,observe=True):
        code=require(x.XKeysymToKeycode(display,sym),'Unmapped key')
        for down in (1,0):
            require(xt.XTestFakeKeyEvent(display,code,down,0),'Key failed')
            if down:held.append(('key',code))
            else:held.remove(('key',code))
            x.XFlush(display);time.sleep(.15)
        action=dict(keysym=sym)
        if observe:action['last_order_sequence']=orders()[-1]['sequence']
        result['actions'].append(action)
    def press(button):
        for down in (1,0):
            require(xt.XTestFakeButtonEvent(display,button,down,0),'Button failed')
            if down:held.append(('button',button))
            else:held.remove(('button',button))
            x.XFlush(display);time.sleep(.15)
        result['actions'].append(dict(button=button))
    def motion(point):
        x0,y0,w,h=a.rect;width,height=a.frame_size
        require(xt.XTestFakeMotionEvent(display,-1,x0+round(point[0]*w/width),y0+round(point[1]*h/height),0),'Motion failed')
        x.XFlush(display);time.sleep(.3)
        observation=inputs((a.experiment/'input-admission.bin').read_bytes())['rows']
        original=[r for r in observation if r['kind']==3]
        result['actions'].append(dict(motion=list(point),latest_original_callback=original[-1] if original else None,
                                      latest_world=observation[-1] if observation else None))
    try:
        require(mode in ('orders','paused-orders','paused-cast'),'Unknown bounded checkpoint probe')
        display=require(x.XOpenDisplay(os.environ['DISPLAY'].encode()),'Display unavailable')
        require(wait(lambda:orders() and actors()),'Original observers unavailable')
        initial=actors();support=[v for v in initial.values() if v['active'] and v['health']>0 and v['owner']==0 and v['type']==14]
        require(len(support)==1,'Probe requires sole living owned Zombie')
        support=support[0];result['support_before']=support
        key(ord('w'));key(ord('w'));time.sleep(.4)
        selection_after=orders()[-1]['sequence']
        key(ord('x'))
        selected=wait(lambda:next((r for r in reversed(orders()) if r['sequence']>selection_after and r['kind']==1 and r['flags']&1 and r['primary']==[support['slot']]),None))
        result['selection']=selected
        require(selected,'X did not produce sole-Zombie primary selection')
        width,height=a.frame_size
        # Compare small and large real motions without synthesising original
        # callback records. A final hover must still satisfy the existing gate.
        points=[(width//2,height//2+70),(width//2+1,height//2+70),
                (width//2+5,height//2+70),(width//2+35,height//2+70),(width//2,height//2+70)]
        before=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
        for point in points:motion(point)
        admitted=wait(lambda:fresh_hover(inputs((a.experiment/'input-admission.bin').read_bytes())['rows'],points[-1],before,False,False))
        result['hover']=admitted
        require(admitted and admitted['mode'] in (0,1),'No original callback/later World hover for move')
        move_before=actors()[support['slot']];result['move_before']=move_before
        last=orders()[-1]['sequence']
        press(3)
        submitted=wait(lambda:next((dict(row=r,command=cmd) for r in orders() if r['sequence']>last for cmd in r['records']
                                   if cmd['opcode']==5 and cmd.get('actor_slot')==support['slot']),None))
        result['submission']=submitted
        require(submitted,'No original opcode5 submission for selected Zombie')
        moved=wait(lambda:(v if (v:=actors().get(support['slot'])) and v['active'] and v['health']>0 and v['type']==14 and v['owner']==0
                           and v['sequence']>move_before['sequence'] and any(v[k]!=move_before[k] for k in ('x','y','z')) else None),8)
        result['support_after']=moved
        require(moved,'Submitted move lacks independent original movement observation')
        if mode in ('paused-orders','paused-cast'):
            hero=next(v for v in actors().values() if v['active'] and v['health']>0 and v['type']==0 and v['owner']==0)
            # Candidate identity is observed independently of selection and
            # never substitutes for a command-buffer receiver witness.
            attempts=[];candidate=None
            for offset in (22,40,54):
                point=(width//2,height//2-offset)
                before_input=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
                before_order=orders()[-1]['sequence'];motion(point)
                def matching_candidate():
                    ready=fresh_hover(inputs((a.experiment/'input-admission.bin').read_bytes())['rows'],point,before_input,False,False)
                    return candidate_after_hover(orders(),ready,hero['slot'],before_order)
                candidate=wait(matching_candidate,2)
                attempts.append(dict(point=list(point),expected=hero,candidate=candidate,latest_observation=orders()[-1]))
                if candidate:break
            result['candidate_hovers']=attempts
            result['candidate_observed']=candidate is not None
            spell_before=0;cast_source=None
            if mode=='paused-cast':
                key(ord('w'));key(ord('w'));time.sleep(.4)
                bar_states=[]
                for icon in spell_selection_points('fireball'):
                    after_input=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
                    motion(hud_point(icon,a.frame_size));press(1)
                    def armed_bar():
                        latest=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]
                        if (latest['sequence']>after_input and latest['kind']==1 and latest['mode']==2
                                and latest['selected_slot']<latest['spell_count'] and latest['receiver_slot_plus_one']==hero['slot']+1
                                and (not bar_states or latest['selected_slot']!=bar_states[-1]['selected_slot'])):return latest
                        return None
                    bar=require(wait(armed_bar),'Original spell-bar selection did not change to the next icon')
                    bar_states.append(bar)
                result['spell_bar_states']=bar_states
                prior=read_spell_rows(a.experiment/'spell-events.bin');spell_before=prior[-1][0] if prior else 0
                cast_source=actors()[hero['slot']];result['cast_source_before']=cast_source
                require(cast_source['active'] and cast_source['health']>0 and cast_source['mana']>=6*256,'No living affordable original Fireball caster')
            before_space=orders()[-1]
            key(0x20,observe=False);paused=True
            before_input=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
            pause_point=(width//2-200,height//2+70) if mode=='paused-cast' else (width//2+40,height//2+70);motion(pause_point)
            def paused_ready():
                ready=fresh_hover(inputs((a.experiment/'input-admission.bin').read_bytes())['rows'],pause_point,before_input,False,mode=='paused-cast')
                if ready is None:return None
                if mode=='paused-cast':
                    observed=empty_candidate_after_hover(orders(),ready,before_space['sequence'])
                    protected=[hero,actors()[support['slot']]]
                    if not observed or any(sum((ready['ground'][i]-v[k])**2 for i,k in enumerate(('x','y')))<16 for v in protected):return None
                else:
                    observed=candidate_after_hover(orders(),ready,support['slot'],before_space['sequence'])
                    if observed is None:return None
                result['paused_target_candidate']=observed
                return ready
            ready=wait(paused_ready,3)
            time.sleep(.8)
            observed=inputs((a.experiment/'input-admission.bin').read_bytes())['rows']
            result['space_toggle_hover']=dict(before=before_space,ready=ready,latest_input=observed[-1],
                callbacks=[v for v in observed if v['sequence']>before_input and v['kind']==3],
                worlds=[v for v in orders() if v['sequence']>before_space['sequence'] and v['kind']==1])
            require(ready,'No hover readiness after a recorded original World return during the Space interval')
            held_worlds=result['space_toggle_hover']['worlds']
            require(len(held_worlds)>=2 and held_worlds[-1]['world_counter']==held_worlds[-2]['world_counter']
                and 0<(held_worlds[-1].get('world_return_ordinal',0)-held_worlds[-2].get('world_return_ordinal',0))%2**32<2**31,
                'Space interval did not retain a held simulation clock with distinct original World returns')
            accepted=None
            def accepted_fireball():
                return next((list(v) for v in read_spell_rows(a.experiment/'spell-events.bin') if v[0]>spell_before
                    and v[1]==1 and v[4]==71 and v[5]==hero['slot'] and v[7]==0 and v[15]==1 and v[8]>v[9]),None)
            if mode=='paused-cast':
                require(ready,'Paused cast lacks original callback/later World/cast readiness')
                before_order=orders()[-1]['sequence'];press(3)
                accepted=wait(accepted_fireball,2)
                result['cast_while_clock_held']=accepted
                result['cast_clock_after']=orders()[-1]['world_counter']
                result['cast_submission']=[dict(row=v,command=c) for v in orders() if v['sequence']>before_order for c in v['records'] if c['opcode']==4]
            key(0x20,observe=False);paused=False
            before_input=inputs((a.experiment/'input-admission.bin').read_bytes())['rows'][-1]['sequence']
            motion(points[-1])
            restored=wait(lambda:fresh_hover(inputs((a.experiment/'input-admission.bin').read_bytes())['rows'],points[-1],before_input,False,False))
            result['space_toggle_recovery']=restored
            require(restored,'Original callback/later World readiness did not recover after the second Space toggle')
            if mode=='paused-cast':
                accepted=accepted or wait(accepted_fireball,5)
                result['cast_accepted']=accepted
                result['cast_source_after']=actors().get(hero['slot'])
                require(accepted,'No original Fireball mana debit observed after release')
            require(candidate,'No original wizard candidate at the bounded centred body points')
        result['success']=True
    except Exception as error:result['error']=str(error)
    finally:
        if display:
            if paused:
                try:key(0x20,observe=False);result['space_released_on_failure']=True
                except Exception as error:result['space_release_error']=str(error)
            for kind,code in reversed(held):
                if kind=='key':xt.XTestFakeKeyEvent(display,code,0,0)
                else:xt.XTestFakeButtonEvent(display,code,0,0)
            x.XFlush(display);x.XCloseDisplay(display)
        result['completed']=True
        a.output.write_text(json.dumps(result,indent=2)+'\n')
    # Preserve negative probes while the Qt owner completes normal cleanup.
    return 0


if __name__=='__main__':raise SystemExit(main())
