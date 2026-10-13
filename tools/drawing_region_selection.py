"""Bounded Region click/raw-latch evidence; not actor generation or full semantics."""
def identity(sample):
    if sample['actor_state']!=1 or not sample['actor_active'] or not sample['actor_health']:
        return None
    return tuple(sample[k] for k in ('pool_id','capacity','slot_plus_one','actor_type','actor_owner'))

def observe(region,actions):
    if region['wire_version']!=2 or region['dropped'] or region['foreign_thread_calls'] or region['pending']:
        raise ValueError('Closed owner-only RegionV2required')
    matches=[]
    for action in actions:
        if action.get('kind')!='click' or action.get('button')!=1:continue
        xy=(action['x'],action['y'])
        down=[c for c in region['calls'] if c['mask']==1 and tuple(c['mouse_record'][:2])==xy]
        up=[c for c in region['calls'] if c['mask']==2 and tuple(c['mouse_record'][:2])==xy]
        if not down and not up:continue
        if len(down)!=1 or len(up)!=1:raise ValueError('Ambiguous original button callback match')
        start,end=down[0]['entry'],down[0]['returned'];key=identity(start)
        if not key or identity(end)!=key:raise ValueError('Click actor identity/validity changed')
        if (start['actor_flag_68'],end['actor_flag_68'])!=(0,1) or (start['raw_latch_count'],end['raw_latch_count'])!=(0,1) or (start['mode'],end['mode'])!=(0,1):
            raise ValueError('Observed callback does not establish latch transition')
        release=up[0]
        if release['call']<=down[0]['call'] or any(identity(s)!=key or s['actor_flag_68']!=1 or s['raw_latch_count']!=1 for s in (release['entry'],release['returned'])):
            raise ValueError('Button release lacks matching observed latch')
        later=[]
        for call in region['calls']:
            if call['call']<=release['call']:continue
            sample=call['returned'];elapsed=(sample['tick']-end['tick'])&0xffffffff
            if identity(sample)==key and sample['actor_flag_68']==1 and sample['raw_latch_count']==1 and 1000<=elapsed<0x80000000:
                later.append(dict(call=call['call'],elapsed_ms=elapsed,actor_health=sample['actor_health']))
        if not later:raise ValueError('No later validated observed-slot latch')
        matches.append(dict(action_ordinal=action['ordinal'],coordinates=list(xy),down_call=down[0]['call'],up_call=release['call'],observed_identity=list(key),later=later))
    if len(matches)!=1:raise ValueError('Exactly one unique admitted click required')
    return dict(callback_latch_transition_observed=True,later_observed_slot_latch=True,matches=matches,
                actor_allocation_generation_proved=False,persistent_selection_semantics_proved=False,
                tooltip_proved=False,complete_presented_journey=False)
