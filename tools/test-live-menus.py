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
def validate(root,args):
    report=json.loads((root/'report.json').read_text())
    expected_screens=[3,22,14,22,14,25,14,25,14,14,14,14] if args.battle else [3,22,3]
    if args.battle=='results':expected_screens += [26,26,22]
    if args.battle_repeat:expected_screens += [22,14,22]
    if args.battle=='spells':expected_screens += [7,22]
    if not report['success'] or [(s['screen'],s['ack']) for s in report['states']]!=list(zip(expected_screens,range(len(expected_screens)))):
        raise RuntimeError('Incomplete live transition evidence')
    lines=(root/'shell.log').read_text().splitlines()
    if (args.exit_from or args.battle_repeat or args.battle in ('spells','results')) and not any(line.startswith('Menu launcher exited with status 0.') for line in lines):
        raise RuntimeError('Original Quit did not complete with a successful launcher exit')
    if args.battle_repeat and (report.get('native_returns')!=2 or report.get('battles')!=2 or any(line.startswith('Smoke test passed:') for line in lines)):
        raise RuntimeError('Repeated battle validation must end through original Quit, not the run deadline')
    experiments=[line.removeprefix('Evidence directory: ') for line in lines if line.startswith('Evidence directory: ')]
    if len(experiments)!=1:raise RuntimeError('Missing staged experiment provenance')
    experiment=Path(experiments[0])
    spec=importlib.util.spec_from_file_location('menu_decode',REPO/'tools/test-menu-observer.py')
    decoder=importlib.util.module_from_spec(spec);spec.loader.exec_module(decoder)
    rows=decoder.decode((experiment/'events.bin').read_bytes())
    actions=[(r['menu_id'],r['argument']) for r in rows if r['event']==3]
    expected_actions=[(3,2),(22,2),(14,0),(22,2),(14,2),(25,1),(14,2),(25,0),(14,256),(14,257),(14,259),(14,1)] if args.battle else ([(3,2),(22,3),(3,4)] if args.exit_from else [(3,2),(22,3)])
    if args.battle=='results':expected_actions += [(26,1),(26,2),(22,3),(3,4)]
    if args.battle_repeat:expected_actions += [(22,2),(14,1),(22,3),(3,4)]
    if args.battle=='spells':
        shelves=report['spell_initial_shelves'][:]
        expected=[-1]*63
        for entry in report['spell_assignments']:expected[entry['slot']]=entry['item']
        for position,item in enumerate(report['spell_initial_assignments']):
            if item<0:continue
            shelf=shelves.index(-1);shelves[shelf]=item
            expected_actions += [(7,2046+(position//21)*7+position%21),(7,2003+shelf)]
        for position,item in enumerate(expected):
            if item<0:continue
            shelf=shelves.index(item);shelves[shelf]=-1
            expected_actions += [(7,2003+shelf),(7,2046+(position//21)*7+position%21)]
        expected_actions += [(7,12),(22,3),(3,4)]
        raw=(experiment/'channel.bin').read_bytes()
        actual=struct.unpack_from('<63i',raw,18000+28)
        if list(actual)!=expected:raise RuntimeError('Original spell controls disagree with Qt loadout')
        report['engine_spell_assignments']=list(actual)
    if args.battle=='results':
        states=[s for s in report['states'] if s['screen']==26]
        if len(states)!=2 or any(s['result_actions']!=3 for s in states) or report['native_returns']!=1 or report['battles']!=1:raise RuntimeError('Incomplete native results ownership/return evidence')
        raw=(experiment/'channel.bin').read_bytes();actual=[]
        for i in range(4):
            at=42100+16+i*648;active,portrait=struct.unpack_from('<II',raw,at)
            cells=[raw[at+8+c*128:at+8+(c+1)*128].split(b'\0',1)[0].decode('cp1252') for c in range(5)]
            actual.append(dict(zip(['name','kills','deaths','handicap','score'],cells),active=bool(active)))
        if states[-1]['results']!=actual:raise RuntimeError('Qt results disagree with engine display snapshot')
        report['engine_result_rows']=actual
    if actions!=expected_actions:raise RuntimeError('Unexpected original callback trace')
    words=struct.unpack_from('<48I',(experiment/'channel.bin').read_bytes())
    if words[5]!=0 or words[36]!=(16 if args.battle=='results' or args.battle_repeat else 15 if args.battle=='spells' else 12 if args.battle else 3 if args.exit_from else 2) or (not args.exit_from and not args.battle and words[37]!=5):raise RuntimeError('Fallback did not retire acknowledged channel')
    if args.battle=='direct' and not args.battle_repeat and words[34] not in (3,22):raise RuntimeError('Original battle exit did not return to Main or Quick Battle')
    report.update(original_return_screen=(report['states'][-1]['screen'] if args.battle_repeat or args.battle in ('spells','results') else words[34]) if args.battle else None,experiment=str(experiment),original_callback_trace=actions,channel_retired=True,normal_exit=bool(args.exit_from or args.battle_repeat or args.battle in ('spells','results')))
    (root/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    (root/'artifacts.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest()
        for p in root.iterdir() if p.is_file() and p.name!='artifacts.json'},indent=2)+'\n')
    print('Live Qt menu round trip passed',flush=True)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exit-from',choices=['main','quick','results'],help='Validate native Quit or window close instead of original fallback')
    parser.add_argument('--battle',choices=['direct','spells','results'],help='Validate Single Player setup and original Start handoff')
    parser.add_argument('--battle-repeat',action='store_true',help='Require two battles, two native menu returns and normal Quit (use with --battle direct)')
    parser.add_argument('--shell',type=Path,default=REPO/'working/build/qt-shell/mnm-qt-shell',help='Alternate shell binary for a dedicated validation build')
    parser.add_argument('--validate-run',type=Path,help='Recheck existing live evidence without launching another game')
    args=parser.parse_args()
    if args.battle_repeat and args.battle!='direct':parser.error('--battle-repeat requires --battle direct')
    if args.exit_from=='results' and args.battle!='results':parser.error('--exit-from results requires --battle results')
    if args.battle and args.exit_from and args.exit_from!='results':parser.error('Choose battle or exit validation')
    if args.validate_run:
        validate(args.validate_run.resolve(),args);return
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
            if args.battle_repeat:env['MNM_LIVE_MENU_TEST_REPEAT']='1'
            else:env.pop('MNM_LIVE_MENU_TEST_REPEAT',None)
            if args.battle:env['MNM_LIVE_MENU_TEST_BATTLE']=args.battle
            else:env.pop('MNM_LIVE_MENU_TEST_BATTLE',None)
            if args.exit_from:env['MNM_LIVE_MENU_TEST_EXIT']=args.exit_from
            else:env.pop('MNM_LIVE_MENU_TEST_EXIT',None)
            with (root/'shell.log').open('w') as log:
                shell=subprocess.Popen([str(args.shell.resolve()),'--repo',str(REPO),
                    '--live-menus','--live-menu-test',str(root/'report.json')],env=env,cwd=REPO,
                    stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
                if shell.wait(timeout=540 if args.battle_repeat else 420):raise RuntimeError('Live Qt menu test failed; inspect shell.log')
        validate(root,args)
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
