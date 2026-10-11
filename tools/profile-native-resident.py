#!/usr/bin/env python3
"""Fresh private TITAN resident campaign diagnostics; sampling perturbs cadence."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time
import signal
from profile_native_resident_helpers import sha, summarize

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture',action='store_true',help='Retain bounded ordered gameplay corpus for offline replay')
    parser.add_argument('--cpu-sample',action='store_true',help='99Hz user CPU stacks for30 seconds after40 seconds of physical gameplay')
    args=parser.parse_args()
    parent=ROOT/'working/tests/resident-stage-profile';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.resident-stage-profiling',
                    '--scenario','NR.resident-stage-profiling.live','--output',str(out/'prospective-claims.json')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
    declaration=json.loads((out/'prospective-claims.json').read_text())
    report=dict(success=False,sources=declaration['sources'],claims=declaration['claims'],
                scope='Opt-in private TITAN resident public campaign stage/pacing/GPU observation. CPU sampling and optional source capture perturb cadence; not an uninstrumented performance proof.')
    command=['python3',str(ROOT/'tools/test-native-campaign.py'),'--resident-world','--hardware','--difficulty','1',
             '--require-casting-combat','--spell-cases','--portrait-stress-seconds','10','--stress-seconds','60',
             '--min-fps','19','--normal-quit','--timeout','600','--profile-resident']
    if args.capture:command.append('--capture-resident')
    report['command']=command;child=None;campaign=None
    try:
        with (out/'campaign.log').open('x') as log:
            child=subprocess.Popen(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
        if args.cpu_sample:
            deadline=time.monotonic()+300;pid=None
            while child.poll() is None and time.monotonic()<deadline:
                paths=[Path(t.partition(': ')[2]) for t in (out/'campaign.log').read_text().splitlines() if t.startswith('Campaign test evidence: ')]
                if paths:
                    campaign=paths[0];state_path=campaign/'input-0.json'
                    if state_path.exists():
                        try:state=json.loads(state_path.read_text())
                        except json.JSONDecodeError:state={}
                        if state.get('seconds',0)>=40:
                            exe=ROOT/'working/build/qt-shell/mnm-qt-shell'
                            for entry in Path('/proc').iterdir():
                                if not entry.name.isdigit():continue
                                try:cmdline=(entry/'cmdline').read_bytes().split(b'\0')
                                except OSError:continue
                                if cmdline and cmdline[0]==str(exe).encode() and str(campaign/'flow.json').encode() in cmdline:
                                    pid=int(entry.name);report['cpu_sample']=dict(pid=pid,shell_sha256=sha(exe),input_seconds=state['seconds'],actions=state.get('actions',[])[-2:]);break
                            if pid:break
                time.sleep(1)
            if not pid:raise RuntimeError('No owned native shell at physical gameplay sampling window')
            perf=['perf','record','-q','-e','cpu-clock:u','-F','99','-g','--call-graph','dwarf,16384',
                  '-p',str(pid),'-o',str(out/'perf.data'),'--','sleep','30']
            report['cpu_sample']['command']=perf
            with (out/'perf.log').open('x') as log:subprocess.run(perf,stdout=log,stderr=subprocess.STDOUT,timeout=60,check=True)
            for name,extra in [('flat',['--no-children']),('inclusive',['--children'])]:
                with (out/(name+'.txt')).open('x') as log:
                    subprocess.run(['perf','report','--stdio','--sort','symbol','-i',str(out/'perf.data'),*extra],stdout=log,stderr=subprocess.STDOUT,check=True,timeout=60)
            report['cpu_sample']['artifacts']={name:sha(out/name) for name in ['perf.data','flat.txt','inclusive.txt','perf.log']}
        report['campaign_exit_code']=child.wait(timeout=660)
        if not campaign:
            campaign=next(Path(t.partition(': ')[2]) for t in (out/'campaign.log').read_text().splitlines() if t.startswith('Campaign test evidence: '))
        game=json.loads((campaign/'report.json').read_text());report['campaign_report']=str(campaign/'report.json');report['campaign_report_sha256']=sha(campaign/'report.json')
        report['campaign_success']=game['success'];report['campaign_error']=game.get('error');report['resident_world']=game.get('resident_world',{})
        report['private_wine_stopped']=game.get('private_wine_stopped',False);report['original_manifest_verified_after']=game.get('original_manifest_verified_after',False)
        report['hardware_driver']=game.get('hardware_driver');report['resident_capture']=game.get('resident_capture')
        stages=report['resident_world'].get('stage_profile',{});report['summary']=summarize(stages.get('samples',[]))
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in report['sources'].items())
        report['success']=bool(game['success'] and report['sources_stable'] and stages.get('samples') and not stages['samples_dropped'] and not stages['gpu_queries_dropped'])
        if not report['success']:report['error']='Profile did not complete every required diagnostic/gameplay check; original campaign failure retained without relaxing FPS gates'
    except (OSError,RuntimeError,subprocess.SubprocessError,StopIteration,KeyError,ValueError) as error:
        report['error']=str(error)
    finally:
        if child and child.poll() is None:
            child.send_signal(signal.SIGINT)
            try:child.wait(timeout=60)
            except subprocess.TimeoutExpired:child.kill();child.wait()
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Resident profile report:',out/'report.json',flush=True)
    return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
