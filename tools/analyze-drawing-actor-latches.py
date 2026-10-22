#!/usr/bin/env python3
"""Recheck exact callback/cursor joins and bounded hover-independent raw latches."""
import argparse
import json
from pathlib import Path
from coverage_evidence import read_bytes, read_json, write_report, evidence_digest
from drawing_api_session import BUILD, sha
from drawing_actor_latches import decode, bind_callbacks
from drawing_hud_input_binding import bind as bind_cursor

ROOT = Path(__file__).resolve().parents[1]


def analyze(latches, region, api):
    if [s['ordinal'] for s in latches['samples']]!=list(range(1,len(latches['samples'])+1)):
        raise ValueError('Latch sample order/identity differs')
    callbacks = bind_callbacks(latches, region)
    samples = []
    for sample in latches['samples']:
        s = dict(sample)
        point = dict(s['input_point']) if s['input_point'] is not None else None
        if point is not None:
            age = (s['tick'] - point['cursor_tick']) % 2**32
            point['cursor_age_ms'] = age
            point['timely_cursor'] = point['current_cursor_admissible'] and age <= 100
        s.update(input_point=point, candidate_state=s['hit_state'], actor_slot=s['hit_slot'], actor_type=None, actor_owner=None)
        samples.append(s)
    cursor_links = bind_cursor(dict(samples=samples), api)
    by_ordinal = {s['ordinal']: s for s in samples}
    timely = {x['hud_ordinal']: x for x in cursor_links['links'] if x['timely_cursor'] and x['journal_active']}
    by_phase = {(s['call'], s['kind']): s for s in samples if s['kind']}
    observations = []
    for call in region['calls']:
        entry, returned = call['entry'], call['returned']
        if not call['mask'] & 1 or entry['actor_state'] != 1 or returned['actor_state'] != 1:
            continue
        if entry['actor_flag_68'] != 0 or returned['actor_flag_68'] != 1 or entry['slot_plus_one'] != returned['slot_plus_one'] or entry['pool_id'] != returned['pool_id'] or any(entry[k]!=returned[k] for k in ('capacity','actor_type','actor_owner')):
            continue
        slot = returned['slot_plus_one'] - 1
        before, after = by_phase[(call['call'], 1)], by_phase[(call['call'], 2)]
        if not before['flags'] & 1 or not after['flags'] & 1 or before['pool_id'] != after['pool_id'] or before['capacity']!=after['capacity'] or slot in before['raw_68_slots'] or slot not in after['raw_68_slots']:
            continue
        if before['raw_count'] != entry['raw_latch_count'] or after['raw_count'] != returned['raw_latch_count']:
            continue
        actor = after['first_latched_actor']
        if actor is None or actor['slot'] != slot or (actor['actor_type'],actor['owner'])!=(returned['actor_type'],returned['actor_owner']):
            continue
        persistent, away = [], []
        for s in samples[after['ordinal'] :]:
            # Stop at the first observed pool/flag change or invalid sample.
            # Never bridge a refusal to infer continuity.
            if not s['flags'] & 1 or s['pool_id'] != after['pool_id'] or s['capacity']!=after['capacity'] or s['raw_mode']!=after['raw_mode'] or slot not in s['raw_68_slots']:
                break
            observed_actor = s['first_latched_actor']
            if observed_actor is None or any(observed_actor[k] != actor[k] for k in ('slot', 'actor_type', 'owner', 'active','raw_68','raw_70')):
                break
            persistent.append(s['ordinal'])
            link = timely.get(s['ordinal'])
            if link and link['cursor'] != call['mouse_record'][:2] and s['hit_state'] in (0, 1) and s['hit_slot'] != slot:
                away.append(dict(latch_ordinal=s['ordinal'], tick=s['tick'], cursor=link['cursor'], api_record=link['api_record'], hit_slot=s['hit_slot'], raw_68_slots=s['raw_68_slots']))
        observations.append(dict(call=call['call'], slot=slot, observed_pool_id=after['pool_id'], actor=actor, before_latch_ordinal=before['ordinal'], after_latch_ordinal=after['ordinal'], raw_count_before=before['raw_count'], raw_count_after=after['raw_count'], subsequent_equal_identity_latch_samples=persistent, hover_away_witnesses=away))
    return dict(success=True, callback_binding=callbacks, cursor_binding=cursor_links, observations=observations, bounded_hover_independent_latch_observed=any(o['hover_away_witnesses'] for o in observations), continuous_unsampled_state_proved=False, actor_generation_proved=False, complete_selection_proved=False, tooltip_proved=False, source_frame_parity_proved=False)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--claims', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    if a.report.exists(): p.error('Refusing evidence overwrite')
    declaration = json.loads(a.claims.read_text())
    if any(evidence_digest(ROOT/name,root=ROOT) != digest for name, digest in declaration['sources'].items()):
        raise ValueError('Prospective analysis sources changed')
    raw = read_bytes(a.reference, root=ROOT); reference = json.loads(raw)
    if reference.get('success') is not True or reference.get('source_sha256') != BUILD or not reference.get('normal_original_quit_verified') or not reference['staging'].get('actor_latches'):
        raise ValueError('Complete original latch reference required')
    capture = Path(reference['experiment']) / 'capture'
    latch_raw = (capture / 'actor-latches.bin').read_bytes()
    api = (capture / 'session.bin').read_bytes()
    from drawing_region_input import decode as decode_region
    region_raw = (capture / 'region-input.bin').read_bytes()
    latches, region = decode(latch_raw), decode_region(region_raw)
    if latches != reference['actor_latch_observation'] or region != reference['region_input_observation'] or sha(latch_raw) != reference['actor_latch_sha256'] or sha(api) != reference['observation']['journal_sha256'] or sha(region_raw) != reference['region_input_sha256']:
        raise ValueError('Pinned original reference journal differs')
    if (capture / 'session.done').read_bytes() != api[-128:]:
        raise ValueError('Original API terminal differs')
    result = analyze(latches, region, api)
    if any(evidence_digest(ROOT/name,root=ROOT) != digest for name, digest in declaration['sources'].items()):
        raise ValueError('Analysis sources changed during execution')
    report = dict(schema=1, success=True, sources=declaration['sources'], sources_stable=True, claims=declaration['claims'], reference=dict(path=str(a.reference), sha256=sha(raw)), journal_hashes=dict(latches=sha(latch_raw), region=sha(region_raw), api=sha(api)), analysis=result, original_game_executed=False, scope='Fresh offline exact callback/cursor joins and bounded sampled hover-independent raw latch persistence over an immutable original run. No continuous unsampled state, allocation/generation, full selection/tooltip, source/physical/native parity or new original execution claim.')
    write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=True, selecting_callbacks=len(result['observations']), hover_independent_latch_observed=result['bounded_hover_independent_latch_observed'])))


if __name__ == '__main__': main()
