"""Bounded callback toggle and mapped-key/scalar observations; no frame closure."""
from drawing_region_selection import identity
from drawing_observed_inputs import unique


def tracked_identity(sample):
    t = sample.get('tracked_actor', {})
    if not t.get('actor_valid') or not t['actor_active'] or not t['actor_health']:
        return None
    return tuple(t[k] for k in ('pool_id', 'capacity', 'slot_plus_one', 'actor_type', 'actor_owner'))


def latch(sample):
    return (sample['tracked_actor']['actor_flag_68'], sample['raw_latch_count'], sample['mode'])


def observe_toggles(region, actions):
    if region['wire_version'] != 3 or any(region[k] for k in ('dropped', 'foreign_thread_calls', 'pending')):
        raise ValueError('Closed V3 owner-only actor tracking required')
    unique(region['calls'],'call')
    if [c['call'] for c in region['calls']]!=sorted(c['call'] for c in region['calls']):raise ValueError('Toggle callback order differs')
    clicks = [a for a in actions if a.get('kind') == 'click' and a.get('button') == 1]
    if len(clicks) != 3 or [a.get('modifiers', []) for a in clicks] != [[], ['Shift'], ['Shift']]:
        raise ValueError('One select and two explicit Shift clicks required')
    unique(clicks,'ordinal')
    if [a['ordinal'] for a in clicks]!=sorted(a['ordinal'] for a in clicks):raise ValueError('Toggle action order differs')
    transitions, key = [], None
    for a, before, after in zip(clicks, (0, 1, 0), (1, 0, 1)):
        xy = (a['x'], a['y'])
        pairs = [[c for c in region['calls'] if c['mask'] == mask and tuple(c['mouse_record'][:2]) == xy] for mask in (1, 2)]
        if any(len(p) != 1 for p in pairs):
            raise ValueError('Ambiguous toggle callback coordinates')
        down, up = pairs[0][0], pairs[1][0]
        current = identity(down['entry'])
        if not current or key is not None and current != key:
            raise ValueError('Toggle lacks same validated actor hit')
        key = current
        if any(identity(s)!=key or tracked_identity(s) != key for c in (down, up) for s in (c['entry'], c['returned'])):
            raise ValueError('Tracked toggle actor identity changed')
        if latch(down['entry']) != (before,) * 3 or latch(down['returned']) != (after,) * 3:
            raise ValueError('Original toggle fields differ')
        if up['call'] <= down['call'] or any(latch(s) != (after,) * 3 for s in (up['entry'], up['returned'])):
            raise ValueError('Toggle release does not retain state')
        if transitions and down['call'] <= transitions[-1]['up_call']:
            raise ValueError('Toggle callback order differs')
        transitions.append(dict(action_ordinal=a['ordinal'], coordinates=list(xy),
                                down_call=down['call'], up_call=up['call'], before=before,
                                after=after, observed_identity=list(key), return_tick=down['returned']['tick']))
    for i, t in enumerate(transitions):
        stop = transitions[i + 1]['down_call'] if i + 1 < len(transitions) else float('inf')
        retained = []
        for c in region['calls']:
            s = c['returned']
            elapsed = (s['tick'] - t['return_tick']) & 0xffffffff
            if t['up_call'] < c['call'] < stop and any(tracked_identity(v)!=key or latch(v)!=(t['after'],)*3 for v in (c['entry'],c['returned'])):
                raise ValueError('Intervening toggle identity/latch refusal')
            if t['up_call'] < c['call'] < stop and s['actor_state'] == 0 and tracked_identity(s) == key and latch(s) == (t['after'],) * 3 and 100 <= elapsed < 0x80000000:
                retained.append(dict(call=c['call'], elapsed_ms=elapsed))
        if not retained:
            raise ValueError('No independent pointer-away state after toggle')
        t['independent_pointer_away'] = retained
    return dict(transitions=transitions, bounded_select_clear_reselect_observed=True,
                actor_allocation_generation_proved=False, complete_selection_semantics=False,
                synchronized_frame_source_proved=False)


