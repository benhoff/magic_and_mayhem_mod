#!/usr/bin/env python3
"""Bounded original-game observation of the opt-in private native output profile."""
import argparse
from coverage_evidence import write_report
import hashlib
import importlib.util
import json
import os
import secrets
from pathlib import Path
import signal
import subprocess
import tempfile
import time
from native_prefix_template import clone

ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--functional-lifecycle',action='store_true',help='Require both stress phases, original Mini/No/Yes callbacks,64balanced Worlds and complete resource retirement; FPS is diagnostic')
    parser.add_argument('--native-only',action='store_true',help='Exercise the strict native-only startup/failure/outcome contract')
    parser.add_argument('--restart',action='store_true',help='Inject a live producer failure, use public Hard restart, then quit a fresh generation')
    parser.add_argument('--independent-world',action='store_true',help='Use and verify the dedicated render worker in a native-only restart journey')
    parser.add_argument('--pending-cpu-read-fault',action='store_true',help='Require an original pending CPU-read witness during dedicated-worker restart')
    parser.add_argument('--prefix-template',type=Path)
    parser.add_argument('--player-entrypoint',choices=['fresh','reuse'],help='Exercise run-x86.sh with the real default persistent player prefix')
    parser.add_argument('--source-game',type=Path,required=True)
    parser.add_argument('--stress-seconds',type=int,choices=[0,*range(10,121)],default=0)
    parser.add_argument('--qt-build-type',choices=['Release','RelWithDebInfo'],default='RelWithDebInfo',help='Explicit native shell configuration retained in the run record')
    parser.add_argument('--file-trace',action='store_true',help='Diagnostic Wine file-API trace; changes timing and supplies no performance acceptance')
    parser.add_argument('--exception-trace',action='store_true',help='Diagnostic Wine SEH trace; changes timing and supplies no performance acceptance')
    parser.add_argument('--no-native-music',action='store_true',help='Exercise rendering without requiring a prepared CD music catalog')
    parser.add_argument('--default-media',action='store_true',help='Leave both intros and default native music enabled and require decoded/presented completion and retired audio')
    parser.add_argument('--allow-initial-silence',action='store_true',help='Admit only explicitly reported initial no-device native output, with decoded/consumed PCM still required')
    parser.add_argument('--hardware',action='store_true',help='Require the existing private NVIDIA display route and reject software fallback')
    parser.add_argument('--menu-cycles',type=int,choices=range(1,6),default=1)
    parser.add_argument('--save-load',action='store_true',help='Require the public native Save/Load journey and independent original save validation')
    parser.add_argument('--recreate-world',action='store_true',help='Also rebuild display modes and load the saved map from Main')
    parser.add_argument('--quick-map-row',type=int,choices=range(32),help='Select this offered map through the public Quick Battle list, then exercise two World visits and normal Quit')
    parser.add_argument('--copy-census',action='store_true',help='Diagnostic all-copy caller census and independent GPU destination checks; record missing natural branches separately')
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--report',type=Path,required=True)
    args=parser.parse_args()
    if args.pending_cpu_read_fault and not args.independent_world:parser.error('Pending CPU fault requires the isolated dedicated-worker restart journey')
    if args.independent_world and (not args.native_only or not args.restart or args.player_entrypoint):parser.error('Independent World requires the isolated native-only restart journey')
    if args.recreate_world:args.save_load=True
    if args.default_media and args.no_native_music:parser.error('Default media cannot disable native music')
    if args.allow_initial_silence and not args.default_media:parser.error('Initial silence admission requires default media validation')
    if args.save_load and (args.restart or args.functional_lifecycle or args.stress_seconds or args.menu_cycles!=1):parser.error('Save/Load is a separate journey from restart and physical stress')
    if args.quick_map_row is not None and (args.default_media or args.save_load or args.restart or args.functional_lifecycle or args.stress_seconds or args.menu_cycles!=1):parser.error('Quick maps use their separate bounded public journey')
    if args.functional_lifecycle and args.menu_cycles!=1:parser.error('The strict functional policy requires one menu cycle')
    if args.player_entrypoint:
        if not args.native_only or args.restart or args.prefix_template:parser.error('Player entrypoint requires native-only without a template or injected restart')
    elif args.prefix_template is None:parser.error('Require a prefix template for the isolated owner entrypoint')
    if args.restart and (not args.native_only or args.stress_seconds):parser.error('Restart requires native-only and a separate stress run')
    if args.functional_lifecycle:
        from campaign_functional_acceptance import load_policy
        load_policy()
    if args.functional_lifecycle and (not args.native_only or args.restart or args.stress_seconds<10):parser.error('Functional lifecycle requires native-only,10..120seconds stress and no injected restart')
    if args.report.exists():parser.error('Refusing evidence overwrite')
    declaration=json.loads(args.claims.read_text());sources=declaration['sources']
    def stable():return all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
    if not stable():raise RuntimeError('Declared ownership sources changed before the game run')
    parent=ROOT/'working/tests/native-output-owner';parent.mkdir(parents=True,exist_ok=True)
    root=Path(tempfile.mkdtemp(prefix='campaign-',dir=parent));(root/'.retain').write_text('Native output ownership actual-game observation, preserve attempts.\n')
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_game_executed=False,complete_replacement=False,ordinary_campaign_accepted=False,
        artifacts=str(root.relative_to(ROOT)),stress_seconds=args.stress_seconds,file_trace=args.file_trace,exception_trace=args.exception_trace,
        qt_build_type=args.qt_build_type,fps_policy='diagnostic',functional_lifecycle_requested=args.functional_lifecycle,
        native_music_disabled=args.no_native_music,default_media_requested=args.default_media,
        hardware_requested=args.hardware,menu_cycles=args.menu_cycles,save_load_requested=args.save_load,
        recreate_world_requested=args.recreate_world,quick_map_row_requested=args.quick_map_row,
        pending_cpu_read_fault_requested=args.pending_cpu_read_fault,render_owner_requested='dedicated-thread' if args.independent_world else 'gui-resident',scope=__doc__)
    journey_seconds=max(270,240+(args.menu_cycles+1)*args.stress_seconds+args.menu_cycles*30)+ (120 if args.default_media else 0)
    env={k:v for k,v in os.environ.items() if not k.startswith(('MNM_','WINE'))}
    env.update(XDG_CONFIG_HOME=str(root/'config'),PYTHONDONTWRITEBYTECODE='1',TMPDIR=str(root),LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='4',
        QT_QPA_PLATFORM='xcb',QT_XCB_GL_INTEGRATION='xcb_glx',WINEDEBUG='-all,+seh' if args.exception_trace else '-all,+file' if args.file_trace else '-all',
        MNM_CAMPAIGN_SMOKE_TEST='1',MNM_CAMPAIGN_DIFFICULTY='1',MNM_CAMPAIGN_NORMAL_QUIT='1',
        MNM_NO_ROUTE_SCREENSHOTS='1',MNM_CAMPAIGN_STRESS_SECONDS=str(args.stress_seconds),MNM_CAMPAIGN_MENU_CYCLES=str(args.menu_cycles),
        MNM_CAMPAIGN_TIMEOUT_MS=str(journey_seconds*1000),MNM_GLYPH_BACKEND='takeover',MNM_CACHE_ADAPTERS='1',
        MNM_MENU_GAME_SOURCE=str(args.source_game.resolve()),MNM_QT_BUILD_TYPE=args.qt_build_type)
    if args.save_load:
        env.update(MNM_CAMPAIGN_STORAGE='1',MNM_CAMPAIGN_STORAGE_NAME='native-render-'+secrets.token_hex(6),
            MNM_CAMPAIGN_RECREATE_WORLD=str(int(args.recreate_world)))
        if args.recreate_world:env['MNM_MENU_DISPLAY_OBSERVE']='1'
    if args.quick_map_row is not None:
        env.pop('MNM_CAMPAIGN_SMOKE_TEST')
        env.update(MNM_LIVE_MENU_TEST_QT_JOURNEY='1',MNM_QUICK_BATTLE_MAP_ROW=str(args.quick_map_row))
    if args.copy_census:
        env.update(MNM_NATIVE_COPY_DIAGNOSTICS=str(root/'copy-diagnostics'),MNM_NATIVE_COPY_DIAGNOSTIC_SCOPE='all')
    if args.pending_cpu_read_fault:
        env.update(MNM_RESIDENT_PENDING_CPU_TEST='1',MNM_NATIVE_CPU_READER_JOURNAL='1',MNM_ACCEPTANCE_DIAGNOSTIC='1',MNM_ACCEPTANCE_PILOT='1')
    if args.restart:
        env.pop('MNM_CAMPAIGN_SMOKE_TEST');env['MNM_RESIDENT_RESTART_TEST']='1'
    server=child=None;verified=False;owner=root/'owner'
    expected_prefix=root/'wineprefix'
    previous_owners=set(parent.glob('run-*'))
    if args.player_entrypoint:
        env['TMPDIR']=os.environ.get('TMPDIR',str(root))
        owner=None;expected_prefix=ROOT/'working/native-player/wineprefix'
        if expected_prefix.exists() != (args.player_entrypoint=='reuse'):parser.error('Player prefix state differs from the requested fresh/reuse control')
        report['player_entrypoint']=args.player_entrypoint
        report['player_prefix']=str(expected_prefix)
        report['player_marker_before']=hashlib.sha256((expected_prefix/'.mnm-directplay-dx6.json').read_bytes()).hexdigest() if expected_prefix.exists() else None
    try:
        with (root/'original-before.log').open('x') as out:
            subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=out,stderr=subprocess.STDOUT,check=True)
        verified=True
        spec=importlib.util.spec_from_file_location('owned_launcher',ROOT/'tools/run-native-owned.py');launcher=importlib.util.module_from_spec(spec);spec.loader.exec_module(launcher)
        campaign_spec=importlib.util.spec_from_file_location('owned_campaign_contract',ROOT/'tools/test-native-campaign.py')
        campaign=importlib.util.module_from_spec(campaign_spec);campaign_spec.loader.exec_module(campaign)
        host=root/'host';host.mkdir()
        if args.hardware:server=campaign.start_native_hardware(host,env)
        else:
            server,receipt,_=launcher.start_display(host,allow_glx=True)
            env.update(DISPLAY=receipt['display'],XAUTHORITY=receipt['authority'])
        if args.player_entrypoint:
            command=[str(ROOT/'run-x86.sh'),'--']
        else:
            report['prefix_template']=clone(args.prefix_template,expected_prefix,env)
            command=[str(ROOT/'tools/run-native-owned.py'),'--prefix',str(expected_prefix),'--output',str(owner),*(['--native-only'] if args.native_only else []),'--']
        command += [
                 *([] if args.default_media else ['--skip-movies']),*(['--no-native-music'] if args.no_native_music else []),
                 *(['--independent-world'] if args.independent_world else []),
                 '--live-menu-test',str(root/'flow.json')]
        with (root/'shell.log').open('x') as out:
            child=subprocess.Popen(command,cwd=ROOT,env=env,stdout=out,stderr=subprocess.STDOUT,start_new_session=True)
            deadline=time.monotonic()+journey_seconds+(330 if args.restart or args.player_entrypoint else 30)
            while time.monotonic()<deadline:
                if args.player_entrypoint and owner is None:
                    created=set(parent.glob('run-*'))-previous_owners
                    if len(created)>1:raise RuntimeError('Ambiguous new player owner directories')
                    if created:owner=created.pop();report['player_owner']=str(owner.relative_to(ROOT))
                journals=sorted((owner/'sessions').glob('*.json'),key=lambda p:p.stat().st_mtime_ns) if owner is not None else []
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
        if owner is None:raise RuntimeError('Player did not create an owned session')
        admitted=[json.loads(p.read_text()) for p in (owner/'sessions').glob('*.json')]
        if len(admitted)!=(2 if args.restart else 1):raise RuntimeError('Actual game ownership admission is missing or ambiguous')
        if any(a['prefix']!=str(expected_prefix) for a in admitted):raise RuntimeError('Ownership prefix differs from the declared run')
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
            report['engine_journey']=validate_flow(flow,expected_owner='dedicated-thread' if args.independent_world else None,
                expected_stream_version=5 if args.independent_world else None,pending_cpu=args.pending_cpu_read_fault)
            if args.independent_world and report['resident'].get('native_render_owner')!='dedicated-thread':
                raise RuntimeError('Final resident report did not confirm the dedicated render owner')
        elif args.save_load:
            from campaign_storage import validate_storage,validate_storage_flow,validate_mini_dispatch,validate_mini_inputs
            if bool(flow.get('recreate_world'))!=args.recreate_world:raise ValueError('Requested display/map recreation was not executed')
            validate_storage_flow(flow)
            report['storage']=validate_storage(session,flow,1,root)
            report['mini_dispatch']=validate_mini_dispatch(flow)
            report['mini_inputs']=validate_mini_inputs(root,flow)
        elif args.quick_map_row is not None:
            from native_player_quick_acceptance import validate_quick
            report['quick_map']=validate_quick(session,flow,args.quick_map_row)
        else:
            report['engine_journey']=campaign.validate_events(session,cycles=args.menu_cycles,difficulty=1,normal_quit=True)
            campaign.validate_flow(flow,cycles=args.menu_cycles,stress_seconds=args.stress_seconds,min_fps=None,normal_quit=True)
        report['original_process']=json.loads((session/'original-process-result.json').read_text())
        resident=report['resident'];report['original_game_executed']=int(resident['completed'])>0
        fallback_key='native_fallback' if args.quick_map_row is not None else 'native_command_fallback'
        if code or flow.get('success') is not True or (not args.restart and flow.get(fallback_key) is not False):
            raise RuntimeError(flow.get('error') or 'Native ownership flow did not complete')
        if resident['error'] or not resident['normal_quit_retired'] or resident['pending'] or not (
                0<int(resident['completed'])==int(resident['imports'])==int(resident['verified_returns'])):
            raise RuntimeError('Native ownership World retirement was not balanced')
        if report['original_process']['original_exit_code']!=0:raise RuntimeError('Original game did not exit normally')
        if args.hardware and ('NVIDIA' not in resident['renderer'] or any(x in resident['renderer'].lower() for x in ('llvmpipe','lavapipe','softpipe'))):raise ValueError('Resident World did not use NVIDIA hardware')
        if args.default_media:
            from native_media_acceptance import validate_media
            from native_music_launch import validate_activity,catalog_inputs
            from native_audio_launch import validate_activity as validate_audio,validate_retirement
            report.update(validate_media(session,native_factory=True,allow_initial_silence=args.allow_initial_silence,require_intro_completion=True))
            report['native_music']=validate_activity(json.loads((session/'native-music-report.json').read_text()),allow_initial_silence=args.allow_initial_silence)
            manifest=json.loads((session/'manifest.json').read_text());inputs,toc=catalog_inputs(session/'native-music')
            if inputs!=manifest['music_inputs'] or [list(x) for x in toc]!=manifest['music_toc']:raise ValueError('Native music staged inputs changed')
            report['native_music_inputs']=inputs
            report['native_audio']=validate_audio(json.loads((session/'native-audio-report.json').read_text()),native_wave=True,native_factory=True,allow_initial_silence=args.allow_initial_silence)
            report['native_audio_retirement']=validate_retirement(session/'native-voices.bin',report['native_audio'])
        compiled=campaign.linked_compiler_sources(ROOT/'working/build/qt-shell',ROOT)
        if any(path not in sources for path in compiled):raise ValueError('Undeclared actual shell compiler input')
        report['native_compiler_inputs_checked']=len(compiled)
        if args.copy_census:
            from native_copy_routes import validate as validate_routes_census
            from native_gpu_copy_diagnostics import validate as validate_gpu_census
            try:
                report['copy_routes']=validate_routes_census(session)
                report['gpu_copy']=validate_gpu_census(session,root/'copy-diagnostics')
                report['copy_census_verified']=True
                from natural_gpu_copy_branches import analyze
                report['natural_gpu_callers']=analyze(session,root/'copy-diagnostics')
            except (OSError,ValueError,KeyError) as error:
                report['copy_census_verified']=False
                report['copy_census_error']=str(error)
            # Caller census failure has no effect on this separate player
            # lifecycle result and supplies no positive/absence copy claim.
        from native_draw_retirement import require_retired, read_startup_retirement
        report['graphics_retirement']=require_retired(session/'native-draw-retirement.bin')
        report['graphics_retirement']['startup_probe']=read_startup_retirement(session/'game/native-draw-startup-retirement.bin')
        if args.native_only:
            owner_result=json.loads((owner/'launch-result.json').read_text())
            if owner_result.get('success') is not True or not owner_result.get('native_only_sessions'):
                raise RuntimeError('Native-only launch did not accept its session outcomes')
            report['native_only_outcomes']=owner_result['native_only_sessions']
        if args.player_entrypoint:
            marker=expected_prefix/'.mnm-directplay-dx6.json'
            report['player_marker_after']=hashlib.sha256(marker.read_bytes()).hexdigest()
            if args.player_entrypoint=='reuse' and report['player_marker_after']!=report['player_marker_before']:raise RuntimeError('Reused prefix ownership changed')
            report['actual_prefix_initialized']=args.player_entrypoint=='fresh'
            report['actual_player_entrypoint']=True
        if args.stress_seconds:
            campaign.validate_flow(flow,cycles=args.menu_cycles,stress_seconds=args.stress_seconds,min_fps=None,normal_quit=True)
            from campaign_camera_dialogue import validate_camera_mini
            report['mini_readiness']=[validate_camera_mini(json.loads((root/f'mini-input-{i}.json').read_text()),root,True,session/'events.bin',4 if i<args.menu_cycles else 3) for i in range(args.menu_cycles+2)]
            report['physical_stress']=[]
            for i in range(args.menu_cycles+1):
                path=root/f'input-{i}.json';inputs=json.loads(path.read_text());actions=inputs.get('actions',[])
                if inputs.get('success') is not True or inputs.get('seconds',0)<args.stress_seconds or len(actions)<20:raise ValueError('Incomplete physical stress')
                report['physical_stress'].append(dict(verified=True,seconds=inputs['seconds'],actions=len(actions),sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        report['success']=True
    except Exception as error:report['error']=str(error)
    finally:
        if child is not None and child.poll() is None:
            os.killpg(child.pid,signal.SIGTERM)
            try:child.wait(timeout=15)
            except subprocess.TimeoutExpired:os.killpg(child.pid,signal.SIGKILL);child.wait(timeout=10)
        if server is not None and server.poll() is None:
            if args.hardware:campaign.stop_group(server)
            else:server.terminate();server.wait(timeout=10)
        if verified:
            with (root/'original-after.log').open('x') as out:
                result=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],stdout=out,stderr=subprocess.STDOUT)
            report['original_verified_after']=result.returncode==0
            if result.returncode:report['success']=False
        report['sources_stable']=stable();report['success']=report['success'] and report['sources_stable']
        if args.functional_lifecycle and report['success']:
            try:
                from campaign_functional_acceptance import validate_strict_functional
                report['functional_acceptance']=validate_strict_functional(report)
            except (KeyError,TypeError,ValueError) as error:report.update(success=False,error=str(error))
        write_report(ROOT,args.report,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps({key:report.get(key) for key in ('success','sources_stable','error','session')}))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
