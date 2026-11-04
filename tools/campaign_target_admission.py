"""Physical-driver candidate observations; neither final receiver nor delivery."""


def candidate_after_hover(rows,ready,slot,after_sequence):
    if not isinstance(slot,int) or not 0<=slot<256 or after_sequence<0:
        raise ValueError('Invalid bounded actor/observation identity')
    return _candidate_match(rows,ready,slot+1,after_sequence)


def empty_candidate_after_hover(rows,ready,after_sequence):
    if after_sequence<0:raise ValueError('Invalid observation identity')
    return _candidate_match(rows,ready,0,after_sequence)


def _candidate_match(rows,ready,expected,after_sequence):
    if ready is None:return None
    worlds=[r for r in rows if r['kind']==1]
    if not worlds:return None
    latest=worlds[-1]
    ready_return=ready.get('world_return_ordinal');observed_return=latest.get('world_return_ordinal')
    if (ready_return is None)!=(observed_return is None):return None
    ready_counter=ready['world_counter'] if ready_return is None else ready_return
    observed_counter=latest['world_counter'] if observed_return is None else observed_return
    if (rows[-1]['kind']!=1 or latest['sequence']<=after_sequence or not latest['candidate_valid']
            or latest['candidate_slot_plus_one']!=expected
            or (observed_counter-ready_counter)%2**32>=2**31):
        return None
    return latest


def hover_nudge(point,size):
    width,height=tuple(size);x,y=point
    if (width,height) not in ((640,480),(800,600)) or not 0<=x<width or not 0<=y<height:
        raise ValueError('Invalid bounded physical hover point')
    return (max(1,x-16) if x>width/2 else min(width-2,x+16),y)


def reserved_boss_target(hero,actors,rearmed,interrupted=False):
    """Authored first-map reserve, not recovered damage/range or admission.

    The observed six-mana cast and twenty-point hits supply a bounded estimate;
    original cast/debit, damage and outcome witnesses remain mandatory.
    """
    if (not rearmed or interrupted or not hero.get('active') or hero.get('type')!=0
            or hero.get('owner')!=0 or hero.get('health',0)<80):return None
    friendly=[v for v in actors.values() if v['slot']!=hero['slot'] and v.get('active') and v.get('owner')==0 and v.get('health',0)>0]
    bosses=[v for v in actors.values() if v.get('active') and v.get('owner')==1 and v.get('type')==24 and v.get('health',0)>0]
    if len(friendly)!=1 or friendly[0]['type']!=14 or len(bosses)!=1:return None
    boss=bosses[0];casts=(boss['health']+19)//20
    return boss if casts<=6 and hero.get('mana',0)>=casts*6*256 else None


def primary_selection_after(rows,slot,after_sequence):
    """Fresh complete-prefix primary bitmap; actor roles belong to the caller."""
    if (type(after_sequence) is not int or after_sequence<0
            or type(slot) is not int or not 0<=slot<256):
        raise ValueError('Invalid bounded selection observation identity')
    if not rows:return None
    latest=rows[-1]
    if (latest['kind']!=1 or latest['sequence']<=after_sequence
            or not latest['flags']&1 or latest['primary']!=[slot]):return None
    pending=set()
    for row in rows:
        if row['kind']==2:pending.add(row['call'])
        elif row['kind']==3:pending.discard(row['call'])
    if pending:return None
    return latest


def support_selection_after(rows,source,after_sequence):
    """Fresh sole-primary World witness, independent of the selection input."""
    if type(after_sequence) is not int or after_sequence<0:
        raise ValueError('Invalid bounded selection observation identity')
    if (not source.get('active') or source.get('health',0)<=0 or source.get('type')!=14
            or source.get('owner')!=0 or type(source.get('slot')) is not int
            or not 0<=source['slot']<256):return None
    return primary_selection_after(rows,source['slot'],after_sequence)


def support_attack_submission(rows,source,target,after_sequence):
    """Witness original opcode7 buffer submission; never infer engagement."""
    if not (source.get('active') and source.get('health',0)>0 and source.get('type')==14 and source.get('owner')==0
            and target.get('active') and target.get('health',0)>0 and target.get('type')==24 and target.get('owner')==1):
        return None
    if any(type(v.get('slot')) is not int or not 0<=v['slot']<256 for v in (source,target)):return None
    entries={r['call']:r for r in rows if r['kind']==2}
    for r in rows:
        if r['sequence']<=after_sequence or r['kind']!=3 or r['mask']!=4 or r['flags']&5!=5:continue
        before=entries.get(r['call'])
        if not before or before['sequence']<=after_sequence or not before['flags']&1 or before['primary']!=[source['slot']]:continue
        for command in r['records']:
            if command['opcode']==7 and command.get('actor_slot')==source['slot'] and command.get('target_slot')==target['slot']:
                return dict(entry=before,returned=r,command=command,command_delivery_proved=False)
    return None


def portal_destination(hero,portal,point):
    """Authored adjacent-cell sidestep bound, not a recovered path/exit trigger."""
    if (not isinstance(point,(list,tuple)) or len(point)!=3
            or any(type(v) is not int for v in point)):return False
    remaining=sum((point[i]-portal[k])**2 for i,k in enumerate(('x','y')))
    moved=any(point[i]!=hero[k] for i,k in enumerate(('x','y','z')))
    return remaining<=2 and abs(point[2]-portal['z'])<=1 and (moved or remaining==0)


def portal_move_submission(rows,hero,portal,after_sequence):
    """Fresh sole-Wizard opcode5 move acknowledgement, never command delivery."""
    if type(after_sequence) is not int or after_sequence<0:raise ValueError('Invalid observation identity')
    if (hero.get('type')!=0 or hero.get('owner')!=0 or not hero.get('active')
            or hero.get('health',0)<=0 or type(hero.get('slot')) is not int
            or not 0<=hero['slot']<256):return None
    if primary_selection_after(rows,hero['slot'],after_sequence) is None:return None
    entries={r['call']:r for r in rows if r['kind']==2}
    for row in rows:
        if row['sequence']<=after_sequence or row['kind']!=3 or row['mask']!=4 or row['flags']&5!=5:continue
        before=entries.get(row['call'])
        if (not before or before['sequence']<=after_sequence or not before['flags']&1
                or before['primary']!=[hero['slot']] or row['primary']!=[hero['slot']]
                or len(row['records'])!=1):continue
        command=row['records'][0]
        if (command['opcode']==5 and command.get('length')==21
                and command.get('mode')==0 and command.get('extra')==0
                and command.get('actor_slot')==hero['slot']
                and portal_destination(hero,portal,command.get('destination'))):
            return dict(entry=before,returned=row,command=command,command_delivery_proved=False)
    return None
