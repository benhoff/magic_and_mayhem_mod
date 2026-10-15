#!/usr/bin/env python3
"""Reanalyze a pinned original API capture without executing or replacing the game."""
import argparse
import json
from pathlib import Path
import subprocess
from drawing_api_session import BUILD, sha
from drawing_surface_lifetimes import analyze_lifetimes
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--evidence',required=True);p.add_argument('--claims',type=Path,required=True);p.add_argument('--report',type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    register=json.loads((ROOT/'research/runtime/coverage/register.json').read_text())
    evidence=next(e for e in register['evidence'] if e['id']==a.evidence)
    if evidence['kind'] not in ('live_observation','isolated_original'):raise ValueError('Original execution evidence required')
    reference_path=(ROOT/evidence['record']).resolve()
    if not reference_path.is_relative_to(ROOT/'research/runtime'):raise ValueError('Reference path escapes research')
    raw=reference_path.read_bytes()
    if sha(raw)!=evidence['sha256']:raise ValueError('Pinned original report changed')
    reference=json.loads(raw)
    if reference.get('success') is not True or reference.get('source_sha256')!=BUILD or reference.get('original_drawing_active') is not True or reference.get('original_manifest_verified_before_after') is not True:raise ValueError('Unsupported original reference provenance')
    capture=(Path(reference['experiment'])/'capture').resolve()
    if not capture.is_relative_to(ROOT/'working'):raise ValueError('Capture path escapes working storage')
    intervals=(capture/'api-call-intervals.bin').read_bytes();journal=(capture/'session.bin').read_bytes()
    if sha(intervals)!=reference['call_intervals']['interval_sha256'] or sha(journal)!=reference['observation']['journal_sha256']:raise ValueError('Original interval/journal artifact changed')
    claims=json.loads(a.claims.read_text());sources=dict(claims['sources'])
    for name in ('tools/analyze-drawing-lifetimes.py','tools/drawing_surface_lifetimes.py','tools/drawing_call_intervals.py','tools/drawing_api_session.py','tests/test-drawing-surface-lifetimes.py','tests/test-drawing-call-intervals.py'):sources[name]=sha((ROOT/name).read_bytes())
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Prospective sources changed')
    guards=[]
    for name in ('tests/test-drawing-call-intervals.py','tests/test-drawing-surface-lifetimes.py'):
        run=subprocess.run(['python3',str(ROOT/name)],capture_output=True,text=True,check=True,timeout=30)
        guards.append(dict(path=name,success=True,output=run.stdout+run.stderr))
    result=analyze_lifetimes(intervals,journal)
    if any(sha((ROOT/name).read_bytes())!=digest for name,digest in sources.items()):raise ValueError('Analysis sources changed during execution')
    report=dict(schema=1,success=True,sources=sources,sources_stable=True,claims=claims['claims'],
                source_reference=dict(evidence=a.evidence,path=evidence['record'],sha256=evidence['sha256'],original_sha256=BUILD),
                input_artifacts=dict(journal_sha256=sha(journal),interval_sha256=sha(intervals)),
                original_game_executed=False,original_pixels_used_as_native_inputs=False,native_pixel_parity=False,
                scope='Offline guarded temporal QueryInterface alias/CPU/DC lifetime analysis of a pinned original-active capture. No original reexecution, current engine comparison, owned producer state, complete presentation route or journey claim.',
                unit_validation=guards,analysis=result)
    a.report.parent.mkdir(parents=True,exist_ok=True)
    a.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(success=True,query_links=len(result['query_links']),leases=len(result['leases']),lease_gaps=len(result['lease_gaps']),remaining_leases=len(result['remaining_leases']),frames_with_open_leases=result['frames_with_open_leases'],frames_with_source_or_destination_leases=result['frames_with_source_or_destination_leases'])))

if __name__=='__main__':main()
