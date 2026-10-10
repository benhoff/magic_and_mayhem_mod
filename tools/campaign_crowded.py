"""Passive population/combat accounting for the native crowded Quick Battle case."""
import math
from campaign_gameplay import creatures, read_rows, signed
from campaign_spell import read_spell_rows


def crowded_metrics(rows, spells, owner, wizard_slot):
    if not rows or len({r[2] for r in rows + spells}) != 1:
        raise ValueError('Missing same-thread crowded observation')
    state = {}
    groups = []
    for row in rows:
        if row[1] != 1:
            continue
        if not groups or groups[-1][0] != row[3]:
            groups.append((row[3], []))
        groups[-1][1].append(row)
    peak = local_peak = 0
    dense_ms = observed_ms = 0
    previous = None
    summon_slots = set()
    initial_slots = set(r[4] for r in groups[0][1])
    for tick, batch in groups:
        state.update(creatures(batch))
        wizard = state.get(wizard_slot)
        if not wizard or wizard['type'] != 0 or wizard['owner'] != owner or not wizard['active'] or wizard['health'] <= 0:
            raise ValueError('Crowded human wizard died or changed identity')
        actors = [v for v in state.values() if v['active'] and v['health'] > 0 and v['owner'] != 0xffffffff]
        owners = {v['owner'] for v in actors}
        local = sum(math.hypot(v['x']-wizard['x'], v['y']-wizard['y']) <= 12 for v in actors)
        dense = len(actors) >= 12 and len(owners) >= 2 and local >= 6
        peak = max(peak, len(actors)); local_peak = max(local_peak, local)
        summon_slots.update(v['slot'] for v in actors if v['slot'] not in initial_slots and v['owner'] == owner and v['type'] == 14)
        if previous is not None:
            delta = (tick-previous[0]) & 0xffffffff
            if delta > 1500:
                raise ValueError('Crowded observation stalled')
            observed_ms += delta
            if dense and previous[1]:
                dense_ms += delta
        previous = tick, dense
    casts = [r for r in spells if r[1] == 1 and r[4] == 14 and r[5] == wizard_slot and r[6] == 0 and r[7] == owner and r[15] == 1 and signed(r[8]) > signed(r[9]) >= 0]
    hits = [r for r in rows if r[1] == 2 and r[6] == owner and r[10] not in (owner, 0xffffffff) and r[13] and signed(r[7]) > 0 and signed(r[11]) > 0 and signed(r[12]) < signed(r[11])]
    if len(summon_slots) < 6 or len(casts) < 6:
        raise ValueError('Fewer than six observed player summons and mana-debited casts')
    if dense_ms < 30000:
        raise ValueError('Crowded battle did not sustain twelve actors/two owners/six nearby for thirty seconds')
    if not hits:
        raise ValueError('Crowded battle lacks original player-versus-enemy melee damage')
    return dict(peak_actors=peak, peak_near_wizard=local_peak, crowded_seconds=dense_ms/1000,
                observed_seconds=observed_ms/1000, player_summon_slots=sorted(summon_slots),
                mana_debited_summons=len(casts), player_melee_damage_events=len(hits), first_player_melee=list(hits[0]))


def validate_crowded(experiment, inputs, flow, capture_root, seconds, min_fps=20):
    if not flow.get('crowded_mode') or not inputs.get('success') or inputs.get('seconds', 0) < seconds:
        raise ValueError('Incomplete crowded native journey')
    setup = [s for s in flow['steps'] if s['step'] == 'crowded-setup']
    if setup != [dict(step='crowded-setup', mana=200, health=800, control_limit=30)]:
        raise ValueError('Original crowded setup did not acknowledge ordinary configured rules')
    rows = read_rows(experiment/'gameplay-events.bin', complete=True)
    spells = read_spell_rows(experiment/'spell-events.bin', complete=True)
    metrics = crowded_metrics(rows, spells, inputs['owner'], inputs['wizard_slot'])
    if metrics['observed_seconds'] < seconds-2:
        raise ValueError('Missing full crowded gameplay observation interval')
    if sum(a['action'] == 'stress-camera-and-portrait' for a in inputs['actions']) < 10:
        raise ValueError('Missing sustained native portrait/camera input')
    metrics.update(verified=True, scope='Four-player map2 Quick Battle, native ordinary mana200/health800/control30 setup, at least six original player summons with actual mana debit, thirty cumulative sampled seconds with twelve living actors/two owners/six within twelve tiles of living human wizard, player melee damage and180..240 seconds observation. Spatial population is not a visible-pixel actor count; original simulation/drawing and full replacement excluded.')
    return metrics
