#!/usr/bin/env python3
"""Reconcile four retained nominal storage CPU-read histories without acquisition."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from rendering_pilot import require
from storage_cpu_read_binding import join_reads, sha

ROOT = Path(__file__).resolve().parents[1]
PROGRESS = 'research/runtime/storage-main-stall-results-20261101/progress01.json'
BEHAVIOR = 'TOOL.storage-cpu-read-binding'
SCENARIO = 'SC.storage-cpu-read-binding'


def main():
    parser = argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args()
    require(args.report.startswith('research/runtime/cpu-read-binding-results-20261102/')
            and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(), 'New owned immutable report required')
    reg=json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior=next(b for b in reg['behaviors'] if b['id']==BEHAVIOR)
    scenario=next(s for s in reg['scenarios'] if s['id']==SCENARIO)
    result=dict(schema=1,kind='retained_storage_cpu_read_binding',success=False,executions=[],
        new_game_acquisitions=0,original_executable_run=False,independent_pixel_comparison_reexecuted=False,
        exact_original_callers_recovered=0,whole_operations_accepted=0,player_accepted=False,original_recovery_accepted=False,
        claims=[dict(behavior=BEHAVIOR,contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}),
                     scenarios={SCENARIO:scenario_contract(scenario)})])
    try:
        with Reader(ROOT) as reader:
            result['sources']={n:reader.digest(n) for n in sorted(required_sources(behavior))}
            progress=reader.json(PROGRESS);require(progress['success'] and len(progress['executions'])==4, 'Changed retained storage matrix')
            result['progress']=dict(record=PROGRESS,sha256=reader.digest(PROGRESS))
            for execution in progress['executions']:
                pair=execution['pair']
                for item in pair.values():require(reader.digest(item['record'])==item['sha256'], 'Changed retained matrix input')
                acquired,compared=[reader.json(pair[k]['record']) for k in ('acquisition','comparison')]
                require(acquired['success'] and compared['success'] and acquired['sources_stable']
                        and acquired['original_manifest_verified_before_after']
                        and compared['acquisition']==pair['acquisition'], 'Unbound successful acquisition/comparison')
                require(all(reader.digest(n)==h for n,h in acquired['sources'].items()), 'Retained acquisition source phase changed')
                selected=[]
                def artifact(suffix):
                    matches=[a for a in acquired['artifacts'] if a['path'].endswith(suffix)]
                    require(len(matches)==1, 'Ambiguous retained CPU-read input: '+suffix)
                    item=matches[0];raw=reader.read_bytes(item['record'])
                    require(len(raw)==item['bytes'] and sha(raw)==item['sha256'], 'Retained CPU-read input identity differs')
                    selected.append(dict(path=item['path'],record=item['record'],sha256=item['sha256']))
                    return raw
                command_paths=[a['path'] for a in acquired['artifacts'] if '/capture/commands-' in a['path']]
                require(len(command_paths)==1, 'Ambiguous command history capture')
                joined=join_reads(artifact('/'+Path(command_paths[0]).name),acquired['session_identity'],
                    artifact('/native-cpu-read-counts.bin'),compared['comparison']['cpu_reads'],
                    artifact('/lock-capture/lifecycle.log'),artifact('/render-commands.bin.cpu-reply'))
                joined.update(configuration=execution['configuration'],world_extents=execution['world_extents'],
                    session_identity=acquired['session_identity'],execution=pair,bound_artifacts=selected)
                result['executions'].append(joined)
                print(json.dumps(dict(configuration=execution['configuration'],extents=execution['world_extents'],
                    requests=joined['request_count'],lifecycle_details=joined['lifecycle_detail_count'],
                    kinds=joined['counters']['kinds'])),flush=True)
            require(len({e['session_identity'] for e in result['executions']})==4, 'Duplicate retained session')
            result['requests_bound']=sum(e['request_count'] for e in result['executions'])
            result['reply_comparisons_bound']=sum(e['complete_reply_matches'] for e in result['executions'])
            result['lifecycle_details_bound']=sum(e['lifecycle_detail_count'] for e in result['executions'])
            result['requests_without_lifecycle_detail']=sum(e['requests_without_lifecycle_detail'] for e in result['executions'])
            result['payload_bytes_bound']=sum(e['captured_payload_bytes'] for e in result['executions'])
            result['retained_requirements_passed']=progress['passed_requirements']
            result['retained_requirements_pending']=progress['pending_requirements']
            run=subprocess.run([sys.executable,'-B',str(ROOT/'tests/test-storage-cpu-read-binding.py')],
                               cwd=ROOT,capture_output=True,text=True,timeout=30)
            result['controls']=dict(returncode=run.returncode,output=run.stdout+run.stderr)
            require(run.returncode==0, 'CPU-read binding control failure')
            require(result['requests_bound']==result['reply_comparisons_bound']==212, 'Selected CPU-read denominator changed')
            result['sources_stable']=all(reader.digest(n)==h for n,h in result['sources'].items())
            result['success']=result['sources_stable']
    except Exception as error:result['error']=str(error)
    digest=write_report(ROOT,args.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ('success','error','requests_bound','lifecycle_details_bound','sources_stable')}
                     |dict(record=args.report,sha256=digest)),flush=True)
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
