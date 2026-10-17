#!/usr/bin/env python3
"""Compare bounded actual queued-update timing against a counter-only control."""
import argparse
import json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_engine_windows import analyze
from drawing_diagnostic_cadence import compare,results_action
from drawing_process_exit import decode as decode_process_exit
ROOT=Path(__file__).resolve().parents[1]


def executed_schedule(report, raw=None, name=None):
    """Verify a selected declaration, or discover its unique executed schedule."""
    root=ROOT
    if isinstance(raw,Path) and name is None:
        root=raw;raw=None
    observed=report.get('input_schedule',{})
    if observed.get('success') is not True or observed.get('error') or not observed.get('actions'):
        raise ValueError('Successful executed input schedule required')
    metadata={'ordinal','start_ms','end_ms','journal_bytes_before_image','journal_bytes_after_image',
              'presentation_file','synchronized_original_frame'}
    actions=[];previous=0
    for ordinal,row in enumerate(observed['actions'],1):
        start,end=row.get('start_ms'),row.get('end_ms')
        if type(row.get('ordinal')) is not int or row['ordinal']!=ordinal or type(start) is not int or type(end) is not int or start<previous or end<start:
            raise ValueError('Executed input action order/timing')
        actions.append({k:v for k,v in row.items() if k not in metadata});previous=end
    encode=lambda value:json.dumps(value,sort_keys=True,separators=(',',':'))
    if name is not None:
        if not isinstance(raw,bytes) or report['sources'].get(name)!=sha(raw):
            raise ValueError('Declared input schedule fingerprint missing/different')
        if encode(json.loads(raw).get('actions'))!=encode(actions):
            raise ValueError('Executed input actions differ from declared schedule')
        return sha(raw)
    if raw is not None:raise ValueError('Explicit schedule bytes require a declared name')
    matches=[]
    for declared,digest in report['sources'].items():
        path=Path(declared)
        if not declared.startswith('tests/fixtures/drawing-journeys/') or not path.name.endswith('-schedule.json'):
            continue
        if path.is_absolute() or '..' in path.parts:raise ValueError('Unsafe declared input schedule')
        data=(root/path).read_bytes()
        if sha(data)!=digest:raise ValueError('Declared input schedule changed')
        if encode(json.loads(data).get('actions'))==encode(actions):matches.append(dict(path=declared,sha256=digest))
    if len(matches)!=1:raise ValueError('Executed input schedule missing/ambiguous in declared sources')
    return matches[0]

def read(path,schedule,scheduled_bytes):
    raw=path.read_bytes();r=json.loads(raw)
    if not r.get('success') or r.get('source_sha256')!=BUILD or not r.get('original_drawing_active') or not r.get('original_manifest_verified_before_after') or not r.get('normal_original_quit_verified'):
        raise ValueError('Complete original-active counter reference required')
    trace=Path(r['experiment'])/'capture/engine-cadence.bin';observed=analyze(trace)
    if observed!=r['engine_update_observation']:
        raise ValueError('Recorded counter trace changed')
    quit=r.get('quit_observation',{})
    if not quit.get('original_child_exit_code_verified') or not quit.get('dispatch_observed'):raise ValueError('Direct original process exit and Quit dispatch required')
    exit_raw=(Path(r['experiment'])/'process-exit.bin').read_bytes();child=decode_process_exit(exit_raw)
    if child!=quit.get('original_child_exit') or child['exit_code'] or sha(exit_raw)!=quit.get('process_exit_journal_sha256'):raise ValueError('Original child exit evidence changed')
    schedule_sha=executed_schedule(r,scheduled_bytes,schedule)
    return r,dict(path=str(path),sha256=sha(raw),trace_sha256=sha(trace.read_bytes()),schedule=schedule,schedule_sha256=schedule_sha,process_exit_sha256=sha(exit_raw))


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--control',type=Path,required=True);p.add_argument('--diagnostic',type=Path,action='append',required=True);p.add_argument('--schedule',type=Path,default=ROOT/'tests/fixtures/drawing-journeys/wine-submission-schedule.json');p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    schedule=a.schedule.resolve().relative_to(ROOT).as_posix();scheduled_bytes=a.schedule.read_bytes();sources[schedule]=sha(scheduled_bytes)
    control,control_id=read(a.control,schedule,scheduled_bytes);s=control['staging']
    if not control.get('cadence_control') or s['api_observer_enabled'] or s['producer_samples'] or s['wine_submission'] or s['x11_delivery_library'] or control['observer_mode']!='metadata':
        raise ValueError('Explicit counter-only control required')
    comparisons=[]
    for path in a.diagnostic:
        r,identity=read(path,schedule,scheduled_bytes)
        if r.get('cadence_control') or not r['staging']['api_observer_enabled']:raise ValueError('Diagnostic drawing capture required')
        if results_action(r['staging'].get('results_key_hold_ms',60))!=results_action(s.get('results_key_hold_ms',60)):raise ValueError('Control Results key policy mismatch')
        if identity['schedule_sha256']!=control_id['schedule_sha256'] or r['staging']['preferences']!=s['preferences'] or r['input_binary_sha256']!=control['input_binary_sha256'] or r['staging']['binaries']['MnmEngineCadence.dll']!=s['binaries']['MnmEngineCadence.dll']:
            raise ValueError('Control schedule/settings/counter/input executable mismatch')
        comparisons.append(dict(reference=identity,measurement=compare(control['engine_update_observation'],r['engine_update_observation'])))
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Comparison sources changed')
    report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],control=control_id,comparisons=comparisons,original_game_executed=False,identical_gameplay_work_proved=False,unobtrusive_capture_proved=False,complete_presented_journey=False,scope='Timing distributions of first bounded queued updates under shared declared inputs/settings. Control still observes counters/menu/input; randomized workload and capture duration differ. No source/frame/semantic equivalence or60FPS/unobtrusive proof.')
    a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,comparisons=len(comparisons),median_ratios=[x['measurement']['median_ratio'] for x in comparisons])))


if __name__=='__main__':main()
