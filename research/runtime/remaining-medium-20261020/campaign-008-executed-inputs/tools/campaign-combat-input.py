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

from campaign_input_admission import DialogueReadiness, spell_selection_points, fresh_hover, logical_input_point, pursuit_waypoint, living_engaged_targets, ground_waypoint
from campaign_gameplay import read_rows, creatures, damage_rows, player_lethal_rows
from campaign_spell import read_spell_rows, player_fireball_damage, player_engaged_fireball_lethal
from campaign_input_observation import decode as decode_input_observation
from campaign_render_oracle import oracle, validate_summon_capture


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
    button_hold_ms=int(os.environ.get('MNM_CAMPAIGN_BUTTON_HOLD_MS','120'))
    if not 30<=button_hold_ms<=1500:raise ValueError('Invalid bounded physical button hold')
    hover_required=os.environ.get('MNM_CAMPAIGN_HOVER_READINESS')=='1'
    admission_path=args.experiment/'input-admission.bin'
    start=time.monotonic(); actions=[]; captures=[]
    resident_dialogue=os.environ.get('MNM_PUBLIC_RESIDENT_WORLD')=='1'
    result=dict(success=False,input_readiness_policy="continuous-quiet-distinct-spell-v2",button_hold_ms=button_hold_ms,hover_readiness_policy="original-world-after-motion-v1" if hover_required else "fixed-delay-legacy",monotonic_start=start,dialogue_oracle=oracle(resident_dialogue),summon_count_oracle=oracle(resident_dialogue))
    def record(action,**values):
        actions.append(dict(action=action,seconds=time.monotonic()-start,**values))
        args.output.write_text(json.dumps(dict(result,seconds=time.monotonic()-start,actions=actions,captures=captures),indent=2)+'\n')
    def input_rows():
        if admission_path.is_symlink():raise RuntimeError('Redirected input readiness trace')
        return decode_input_observation(admission_path.read_bytes())['rows']
    def click(px,py,b=1,admit_cast=False):
        px,py=logical_input_point(px,py)
        prior=input_rows() if hover_required and b==3 else []
        after=prior[-1]['sequence'] if prior else 0
        already=bool(prior and prior[-1]['input_active']==1 and abs(prior[-1]['x']-px)<=1 and abs(prior[-1]['y']-py)<=1)
        if not xt.XTestFakeMotionEvent(display,-1,x0+int(w*px/800),y0+int(h*py/600),0):raise RuntimeError('Motion injection failed')
        x.XFlush(display);record('motion',logical_x=px,logical_y=py);time.sleep(.1)
        if hover_required and b==3:
            deadline=time.monotonic()+10
            admitted=None
            while time.monotonic()<deadline:
                try:admitted=fresh_hover(input_rows(),(px,py),after,already,admit_cast)
                except (OSError,ValueError):admitted=None
                if admitted is not None:break
                time.sleep(.05)
            if admitted is None:raise RuntimeError('Original hover did not become ready before physical right-click')
            proof=dict(after_sequence=after,ready_sequence=admitted['sequence'],point=[px,py],already_there=already,require_cast=admit_cast,ground=admitted['ground'],cursor_type=admitted['cursor_type'])
            result.setdefault('hover_admissions',[]).append(proof);record('original-hover-ready',**proof)

        press_sequence=read_rows(trace)[-1][0] if b==3 else None
        for down in (1,0):
            if not xt.XTestFakeButtonEvent(display,b,down,0):raise RuntimeError('Button injection failed')
            x.XFlush(display);time.sleep(button_hold_ms/1000 if down else .12)
        record('click',button=b,hold_ms=button_hold_ms,before_sequence=press_sequence)
        return press_sequence
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
    def settle_dialogue(label, minimum_seconds=12):
        initial = label == 'dialogue-check'
        checks = result.setdefault('dialogue_checks' if initial else 'additional_dialogue_checks', [])
        gate = DialogueReadiness(resident_dialogue, minimum_seconds, 48)
        while True:
            item = capture(label+'-'+str(gate.observations))
            texts = {}
            from PIL import Image
            for route in ('native', 'original'):
                crop = out/(item['name']+'-'+route+'-dialogue.png')
                Image.open(out/(item['name']+'-'+route+'.png')).crop((225,90,600,270)).save(crop)
                texts[route] = subprocess.run(['tesseract',str(crop),'stdout','--psm','6'],capture_output=True,text=True,check=True,timeout=10).stdout.strip()
            visible = {r: 'hermes' in texts[r].lower() for r in texts}
            observed = time.monotonic()
            decision = gate.observe(visible, observed)
            checks.append(dict(stage=label,capture=item['name'],texts=texts,visible=visible,observed_monotonic=observed,quiet_seconds=minimum_seconds,decision=decision))
            record('dialogue-check',stage=label,visible=visible,decision=decision)
            if decision == 'ready':
                return gate.dismissals
            if decision == 'dismiss':
                click(400,300)
                time.sleep(.6)
            else:
                time.sleep(3 if minimum_seconds>=12 else 2)
    def select_spell(spell):
        # Re-admit selection after a modal, never repeat an executed cast.
        for attempt in range(3):
            for point in spell_selection_points(spell):
                click(*point)
                time.sleep(.25)
            # After the first observed summon, gameplay checks each cast's
            # original effect. Introductory readiness does not become an
            # expensive per-cast framebuffer sampling loop during combat.
            if 'summon' in result or not settle_dialogue(spell+'-selection-'+str(attempt), 2):
                record('spell-selected',spell=spell,attempt=attempt)
                return
        raise RuntimeError('Spell selection repeatedly consumed by dialogue')
    def center():
        click(744,522);click(744,522);time.sleep(.75)
    def blocked_cast(label, owner, wizard_slot):
        select_spell('zombie')
        prior_rows=read_rows(trace);begin=prior_rows[-1][0];prior=creatures(prior_rows)
        click(90 if label=='invalid-target' else 440,90 if label=='invalid-target' else 310,3)
        time.sleep(2);current_rows=read_rows(trace);current=creatures(current_rows)
        created=[a['slot'] for a in current.values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and
                 (a['slot'] not in prior or not prior[a['slot']]['active'] or prior[a['slot']]['type']!=14 or prior[a['slot']]['owner']!=owner)]
        if created or current[wizard_slot]['mana']<prior[wizard_slot]['mana']:raise RuntimeError('Expected blocked cast created a Zombie or spent mana')
        item=dict(kind=label,before_sequence=begin,after_sequence=current_rows[-1][0],mana_before=prior[wizard_slot]['mana'],mana_after=current[wizard_slot]['mana'],created_slots=created)
        capture(label,True);result.setdefault('blocked_casts',[]).append(item);record('blocked-cast',**item)
        settle_dialogue('after-'+label)
    try:
        deadline=time.monotonic()+12
        while not trace.exists() or not state():
            if time.monotonic()>deadline:raise RuntimeError('No creature snapshots from original World')
            time.sleep(.2)
        initial=state();wizards=[a for a in initial.values() if a['type']==0 and a['active'] and a['health']>0]
        if len(wizards)!=1:raise RuntimeError('Expected unique living player wizard: '+str(wizards))
        wizard=wizards[0];owner=wizard['owner'];result.update(player_owner=owner,wizard_slot=wizard['slot'],initial_creatures=initial)
        click(400,300);time.sleep(.5) # dismiss ordinary Hermes dialogue
        # Introductory dialogues consume HUD/scene clicks. Retain both images.
        # Resident World drawing does not clear old CPU World pixels, so only
        # the resident image can classify dialogue there. Successful original
        # summon/cast/combat observations below separately prove input admission.
        settle_dialogue('dialogue-check')
        before=capture('before-summon',True)
        validate_summon_capture(before,out,'0/15',resident_dialogue)
        if args.spell_cases:
            center();settle_dialogue('after-center');blocked_cast('invalid-target',owner,wizard['slot'])
        select_spell('zombie');cast_before=read_rows(trace)[-1][0];cast_prior=state()
        cast_before=click(440,310,3,admit_cast=True)
        cast_prior=creatures([r for r in read_rows(trace) if r[0]<=cast_before])
        record('summon-order',before_sequence=cast_before)
        deadline=time.monotonic()+12; summoned=[]
        while time.monotonic()<deadline:
            summoned=[a for a in state().values() if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and
                      (a['slot'] not in cast_prior or not cast_prior[a['slot']]['active'] or cast_prior[a['slot']]['type']!=14 or cast_prior[a['slot']]['owner']!=owner)]
            if summoned:break
            time.sleep(.25)
        if not summoned:raise RuntimeError('Native right-click did not create a living player Zombie')
        time.sleep(1);after=capture('after-summon',True)
        validate_summon_capture(after,out,'1/15',resident_dialogue)
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
            # Walking deltas depend on pathfinding/obstacles and elapsed travel;
            # they establish input progress, not the camera transform. Keep the
            # independent probes, then use the existing isometric projection.
            waypoint=ground_waypoint(actor,enemy)
            record('approach-enemy',wizard=actor,enemy=enemy,logical_point=list(waypoint),
                   projection_policy='isometric-ground-v1')
            center();click(*waypoint,3);time.sleep(3)
            center()
            if math.hypot(dx,dy)<7:
                if args.spell_cases and not result.get('ranged'):
                    # The starting green spell is Fireball71. Cast at nearby
                    # enemy terrain and require damage in its original effect.
                    capture('ranged-aim')
                    actor=state()[wizard['slot']];enemy=state()[enemy['slot']]
                    tx,ty,tz=enemy['x']-actor['x'],enemy['y']-actor['y'],enemy['z']-actor['z']
                    predicted=(max(90,min(650,400+32*(tx-ty))),max(100,min(430,300+16*(tx+ty-tz)-22)))
                    record('ranged-target-projection',wizard=actor,enemy=enemy,logical_point=list(predicted))
                    spent=0
                    for px,py in (predicted,(400,220),(400,200),(420,225),(380,225),(400,270),(400,240),(430,270),(370,270)):
                        mana_before=state()[wizard['slot']]['mana']
                        select_spell('fireball');begin=read_rows(trace)[-1][0];click(px,py,3);time.sleep(1)
                        effects=player_fireball_damage(read_spell_rows(args.experiment/'spell-events.bin'),owner,wizard['slot'])
                        record('fireball-order',before_sequence=begin,logical_x=px,logical_y=py)
                        spent+=int(state()[wizard['slot']]['mana']<mana_before)
                        if effects:result['ranged']=dict(first_damage=list(effects[0]));capture('fireball-damage');break
                        if spent>=2:break
                    if not result.get('ranged'):raise RuntimeError('No player Fireball enemy damage after bounded shots')
                # Summon beside the enemy, through the same native spell HUD.
                # The approach delta was computed before moving the wizard.
                # Recompute this target from current observed positions; using
                # the old movement vector can place the Zombie past the enemy.
                actor=state()[wizard['slot']];enemy=state()[enemy['slot']]
                tx,ty,tz=enemy['x']-actor['x'],enemy['y']-actor['y'],enemy['z']-actor['z']
                summon_point=(max(90,min(650,400+32*(tx-ty)+32)),
                              max(100,min(430,300+16*(tx+ty-tz)+16)))
                record('combat-summon-projection',wizard=actor,enemy=enemy,logical_point=list(summon_point))
                select_spell('zombie');cast_sequence=read_rows(trace)[-1][0];prior=state()
                click(*summon_point,3)
                # The original effect can create its creature after the old
                # fixed two-second sample. Retain the actual creation interval;
                # unrelated owned Zombies cannot silently become proven casts.
                deadline=time.monotonic()+12
                while True:
                    current=state();new=[]
                    for a in current.values():
                        old=prior.get(a['slot'])
                        if a['type']==14 and a['owner']==owner and a['active'] and a['health']>0 and (not old or not old['active'] or old['type']!=14 or old['owner']!=owner):
                            new.append(dict(before_sequence=cast_sequence,after_sequence=a['sequence'],slot=a['slot']))
                    if new or time.monotonic()>=deadline:break
                    time.sleep(.25)
                result['additional_summons'].extend(new)
                record('combat-summon-observation',summons=new,before_sequence=cast_sequence)
                extra_slots=[a['slot'] for a in [result['summon']]+result['additional_summons']]
                capture('combat-approach')
                # Keep the wizard's original engagement while the adjacent
                # summon assists. Premature clearance can divert both actors
                # onto another enemy; retreat after the verified kill instead.
                time.sleep(3)
        if 'combat' not in result:raise RuntimeError('No player Zombie melee damage before bounded combat deadline')
        # Autonomous enemies can flee or draw the Zombie onto another target.
        # Retain a finite observation window without changing AI/health or the
        # same-engaged-enemy player-party lethal and living-wizard requirements.
        finish_deadline=time.monotonic()+120
        finishing_casts=0; pursuit_orders=0; last_progress=time.monotonic(); last_hits=0; last_pursuit=0
        while True:
            rows=read_rows(trace);hits=damage_rows(rows,owner,extra_slots,cast_before)
            lethal=player_lethal_rows(rows,owner,wizard['slot'],hits)
            spell_lethal=player_engaged_fireball_lethal(read_spell_rows(args.experiment/'spell-events.bin'),rows,owner,wizard['slot'],hits)
            if (lethal or spell_lethal) and time.monotonic()-start>=args.seconds:
                result['combat'].update(hit_count=len(hits))
                if lethal:result['combat']['lethal_hit']=list(lethal[0])
                if spell_lethal:result['combat']['lethal_fireball_hit']=list(spell_lethal[0])
                break
            if state()[wizard['slot']]['health']<=0:raise RuntimeError('Player wizard died before required combat completion')
            if time.monotonic()>finish_deadline:raise RuntimeError('No player party lethal health depletion before combat deadline')
            actor=state()[wizard['slot']]
            if len(hits)!=last_hits:
                last_hits=len(hits);last_progress=time.monotonic()
            living=state()
            targets=living_engaged_targets(living,hits)
            stalled=time.monotonic()-last_progress>=8
            if args.spell_cases and (actor['health']<=120 or stalled) and actor['mana']>=6*256 and finishing_casts<2:
                # Exercise another ordinary ranged cast to support the same
                # melee-engaged enemy. Waiting motionless can kill the wizard
                # before the required party lethal; no game state is patched.
                living=state();targets=living_engaged_targets(living,hits)
                if targets:
                    enemy=targets[-1];center();actor=state()[wizard['slot']];enemy=state()[enemy['slot']]
                    tx,ty,tz=enemy['x']-actor['x'],enemy['y']-actor['y'],enemy['z']-actor['z']
                    point=(max(90,min(650,400+32*(tx-ty))),max(100,min(430,300+16*(tx+ty-tz)-22)))
                    select_spell('fireball');begin=read_rows(trace)[-1][0];mana=actor['mana'];click(*point,3);time.sleep(1);finishing_casts+=1
                    record('combat-finishing-fireball',before_sequence=begin,wizard=actor,enemy=enemy,logical_point=list(point),mana_before=mana,mana_after=state()[wizard['slot']]['mana'])
                    continue
            # Follow only an enemy already struck by a proven player summon.
            # The original AI can retreat beyond passive melee range. Physical
            # ground orders retain the same lethal/living-wizard assertions,
            # twelve-order budget and original 120-second finish deadline.
            if targets and stalled and pursuit_orders<12 and time.monotonic()-last_pursuit>=8:
                center();living=state();actor=living[wizard['slot']];targets=living_engaged_targets(living,hits)
                if not targets:continue
                enemy=targets[-1];point=pursuit_waypoint(actor,enemy)
                if point is not None:
                    before_sequence=read_rows(trace)[-1][0]
                    click(*point,3);pursuit_orders+=1;last_pursuit=time.monotonic()
                    record('combat-pursuit',order=pursuit_orders,before_sequence=before_sequence,
                           wizard=actor,enemy=enemy,logical_point=list(point))
                    time.sleep(2);continue
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
            # Return to a previously occupied ground tile after the opening
            # enemies have been depleted. A guessed offset near the map edge
            # can overshoot when HUD centering clamps the camera.
            retreat_x, retreat_y = wizard['x'], wizard['y']
            result['retreat_target'] = [retreat_x, retreat_y]
            center()
            # Long retreats can need more than eight bounded steps at native
            # cadence. Keep exact waypoint/aliveness admission unchanged.
            for attempt in range(12):
                actor=state()[wizard['slot']];dx=retreat_x-actor['x'];dy=retreat_y-actor['y']
                if math.hypot(dx,dy)<3:break
                # Short movement probes measure path progress, not the camera
                # transform: obstacles and incomplete travel skew that basis.
                # Use the same isometric XY/height projection as the ordinary
                # ranged/summon targets, retaining exact waypoint admission.
                dz=wizard['z']-actor['z']
                sx,sy=32*(dx-dy),16*(dx+dy-dz)
                scale=max(1,abs(sx)/170,abs(sy)/110);sx/=scale;sy/=scale
                record('retreat-ground-projection',wizard=actor,target=[retreat_x,retreat_y,wizard['z']],screen_delta=[sx,sy])
                center();click(400+sx,280+sy,3);time.sleep(3)
                record('post-combat-retreat',attempt=attempt+1,wizard=state()[wizard['slot']])
            actor=state()[wizard['slot']]
            if actor['health']<=0 or math.hypot(retreat_x-actor['x'],retreat_y-actor['y'])>=3:
                raise RuntimeError('Post-combat ground retreat did not reach its observed waypoint')
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
