#!/usr/bin/env python3
"""Bounded TITAN production/state experiment; original pacing is unchanged."""
import argparse
import hashlib
import json
from pathlib import Path
import signal
import subprocess
import tempfile
from scene_production import summarize

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--candidate',choices=('observe','full','slice','frozen-history'),required=True);args=parser.parse_args()
    declaration=json.loads(args.claims.read_text())
    if any(sha(ROOT/p)!=h for p,h in declaration['sources'].items()):raise ValueError('Changed prospective production sources')
    parent=ROOT/'working/tests/scene-production';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Scene production evidence: '+str(out),flush=True)
    report=dict(success=False,candidate=args.candidate,claims=declaration['claims'],sources=declaration['sources'],
        scope='Bounded private TITAN extra original scene production with exact protected state snapshots and original50ms pacing; not independent60FPS, full map/nested state or original pixel/driver equivalence.')
    command=['python3',str(ROOT/'tools/test-native-campaign.py'),'--resident-world','--hardware','--difficulty','1',
             '--require-casting-combat','--spell-cases','--portrait-stress-seconds','10','--stress-seconds','60','--min-fps','19',
             '--normal-quit','--timeout','600','--scene-production-probe',args.candidate]
    child=None
    try:
        with (out/'campaign.log').open('x') as log:
            child=subprocess.Popen(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT);exit_code=child.wait(timeout=720)
        paths=[Path(l.partition(': ')[2]) for l in (out/'campaign.log').read_text().splitlines() if l.startswith('Report: ')]
        if len(paths)!=1:raise ValueError('Missing unique production campaign report')
        game=json.loads(paths[0].read_text());report.update(campaign_report=str(paths[0]),campaign_report_sha256=sha(paths[0]),
            campaign_success=game['success'],campaign_exit_code=exit_code,experiment=game['experiment'])
        report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
        report['private_wine_stopped']=game.get('private_wine_stopped') is True
        report['original_manifest_verified_after']=game.get('original_manifest_verified_after') is True
        report['hardware_driver']=game.get('hardware_driver');report['resident_world']=game.get('resident_world')
        report['gameplay_checks']={k:game.get(k) for k in ('casting_verified','combat_verified','movement_verified','normal_quit_verified')}
        report['campaign_error']=game.get('error')
        trace=Path(game['experiment'])/'scene-production.bin';report['trace_sha256']=sha(trace);report['trace_bytes']=trace.stat().st_size;report['production']=summarize(trace)
        if not all(report[k] for k in ('sources_unchanged','private_wine_stopped','original_manifest_verified_after')):raise ValueError('Production integrity/cleanup failed')
        if exit_code or not game['success']:raise ValueError('Campaign failed; preserve production diagnostic separately')
        if args.candidate!='observe' and not report['production']['protected_state_invariance']:raise ValueError('Repeated producer changed protected state or refused; not an invariance pass')
        if args.candidate!='observe' and not report['production']['scene_record_equality']:raise ValueError('Extra producer changed ordered scene records; not a scene equality pass')
        if args.candidate=='observe' and len(report['production']['cases'])<8:raise ValueError('Insufficient natural producer observations')
        report['success']=True
    except (OSError,KeyError,ValueError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if child and child.poll() is None:
            child.send_signal(signal.SIGINT)
            try:child.wait(timeout=60)
            except subprocess.TimeoutExpired:child.kill();child.wait();report['cleanup_unconfirmed']=True
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report: '+str(out/'report.json'),flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
