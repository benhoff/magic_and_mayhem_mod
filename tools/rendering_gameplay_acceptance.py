"""Bounded public-player state witnesses; periodic samples never invent frame identity."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

from rendering_pilot import require

ACTIONS = ('center-first', 'center-second', 'hover-creature', 'select-creature',
           'pointer-away-selected', 'hover-creature-again', 'deselect-creature',
           'pointer-away-deselected', 'hover-tooltip', 'close-tooltip',
           'rotate-1', 'rotate-2', 'rotate-3', 'rotate-4', 'pan-up', 'pause', 'resume')


def windows(rows):
    require(len(rows) == len(ACTIONS)*3, 'Incomplete bounded gameplay schedule')
    result = {}
    previous = 0
    for i, action in enumerate(ACTIONS):
        group = rows[i*3:i*3+3]
        require([r['phase'] for r in group] == ['begin', 'released', 'settled']
                and all(r['action'] == action and r['accepted'] is True for r in group),
                'Gameplay action/phase/refusal differs')
        stamps = [int(r['host_us']) for r in group]
        ticks = [int(r['original_tick']) for r in group]
        require(previous <= stamps[0] <= stamps[1] < stamps[2] and ticks == sorted(ticks)
                and ticks[-1] >= ticks[0] and all(int(r['world']) > 0 for r in group),
                'Gameplay clocks or presented World are missing/reordered')
        previous = stamps[-1]
        result[action] = dict(begin=group[0], released=group[1], settled=group[2])
    return result


def bind_markers(rows, admission):
    previous=0
    for marker in rows:
        ordinal=marker['original_row']
        require(type(ordinal) is int and previous<=ordinal<=len(admission['rows']) and ordinal>0,
                'Gameplay marker original row is missing/reordered')
        observed=admission['rows'][ordinal-1]
        require(observed['sequence']==ordinal and observed['tick']==int(marker['original_tick'])
                and observed['thread']==admission['owner_thread'],
                'Gameplay marker does not bind the exact original admission row')
        previous=ordinal
    return dict(verified=True,markers=len(rows),owner=admission['owner_thread'],
                scope='Exact retained original-admission row/tick identities. No interpolation, original input-poll or scalar/source-frame identity is inferred.')


def assess(rows, hud, semantic, latches, region, paints):
    w = windows(rows)
    owners = (hud['owner_thread'], semantic['owner_thread'], latches['owner'], region['owner'])
    require(owners[0] and len(set(owners)) == 1, 'Gameplay original owners differ')
    by_tick = {s['tick']: s for s in semantic['samples']}
    require(len(by_tick) == len(semantic['samples']), 'Ambiguous semantic tick')
    pairs = []
    for h in hud['samples']:
        s = by_tick.get(h['tick'])
        if s is not None:
            require(h['thread'] == s['thread'] and h['menu_screen'] == s['menu_screen'],
                    'Exact original sampler identity differs')
            if h['validity'] & 8 and s['validity'] & 4 and s['top_screen'] == 2:
                pairs.append((h, s))
    require(pairs, 'No exact-tick original World/camera pairs')
    def samples(action):
        start = int(w[action]['released']['original_tick'])
        end = int(w[action]['settled']['original_tick'])
        return [(h, s) for h, s in pairs if start < h['tick'] <= end]
    def planes(action):
        start, end = (int(w[action][p]['host_us']) for p in ('released', 'settled'))
        return [r for r in paints if start < int(r['host_us']) <= end and int(r['world']) > 0]
    facts = {}
    orientations = []
    for action in ('center-second', 'rotate-1', 'rotate-2', 'rotate-3', 'rotate-4'):
        selected = samples(action)
        # Three trailing samples check a settled value; no interpolation.
        states = [h['camera_orientation'] for h, s in selected[-3:]]
        orientations.append(states[-1] if len(states) == 3 and len(set(states)) == 1 else None)
    four = (None not in orientations and set(orientations[:4]) == set(range(4))
            and orientations[0] == orientations[4]
            and len({(b-a) % 4 for a,b in zip(orientations, orientations[1:])}) == 1
            and (orientations[1]-orientations[0]) % 4 in (1,3))
    facts['four_orientations'] = dict(verified=four, observed=orientations)
    before, after = samples('rotate-4'), samples('pan-up')
    pan = bool(before and after and before[-1][0]['camera_orientation'] == after[-1][0]['camera_orientation']
               and before[-1][0]['camera_whole'] != after[-1][0]['camera_whole'])
    facts['pan'] = dict(verified=pan, before=before[-1][0] if before else None,
                        after=after[-1][0] if after else None)
    intervals = {}
    for action in ('pan-up', 'pause', 'resume'):
        selected = samples(action)
        counters = [s['world_counter'] for h,s in selected]
        duration = selected[-1][0]['tick']-selected[0][0]['tick'] if selected else 0
        intervals[action] = dict(samples=len(selected), duration_ms=duration,
                                 counters=sorted(set(counters)), paints=len(planes(action)),
                                 generations=len({int(r['world']) for r in planes(action)}))
    # The live admission receipt is periodic and can precede key delivery.
    # Admit the actual original mode branch, retaining its active preamble.
    # One monotonic0->1->0 mode sequence is required; never trim counters until
    # they happen to stop, or treat absence of samples as a paused interval.
    pause_rows=samples('pause');resume_rows=samples('resume');active_rows=samples('pan-up')
    mode_sequence=[s['mode'] for h,s in active_rows+pause_rows+resume_rows]
    transitions=[(a,b) for a,b in zip(mode_sequence,mode_sequence[1:]) if a!=b]
    paused_rows=[(h,s) for h,s in pause_rows if s['mode']==1]
    resumed_rows=[(h,s) for h,s in resume_rows if s['mode']==0]
    paused_counter={s['world_counter'] for h,s in paused_rows}
    resumed_counter={s['world_counter'] for h,s in resumed_rows}
    paused_ms=paused_rows[-1][0]['tick']-paused_rows[0][0]['tick'] if paused_rows else 0
    pause = (transitions==[(0,1),(1,0)] and active_rows and all(s['mode']==0 for h,s in active_rows)
             and pause_rows and pause_rows[-1][1]['mode']==1
             and resume_rows and resume_rows[-1][1]['mode']==0
             and paused_ms>=1000 and len(paused_counter)==1 and len(resumed_counter)>=2
             and len(intervals['pan-up']['counters'])>=2
             and intervals['pause']['generations']>=2 and intervals['resume']['generations']>=2)
    facts['pause_resume'] = dict(verified=bool(pause), intervals=intervals,
        mode_transitions=[list(t) for t in transitions], paused_mode_duration_ms=paused_ms,
        paused_mode_samples=len(paused_rows),paused_mode_counters=sorted(paused_counter),
        resumed_mode_counters=sorted(resumed_counter),
        all_simulation_and_animation_pause_proved=False)
    toggles = []
    for action in ('select-creature', 'deselect-creature'):
        start = int(w[action]['begin']['original_tick']); end = int(w[action]['settled']['original_tick'])
        found = []
        for call in region['calls']:
            a,b = call['entry'],call['returned']
            if not start < a['tick'] <= b['tick'] <= end or call['mouse_record'][:2] != [400,265]:continue
            if (a['actor_state'] == b['actor_state'] == 1 and a['slot_plus_one'] == b['slot_plus_one'] == 1
                    and a['pool_id'] == b['pool_id'] and a['actor_type'] == b['actor_type'] == 0
                    and a['actor_owner'] == b['actor_owner'] == 0
                    and a['actor_active'] and b['actor_active']
                    and 0<a['actor_health']<2**31 and 0<b['actor_health']<2**31
                    and a['actor_flag_68'] in (0,1) and b['actor_flag_68'] == 1-a['actor_flag_68']):
                found.append(dict(call=call['call'],slot=0,pool_id=a['pool_id'],before=a['actor_flag_68'],after=b['actor_flag_68']))
        toggles.append(found[0] if len(found) == 1 else None)
    continuity = []
    for action,toggle in zip(('pointer-away-selected','pointer-away-deselected'),toggles):
        start=int(w[action]['released']['original_tick']);end=int(w[action]['settled']['original_tick'])
        selected=[s for s in latches['samples'] if start < s['tick'] <= end]
        okay=bool(toggle and len(selected)>=3 and all(s['flags']&1 and s['pool_id']==toggle['pool_id']
            and (0 in s['raw_68_slots'])==bool(toggle['after']) for s in selected))
        continuity.append(okay)
    selection=bool(all(toggles) and toggles[0]['after']==toggles[1]['before']
                   and toggles[1]['after']==toggles[0]['before'] and all(continuity))
    facts['wizard_latch_toggle'] = dict(verified=selection,toggles=toggles,pointer_away_continuity=continuity,
                                      actor_generation_proved=False,complete_selection_proved=False)
    facts['presented_action_windows'] = {a:dict(paints=len(planes(a)),
        generations=len({int(r['world']) for r in planes(a)})) for a in ACTIONS}
    output=all(v['paints'] > 0 for v in facts['presented_action_windows'].values())
    facts['presented_all_action_windows'] = output
    return dict(success=bool(four and pan and pause and selection and output),facts=facts,
        exact_tick_pairs=len(pairs),exact_scalar_to_source_frame_binding_proved=False,
        tooltip_text_and_restoration_proved=False,complete_camera_picking_hud_requirement_proved=False,
        scope='Finite original-state association within explicit action windows and actual presented output. All presented pixels require the separate continuous-history comparison. Periodic scalar samples do not establish exact source-generation identity, full picking, tooltip or selection recovery.')


def minimap(raw):
    require(len(raw)==(4+6*32*3)*4,'Minimap counter envelope')
    words=struct.unpack('<580I',raw)
    require(words[:4]==(0x50504e4d,2,len(raw),6),'Minimap counter version')
    names=('terrain','cell','creature','camera0','camera1','camera2','camera3')
    rows={}
    for i,name in enumerate(names):
        installed,native,forwarded=words[4+(3*32+i)*3:7+(3*32+i)*3]
        require(installed==1 and forwarded==0,'Minimap route installation/forwarding differs')
        rows[name]=native
    require(all(rows[n]>0 for n in names[:-1]) and rows['camera3']==0,
            'Missing selected four-case minimap routes or unexpected camera3')
    return dict(verified=True,native=rows,
                scope='Co-retained cumulative production counters: terrain, cell, creature and the three bodies selected by the original four-case camera table. camera3 has no branch in that table. No per-source-frame counter or whole minimap semantics claim.')


def main():
    from coverage_evidence import Reader, write_report
    from drawing_hud_state import decode as hud_decode
    from drawing_semantic_state import decode as semantic_decode
    from drawing_actor_latches import decode as latch_decode, bind_callbacks
    from drawing_region_input import decode as region_decode
    from campaign_input_observation import decode as admission_decode
    from rendering_pilot import acquisition
    root=Path(__file__).resolve().parents[1]
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--acquisition',required=True);p.add_argument('--comparison',required=True);p.add_argument('--report',required=True)
    a=p.parse_args()
    require(a.report.startswith('research/runtime/rendering-gameplay-results-20261105/') and not (root/a.report).exists(),'New owned gameplay report required')
    result=dict(schema=1,kind='public_player_gameplay_witness',success=False)
    with Reader(root) as reader:
        acquired=reader.json(a.acquisition);compared=reader.json(a.comparison)
        result['inputs']={k:dict(record=n,sha256=reader.digest(n)) for k,n in (('acquisition',a.acquisition),('comparison',a.comparison))}
        result['sources']={n:hashlib.sha256((root/n).read_bytes()).hexdigest() for n in ('tools/rendering_gameplay_acceptance.py','tools/drawing_hud_state.py','tools/drawing_semantic_state.py','tools/drawing_actor_latches.py','tools/drawing_region_input.py','tools/rendering_capture.py','tools/campaign_input_observation.py','tests/test-rendering-gameplay-acceptance.py')}
        try:
            require(acquired['success'] is True and compared['success'] is True
                    and compared['acquisition']==result['inputs']['acquisition']
                    and acquired['configuration'].get('gameplay_witness') is True
                    and acquired['scenario']=='normal-quick','Incomplete or mixed public gameplay execution')
            for source,digest in (acquired['sources']|compared['sources']).items():
                require(hashlib.sha256((root/source).read_bytes()).hexdigest()==digest,'Gameplay source drift: '+source)
            artifacts=acquired['artifacts']
            def raw(suffix):
                matches=[v for v in artifacts if v['path'].endswith('/'+suffix)]
                require(len(matches)==1,'Ambiguous gameplay journal: '+suffix)
                v=matches[0];body=reader.read_bytes(v['record'])
                require(hashlib.sha256(body).hexdigest()==v['sha256'],'Gameplay journal hash differs')
                return body
            hud=hud_decode(raw('hud-state.bin'));semantic=semantic_decode(raw('semantic-state.bin'))
            latches=latch_decode(raw('actor-latches.bin'));region=region_decode(raw('input-admission-region.bin'))
            result['callback_binding']=bind_callbacks(latches,region)
            admission=admission_decode(raw('input-admission.bin'))
            require(admission['wire_version']==3 and admission['owner_thread']==hud['owner_thread'],'Gameplay admission owner/version differs')
            result['marker_binding']=bind_markers(acquired['flow']['gameplay_witness'],admission)
            streams=[reader.read_bytes(v['record']) for v in artifacts if '/capture/' in v['path'] and v['path'].endswith('.bin')]
            observed=acquisition(streams,acquired['session_identity'])
            paints=[observed['paints'][i][0] for i in observed['swaps']]
            result.update(assess(acquired['flow']['gameplay_witness'],hud,semantic,latches,region,paints))
            result['pixel_comparison']=compared['comparison']
            result['minimap_routes']=minimap(raw('native-pixel-producers.bin'))
        except (ValueError,KeyError,OSError) as error:result.update(success=False,error=str(error))
        result['sources_stable']=all(hashlib.sha256((root/n).read_bytes()).hexdigest()==h for n,h in result['sources'].items())
        result['success'] &= result['sources_stable']
    sha=write_report(root,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],error=result.get('error'),record=a.report,sha256=sha,facts=result.get('facts'))))
    return 0 if result['success'] else 1

if __name__=='__main__':raise SystemExit(main())
