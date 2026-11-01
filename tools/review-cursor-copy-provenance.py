#!/usr/bin/env python3
"""Join a stopped player acquisition to complete cursor/COPY state observations."""
import argparse
from collections import Counter
import json
from pathlib import Path
import sys
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from cursor_copy_state import validate
from natural_gpu_copy_branches import analyze
from rendering_operations_history import sha

ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ['acquisition','controls','report']:p.add_argument('--'+name,required=True)
    a=p.parse_args()
    if (ROOT/a.report).exists():p.error('New immutable report required')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior=next(b for b in register['behaviors'] if b['id']=='NP.cursor-copy-state-observation')
    scenario=next(s for s in register['scenarios'] if s['id']=='SC.cursor-copy-state-observation')
    result=dict(schema=1,success=False,sources={},inputs={},
                claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{}),
                             scenarios={scenario['id']:scenario_contract(scenario)})],
                original_pixels_used_as_native_inputs=False,player_accepted=False,
                scope='Source-bound current diagnostic player observations and complete companion/census joins. Any actual call failure remains a failure. Published state is distinct from the actual earlier hooked input return; original-driver equivalence and acceptance remain pending.')
    try:
        with Reader(ROOT) as reader:
            def read(name):
                result['inputs'][name]=reader.digest(name);return reader.json(name)
            acquired=read(a.acquisition);controls=read(a.controls)
            if not controls['success'] or not controls['sources_stable']:
                raise ValueError('Current controls required')
            if acquired.get('private_wine_stopped') is not True or acquired.get('sources_stable') is not True:
                raise ValueError('Stopped source-stable original process required')
            if acquired.get('configuration',{}).get('operations_provenance') != dict(copy_scope='all',cursor_state_version=1,qt_cursor_exercise=True):
                raise ValueError('Different acquisition policy')
            sources=dict(controls['sources']);sources.update(acquired['sources'])
            sources.update(acquired['native_admission']['native_only']['sources'])
            for name in ['tools/review-cursor-copy-provenance.py','tools/cursor_copy_state.py',
                         'tools/native_copy_routes.py','tools/native_copy_output.py',
                         'tools/natural_gpu_copy_branches.py','tools/native_gpu_copy_diagnostics.py',
                         'tools/rendering_operations_history.py','tools/coverage_claims.py']:
                sources[name]=sha((ROOT/name).read_bytes())
            result['sources']=sources
            if not required_sources(behavior)<=set(sources) or not set(scenario['tests'])<=set(sources):
                raise ValueError('Observation source/test closure incomplete')
            artifacts={r['path']:r for r in acquired['artifacts']}
            session=Path(acquired['session']);directory=Path(acquired['temporary_root'])/'copy-diagnostics'
            if not session.resolve().is_relative_to(ROOT/'working') or not directory.resolve().is_relative_to(ROOT/'working'):
                raise ValueError('Foreign acquisition outputs')
            selected=[session/'lock-capture'/n for n in ['copy-routes-v2.bin','gpu-copy-census-v1.bin','cursor-copy-state-v1.bin']]
            selected += list(directory.rglob('*'))
            verified=[]
            for path in selected:
                if not path.is_file() or path.is_symlink():
                    if path.is_dir():continue
                    raise ValueError('Missing physical retained interchange: '+str(path))
                name=path.relative_to(ROOT).as_posix();row=artifacts.get(name)
                if row is None or sha(path.read_bytes())!=row['sha256'] or reader.digest(row['record'])!=row['sha256']:
                    raise ValueError('Unbound retained interchange: '+name)
                verified.append(dict(record=row['record'],sha256=row['sha256']))
            companion=validate(session)
            result['companion']={k:v for k,v in companion.items() if k!='observations'}
            rows=companion['observations']
            result['companion'].update(examples=rows[:8],failure_examples=[r for r in rows if r['failed']][:32],
                                       window_client_domains=dict(Counter(str((r['source_extent'],r['target_extent'],r['wire_extent'],r['client_rectangle'])) for r in rows)),
                                       request_saved_matches=dict(source=sum(r['request_source_equals_saved'] for r in rows),target=sum(r['request_target_equals_saved'] for r in rows)))
            result['verified_interchange']=verified
            result['copy_output']=analyze(session,directory)
            result['qt_cursor_probe']=acquired.get('flow',{}).get('cursor_probe',[])
            result['input_exercise_verified']=len(result['qt_cursor_probe'])==14 and all(r['accepted'] is True and r['mapped'] is True for r in result['qt_cursor_probe'])
            result['acquisition_success']=acquired['success']
            result['normal_quit_and_retirement']=acquired.get('retirement',{}).get('state')=='completed'
            result['observation_verified']=companion['verified'] and companion['selected_calls']>0
            result['sources_stable']=all(sha((ROOT/n).read_bytes())==h for n,h in sources.items())
            result['success']=result['observation_verified'] and result['sources_stable'] and result['input_exercise_verified']
    except Exception as error:result['error']=str(error)
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ['success','error','observation_verified','input_exercise_verified','normal_quit_and_retirement']}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
