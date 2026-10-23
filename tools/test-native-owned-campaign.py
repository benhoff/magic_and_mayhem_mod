#!/usr/bin/env python3
"""Bounded original-game observation of the opt-in private native output profile."""
import argparse
from coverage_evidence import write_report
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time
from native_prefix_template import clone

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-only',action='store_true',help='Exercise the strict native-only startup/failure/outcome contract')
    parser.add_argument('--restart',action='store_true',help='Inject a live producer failure, use public Hard restart, then quit a fresh generation')
    parser.add_argument('--prefix-template',type=Path,required=True)
    parser.add_argument('--source-game',type=Path,required=True)
    parser.add_argument('--stress-seconds',type=int,choices=[0,*range(10,121)],default=0)
    parser.add_argument('--file-trace',action='store_true',help='Diagnostic Wine file-API trace; changes timing and supplies no performance acceptance')
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.restart and (not args.native_only or args.stress_seconds):parser.error('Restart requires native-only and a separate stress run')
    if args.report.exists():parser.error('Refusing evidence overwrite')
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable():raise RuntimeError('Declared ownership sources changed before the game run')
    parent=ROOT/'working/tests/native-output-owner';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='campaign-',dir=parent));(root/'.retain').write_text('Native output ownership actual-game observation, preserve attempts.\n')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_game_executed=False,complete_replacement=False,ordinary_campaign_accepted=False,
        artifacts=str(root.relative_to(ROOT)),stress_seconds=args.stress_seconds,file_trace=args.file_trace,scope=__doc__)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(XDG_CONFIG_HOME=str(root/'config'),PYTHONDONTWRITEBYTECODE='1',TMPDIR=str(root),LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4',
        QT_QPA_PLATFORM='xcb',QT_XCB_GL_INTEGRATION='xcb_glx',WINEDEBUG='-all,+file' if args.file_trace else '-all',
        MNM_CAMPAIGN_SMOKE_TEST='1',MNM_CAMPAIGN_DIFFICULTY='1',MNM_CAMPAIGN_NORMAL_QUIT='1',
        MNM_NO_ROUTE_SCREENSHOTS='1',MNM_CAMPAIGN_STRESS_SECONDS=str(args.stress_seconds),MNM_CAMPAIGN_MENU_CYCLES='1',
        MNM_CAMPAIGN_TIMEOUT_MS='270000',MNM_GLYPH_BACKEND='takeover',MNM_CACHE_ADAPTERS='1',
        MNM_MENU_GAME_SOURCE=str(args.source_game.resolve()))
    if args.restart:
        env.pop('MNM_CAMPAIGN_SMOKE_TEST');env['MNM_RESIDENT_RESTART_TEST']='1'
    server=child=None;verified=False
    try:
        with (root/'original-before.log').open('x') as out:
            subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=out,stderr=subprocess.STDOUT,check=True)
        verified=True
        spec=importlib.util.spec_from_file_location('owned_launcher',ROOT/'tools/run-native-owned.py');launcher=importlib.util.module_from_spec(spec);spec.loader.exec_module(launcher)
        host=root/'host';host.mkdir();server,receipt,_=launcher.start_display(host,allow_glx=True)
        env.update(DISPLAY=receipt['display'],XAUTHORITY=receipt['authority'])
        report['prefix_template']=clone(args.prefix_template,root/'wineprefix',env)
        command=[str(ROOT/'tools/run-native-owned.py'),'--prefix',str(root/'wineprefix'),'--output',str(root/'owner'),*(['--native-only'] if args.native_only else []),'--',
                 '--skip-movies','--live-menu-test',str(root/'flow.json')]
        campaign_spec=importlib.util.spec_from_file_location('owned_campaign_contract',ROOT/'tools/test-native-campaign.py')
        campaign=importlib.util.module_from_spec(campaign_spec);campaign_spec.loader.exec_module(campaign)
        with (root/'shell.log').open('x') as out:
            child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=out,stderr=subprocess.STDOUT,start_new_session=True)
            deadline=time.monotonic()+(600 if args.restart else 300)
            while time.monotonic()<deadline:
                journals=sorted((root/'owner/sessions').glob('*.json'),key=lambda p:p.stat().st_mtime_ns)
                sessions=[json.loads(p.read_text()) for p in journals]
                if len(sessions)>(2 if args.restart else 1):raise RuntimeError('Ambiguous private ownership session')
                if sessions:
                    session=Path(sessions[-1]['session'])
                    if not session.is_relative_to(ROOT/'working/experiments/menu-observer'):
                        raise RuntimeError('Unexpected private session path')
                    report['session']=str(session.relative_to(ROOT))
                    # The deliberately failed generation gets no rendering
                    # credit; validate its event order in the final flow. Apply
                    # strict reader checks to every normally completing one.
                    if not args.restart or len(sessions)>1:
                        for name in ('native-cpu-guard-fault.bin','native-cpu-guard-refusal.bin',
                                     'native-cpu-history-refusal.bin','native-cpu-transfer-refusal.bin',
                                     'native-pixel-forward-refusal.bin'):
                            fault=session/'game'/name
                            if fault.exists():raise RuntimeError('Native ownership refused: '+str(fault))
                    if campaign.world_ready(session):
                        report['original_game_executed']=True
                        (root/'flow.json.world-ready').touch(exist_ok=True)
                flow_path=root/'flow.json'
                if flow_path.exists():
                    report['flow']=json.loads(flow_path.read_text())
                    if report['flow'].get('success') is not True:
                        raise RuntimeError(report['flow'].get('error') or 'Native ownership flow failed')
                    break
                if child.poll() is not None:raise RuntimeError('Owned shell exited before writing the journey result')
                time.sleep(.2)
            else:raise RuntimeError('Native ownership journey exceeded its deadline')
            code=child.wait(timeout=30)
        report['shell_exit']=code
        flow=json.loads((root/'flow.json').read_text());report['flow']=flow
        if code or flow.get('success') is not True:
            raise RuntimeError(flow.get('error') or 'Native ownership flow did not complete')
        from native_pixel_counters import read_counters
        from pixel_route_requirements import validate_routes
        admitted=[json.loads(p.read_text()) for p in (root/'owner/sessions').glob('*.json')]
        if len(admitted)!=(2 if args.restart else 1):raise RuntimeError('Actual game ownership admission is missing or ambiguous')
        if any(a['prefix']!=str(root/'wineprefix') for a in admitted):raise RuntimeError('Ownership prefix differs from the declared run')
        if args.restart:
            final=[a for a in admitted if json.loads((Path(a['session'])/'native-only-session.json').read_text())['state']=='completed']
            if len(final)!=1:raise RuntimeError('Restart did not leave one completed final generation')
            admission=final[0]
        else:admission=admitted[0]
        session=Path(admission['session']);report['session']=str(session.relative_to(ROOT))
        report['admission']=json.loads((session/'native-output-admission.json').read_text())
        report['resident']=json.loads((session/'resident-world-report.json').read_text())
        report['routes']=validate_routes(read_counters(session/'game'))
        if args.restart:
            from test_native_restart_support import validate_flow
            report['engine_journey']=validate_flow(flow)
        else:report['engine_journey']=campaign.validate_events(session,cycles=1,difficulty=1,normal_quit=True)
        report['original_process']=json.loads((session/'original-process-result.json').read_text())
        resident=report['resident'];report['original_game_executed']=int(resident['completed'])>0
        if code or flow.get('success') is not True or (not args.restart and flow.get('native_command_fallback') is not False):
            raise RuntimeError(flow.get('error') or 'Native ownership flow did not complete')
        if resident['error'] or not resident['normal_quit_retired'] or resident['pending'] or not (
                0<int(resident['completed'])==int(resident['imports'])==int(resident['verified_returns'])):
            raise RuntimeError('Native ownership World retirement was not balanced')
        if report['original_process']['original_exit_code']!=0:raise RuntimeError('Original game did not exit normally')
        from native_draw_retirement import require_retired, read_startup_retirement
        report['graphics_retirement']=require_retired(session/'native-draw-retirement.bin')
        report['graphics_retirement']['startup_probe']=read_startup_retirement(session/'game/native-draw-startup-retirement.bin')
        if args.native_only:
            owner_result=json.loads((root/'owner/launch-result.json').read_text())
            if owner_result.get('success') is not True or not owner_result.get('native_only_sessions'):
                raise RuntimeError('Native-only launch did not accept its session outcomes')
            report['native_only_outcomes']=owner_result['native_only_sessions']
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if child is not None and child.poll() is None:
            os.killpg(child.pid,signal.SIGTERM)
            try:child.wait(timeout=15)
            except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
        if server is not None and server.poll() is None:server.terminate();server.wait(timeout=10)
        if verified:
            with (root/'original-after.log').open('x') as out:
                result=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=out,stderr=subprocess.STDOUT)
            report['original_verified_after']=result.returncode==0
            if result.returncode:report['success']=False
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({key:report.get(key) for key in ('success','sources_stable','error','session')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
