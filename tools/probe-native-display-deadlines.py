#!/usr/bin/env python3
"""Source-bound TITAN display-deadline probe; preserves campaign/performance failures."""
import argparse,hashlib,json,signal,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path,required=True);parser.add_argument('--full',action='store_true');parser.add_argument('--frame-trace',action='store_true');args=parser.parse_args()
    declared=json.loads(args.claims.read_text());assert all(sha(ROOT/p)==h for p,h in declared['sources'].items())
    parent=ROOT/'working/tests/native-display-deadlines';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Display deadline evidence:',out,flush=True)
    report=dict(success=False,claims=declared['claims'],sources=declared['sources'],scope=__doc__)
    command=['python3',str(ROOT/'tools/test-native-campaign.py'),'--resident-world','--independent-world','--hardware','--difficulty','1','--stress-seconds','60' if args.full else '10','--min-fps','19','--normal-quit','--skip-movies','--timeout','600' if args.full else '240']
    if args.frame_trace:command+=['--frame-trace']
    if args.full:command+=['--require-casting-combat','--spell-cases','--portrait-stress-seconds','10']
    process=None
    try:
        with (out/'campaign.log').open('x') as log:
            process=subprocess.Popen(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT);exit_code=process.wait(timeout=720 if args.full else 360)
        paths=[Path(line.split(': ',1)[1]) for line in (out/'campaign.log').read_text().splitlines() if line.startswith('Report: ')];assert len(paths)==1
        child=json.loads(paths[0].read_text());flow=child.get('flow',{});report.update(campaign_report=str(paths[0]),campaign_report_sha256=sha(paths[0]),campaign_success=child['success'],campaign_exit_code=exit_code,campaign_error=child.get('error'),functional_flow_success=flow.get('success') is True,resident_world=child.get('resident_world'),hardware_driver=child.get('hardware_driver'),sources_unchanged=all(sha(ROOT/p)==h for p,h in declared['sources'].items()),private_wine_stopped=child.get('private_wine_stopped') is True,original_manifest_verified_after=child.get('original_manifest_verified_after') is True)
        rates=flow.get('gameplay_rates',[]);report['visible_paint_windows']=[]
        for cycle in sorted({s['cycle'] for s in rates}):
            ss=[s for s in rates if s['cycle']==cycle];seconds=sum(s['seconds'] for s in ss);report['visible_paint_windows'].append(dict(cycle=cycle,seconds=seconds,paint_fps=sum(s['painted_frames'] for s in ss)/seconds,worst_window_paint_fps=min(s['paint_fps'] for s in ss)))
        report['success']=child['success'] and all(report[k] for k in ['sources_unchanged','private_wine_stopped','original_manifest_verified_after'])
    except (OSError,ValueError,AssertionError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if process and process.poll() is None:
            process.send_signal(signal.SIGINT)
            try:process.wait(timeout=60)
            except subprocess.TimeoutExpired:process.kill();process.wait();report['cleanup_unconfirmed']=True
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('Report:',out/'report.json',flush=True);return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
