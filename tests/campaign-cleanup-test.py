#!/usr/bin/env python3
"""Exercise actual runner cleanup after a torn auxiliary profile, with fake processes."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',type=Path);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    report=dict(success=False,scope='Synthetic mocked process/engine fixture of actual public runner error cleanup; no game, original media, X display or Wine execution',sources={p:sha(ROOT/p) for p in ('tests/campaign-cleanup-test.py','tools/test-native-campaign.py')})
    if args.claims:
        d=json.loads(args.claims.read_text());report.update(claims=d['claims']);report['sources'].update(d['sources'])
    if any(sha(ROOT/p)!=h for p,h in report['sources'].items()):raise ValueError('Declared source changed')
    spec=importlib.util.spec_from_file_location('smoke_cleanup',ROOT/'tools/test-native-campaign.py');smoke=importlib.util.module_from_spec(spec);spec.loader.exec_module(smoke)
    with tempfile.TemporaryDirectory() as temp:
        root=Path(temp);(root/'tools').mkdir();shutil.copyfile(ROOT/'tools/test-native-campaign.py',root/'tools/test-native-campaign.py')
        build=root/'working/build/qt-shell';build.mkdir(parents=True);(build/'mnm-qt-shell').write_bytes(b'fake binary');(build/'CMakeCache.txt').write_text('CMAKE_BUILD_TYPE:STRING=Release\n')
        experiment=root/'working/experiments/menu-observer/owned';experiment.mkdir(parents=True);(experiment/'render-frame.bin').write_bytes(b'fake engine')
        (experiment/'manifest.json').write_text(json.dumps(dict(native_draw_cadence=dict(settings=dict(SkipFrameEvery=0,SkipXFrames=0,MaxSkipXFrames=0)))))
        (experiment/'lock-capture').mkdir();(experiment/'lock-capture/lifecycle.log').write_text('native_pacer_yield owned fake observation\nnative_precise_clock owned fake observation\n')
        stops=[];runs=[];prefixes=[]
        class Process:
            def __init__(self,pid):self.pid=pid;self.returncode=None
            def poll(self):return self.returncode
            def wait(self,timeout):self.returncode=0;return 0
        def popen(cmd,**kw):
            if cmd[0]=='Xvfb':
                (root/'working/tests/native-campaign').glob('run-*').__next__().joinpath('display').write_text('77')
                return Process(101)
            if cmd[0]=='python3':
                Path(cmd[cmd.index('--output')+1]).write_text('{"version":1')
                return Process(103)
            flow=Path(cmd[-1]);out=flow.parent;(out/'wineprefix').mkdir();kw['stdout'].write('Evidence directory: '+str(experiment)+'\n');kw['stdout'].flush()
            steps=[]
            for name in smoke.STEPS:
                shot=out/(name+'.png');shot.write_bytes(b'fake owned image')
                steps.append(dict(step=name,screenshot=str(shot),screenshot_saved=True,native_image_saved=True,original_image_saved=True))
            flow.write_text(json.dumps(dict(success=True,native_command_fallback=False,native_command_frames=6,steps=steps)))
            return Process(102)
        def run(cmd,**kw):
            runs.append(list(map(str,cmd)))
            if cmd[0]=='wineserver':prefixes.append(kw['env']['WINEPREFIX'])
            return subprocess.CompletedProcess(cmd,0)
        with patch.object(smoke,'ROOT',root),patch.object(smoke,'SOURCES',['tools/test-native-campaign.py']),patch.object(smoke.subprocess,'run',side_effect=run),patch.object(smoke.subprocess,'Popen',side_effect=popen),patch.object(smoke.shutil,'which',return_value='/fake/tool'),patch.object(smoke.os,'killpg',side_effect=lambda pid,sig:stops.append(pid)),patch.object(smoke,'validate_events',return_value={}),patch.object(sys,'argv',['smoke']):
            code=smoke.main()
        out=next((root/'working/tests/native-campaign').glob('run-*'));failed=json.loads((out/'report.json').read_text())
        assert code==1 and not failed['success'] and failed['status']=='failed' and failed.get('profile_error')
        assert stops==[103,102,101],stops
        assert prefixes==[str(out/'wineprefix')],prefixes
        assert sum(cmd[0]=='wineserver' and cmd[1]=='-k' for cmd in runs)==1
        assert sum(cmd[-1]=='verify' and cmd[0].endswith('original-manifest.sh') for cmd in runs)==2
        assert failed['original_manifest_verified_after'] and failed['sources_unchanged']
        report.update(success=True,owned_stop_order=stops,original_manifest_calls=2,private_prefix_kills=1,failed_run_report_written=True)
    report['sources_unchanged']=all(sha(ROOT/p)==h for p,h in report['sources'].items())
    if not report['sources_unchanged']:report['success']=False
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n');print('Cleanup fixture report: '+str(args.output));return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
