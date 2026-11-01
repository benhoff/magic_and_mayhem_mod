#!/usr/bin/env python3
"""Close the bounded provenance batch without admitting player acceptance."""
from collections import Counter
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
from coverage_claims import behavior_contract,scenario_contract,required_sources
from coverage_evidence import Reader,write_report
from rendering_operations_history import sha
from resident_capture import records

ROOT=Path(__file__).resolve().parents[1]
RESULTS='research/runtime/rendering-provenance-results-20261101/'
NORMAL='research/runtime/rendering-normal-results-20261031/'
BEHAVIORS=['NP.cursor-copy-state-observation','TOOL.cursor-invalid-copy-providers','TOOL.rendering-provenance-comparison']


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    if (ROOT/a.report).exists():p.error('New immutable report required')
    registered=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    behavior={b['id']:b for b in registered['behaviors'] if b['id'] in BEHAVIORS}
    scenario=next(s for s in registered['scenarios'] if s['id']=='SC.cursor-copy-state-observation')
    claims=[dict(behavior=bid,contract_sha256=behavior_contract(behavior[bid],{}),
                 scenarios={scenario['id']:scenario_contract(scenario)} if bid.startswith('NP.') else {}) for bid in BEHAVIORS]
    result=dict(schema=1,success=False,sources={},inputs={},claims=claims,cases={},controls=[],
                player_accepted=False,complete_replacement=False,
                scope='Verified bounded investigation: current cursor observations, supplied provider contexts, policy regressions and independent diagnostic comparisons, including an explicitly failed high-resolution pixel comparison. Review success is not rendering equivalence. No nominal acceptance, old coordinate reconstruction, missing prior writer or whole original-driver claim.')
    try:
        with Reader(ROOT) as reader:
            def read(name,positive=True):
                result['inputs'][name]=reader.digest(name);value=reader.json(name)
                if positive:
                    for path,digest in value.get('sources',{}).items():
                        if path in result['sources'] and result['sources'][path]!=digest:raise ValueError('Positive source inventories differ: '+path)
                        result['sources'][path]=digest
                return value
            original=read(RESULTS+'history-dependency03.json')
            if not original['success'] or not original['source_changes_target'] or original['source_value_changes_before_selected_record']:
                raise ValueError('Original-only source dependency proof differs')
            result['history_dependency']=dict(verified=True,target=[548,433],source=[563,433],entry_value=6338,
                                              source_survives_retained_prefix=True,prior_writer_proved=False,
                                              native_pixels_from_original_inputs=False)
            provider=read(RESULTS+'invalid-copy-providers01.json')
            if not provider['success'] or not provider['contexts_match']:raise ValueError('Provider contexts differ')
            result['provider_comparison']={k:provider[k] for k in ['contexts_match','wine_version','native_provider_sha256']}
            result['provider_comparison']['words_checked_per_provider']={k:v['words_checked'] for k,v in provider['providers'].items()}
            controls=read(RESULTS+'cursor-controls02.json')
            if not controls['success']:raise ValueError('Cursor controls failed')
            inspector_spec=importlib.util.spec_from_file_location('provenance_producers',ROOT/'tools/inspect-canvas-producers.py')
            inspector=importlib.util.module_from_spec(inspector_spec);inspector_spec.loader.exec_module(inspector)
            for label,acquisition,joined,comparison in [
                    ('high',NORMAL+'operations-provenance-high03.json',RESULTS+'cursor-high-joined03.json',NORMAL+'operations-provenance-high-comparison04.json'),
                    ('low',NORMAL+'operations-provenance-low03.json',RESULTS+'cursor-low-joined03.json',NORMAL+'operations-provenance-low-comparison03.json')]:
                acquired=read(acquisition);observed=read(joined);compared=read(comparison)
                if (not acquired['success'] or not observed['success'] or compared.get('error')
                        or not observed['normal_quit_and_retirement'] or not compared['input_acknowledgements']['verified']
                        or not compared['fullscreen']['verified']):raise ValueError('Incomplete final diagnostic: '+label)
                pixels=compared['comparison']
                if compared['success'] != pixels['success'] or pixels['success'] != (pixels['mismatching_pixels']==0):
                    raise ValueError('Contradictory pixel outcome: '+label)
                failed=[r for r in pixels['comparisons'] if r['mismatching_pixels']]
                earlier=[r for r in pixels['comparisons'] if not failed or r['paint']<failed[0]['paint']]
                if acquired['source_commit']!='a4d3a6ee3394536977dab1cf12892d105da93308':raise ValueError('Wrong combined committed base')
                source=Path(acquired['temporary_root'])/'worlds.bin'
                artifact=next(r for r in acquired['artifacts'] if r['path']==source.relative_to(ROOT).as_posix())
                if sha(source.read_bytes())!=artifact['sha256'] or reader.digest(artifact['record'])!=artifact['sha256']:
                    raise ValueError('World profile identity differs')
                operations=Counter();worlds=chunks=0;direct_worlds=[]
                for generation,chunk,stream,request in records(source):
                    chunks+=bool(chunk);worlds+=not bool(chunk)
                    _,rows=inspector.decode(stream,complete=False)
                    current=Counter((r[14],r[15]) for r,payload in rows if r[2]==9)
                    operations.update(current)
                    if current[(3,9)]:direct_worlds.append(dict(world=generation,calls=current[(3,9)]))
                closure=observed['copy_output']['whole_output_closure']
                if not observed['copy_output']['all_application_output_closed'] or observed['companion']['failed_calls']:
                    raise ValueError('Current cursor/COPY refusal remains')
                result['cases'][label]=dict(source_commit=acquired['source_commit'],configuration=acquired['configuration'],
                                             world_domain=compared['world_domain'],cursor_calls=observed['companion']['selected_calls'],
                                             cursor_failures=observed['companion']['failed_calls'],qt_cursor_probes=len(observed['qt_cursor_probe']),
                                             copy_commands=closure['caller_bound_commands'],copy_calls=closure['cpu_copy_calls']+closure['gpu_copy_calls'],
                                             retained_copy_words=observed['copy_output']['retained_python_pixels'],
                                             world_swaps=compared['observations']['world_swaps'],
                                             original_reference=compared['original_reference'],pixels_compared=compared['comparison']['pixels_compared'],
                                             pixel_equivalence_passed=pixels['success'],mismatching_pixels=pixels['mismatching_pixels'],
                                             failing_presentations=failed,
                                             last_observed_world_before_divergence=max((r['world'] for r in earlier),default=0),
                                             original_input_ack_verified=True,fullscreen_verified=True,normal_quit_and_retirement=True,
                                             producer_worlds=worlds,producer_source_chunks=chunks,
                                             raster_operations=[dict(operation=op,backend=backend,calls=count) for (op,backend),count in sorted(operations.items())],
                                             selected_direct_distortion_calls=operations[(3,9)],direct_distortion_worlds=direct_worlds,
                                             complete_owned_prefix=dict(record=artifact['record'],sha256=artifact['sha256']))
            high=result['cases']['high'];low=result['cases']['low']
            if high['pixel_equivalence_passed'] or not low['pixel_equivalence_passed']:
                raise ValueError('The retained high-failure/low-success regression pair changed')
            first=high['failing_presentations'][0]
            result['current_failure']=dict(verified=True,comparison=NORMAL+'operations-provenance-high-comparison04.json',
                first_presentation=first,last_observed_world_before_divergence=high['last_observed_world_before_divergence'],
                native_original_cause_recovered=False,
                source_history_or_raster_cause='Unresolved; backend9 reachability alone does not identify the failing draw.',
                next_step='Replay the complete owned prefix through World50 and compare intermediate raster checkpoints against the independent original oracle; isolate the first divergent draw/read before changing initialization or displacement semantics.')
            if first['world']!=50 or high['last_observed_world_before_divergence']!=49 or not high['selected_direct_distortion_calls']:
                raise ValueError('First-divergence or direct-distortion reachability binding differs')
            mixed=read(NORMAL+'operations-provenance-high-comparison02.json',False)
            corrected=read(NORMAL+'operations-provenance-high-comparison03.json',False)
            result['contradictory_comparison_quarantined']=(mixed['success'] is True and bool(mixed.get('error'))
                                                          and corrected['success'] is False and corrected.get('error')==mixed.get('error'))
            if not result['contradictory_comparison_quarantined']:raise ValueError('Late-failure evidence not preserved')
            result['historical_failure']=dict(record=NORMAL+'operations-provenance-high-comparison02.json',
                                             rejected=True,reason=mixed['error'],corrected_record=NORMAL+'operations-provenance-high-comparison03.json')
            history=read(RESULTS+'committed-history02.json',False)
            if not history['success'] or history['unresolved']:raise ValueError('Newly committed range has unresolved exact receipts')
            result['reviewed_history']=history['reviewed_range']
        for name in ['tests/test-cursor-copy-state.py','tests/test-rendering-provenance-policy.py',
                     'tests/test-rendering-normal-acceptance.py','tests/test-rendering-pilot.py']:
            source=ROOT/name;result['sources'][name]=sha(source.read_bytes())
            run=subprocess.run([sys.executable,'-B',source],cwd=ROOT,capture_output=True,text=True,timeout=60)
            result['controls'].append(dict(path=name,returncode=run.returncode,output=(run.stdout+run.stderr)[-16000:]))
            if run.returncode:raise ValueError('Current policy/reader control failed: '+name)
        for name in ['tools/review-rendering-provenance.py','tools/resident_capture.py','tools/inspect-canvas-producers.py',
                     'tools/rendering_provenance_policy.py','tools/coverage_claims.py']:
            result['sources'][name]=sha((ROOT/name).read_bytes())
        if any(not required_sources(b)<=set(result['sources']) for b in behavior.values()):raise ValueError('Final behavior source closure incomplete')
        result['sources_stable']=all(sha((ROOT/n).read_bytes())==digest for n,digest in result['sources'].items())
        result['current_cursor_observations_verified']=all(c['cursor_calls']>0 and not c['cursor_failures'] for c in result['cases'].values())
        result['input_fullscreen_retirement_guards_passed']=True
        result['default_comparison_guards_passed']=all(c['pixel_equivalence_passed'] for c in result['cases'].values())
        result['selected_direct_distortion_reached']=result['cases']['high']['selected_direct_distortion_calls']>0
        result['success']=result['sources_stable'] and result['current_cursor_observations_verified']
    except Exception as error:result.update(success=False,error=str(error))
    write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ['success','error','sources_stable','current_cursor_observations_verified','default_comparison_guards_passed']}),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
