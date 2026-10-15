#!/usr/bin/env python3
"""Original Mini/Preferences bounded scalar dwell and resumed counter evidence."""
import argparse
import json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_semantic_state import decode,menu_interval,normalize_legacy_labels
ROOT=Path(__file__).resolve().parents[1]


def assess(observation,actions,quit_observation=None):
    expected=[(3,22),(18,23),(18,25),(17,14),(10,20),(17,15),(6,26),(3,3)]
    if [(a['screen'],a['action']) for a in actions]!=expected or any(a['status']!=1 for a in actions[:-1]):
        raise ValueError('Campaign action/ack sequence differs')
    last=actions[-1]
    if last['status']!=1:
        if last['status'] is not None or last.get('normal_exit') is not True or type(last.get('request')) is not int or last['request']<=0 or last.get('argument')!=0 or quit_observation is None or quit_observation!={'request':last['request'],'screen':3,'action':3,'ack_observed':False,'normal_exit':True,'exit_code':0}:
            raise ValueError('Unacknowledged Quit needs exact normal-exit observation')
    minis=menu_interval(observation,17,1000);preferences=menu_interval(observation,10,1000)
    if len(minis)<2 or len(preferences)!=1:raise ValueError('Missing bounded Mini/Preferences dwells')
    selected=[minis[0],preferences[0]]
    if any(not all(i['stable'].values()) for i in selected):raise ValueError('Selected original state advanced during menu dwell')
    if minis[0]['last_ordinal']>=preferences[0]['first_ordinal'] or preferences[0]['last_ordinal']>=minis[1]['first_ordinal']:
        raise ValueError('Menu dwell sequence differs')
    prior=preferences[0]['last'];resumed=[s for s in observation['samples'] if preferences[0]['last_ordinal']<s['ordinal']<minis[1]['first_ordinal'] and s['top_screen']==2 and s['validity']==63]
    advanced=[s for s in resumed if s['world_counter']>prior['world_counter'] and s['accepted_sequence']>prior['accepted_sequence']]
    if not advanced:raise ValueError('No observed World advancement after Preferences')
    return dict(success=True,mini_dwells=minis,preferences_dwell=preferences[0],resumed_samples=len(resumed),first_advancing_resume=advanced[0],selected_state_pause_observed=True,world_resume_observed=True,quit_ack_observed=last['status']==1,allanimation_pause_equivalence=False,selection_verified=False,tooltip_verified=False,complete_presented_journey=False)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    names=['tools/analyze-drawing-campaign-pause.py','tools/drawing_semantic_state.py','tests/test-drawing-campaign-pause.py']
    sources={n:sha((ROOT/n).read_bytes()) for n in names}
    raw=a.reference.read_bytes();r=json.loads(raw)
    if r.get('success') is not True or r.get('source_sha256')!=BUILD or r.get('normal_original_quit_verified') is not True or not r['staging'].get('campaign_pause'):
        raise ValueError('Complete original campaign reference required')
    data=(Path(r['experiment'])/'capture/semantic-state.bin').read_bytes();observed=decode(data)
    if observed!=normalize_legacy_labels(r['semantic_state_observation']):raise ValueError('Retained semantic journal differs')
    result=assess(observed,r['menu_actions'],r.get('quit_observation'))
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Pause analysis sources changed')
    a.report.write_text(json.dumps(dict(schema=1,success=True,sources=sources,sources_stable=True,reference=dict(path=str(a.reference),sha256=sha(raw)),journal_sha256=sha(data),analysis=result,original_game_executed=False,legacy_label_correction={'prior':'camera','current':'pacing_fields','addresses':[0x6dbe95,0x6dbe99],'raw_words_unchanged':True,'camera_stability_proved':False},scope='Current offline original Mini/Preferences bounded selected-state pause/resume analysis, not allanimation/timer/gameplay/native/presentation equivalence.'),indent=2)+'\n');print(json.dumps(dict(success=True,mini_ms=result['mini_dwells'][0]['duration_ms'],preferences_ms=result['preferences_dwell']['duration_ms'],resume=True)))


if __name__=='__main__':main()
