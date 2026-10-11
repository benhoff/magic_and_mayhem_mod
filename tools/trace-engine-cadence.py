#!/usr/bin/env python3
"""Run a source-bound private TITAN trace of unchanged original engine scheduling."""
import argparse
import hashlib
import json
import signal
from pathlib import Path
import subprocess
import tempfile
from engine_cadence import summarize

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Changed prospective trace sources')
    parent=ROOT/'working/tests/engine-cadence';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Engine cadence evidence: '+str(out),flush=True)
    report=dict(success=False,claims=declaration['claims'],sources=declaration['sources'],
        scope='Actual unchanged No-CD simulation/animation/World ordering on private TITAN native resident gameplay; tracing perturbs timings. No pacing change or 60FPS claim.')
    command=['python3',str(ROOT/'tools/test-native-campaign.py'),'--resident-world','--hardware','--difficulty','1',
             '--require-casting-combat','--spell-cases','--portrait-stress-seconds','10','--stress-seconds','60',
             '--min-fps','19','--normal-quit','--timeout','600','--trace-engine-cadence']
    child=None
    try:
        with (out/'campaign.log').open('x') as log:
            child=subprocess.Popen(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
            exit_code=child.wait(timeout=720)
        lines=(out/'campaign.log').read_text().splitlines()
        reports=[Path(s.partition(': ')[2]) for s in lines if s.startswith('Report: ')]
        if len(reports)!=1:raise ValueError('Missing unique campaign report')
        game=json.loads(reports[0].read_text());report.update(campaign_report=str(reports[0]),campaign_report_sha256=sha(reports[0]),
                campaign_success=game['success'],campaign_exit_code=exit_code)
        experiment=Path(game['experiment']);report['experiment']=str(experiment)
        trace=experiment/'engine-cadence.bin';report['trace_sha256']=sha(trace);report['cadence']=summarize(trace)
        report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        report['original_manifest_verified_after']=game.get('original_manifest_verified_after') is True
        report['private_wine_stopped']=game.get('private_wine_stopped') is True
        report['resident_world']=game.get('resident_world')
        report['hardware_driver']=game.get('hardware_driver')
        if not all(report[k] for k in ('sources_unchanged','original_manifest_verified_after','private_wine_stopped')):
            raise ValueError('Source/cleanup/original integrity failure')
        if exit_code or not game['success']:raise ValueError('Original campaign failed; trace retained as diagnostic only')
        report['success']=True
    except (OSError,ValueError,KeyError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if child and child.poll() is None:
            child.send_signal(signal.SIGINT)
            try:child.wait(timeout=60)
            except subprocess.TimeoutExpired:child.kill();child.wait();report['cleanup_unconfirmed']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
