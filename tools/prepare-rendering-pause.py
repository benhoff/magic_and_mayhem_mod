#!/usr/bin/env python3
"""Pin unchanged executables and a new finite external Mini-pause contract."""
import argparse,hashlib,json
from pathlib import Path
from coverage_claims import behavior_contract,required_sources,scenario_contract
from coverage_evidence import Reader,write_report
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-pause-results-20261107/'
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default=PREFIX+'input-plan-01.json');p.add_argument('--repair-reason');p.add_argument('--sampling-repair-review');p.add_argument('--remaining-read-repair');p.add_argument('--hermes-ingress-review');p.add_argument('--low-source-review');p.add_argument('--current-player-plan');p.add_argument('--command-cap-review');p.add_argument('--escape-focus-review');a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh pause plan required')
    with Reader(ROOT) as r:
        parent='research/runtime/rendering-interaction-results-20261107/input-plan-07.json';plan=r.json(parent)
        if a.current_player_plan:
            current=r.json(a.current_player_plan)
            require(current['success'] and a.low_source_review,'Current player phase requires explicit bounded review')
            for key in ('compiler_sources','runtime_sources','player_binary'):plan[key]=current[key]
            plan['current_player_phase']=dict(record=a.current_player_plan,sha256=r.digest(a.current_player_plan),historical_oracle_parent=parent)
        require(all(r.digest(n)==h for section in ('compiler_sources','runtime_sources') for n,h in plan[section].items()),'Recorded native source phase differs')
        require(all(r.digest(n)==h for n,h in plan['oracle']['native_sources'].items()),'Recorded original oracle source differs')
        for binary in [plan['player_binary'],*plan['oracle']['binaries']]:
            require(hashlib.sha256((ROOT/binary['path']).read_bytes()).hexdigest()==r.digest(binary['record'])==binary['sha256'],'Recorded executable differs')
        reg=r.json('research/runtime/coverage/register.json');b=next(v for v in reg['behaviors'] if v['id']=='NP.pause-mini-acceptance');s=next(v for v in reg['scenarios'] if v['id']=='native.pause-mini-acceptance')
        sources={n:r.digest(n) for n in required_sources(b)|{'tools/prepare-rendering-pause.py'}}
        plan.update(kind='finite_public_campaign_mini_pause_plan',parent=dict(record=parent,sha256=r.digest(parent)),sources=sources,
            bounds=dict(maximum_requests=6,maximum_original_games=5,primary_games=4,repairs=1,stop_on_first_failure=True,successful_pilot_comparison_before_expansion=True),
            domains=[dict(software=v,size=z) for v in (False,True) for z in ([800,600],[640,480])],
            input_policy=dict(entry='New Game/Adept/Enter',dwells_per_case=2,dwell_seconds=3,minimum_samples=30,minimum_dwell_ns=2000000000,
                controls='Physical Escape, blocked gameplay Up in Mini, native Cancel then native Escape resume, positive gameplay Up after each resume, normal Campaign/Main Quit',
                media='Movie owner idle/acknowledged; music remains ready/live without asserting playback freeze',memory_writes=False,compiled_test_driver=False),
            claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,{v['id']:v for v in reg['builds']}),scenarios={s['id']:scenario_contract(s)})],
            scope=b['scope'],repair_reason=a.repair_reason,original_game_runs=0,new_native_build=False)
        if a.sampling_repair_review:
            review=r.json(a.sampling_repair_review)
            require(review['success'] and review['kind']=='bounded_pause_sampling_control_repair' and not review['new_acceptance_credit'] and review['original_game_runs']==0,'Invalid sampling-control repair review')
            require(r.digest(review['failed_acquisition']['record'])==review['failed_acquisition']['sha256'] and r.json(review['failed_acquisition']['record'])['error']=='Insufficient actual Mini dwell','Missing exact sample-density failure')
            plan['bounds'].update(review['reviewed_bounds'],repairs=2,final_control_repair=True)
            plan['input_policy'].update(minimum_target_samples=40,maximum_dwell_seconds=8,sqlite_commit_batch=10)
            plan['sampling_control_repair_review']=dict(record=a.sampling_repair_review,sha256=r.digest(a.sampling_repair_review))
        if a.remaining_read_repair:
            review=r.json(a.remaining_read_repair)
            require(review['success'] and review['kind']=='bounded_pause_remaining_read_repair' and not review['new_acceptance_credit'],'Invalid remaining-read repair')
            require(r.digest(review['failed_acquisition']['record'])==review['failed_acquisition']['sha256'] and r.json(review['failed_acquisition']['record'])['error']=="'NoneType' object is not subscriptable",'Missing exact remaining-read failure')
            plan['bounds'].update(review['reviewed_bounds'],repairs=3)
            plan['remaining_read_control_repair']=dict(record=a.remaining_read_repair,sha256=r.digest(a.remaining_read_repair))
        if a.hermes_ingress_review:
            review=r.json(a.hermes_ingress_review)
            require(review['success'] and review['kind']=='bounded_pause_hermes_ingress_review' and not review['new_acceptance_credit'],'Invalid Hermes ingress review')
            require(r.digest(review['failed_acquisition']['record'])==review['failed_acquisition']['sha256'] and r.json(review['failed_acquisition']['record'])['error'] in ('Original ready menu deadline: (17, 6)','Bounded Hermes/Mini admission exhausted4qualified controls'),'Missing exact Hermes admission failure')
            plan['bounds'].update(review['reviewed_bounds'],repairs=review.get('repairs',4))
            plan['input_policy']['hermes_ingress']=dict(policy='Existing title-qualified Hermes dismissal only; separate physical Escape; original ready/owner/ACK required independently',maximum_controls=review.get('maximum_ingress_controls',4),maximum_escapes=review.get('maximum_escape_controls',4),maximum_seconds=review.get('maximum_ingress_seconds',120))
            plan['hermes_ingress_review']=dict(record=a.hermes_ingress_review,sha256=r.digest(a.hermes_ingress_review))
        if a.low_source_review:
            review=r.json(a.low_source_review)
            require(review['success'] and review['kind']=='bounded_pause_low_source_and_master_phase_review' and not review['new_acceptance_credit'] and review['original_game_runs']==0,'Invalid low-source/current-player review')
            require(r.digest(review['failed_acquisition']['record'])==review['failed_acquisition']['sha256'] and r.json(review['failed_acquisition']['record'])['error']=='Mini ingress exceeded4physical Escape controls','Missing exact low-source failure')
            require(a.command_cap_review or plan['current_player_phase']['sha256']==review['current_player_plan']['sha256'],'Wrong current native phase')
            plan['bounds'].update(review['reviewed_bounds'],repairs=6,historical_successful_original_games=1,current_primary_games=4)
            plan['low_source_and_native_phase_review']=dict(record=a.low_source_review,sha256=r.digest(a.low_source_review))
        if a.command_cap_review:
            review=r.json(a.command_cap_review)
            require(review['success'] and review['kind']=='bounded_pause_command_cap_control_review' and not review['new_acceptance_credit'] and review['original_game_runs']==0,'Invalid command-cap review')
            require(r.digest(review['failed_comparison']['record'])==review['failed_comparison']['sha256'] and r.json(review['failed_comparison']['record'])['error']=='Capture incomplete or truncated','Missing exact command cap refusal')
            require(plan['current_player_phase']['sha256']==review['current_player_plan']['sha256'],'Wrong version4 player phase')
            plan['bounds'].update(review['reviewed_bounds'],repairs=7,historical_successful_original_games=2,current_primary_games=4)
            plan['input_policy']['command_capture']=dict(version=4,policy='command-history-lossless-v1',maximum_rows=65536,maximum_raw_bytes=8*1024**3,maximum_stored_bytes=1024**3,queue_bytes=256*1024**2,queue_rows=4096)
            plan['command_cap_control_review']=dict(record=a.command_cap_review,sha256=r.digest(a.command_cap_review))
        if a.escape_focus_review:
            review=r.json(a.escape_focus_review)
            require(review['success'] and review['kind']=='bounded_pause_escape_focus_review' and not review['new_acceptance_credit'] and review['original_game_runs']==0,'Invalid Escape focus review')
            require(r.digest(review['failed_acquisition']['record'])==review['failed_acquisition']['sha256'] and r.json(review['failed_acquisition']['record'])['error']=='New native source/presentation progress deadline','Missing exact unacknowledged Escape failure')
            require(plan['current_player_phase']['sha256']==review['current_player_plan']['sha256'],'Escape review changed native player')
            plan['bounds'].update(review['reviewed_bounds'],repairs=8)
            plan['input_policy']['escape_admission']='Actual public Cancel focused/enabled/showing and X11 host focus; released/down/up keymap acknowledgements; original Cancel ACK before fresh rendering progress'
            plan['escape_focus_control_review']=dict(record=a.escape_focus_review,sha256=r.digest(a.escape_focus_review))
        print(write_report(ROOT,a.report,(json.dumps(plan,indent=2)+'\n').encode()))
if __name__=='__main__':main()
