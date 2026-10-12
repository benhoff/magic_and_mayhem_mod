#!/usr/bin/env python3
"""Compare bounded actual queued-update timing against a counter-only control."""
import argparse
import json
from pathlib import Path
from drawing_api_session import BUILD,sha
from drawing_engine_windows import analyze
from drawing_diagnostic_cadence import compare
ROOT=Path(__file__).resolve().parents[1]


def read(path):
    raw=path.read_bytes();r=json.loads(raw)
    if not r.get('success') or r.get('source_sha256')!=BUILD or not r.get('original_drawing_active') or not r.get('original_manifest_verified_before_after') or not r.get('normal_original_quit_verified'):
        raise ValueError('Complete original-active counter reference required')
    trace=Path(r['experiment'])/'capture/engine-cadence.bin';observed=analyze(trace)
    if observed!=r['engine_update_observation']:
        raise ValueError('Recorded counter trace changed')
    schedule='tests/fixtures/drawing-journeys/wine-submission-schedule.json'
    if schedule not in r['sources']:
        raise ValueError('Declared shared input schedule missing')
    return r,dict(path=str(path),sha256=sha(raw),trace_sha256=sha(trace.read_bytes()),schedule_sha256=r['sources'][schedule])


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--control',type=Path,required=True);p.add_argument('--diagnostic',type=Path,action='append',required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Prospective sources changed')
    control,control_id=read(a.control);s=control['staging']
    if not control.get('cadence_control') or s['api_observer_enabled'] or s['producer_samples'] or s['wine_submission'] or s['x11_delivery_library'] or control['observer_mode']!='metadata':
        raise ValueError('Explicit counter-only control required')
    comparisons=[]
    for path in a.diagnostic:
        r,identity=read(path)
        if r.get('cadence_control') or not r['staging']['api_observer_enabled']:raise ValueError('Diagnostic drawing capture required')
        if identity['schedule_sha256']!=control_id['schedule_sha256'] or r['staging']['preferences']!=s['preferences'] or r['input_binary_sha256']!=control['input_binary_sha256'] or r['staging']['binaries']['MnmEngineCadence.dll']!=s['binaries']['MnmEngineCadence.dll']:
            raise ValueError('Control schedule/settings/counter/input executable mismatch')
        comparisons.append(dict(reference=identity,measurement=compare(control['engine_update_observation'],r['engine_update_observation'])))
    if any(sha((ROOT/n).read_bytes())!=h for n,h in sources.items()):raise ValueError('Comparison sources changed')
    report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],control=control_id,comparisons=comparisons,original_game_executed=False,identical_gameplay_work_proved=False,unobtrusive_capture_proved=False,complete_presented_journey=False,scope='Timing distributions of first bounded queued updates under shared declared inputs/settings. Control still observes counters/menu/input; randomized workload and capture duration differ. No source/frame/semantic equivalence or60FPS/unobtrusive proof.')
    a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(success=True,comparisons=len(comparisons),median_ratios=[x['measurement']['median_ratio'] for x in comparisons])))


if __name__=='__main__':main()
