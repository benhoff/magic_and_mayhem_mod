#!/usr/bin/env python3
"""Bounded ordinary Quick Battle attachment capture using the unchanged player."""
import argparse
import hashlib
import json
import os
import secrets
import signal
import sqlite3
import subprocess
import sys
import time
from pathlib import Path

from coverage_evidence import Reader, write_report
from native_only_contract import configure, outcome
from native_prefix_template import borrow
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
from scene_attachment_journey import AttachmentJourney, menus

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/attachment-combat-results-20261107/'
PLAYER = 'research/runtime/rendering-normal-results-20261031/ac07-combat-default800-03.json'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--plan', required=True)
    parser.add_argument('--software', action='store_true')
    parser.add_argument('--size', choices=('640x480', '800x600'), required=True)
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh attachment report required')
    require(args.plan.startswith(PREFIX)and '..'not in Path(args.plan).parts,'Owned prospective plan required')
    out = ROOT/'working/pilot'/Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('Ordinary attachment input/output and every negative attempt. Preserve.\n')
    build = (ROOT/'working/build/qt-shell').resolve()
    campaign = menus.module('test-native-campaign.py')
    linked = set(campaign.linked_compiler_sources(build, ROOT))
    names = set(linked) | set(native_runtime_source_paths(ROOT)) | {
        'tools/run-scene-attachment.py', 'tools/scene_attachment_journey.py',
        'tools/run-rendering-menu-acceptance.py', 'tools/rendering_menu_accessibility.py',
        'tools/rendering_interaction_live.py', 'tools/rendering_interaction.py',
        'tools/inspect-canvas-producers.py', 'tools/scene_actor_observation.py',
        'tools/campaign_route.py', 'tools/scene_effects.py', 'tools/run-native-owned.py',
        'tools/campaign_input_observation.py',
        'tools/menu-game-runner.py', 'tools/prepare-menu-observer.py',
        'tools/native_output_owner.py', 'tools/directplay_dx6_launch.py',
        'tools/run-qt-shell.sh', 'tools/run-game.sh', 'tools/menu_preferences_store.py',
        'tools/run-native-game.py', 'run-native.sh', 'tools/run-rendering-acceptance-pilot.py',
        'tools/native_only_contract.py', 'tools/native_prefix_template.py',
        'tools/experiment_environment.py', 'tools/test-native-campaign.py',
        'tools/rendering_progression.py', 'tools/rendering_build_binding.py'}
    sources = {n: menus.sha(ROOT/n) for n in sorted(names)}
    result = dict(schema=1, kind='public_native_normal_acquisition', success=False,
                  scenario='normal-attachment', player_accepted=False, sources=sources,
                  source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  session_identity=secrets.token_hex(16), original_game_runs=0,
                  configuration=dict(software_flag=args.software, skip_intros=False,
                      requested_size=args.size, terrain_lighting='native', source_chunks=True,
                      viewport_capture_policy='campaign-lossless-viewport-v1',
                      viewport_scaling='integer-sharp', scene_actor_observation=2),
                  original_pixels_used_as_native_inputs=False, original_reference_executed=False)
    hs = ls = child = journey = lease = None
    before = False
    initial_experiments = set((ROOT/'working/experiments/menu-observer').glob('run-*'))
    try:
        with Reader(ROOT) as reader:
            plan = reader.json(args.plan)
            require(plan['success'] is True and plan['maximum_actual_games'] in (5,6,8),
                    'Prospective attachment plan missing')
            require(all(reader.digest(n) == h for n, h in plan['sources'].items()), 'Attachment plan sources changed')
            result['input_plan'] = dict(record=args.plan, sha256=reader.digest(args.plan))
            parent = reader.json(PLAYER)
            require(parent['success'] is True and parent['player_binary_stable'] is True,
                    'Unaccepted retained player build')
            binding = parent['player_build']
            binary = ROOT/binding['binary']['path']
            require(set(binding['compiler_sources']) == linked and
                    all(sources[n] == h for n, h in binding['compiler_sources'].items()) and
                    menus.sha(binary) == reader.digest(binding['binary']['record']) == binding['binary']['sha256'],
                    'Unchanged player build binding differs')
            result['player_build'] = binding
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True); before = True
        result['installation_inputs'] = {str(f.relative_to(ROOT/'working/game-nocd')):
            dict(sha256=menus.sha(f), bytes=f.stat().st_size)
            for f in sorted((ROOT/'working/game-nocd').rglob('*')) if f.is_file()}
        from rendering_progression import prepare_launch_manifest
        raw = prepare_launch_manifest(ROOT/'working/game-nocd', ROOT/'working/manifests/game-nocd.tsv')
        name = args.report.removesuffix('.json')+'-launch-manifest.bin'
        result['launch_manifest'] = dict(record=name, sha256=write_report(ROOT, name, raw))
        host, owner = out/'host', out/'owner'; host.mkdir(); owner.mkdir()
        launcher = menus.module('run-native-owned.py')
        hs, hr, _ = launcher.start_display(host, allow_glx=True)
        ls, lr, cookie = launcher.start_display(owner)
        env = configure({k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))
                         and k not in ('DISPLAY', 'XAUTHORITY','AT_SPI_BUS_ADDRESS')}, ['--software-rendering'] if args.software else [])
        env.update(DISPLAY=hr['display'], XAUTHORITY=hr['authority'],
                   MNM_NATIVE_OUTPUT_OWNER=str(owner/'owner.json'), MNM_LEGACY_DISPLAY=lr['display'],
                   MNM_LEGACY_XCOOKIE=cookie.hex(), MNM_MENU_GAME_SOURCE=str(ROOT/'working/game-nocd'),
                   MNM_MENU_DIRECTPLAY_DX6='1', MNM_CACHE_ADAPTERS='1', MNM_VERIFY_ORIGINALS='0',
                   MNM_VERIFY_ASSETS='0', MNM_TERRAIN_LIGHTING='native', QT_QPA_PLATFORM='xcb',
                   QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_ACCESSIBILITY='1', QT_SCALE_FACTOR='1',
                   LP_NUM_THREADS='4', WINEDEBUG='-all', XDG_CONFIG_HOME=str(out/'config'), TMPDIR=str(out),
                   MNM_ACCEPTANCE_NORMAL='1', MNM_ACCEPTANCE_CAMPAIGN='1',
                   MNM_ACCEPTANCE_CAPTURE_DIRECTORY=str(out/'capture'),
                   MNM_ACCEPTANCE_CAPTURE_ID=result['session_identity'], MNM_RESIDENT_STAGE_PROFILE='1',
                   MNM_RESIDENT_PROFILE_CAPTURE=str(out/'worlds.bin'), MNM_RESIDENT_CAPTURE_SOURCE_CHUNKS='1',
                   MNM_SCENE_ACTORS='1', MNM_CAMPAIGN_CASTING_COMBAT='1',MNM_CAMPAIGN_INPUT_ADMISSION='1')
        require(env.get('DBUS_SESSION_BUS_ADDRESS'), 'Run attachment controls in a dedicated dbus-run-session')
        with (out/'accessibility-preflight.log').open('xb')as log:
            subprocess.run(['dbus-update-activation-environment','DISPLAY','XAUTHORITY'],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=10)
            subprocess.run([sys.executable,'-c',"import gi;gi.require_version('Atspi','2.0');from gi.repository import Atspi;Atspi.init();print('Dedicated accessibility bus connected',flush=True)"],env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=15)
        lease, result['prefix_pool'] = borrow(ROOT/'working/fixtures/prefix-seed', out/'wineprefix', env)
        env['MNM_OWNED_WINE_PREFIX'] = str(out/'wineprefix')
        command = [str(binary), '--repo', str(ROOT), '--renderer', 'opengl', '--live-menus',
                   '--native-commands', '--resident-world', '--native-only']
        if args.software: command.append('--software-rendering')
        result['command'] = command
        with (out/'qt.log').open('xb') as log:
            child = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log,
                                     stderr=subprocess.STDOUT, start_new_session=True)
            os.environ.update(DISPLAY=env['DISPLAY'], XAUTHORITY=env['XAUTHORITY'])
            os.environ.pop('AT_SPI_BUS_ADDRESS',None)
            journey = AttachmentJourney(child, out, env, map(int, args.size.split('x')))
            deadline = time.monotonic()+20
            while time.monotonic() < deadline:
                if any(v['name'] == 'Launch game' for n, v in journey.access.nodes()) and journey.input.find_window(): break
                require(child.poll() is None, 'Qt host failed to open'); time.sleep(.1)
            require(child.poll()is None and any(v['name']=='Launch game'for n,v in journey.access.nodes()),'Public launch UI did not become ready')
            journey.prepare_ui()
            budget_path = ROOT/'working/scene-attachment/attempts.sqlite3'
            budget_path.parent.mkdir(parents=True, exist_ok=True)
            with sqlite3.connect(budget_path) as db:
                db.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,domain TEXT,started_ns INTEGER)')
                db.execute('CREATE TABLE IF NOT EXISTS pre_game_refusals(report TEXT PRIMARY KEY,evidence_record TEXT,evidence_sha256 TEXT,reason TEXT)')
                db.execute('BEGIN IMMEDIATE')
                with Reader(ROOT)as reader:
                    for n,record,h,reason in db.execute('SELECT *FROM pre_game_refusals'):
                        require(reason and reader.digest(record)==h and reader.json(record)['original_game_runs']==0,
                                'Invalid retained zero-game certificate')
                previous=list(db.execute('SELECT report,domain FROM attempts WHERE report NOT IN(SELECT report FROM pre_game_refusals)'))
                require(len(previous)<plan['maximum_actual_games'],'Attachment scenario budget exhausted')
                with Reader(ROOT)as reader:
                    successes=[]
                    for n,domain in previous:
                        prior=reader.json(n)
                        if prior['success']is True:successes.append((n,domain));continue
                        reviewed=next((v for v in plan.get('reviewed_runtime_failures',[])if
                            v['acquisition']==dict(record=n,sha256=reader.digest(n))),None)
                        require(reviewed is not None and reader.digest(reviewed['diagnosis']['record'])==reviewed['diagnosis']['sha256'],
                                'Prior actual attachment failure lacks retained diagnosis')
                        diagnosis=reader.json(reviewed['diagnosis']['record'])
                        require(diagnosis['success']is True and diagnosis['actual_games_charged']==1 and
                                diagnosis['failed_acquisition']==reviewed['acquisition'],
                                'Prior attachment diagnosis does not preserve actual failed game')
                domain=str((args.software,args.size))
                require(all(d!=domain for n,d in successes),'Attachment domain already captured')
                db.execute('INSERT INTO attempts(report,domain,started_ns)VALUES(?,?,?)',(args.report,domain,time.monotonic_ns()))
            result['original_game_runs'] = 1
            result['attachment_route'] = journey.run()
            deadline = time.monotonic()+30
            while time.monotonic() < deadline:
                try:
                    retirement = outcome(journey.session)
                    if retirement['state'] == 'completed': break
                except (ValueError, FileNotFoundError, json.JSONDecodeError): pass
                time.sleep(.1)
            result['session'] = str(journey.session)
            result['retirement'] = outcome(journey.session)
            require(result['retirement']['state'] == 'completed', 'Normal original Quit/resources incomplete')
            result['native_admission'] = json.loads((journey.session/'native-output-admission.json').read_text())
            result['resident'] = json.loads((journey.session/'resident-world-report.json').read_text())
            child.wait(timeout=10); result['exit_code'] = child.returncode
            require(child.returncode == 0 and result['attachment_route']['success'] is True,
                    'Actual ordinary attachment case did not complete')
        result['success'] = True
    except Exception as error: result['error'] = str(error)
    finally:
        if journey:
            result.update(states=journey.states, actions=journey.actions, screenshots=journey.screenshots)
            result['attachment_route'] = journey.proof
            if journey.session: result['session'] = str(journey.session)
            try: result['final_accessibility'] = [v for n, v in journey.access.nodes()]
            except Exception: pass
            journey.input.close()
        if child and child.poll() is None:
            os.killpg(child.pid, signal.SIGTERM)
            try: child.wait(timeout=12)
            except subprocess.TimeoutExpired: os.killpg(child.pid, signal.SIGKILL); child.wait(timeout=10)
        if lease:
            try:
                if not result['success']:
                    stopped=subprocess.run(['wineserver', '-k'], env=dict(env,WINEPREFIX=str(out/'wineprefix')),timeout=10)
                    result['failed_prefix_stop_exit_code']=stopped.returncode
                subprocess.run(['wineserver', '-w'], env=dict(env, WINEPREFIX=str(out/'wineprefix')), timeout=10, check=True)
                from experiment_environment import _idle
                deadline = time.monotonic()+15
                while True:
                    try: _idle(out/'wineprefix', 'wineprefix'); break
                    except (ValueError, subprocess.TimeoutExpired):
                        if time.monotonic() >= deadline: raise
                        time.sleep(.2)
                lease.close(); result['private_wine_stopped'] = True
                result['prefix_pool']['returned'] = True
            except Exception as error: result.update(success=False, prefix_return_error=str(error))
        for server in (hs, ls):
            if server and server.poll() is None: server.terminate(); server.wait(timeout=10)
        if before:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                result['original_manifest_verified_before_after'] = True
            except Exception as error: result.update(success=False, original_verification_error=str(error))
        result['sources_stable'] = all(menus.sha(ROOT/n) == h for n, h in sources.items())
        result['compiler_closure_stable'] = set(campaign.linked_compiler_sources(build, ROOT)) == linked
        result['player_binary_stable'] = 'player_build' in result and menus.sha(ROOT/result['player_build']['binary']['path']) == result['player_build']['binary']['sha256']
        result['success'] &= result['sources_stable'] and result['compiler_closure_stable'] and result['player_binary_stable']
        experiments = set((ROOT/'working/experiments/menu-observer').glob('run-*'))-initial_experiments
        result['experiments'] = [str(v) for v in sorted(experiments)]
        candidates = [f for f in out.rglob('*') if f.is_file() and 'wineprefix' not in f.relative_to(out).parts and f.name != 'Xauthority']
        for session in experiments:
            candidates += [f for f in session.rglob('*') if f.is_file() and (
                'game' not in f.relative_to(session).parts or f.suffix.lower() in ('.dll', '.cfg', '.sav', '.ini')
                or f.name.startswith('native-') and f.suffix == '.bin')]
        result['artifacts'] = []
        for f in sorted(set(candidates)):
            limit = 2*1024**3 if f == out/'worlds.bin' else 1024**3 if f.parent == out/'capture' else 512*1024**2
            if f.is_symlink() or f.stat().st_size > limit: continue
            path = str(f.relative_to(ROOT)); name = args.report.removesuffix('.json')+'-raw-'+hashlib.sha256(path.encode()).hexdigest()[:20]+'.bin'
            raw = f.read_bytes(); digest = write_report(ROOT, name, raw)
            result['artifacts'].append(dict(path=path, record=name, sha256=digest, bytes=len(raw)))
        result['temporary_root'] = str(out)
        digest = write_report(ROOT, args.report, (json.dumps(result, indent=2)+'\n').encode())
        print(json.dumps(dict(success=result['success'], error=result.get('error'), record=args.report, sha256=digest)), flush=True)
        if result.get('private_wine_stopped') is True:
            from evidence_workspace import register_completed
            register_completed(ROOT, [out, *experiments], dict(record=args.report, sha256=digest))
    return 0 if result['success'] else 1


if __name__ == '__main__': raise SystemExit(main())
