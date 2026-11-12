#!/usr/bin/env python3
"""Finite public Campaign terminal failure and physical Hard Restart collection."""
import argparse,hashlib,importlib.util,json,os,secrets,signal,sqlite3,subprocess,sys,time
from pathlib import Path
from coverage_evidence import Reader,write_report
from native_only_contract import configure,outcome
from native_prefix_template import borrow
from rendering_pilot import require
from rendering_build_binding import native_runtime_source_paths
from rendering_transition_budget import admit
from rendering_transition_live import TransitionJourney
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-transition-recovery-results-20261107/'
PLAN=PREFIX+'input-plan-01.json'
def module(name):
 s=importlib.util.spec_from_file_location('pause_'+name.replace('-','_'),ROOT/'tools'/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
menus=module('run-rendering-menu-acceptance')
sha=menus.sha

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report', required=True)
    p.add_argument('--software', action='store_true')
    p.add_argument('--low', action='store_true')
    p.add_argument('--plan',default=PLAN)
    p.add_argument('--repair-of')
    p.add_argument('--repair-reason')
    args = p.parse_args()
    if os.environ.get('MNM_TRANSITION_BUS_OWNED')!='1':
        runtime=ROOT/'working/transition-runtime'/Path(args.report).stem
        runtime.mkdir(parents=True,mode=0o700,exist_ok=False)
        isolated=dict(os.environ,XDG_RUNTIME_DIR=str(runtime),MNM_TRANSITION_BUS_OWNED='1')
        isolated.pop('AT_SPI_BUS_ADDRESS',None)
        os.execvpe('dbus-run-session',['dbus-run-session','--',sys.executable,__file__,*sys.argv[1:]],isolated)
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT / args.report).exists(), 'Fresh owned report required')
    require(bool(args.repair_of) == bool(args.repair_reason), 'Repair needs retained failure and concrete reason')
    out = ROOT / 'working/transition-acceptance' / Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').touch()
    require(args.report.endswith('-acquisition.json'),'Acquisition report suffix required')
    size = [640, 480] if args.low else [800, 600]
    names = set(native_runtime_source_paths(ROOT)) | {'tools/run-rendering-transition.py', 'tools/rendering_transition.py', 'tools/rendering_transition_live.py', 'tools/rendering_pause_low_live.py', 'tools/pause-low-dialogue-input.py', 'tools/rendering_pause.py', 'tools/rendering_pause_live.py', 'tools/rendering_interaction.py', 'tools/run-rendering-camera-acceptance.py',
        'tools/run-rendering-menu-acceptance.py', 'tools/rendering_menu_accessibility.py', 'tools/rendering_interaction_live.py', 'tools/rendering_interaction_budget.py', 'tools/resident_capture.py', 'tools/inspect-canvas-producers.py',
        'tools/native_only_contract.py', 'tools/native_prefix_template.py', 'tools/run-native-owned.py',
        'tools/rendering_build_binding.py', 'tools/menu-game-runner.py', 'tools/menu-storage-input.py', 'tools/campaign_storage.py', 'tools/input_wire.py'}
    with Reader(ROOT) as reader:
        plan = reader.json(args.plan)
        require(plan['success'] and all(reader.digest(n) == h for n, h in plan['compiler_sources'].items()), 'Accepted player source drift')
        require(all(reader.digest(n)==h for n,h in plan['runtime_sources'].items()) and all(reader.digest(n)==h for n,h in plan['sources'].items()),'Frozen runtime/controller source drift')
        names.update(plan['compiler_sources'])
        binary = ROOT / plan['player_binary']['path']
        require(sha(binary) == plan['player_binary']['sha256'] and sha(ROOT/plan['fault_binary']['path'])==plan['fault_binary']['sha256'], 'Accepted player/fault binary differs')
    sources = {n: sha(ROOT / n) for n in sorted(names)}
    result = dict(schema=1, kind='public_native_transition_recovery_acquisition', success=False, sources=sources,
        session_identity=secrets.token_hex(16), configuration=dict(software_flag=args.software, high_resolution=not args.low,
        actual_size=size, skip_intros=False, terrain_lighting='native', compiled_test_driver_invoked=False, declared_fault='drained-Mini-producer-invalid',
        original_state_samplers=True, viewport_scaling='sharp', physical_intro_skip=True, viewport_capture_policy='campaign-lossless-viewport-v1',command_capture_policy='command-history-lossless-v1'),
        original_game_runs=0, original_pixels_used_as_native_inputs=False, player_binary=plan['player_binary'],
        compiler_sources=plan['compiler_sources'], temporary_root=str(out), scope=__doc__)
    with Reader(ROOT) as r:result['prospective_plan']=dict(record=args.plan,sha256=r.digest(args.plan))
    result.update(repair_of=args.repair_of, repair_reason=args.repair_reason)
    hs = ls = child = journey = lease = None
    before = False
    env = {}
    def deadline(signum,frame):raise TimeoutError('Frozen600second recovery case deadline')
    signal.signal(signal.SIGALRM,deadline);signal.alarm(600)
    try:
        with sqlite3.connect(ROOT/'working/transition-attempts.sqlite3') as budget:
            budget.execute('CREATE TABLE IF NOT EXISTS attempts(report TEXT PRIMARY KEY,repair_of TEXT,charged_games INTEGER)')
            budget.execute('BEGIN IMMEDIATE')
            with Reader(ROOT) as r:
                for prior in plan.get('prior_requests',[]):
                    data=r.json(prior['record']);require(r.digest(prior['record'])==prior['sha256'] and data['original_game_runs']==0 and not data['success'],'Invalid retained zero-game setup failure')
                    budget.execute('INSERT OR IGNORE INTO attempts VALUES(?,?,?)',(prior['record'],data['repair_of'],0))
                cases=[]
                for n,repair,charge in budget.execute('SELECT report,repair_of,charged_games FROM attempts'):
                    data=r.json(n);matches=[r.json(f.relative_to(ROOT)) for f in sorted((ROOT/PREFIX).glob(Path(n).name.replace('-acquisition.json','-comparison')+'*.json'))]
                    valid=[v for v in matches if v.get('acquisition')==dict(record=n,sha256=r.digest(n))]
                    cases.append(dict(report=n,repair_of=repair,result=data,proved=any(v.get('success') is True and v.get('sources_stable') is True and v.get('proof',{}).get('verified') is True for v in valid),comparison_failed=any(v.get('success') is False for v in valid)))
                result['attempt_admission']=admit(cases,args.repair_of,plan['bounds']['maximum_requests'],plan['bounds']['maximum_original_games'])
            budget.execute('INSERT INTO attempts VALUES(?,?,?)',(args.report,args.repair_of,2))
        subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
        before = True
        menus.module('run-opengl-game.py').patch_staged_preferences(ROOT / 'working/game-nocd', 'VIDEO', {'IsHighRes': 'FALSE' if args.low else 'TRUE'})
        result['installation_inputs'] = {str(f.relative_to(ROOT / 'working/game-nocd')): dict(sha256=sha(f), bytes=f.stat().st_size)
                                         for f in sorted((ROOT / 'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw = prepare_launch_manifest(ROOT / 'working/game-nocd', ROOT / 'working/manifests/game-nocd.tsv')
        n = args.report.removesuffix('.json') + '-launch-manifest.bin'
        result['launch_manifest'] = dict(record=n, sha256=write_report(ROOT, n, raw))
        host, owner = out / 'host', out / 'owner'
        host.mkdir(); owner.mkdir(); (out/'worlds').mkdir()
        launcher = menus.module('run-native-owned.py')
        hs, hr, _ = launcher.start_display(host, allow_glx=True)
        ls, lr, cookie = launcher.start_display(owner)
        env = configure({k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}, ['--software-rendering'] if args.software else [])
        env.update(DISPLAY=hr['display'], XAUTHORITY=hr['authority'], MNM_NATIVE_OUTPUT_OWNER=str(owner / 'owner.json'),
            MNM_LEGACY_DISPLAY=lr['display'], MNM_LEGACY_XCOOKIE=cookie.hex(), MNM_MENU_GAME_SOURCE=str(ROOT / 'working/game-nocd'),
            MNM_MENU_DIRECTPLAY_DX6='1', MNM_CACHE_ADAPTERS='1', MNM_VERIFY_ORIGINALS='0', MNM_VERIFY_ASSETS='0',
            MNM_TERRAIN_LIGHTING='native', QT_QPA_PLATFORM='xcb', QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_ACCESSIBILITY='1',
            LP_NUM_THREADS='4', QT_SCALE_FACTOR='1', WINEDEBUG='-all', XDG_CONFIG_HOME=str(out / 'config'),
            MNM_RESIDENT_STAGE_PROFILE='1', MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY=str(out / 'worlds'), MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',
            MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out / 'capture'), MNM_ACCEPTANCE_CAPTURE_ID=result['session_identity'],
            MNM_ACCEPTANCE_COMMAND_HISTORY='1', MNM_ACCEPTANCE_NORMAL='1', MNM_ACCEPTANCE_CAMPAIGN='1', MNM_ACCEPTANCE_GAMEPLAY='1', MNM_ACCEPTANCE_FRAME_INPUT='1',
            XDG_RUNTIME_DIR=os.environ['XDG_RUNTIME_DIR'], XDG_CACHE_HOME=str(out/'cache'),
            LIBGL_ALWAYS_SOFTWARE='1', __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
        prefix=ROOT/'working/transition-prefixes'/('wineprefix-'+Path(args.report).stem);prefix.parent.mkdir(parents=True,exist_ok=True)
        lease, result['prefix_pool'] = borrow(ROOT / 'working/fixtures/prefix-seed', prefix, env)
        env['MNM_OWNED_WINE_PREFIX'] = str(prefix)
        command = [str(binary), '--repo', str(ROOT), '--renderer', 'opengl', '--live-menus', '--native-commands', '--resident-world', '--native-only']
        if args.software:
            command.append('--software-rendering')
        result['command'] = command
        with (out / 'qt.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update({k:env[k] for k in ('DISPLAY','XAUTHORITY','XDG_RUNTIME_DIR','XDG_CONFIG_HOME','XDG_CACHE_HOME')})
            # libatspi can abort the process on a bad bus. Exercise it in a
            # disposable client before constructing the controller or Launch.
            with (out/'accessibility-preflight.log').open('xb') as preflight:
                subprocess.run([sys.executable,'-c',"import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();assert Atspi.get_desktop(0) is not None;print('private accessibility bus ready')"],env=env,stdout=preflight,stderr=subprocess.STDOUT,check=True,timeout=20)
            journey = TransitionJourney(child, out, env, size, str(ROOT/plan['fault_binary']['path']))
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                require(child.poll() is None, 'Qt host failed to open')
                if journey.input.find_window() and any(v['name'] == 'Launch game' for _, v in journey.access.nodes()):
                    break
                time.sleep(.1)
            result['proof'] = journey.run()
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                try:
                    result['retirement'] = outcome(journey.session)
                    break
                except (ValueError, FileNotFoundError):
                    time.sleep(.1)
            require(result.get('retirement', {}).get('state') == 'completed', 'Original Quit/owner retirement incomplete')
            result['resident'] = json.loads((journey.session / 'resident-world-report.json').read_text())
            result['native_admission'] = json.loads((journey.session / 'native-output-admission.json').read_text())
            for e in result['proof']['epochs']:
                e['resident']=json.loads((Path(e['root'])/'resident-world-report.json').read_bytes())
                e['outcome']=outcome(e['root'])
            child.wait(timeout=10)
            result['exit_code'] = child.returncode
            require(child.returncode == 0 and int(result['resident']['completed']) > 0, 'Native host failed normal completion')
        subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), check=True, timeout=10)
        result['private_wine_stopped'] = True
        result['success'] = True
    except Exception as error:
        result['error'] = str(error)
        if journey:
            try: journey.capture('failure-last-observation')
            except Exception as capture_error: result['failure_capture_error'] = str(capture_error)
    finally:
        signal.alarm(0)
        if journey:
            if journey.samples:
                journey.samples.close();journey.samples=None
            result.update(states=journey.states, actions=journey.actions, pause_actions=journey.pause_actions,
                          screenshots=journey.screenshots, session=str(journey.session) if journey.session else None)
            journey.input.close()
            journey.source_frames.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try: child.wait(timeout=12)
            except subprocess.TimeoutExpired: os.killpg(child.pid, signal.SIGKILL); child.wait(timeout=10)
        if lease and not result['success']:
            subprocess.run(['wineserver', '-k'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
            subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(prefix)), timeout=10)
            result['forced_private_prefix_stop'] = True
        for server in (hs, ls):
            if server and server.poll() is None:
                server.terminate(); server.wait(timeout=10)
        if lease:
            try:
                from experiment_environment import _idle
                deadline = time.monotonic() + 20
                while True:
                    try: _idle(prefix, 'wineprefix'); break
                    except (ValueError, subprocess.TimeoutExpired):
                        if time.monotonic() >= deadline: raise
                        time.sleep(.2)
                lease.close(); result['prefix_pool']['returned'] = True
            except Exception as error:
                result.update(success=False, prefix_return_error=str(error))
        if before:
            try:
                subprocess.run([ROOT / 'tools/original-manifest.sh', 'verify'], check=True)
                result['original_manifest_verified_before_after'] = True
            except Exception as error:
                result.update(success=False, original_verification_error=str(error))
        admissions=list((out/'owner/sessions').glob('*.json'))
        result['original_game_runs']=len(admissions)
        result['observed_session_roots']=[json.loads(f.read_bytes())['session'] for f in admissions]
        if journey:result['epochs_observed']=journey.epochs
        result['sources_stable'] = all(sha(ROOT / n) == h for n, h in sources.items())
        result['player_binary_stable'] = sha(binary) == plan['player_binary']['sha256']
        result['success'] &= result['sources_stable'] and result['player_binary_stable']
        candidates = [f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name != 'Xauthority']
        for root_name in result['observed_session_roots']:
            session_root=Path(root_name)
            candidates += [f for f in session_root.rglob('*') if f.is_file() and ('game' not in f.relative_to(session_root).parts
                or f.suffix.lower() in ('.dll','.cfg','.sav','.ini') or f.name.startswith('native-') and f.suffix=='.bin')]
        result['artifacts'] = []
        for f in sorted(set(candidates)):
            limit = 2 * 1024**3 if f.parent == out / 'worlds' else 1024**3 if f.parent == out / 'capture' else 512 * 1024**2
            if f.is_symlink(): continue
            require(f.stat().st_size<=limit,'Required transition artifact exceeds frozen retention bound: '+str(f))
            n = args.report.removesuffix('.json') + '-raw-' + hashlib.sha256(str(f.relative_to(ROOT)).encode()).hexdigest()[:20] + '.bin'
            raw = f.read_bytes()
            result['artifacts'].append(dict(path=str(f.relative_to(ROOT)), record=n, sha256=write_report(ROOT, n, raw), bytes=len(raw)))
        h = write_report(ROOT, args.report, (json.dumps(result, indent=2) + '\n').encode())
        print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=h,
                              original_game_runs=result['original_game_runs'], pause_dwells=len(result.get('pause_actions', [])))), flush=True)
    return 0 if result['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
