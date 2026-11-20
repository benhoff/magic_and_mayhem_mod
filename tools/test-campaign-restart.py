#!/usr/bin/env python3
"""Two confirmed native Campaign restarts against an engine-written checkpoint.

Uses a private X display, disposable installation and prefix. Report bytes are
retained in SQLite; the bounded live inputs stay in their owned run directory.
"""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import struct
import subprocess
import tempfile
import time

from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def assess(run):
    flow = json.loads((run / 'flow.json').read_bytes())
    if not flow['success'] or flow['restarts'] != 2:
        raise ValueError('Incomplete restart flow: ' + flow.get('error', ''))
    experiment = Path(flow['experiment'])
    spec = importlib.util.spec_from_file_location('menu_events', ROOT / 'tools/test-menu-observer.py')
    decoder = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(decoder)
    rows = decoder.decode((experiment / 'events.bin').read_bytes())
    captured = [r for r in rows if r['event'] == 32]
    loads = [r for r in rows if r['event'] == 35]
    if len(captured) != 1 or captured[0]['result'] != 1 or len(loads) != 2:
        raise ValueError('Missing unique initial checkpoint or repeated original loads')
    if any(r['argument'] != captured[0]['argument'] or r['result'] != 1 for r in loads):
        raise ValueError('Original World clock was not reset to the checkpoint')
    actions = [(r['menu_id'], r['argument']) for r in rows if r['event'] == 3]
    if actions != [(3, 0), (18, 65538), (18, 0), (17, 0), (17, 0)]:
        raise ValueError('Unexpected callback dispatches: ' + str(actions))
    raw = (experiment / 'channel.bin').read_bytes()
    if raw[:16] != b'MNMMCM23' + struct.pack('<II', 23, 589824):
        raise ValueError('Missing V23 restart channel')
    if struct.unpack_from('<I', raw, 144)[0] != 5:
        raise ValueError('Wrong once-only restart acknowledgement count')
    from campaign_checkpoint import decode, compare
    snapshots=decode((experiment/'checkpoint-events.bin').read_bytes())
    if [s['screen'] for s in snapshots]!=[13,12,12]:
        raise ValueError('Expected checkpoint creation and two Load snapshots')
    comparisons=[compare(snapshots[0],s) for s in snapshots[1:]]
    if not all(c['verified'] for c in comparisons):
        raise ValueError('Checkpoint creature HP/mana/positions did not restore: '+str(comparisons))
    checkpoint = experiment / 'game/Save/__mnm_restart/level.sav'
    return dict(flow=flow, creature_restoration=comparisons, checkpoint=dict(path=str(checkpoint), sha256=sha(checkpoint)),
                original_clock=captured[0]['argument'], load_clocks=[r['argument'] for r in loads],
                callback_trace=actions, events=rows,
                artifacts={str(p): sha(p) for p in [run/'flow.json', run/'shell.log',
                           experiment/'events.bin', experiment/'channel.bin', experiment/'bridge-build.json']})


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--shell', type=Path, default=ROOT/'working/build/qt-shell/mnm-qt-shell')
    p.add_argument('--report', required=True, help='New SQLite-backed research/runtime report')
    p.add_argument('--validate-run', type=Path)
    p.add_argument('--prefix-template', type=Path, help='Stopped, hash-verified admitted DirectPlay prefix')
    a = p.parse_args()
    if not a.report.startswith('research/runtime/') or (ROOT/a.report).exists():
        p.error('A new research report path is required')
    parent = ROOT/'working/tests/campaign-restart';parent.mkdir(parents=True,exist_ok=True)
    run = a.validate_run or Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    print(run,flush=True)
    sources = ['tools/test-campaign-restart.py','apps/qt-shell/campaign_restart_test.cpp',
               'apps/qt-shell/mini_menu_widget.cpp','apps/qt-shell/mini_menu_widget.hpp',
               'apps/qt-shell/live_mini_menu_controller.cpp','apps/qt-shell/live_mini_menu_controller.hpp',
               'apps/qt-shell/live_menu_session.cpp','apps/qt-shell/live_menu_session.hpp',
               'apps/qt-shell/menu_bridge.cpp','apps/qt-shell/menu_bridge.hpp',
               'apps/qt-shell/main.cpp','apps/qt-shell/CMakeLists.txt','tests/menu-campaign-restart-test.cpp']
    report = dict(schema=1,success=False,kind='campaign_restart_live',run_directory=str(run),
                  sources={name:sha(ROOT/name) for name in sources},
                  scope='Fresh Celtic Adept entry; No then Yes twice, original World clock reset, fresh resident presentation. Bounded termination; older saves, progressed maps, full save-state/pixel equivalence and normal exit remain separate.')
    server = shell = None
    try:
        if not a.validate_run:
            subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
            with (run/'display').open('w+') as display,(run/'xvfb.log').open('w') as log:
                server=subprocess.Popen(['Xvfb','-displayfd',str(display.fileno()),'-screen','0','1280x1024x24','-nolisten','tcp'],pass_fds=(display.fileno(),),stdout=log,stderr=subprocess.STDOUT)
                deadline=time.monotonic()+10
                while time.monotonic()<deadline:
                    display.seek(0);number=display.read().strip()
                    if number:break
                    if server.poll() is not None:raise ValueError('Private X display failed')
                    time.sleep(.1)
                else:raise ValueError('Private X display deadline')
                env=dict(os.environ,DISPLAY=':'+number,QT_QPA_PLATFORM='xcb',WINEDEBUG='-all',
                         XDG_CONFIG_HOME=str(run/'config'),MNM_CAMPAIGN_RESTART_TEST='1',
                         MNM_RESTART_TEST_REPORT=str(run/'flow.json'),MNM_VERIFY_ORIGINALS='0',
                         LIBGL_ALWAYS_SOFTWARE='1',MNM_CACHE_ADAPTERS='1')
                env.pop('WAYLAND_DISPLAY',None)
                if a.prefix_template:
                    from native_prefix_template import clone
                    report['prefix_template']=clone(a.prefix_template,run/'wineprefix',env)
                with (run/'shell.log').open('w') as output:
                    shell=subprocess.Popen([str(a.shell.resolve()),'--repo',str(ROOT),'--live-menus',
                        '--native-commands','--resident-world','--skip-movies','--legacy-audio','--legacy-media','--no-native-music',
                        '--live-menu-test',str(run/'flow.json')],cwd=ROOT,env=env,stdout=output,stderr=subprocess.STDOUT,start_new_session=True)
                    if shell.wait(timeout=190):raise ValueError('Live restart flow failed; inspect '+str(run/'shell.log'))
        report.update(assess(run));report['success']=True
        metadata=json.loads((Path(report['flow']['experiment'])/'bridge-build.json').read_bytes())
        report['sources'].update(metadata['compiled_sources'])
    except Exception as error:
        report['error']=str(error)
    finally:
        if shell is not None and shell.poll() is None:
            os.killpg(shell.pid,signal.SIGTERM)
            try:shell.wait(timeout=5)
            except subprocess.TimeoutExpired:os.killpg(shell.pid,signal.SIGKILL);shell.wait()
        if server is not None:
            server.terminate()
            try:server.wait(timeout=5)
            except subprocess.TimeoutExpired:server.kill();server.wait()
        if not a.validate_run:
            verified=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True)
            report['original_manifest_after']=verified.returncode==0
            if verified.returncode:report['success']=False;report['manifest_error']=verified.stdout[-2000:]+verified.stderr[-2000:]
        report['sources_stable']=all(sha(ROOT/name)==digest for name,digest in report['sources'].items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],error=report.get('error'),report=a.report,sha256=digest)),flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
