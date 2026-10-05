#!/usr/bin/env python3
"""Validate Qt Main -> Quick Battle -> Cancel -> Main in an isolated Xvfb session."""
import argparse
import hashlib
import json
import os
import struct
import signal
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import time
REPO=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exit-from',choices=['main','quick'],help='Validate native Quit or window close instead of original fallback')
    parser.add_argument('--battle',choices=['direct','spells'],help='Validate Single Player setup and original Start handoff')
    args=parser.parse_args()
    if args.battle and args.exit_from:parser.error('Choose battle or exit validation')
    parent=REPO/'working/tests/live-menus';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(root,flush=True)
    subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    server=None
    shell=None
    try:
        with (root/'display').open('w+') as display,(root/'xvfb.log').open('w') as xlog:
            server=subprocess.Popen(['Xvfb','-displayfd',str(display.fileno()),'-screen','0','1280x1024x24','-nolisten','tcp'],
                                    pass_fds=(display.fileno(),),stdout=xlog,stderr=subprocess.STDOUT)
            deadline=time.monotonic()+10
            while time.monotonic()<deadline:
                display.seek(0);number=display.read().strip()
                if number:break
                if server.poll() is not None:raise RuntimeError('Xvfb failed')
                time.sleep(.1)
            else:raise RuntimeError('Xvfb did not become ready')
            env=dict(os.environ,DISPLAY=':'+number,QT_QPA_PLATFORM='xcb',WINEDEBUG='-all')
            env.pop('WAYLAND_DISPLAY',None)
            if args.battle:env['MNM_LIVE_MENU_TEST_BATTLE']=args.battle
            else:env.pop('MNM_LIVE_MENU_TEST_BATTLE',None)
            if args.exit_from:env['MNM_LIVE_MENU_TEST_EXIT']=args.exit_from
            else:env.pop('MNM_LIVE_MENU_TEST_EXIT',None)
            with (root/'shell.log').open('w') as log:
                shell=subprocess.Popen([str(REPO/'working/build/qt-shell/mnm-qt-shell'),'--repo',str(REPO),
                    '--live-menus','--live-menu-test',str(root/'report.json')],env=env,cwd=REPO,
                    stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
                if shell.wait(timeout=420):raise RuntimeError('Live Qt menu test failed; inspect shell.log')
        report=json.loads((root/'report.json').read_text())
        expected_screens=[3,22,14,22,14,25,14,25,14,14,14,14] if args.battle else [3,22,3]
        if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=list(zip(expected_screens,range(len(expected_screens)))):
            raise RuntimeError('Incomplete live transition evidence')
        lines=(root/'shell.log').read_text().splitlines()
        if args.exit_from and not any(line.startswith('Menu launcher exited with status 0.') for line in lines):
            raise RuntimeError('Original Quit did not complete with a successful launcher exit')
        experiments=[line.removeprefix('Evidence directory: ') for line in lines if line.startswith('Evidence directory: ')]
        if len(experiments)!=1:raise RuntimeError('Missing staged experiment provenance')
        experiment=Path(experiments[0])
        spec=importlib.util.spec_from_file_location('menu_decode',REPO/'tools/test-menu-observer.py')
        decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)
        rows=decoder.decode((experiment/'events.bin').read_bytes())
        actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
        expected_actions=[(3,2),(22,2),(14,0),(22,2),(14,2),(25,1),(14,2),(25,0),(14,256),(14,257),(14,259),(14,1)] if args.battle else ([(3,2),(22,3),(3,4)] if args.exit_from else [(3,2),(22,3)])
        if actions!=expected_actions:raise RuntimeError('Unexpected original callback trace')
        words=struct.unpack_from('<48I',(experiment/'channel.bin').read_bytes())
        if words[5]!=0 or words[36]!=(12 if args.battle else 3 if args.exit_from else 2) or (not args.exit_from and not args.battle and words[37]!=5):raise RuntimeError('Fallback did not retire acknowledged channel')
        if args.battle=='direct' and words[34] not in (3,22):raise RuntimeError('Original battle exit did not return to Main or Quick Battle')
        report.update(original_return_screen=words[34] if args.battle=='direct' else None,experiment=str(experiment),original_callback_trace=actions,channel_retired=True,normal_exit=bool(args.exit_from))
        (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
        (root/'artifacts.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest()
            for p in root.iterdir() if p.is_file()},indent=2)+'\n')
        print('Live Qt menu round trip passed',flush=True)
    finally:
        if shell is not None and shell.poll() is None:
            os.killpg(shell.pid,signal.SIGTERM)
            try:shell.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(shell.pid,signal.SIGKILL);shell.wait()
        prefix=root/'wineprefix'
        if prefix.exists():
            subprocess.run(['wineserver','-k'],env=dict(os.environ,WINEPREFIX=str(prefix)),
                           stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=10,check=False)
        if server is not None:
            server.terminate()
            try:server.wait(timeout=5)
            except subprocess.TimeoutExpired:server.kill();server.wait()
        subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
if __name__=='__main__':main()