def observe_pauses(semantic, rows, actions):
    requested = [a['key'] for a in actions if a.get('kind') == 'tap' and a.get('key') in ('space', 'Insert', 'Delete')]
    if requested != ['space', 'space', 'Insert', 'Delete']:
        raise ValueError('Exact documented pause key sequence required')
    samples = [s for s in semantic['samples'] if s['top_screen'] == 2 and s['validity'] == 63]
    if not samples:
        raise ValueError('Valid original World samples required')
    all_samples=semantic['samples']
    if any(type(s['tick']) is not int or not 0<=s['tick']<2**32 for s in all_samples) or any(b['tick']<=a['tick'] for a,b in zip(all_samples,all_samples[1:])):
        raise ValueError('Pause sample clock order differs')
    if any(type(s[k]) is not int or not 0<=s[k]<2**32 for s in samples for k in ('accepted_sequence','world_counter')) or any(not 0<=(b['accepted_sequence']-a['accepted_sequence'])&0xffffffff<2**31 for a,b in zip(samples,samples[1:])):
        raise ValueError('Pause counter identity/order differs')
    if [r[0] for r in rows]!=sorted(set(r[0] for r in rows)):
        raise ValueError('Mapped key journal order/identity differs')
    first, last = samples[0]['tick'], samples[-1]['tick']
    if last < first:
        raise ValueError('Wrapped pause interval outside this diagnostic policy')
    keys = []
    for r in rows:
        if r[1:3] == (2, 4) and r[21] in (32, 45, 46) and first <= r[10] <= last:
            if r[6] != semantic['owner_thread'] or not 0x400000 <= r[5] < 0x5c0000 or r[22] != 0 or r[11] < r[10]:
                raise ValueError('Mapped original pause key owner/caller/timing differs')
            keys.append(dict(code=r[21], entry_tick=r[10], return_tick=r[11], ordinal=r[0], caller_rva=r[5] - 0x400000))
    groups = []
    for k in keys:
        if not groups or k['code'] != groups[-1][-1]['code'] or k['entry_tick'] - groups[-1][-1]['return_tick'] > 500:
            groups.append([])
        groups[-1].append(k)
    if [g[0]['code'] for g in groups] != [32, 32, 45, 46]:
        raise ValueError('Original mapped-key groups do not match requested sequence')
    changes = []
    for previous, s in zip(samples, samples[1:]):
        if s['mode'] != previous['mode']:
            changes.append((previous, s))
    if len(changes) != 4:
        raise ValueError('Four independent World mode transitions required')
    for i, (g, (previous, s)) in enumerate(zip(groups, changes)):
        prior = [v for v in samples if v['tick'] < g[0]['entry_tick']]
        if not prior or prior[-1]['mode'] != previous['mode'] or not 0 <= g[0]['entry_tick'] - prior[-1]['tick'] <= 500 or (previous['mode'], s['mode']) != ((0, 1) if i % 2 == 0 else (1, 0)) or not 0 <= s['tick'] - g[-1]['return_tick'] <= 500:
            raise ValueError('Mapped key and observed mode transition not bounded')
    pauses = []
    for i in (0, 2):
        start, end = changes[i][1]['tick'], changes[i + 1][1]['tick']
        interval = [s for s in samples if start <= s['tick'] < end]
        if any(s['top_screen']!=2 or s['validity']!=63 for s in all_samples if start<=s['tick']<end):
            raise ValueError('Invalid/unavailable World sample inside pause interval')
        duration = interval[-1]['tick'] - interval[0]['tick']
        if len(interval) < 5 or duration < 1000 or any(s['mode'] != 1 or s['world_counter'] != interval[0]['world_counter'] for s in interval) or interval[-1]['accepted_sequence'] <= interval[0]['accepted_sequence']:
            raise ValueError('Paused counter/continuing rendering interval not observed')
        before = [s for s in samples if s['tick'] < start and s['mode'] == 0]
        after = [s for s in samples if s['tick'] >= end and s['mode'] == 0]
        if len(before) < 2 or len(after) < 2 or before[-1]['world_counter'] <= before[0]['world_counter'] or after[-1]['world_counter'] <= after[0]['world_counter']:
            raise ValueError('Active World counter controls absent')
        pauses.append(dict(first_tick=start, resume_tick=end, samples=len(interval), duration_ms=duration,
                           world_counter=interval[0]['world_counter'],
                           accepted_sequence_delta=interval[-1]['accepted_sequence'] - interval[0]['accepted_sequence'],
                           selected_ani_fields_stable=all(s['ani_fields'] == interval[0]['ani_fields'] for s in interval),
                           all_animation_pause_proved=False))
    return dict(mapped_key_groups=groups, pause_intervals=pauses, selected_counter_pause_resume_observed=True,
                keyboard_message_phase_proved=False, all_simulation_or_animation_pause_proved=False,
                synchronized_frame_source_proved=False)
