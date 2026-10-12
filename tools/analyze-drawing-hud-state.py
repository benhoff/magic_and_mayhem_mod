#!/usr/bin/env python3
"""Revalidate retained hit/camera samples; exact ticks bind only selected state."""
import argparse
from collections import Counter
import json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_hud_state import decode
from drawing_semantic_state import decode as decode_semantic,menu_interval
ROOT=Path(__file__).resolve().parents[1]


def assess(hud,semantic):
    if hud['owner_thread']!=semantic['owner_thread']:raise ValueError('Sampler owners differ')
    by_tick={s['tick']:s for s in semantic['samples']}
    if len(by_tick)!=len(semantic['samples']):raise ValueError('Ambiguous semantic ticks')
    pairs=[];excluded=0
    for h in hud['samples']:
        s=by_tick.get(h['tick'])
        if s is None:excluded+=1;continue
        if h['thread']!=s['thread'] or h['menu_screen']!=s['menu_screen']:raise ValueError('Exact-tick state identity differs')
        pairs.append((h,s))
    if not pairs:raise ValueError('No exact-tick sampler pairs')
    dwells=[]
    for screen in (17,10):
        for interval in menu_interval(semantic,screen,1000):
            rows=[h for h,s in pairs if interval['first_ordinal']<=s['ordinal']<=interval['last_ordinal'] and h['validity']&8]
            if len(rows)<2:continue
            elapsed=(rows[-1]['tick']-rows[0]['tick'])&0xffffffff
            if elapsed<1000:continue
            keys=('camera_orientation','camera_whole','camera_fraction')
            stable={k:all(h[k]==rows[0][k] for h in rows) for k in keys}
            dwells.append(dict(screen=screen,samples=len(rows),duration_ms=elapsed,stable=stable,first={k:rows[0][k] for k in keys},last={k:rows[-1][k] for k in keys}))
    world=[h for h,s in pairs if s['top_screen']==2 and s['validity']&4]
    hits=[h for h in world if h['validity']&4]
    camera=[h for h in world if h['validity']&8]
    return dict(success=True,exact_tick_pairs=len(pairs),unpaired_hud_samples=excluded,world_samples=len(world),candidate_states=dict(Counter(h['candidate_state'] for h in world)),validated_hit_samples=len(hits),hit_identities=[list(x) for x in sorted({(h['observed_pool_id'],h['actor_slot'],h['actor_type'],h['actor_owner']) for h in hits})],world_camera_states=len({(h['camera_orientation'],*h['camera_whole'],*h['camera_fraction']) for h in camera}),camera_menu_dwells=dwells,selected_camera_pause_observed=any(d['screen']==17 and all(d['stable'].values()) for d in dwells) and any(d['screen']==10 and all(d['stable'].values()) for d in dwells),selection_verified=False,tooltip_verified=False,input_to_state_causality_proved=False,allanimation_pause_equivalence=False,allocation_or_actor_generation_proved=False,complete_presented_journey=False)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    raw=a.reference.read_bytes();r=json.loads(raw)
    if r.get('success') is not True or r.get('source_sha256')!=BUILD or r.get('normal_original_quit_verified') is not True or not r['staging'].get('hud_state'):
        raise ValueError('Complete original HUD reference required')
    capture=Path(r['experiment'])/'capture';hud_raw=(capture/'hud-state.bin').read_bytes();semantic_raw=(capture/'semantic-state.bin').read_bytes()
    hud=decode(hud_raw);semantic=decode_semantic(semantic_raw)
    if hud!=r['hud_state_observation'] or semantic!=r['semantic_state_observation']:raise ValueError('Retained scalar journal differs')
    result=assess(hud,semantic);names=['tools/analyze-drawing-hud-state.py','tools/drawing_hud_state.py','tools/drawing_semantic_state.py']
    a.report.write_text(json.dumps(dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},reference=dict(path=str(a.reference),sha256=sha(raw)),journals={'hud':sha(hud_raw),'semantic':sha(semantic_raw)},analysis=result,original_game_executed=False,scope='Current immutable scalar-journal reanalysis with exact owner/tick joins; no semantic selection/tooltip/input causality or whole native presentation claim.'),indent=2)+'\n')
    print(json.dumps(result))


if __name__=='__main__':main()
