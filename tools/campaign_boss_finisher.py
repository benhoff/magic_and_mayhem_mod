"""Bounded physical test policy; original spell admission and hits stay separate."""
import math


def boss_finisher(hero,actors,submission):
    """Keep a wounded, already ordered boss ahead of unrelated melee targets.

    A bounded volley can leave a wounded boss and ordinary casts in reserve.
    Twenty HP belongs to the separate Return-hint observation policy. This
    selects an input target only, never infers projectile damage, delivery of
    the support order or a natural outcome.
    """
    if (hero.get('type')!=0 or hero.get('owner')!=0 or not hero.get('active')
            or hero.get('health',0)<=0 or hero.get('mana',0)<6*256
            or type(hero.get('slot')) is not int or not 0<=hero['slot']<256
            or not isinstance(submission,dict)):return None
    command=submission.get('command',{})
    if (type(command.get('actor_slot')) is not int or not 0<=command['actor_slot']<256
            or type(command.get('target_slot')) is not int or not 0<=command['target_slot']<256):return None
    source=actors.get(command.get('actor_slot'))
    bosses=[v for v in actors.values() if v.get('type')==24 and v.get('owner')==1
            and v.get('active') and v.get('health',0)>0]
    if len(bosses)!=1:return None
    target=bosses[0]
    if (source is None or source.get('type')!=14 or source.get('owner')!=0
            or command.get('opcode')!=7 or command.get('target_slot')!=target.get('slot')
            or not 0<target['health']<=100 or target['health']==20
            or math.hypot(target['x']-hero['x'],target['y']-hero['y'])>12):return None
    entry=submission.get('entry',{});returned=submission.get('returned',{})
    if (entry.get('kind')!=2 or returned.get('kind')!=3
            or type(entry.get('call')) is not int
            or entry.get('call')!=returned.get('call') or entry.get('call',0)<=0
            or entry.get('thread',0)<=0 or entry.get('thread')!=returned.get('thread')
            or entry.get('primary')!=[source['slot']] or returned.get('primary')!=[source['slot']]
            or entry.get('flags',0)&1!=1 or returned.get('flags',0)&5!=5
            or entry.get('mask')!=4 or returned.get('mask')!=4
            or command not in returned.get('records',[])
            or entry.get('sequence',0)>=returned.get('sequence',0)):
        return None
    return target


def finishing_pause_target(hero,actors,submission,first,second_started):
    """One additional ordinary pause after a released original boss volley.

    Input policy only. A source-bound prior volley and current wounded target
    permit another attempt; original hits and a natural outcome stay separate.
    """
    if (second_started or not isinstance(first,dict) or first.get('released') is not True
            or type(first.get('accepted_in_burst')) is not int
            or not 1<=first['accepted_in_burst']<=4):return None
    target=boss_finisher(hero,actors,submission)
    prior=first.get('target',{})
    if (target is None or prior.get('slot')!=target['slot']
            or prior.get('type')!=24 or prior.get('owner')!=1):return None
    return target


def _wizard_melee_roles(hero,boss,continuing=False):
    return (hero.get('type')==0 and hero.get('owner')==0 and hero.get('active')
        and hero.get('health',0)>=(40 if continuing else 80)
        and 0<=hero.get('mana',-1)<6*256
        and boss.get('type')==24 and boss.get('owner')==1 and boss.get('active')
        and 0<boss.get('health',0)<20
        and all(type(v.get('slot')) is int and 0<=v['slot']<256 for v in (hero,boss))
        and hero['slot']!=boss['slot'])


def _closed_wizard_melee(proof,hero,boss):
    if not isinstance(proof,dict):return False
    entry=proof.get('entry',{});returned=proof.get('returned',{});command=proof.get('command',{})
    return (entry.get('kind')==2 and returned.get('kind')==3
        and type(entry.get('call')) is int and entry['call']>0 and entry['call']==returned.get('call')
        and entry.get('thread',0)>0 and entry['thread']==returned.get('thread')
        and entry.get('sequence',0)>0 and entry['sequence']<returned.get('sequence',0)
        and entry.get('mask')==returned.get('mask')==4
        and entry.get('flags',0)&1==1 and returned.get('flags',0)&5==5
        and entry.get('primary')==returned.get('primary')==[hero['slot']]
        and returned.get('records')==[command]
        and type(command.get('actor_slot')) is type(command.get('target_slot')) is int
        and command.get('opcode')==7 and command.get('length')==14
        and command.get('actor_slot')==hero['slot'] and command.get('target_slot')==boss['slot'])


def wizard_melee_submission(rows,hero,boss,after_sequence):
    """Closed original sole-Wizard attack buffer, never delivery or a hit."""
    from campaign_target_admission import primary_selection_after
    if not _wizard_melee_roles(hero,boss):return None
    if primary_selection_after(rows,hero['slot'],after_sequence) is None:return None
    entries={row['call']:row for row in rows if row['kind']==2}
    for row in rows:
        if row['kind']!=3 or row['sequence']<=after_sequence:continue
        before=entries.get(row['call'])
        if before is None or before['sequence']<=after_sequence:continue
        for command in row['records']:
            proof=dict(entry=before,returned=row,command=command,command_delivery_proved=False)
            if _closed_wizard_melee(proof,hero,boss):return proof
    return None


def wizard_melee_finisher(hero,actors,rows,submission=None):
    """Continue an observed Wizard melee against a nearly defeated boss.

    Authored physical policy only: initial80HP, under6mana, target1..19HP,
    recent same-thread original positive Wizard damage and at most6cells.
    An actual closed attack permits up to30seconds of continuation at40HP,
    avoiding repeated attack inputs restarting the ordinary animation. Original
    damage, retirement, portal delivery and outcome remain independent.
    """
    if not rows or hero is None:return None
    bosses=[v for v in actors.values() if v.get('type')==24 and v.get('owner')==1
            and v.get('active') and v.get('health',0)>0]
    if len(bosses)!=1:return None
    boss=bosses[0];now=rows[-1][3]
    if not _wizard_melee_roles(hero,boss,True):return None
    continuing=_closed_wizard_melee(submission,hero,boss)
    if continuing:
        tick=submission['returned'].get('tick')
        if type(tick) is not int or (now-tick)%2**32>30000:return None
    if not _wizard_melee_roles(hero,boss,continuing):return None
    distance=math.hypot(boss['x']-hero['x'],boss['y']-hero['y'])
    if continuing:return boss if distance<=12 else None
    if distance>6:return None
    for row in reversed(rows):
        if (row[1]==2 and row[2]==rows[-1][2] and (now-row[3])%2**32<=10000
                and row[4:7]==(hero['slot'],0,0) and 0<row[7]<2**31
                and row[8:11]==(boss['slot'],24,1)
                and 0<row[12]<20 and row[12]<row[11]<2**31 and row[13]):return boss
    return None
