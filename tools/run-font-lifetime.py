#!/usr/bin/env python3
"""Bounded installed font/DC lifecycle with two public Quick Worlds and Main reloads."""
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
from font_lifetime_journey import FontJourney, menus

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/font-dc-lifetime-results-20261108/'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--plan', required=True)
    parser.add_argument('--software', action='store_true')
    parser.add_argument('--size', choices=('640x480', '800x600'), required=True)
    parser.add_argument('--pilot-review');parser.add_argument('--repair-of')
    args = parser.parse_args()
    require(args.report.startswith(PREFIX) and not (ROOT/args.report).exists(), 'Fresh font/DC report required')

    require(args.plan.startswith(PREFIX)and '..'not in Path(args.plan).parts,'Owned prospective plan required')
    out = ROOT/'working/pilot'/Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out/'.retain').write_text('Installed font/DC input/output and every negative attempt. Preserve.\n')
    build = (ROOT/'working/build/qt-shell').resolve()
    build_commands=[]
    require((build/'mnm-qt-shell').is_file(),'Prepare the tested player before freezing this acquisition')
    campaign = menus.module('test-native-campaign.py')
    linked = set(campaign.linked_compiler_sources(build, ROOT))
    names = set(linked) | set(native_runtime_source_paths(ROOT)) | {
        'tools/check_inputs.py','tools/native_music_launch.py','tools/check-installed-glyph-kernels.py','tools/run-font-lifetime.py', 'tools/prepare-font-lifetime.py','tools/font_lifetime_journey.py','tools/palette_cycle_journey.py',
        'tools/scene_attachment_journey.py', 'tools/palette_observation.py',
        'tools/font_native_lifetime.py','tools/font_lifetime_observation.py','tools/font_lifetime_acceptance.py','tools/resident_capture.py','tools/check-font-lifetime.py','tools/palette_accessibility.py',
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
        'tools/rendering_progression.py', 'tools/rendering_build_binding.py', 'tools/cpu_reader_journal.py', 'tools/font_lifetime_observation.py', 'tools/run-rendering-copy-branches.py'}
    sources = {n: menus.sha(ROOT/n) for n in sorted(names)}
    result = dict(schema=1, kind='public_native_normal_acquisition', success=False,
                  scenario='normal-font-dc', player_accepted=False, sources=sources,
                  source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  session_identity=secrets.token_hex(16), original_game_runs=0, charged_original_games=0,
                  configuration=dict(software_flag=args.software, skip_intros=False,
                      requested_size=args.size, terrain_lighting='native', source_chunks=True,
                      viewport_capture_policy='campaign-lossless-viewport-v1',
                      viewport_scaling='integer-sharp', scene_actor_observation=2,font_observation=1,cpu_reader_journal=1,metadata_sidebands_only=True),
                  original_pixels_used_as_native_inputs=False, original_reference_executed=False)
    hs = ls = child = journey = lease = access_bus = None
    before = False
    preferences=ROOT/'working/game-nocd/CFG/prefs.cfg';initial_preferences=preferences.read_bytes()
    def deadline(signum,frame):raise TimeoutError('Frozen600second font case deadline')
    signal.signal(signal.SIGALRM,deadline)
    initial_experiments = set((ROOT/'working/experiments/menu-observer').glob('run-*'))
    try:
        with Reader(ROOT) as reader:
            plan=reader.json(args.plan)
            require(plan['success']is True and (plan['maximum_launch_attempts'],plan['maximum_actual_game_starts'])in((9,7),(10,8)),'Prospective font/DC plan missing')
            require(all(reader.digest(n)==h for n,h in plan['sources'].items()),'Font/DC plan sources changed')
            amendment=reader.json(plan['budget_amendment']['record']);require(reader.digest(plan['budget_amendment']['record'])==plan['budget_amendment']['sha256']and amendment['authorized']and amendment['maximum_launch_attempts']==plan['maximum_launch_attempts'] and amendment['maximum_actual_game_starts']==plan['maximum_actual_game_starts'],'Explicit user amendment for these prospective caps missing')
            for field in ('controls','dc_controls','hidden_controls','native_font_controls'):
                ref=plan[field];value=reader.json(ref['record']);require(reader.digest(ref['record'])==ref['sha256']and value['success']is True and value['sources_stable']and all(reader.digest(n)==h for n,h in value['sources'].items()),'Font prerequisite missing: '+field)
            controls=reader.json(plan['controls']['record']);require(all(reader.digest(n)==h for n,h in controls['sources'].items()),'Current font/DC controls changed')
            require((not args.repair_of and not plan.get('repair_of'))or args.repair_of==plan.get('repair_of',{}).get('record'),'Unfrozen repair identity')
            from check_inputs import paths
            from native_music_launch import catalog_inputs
            music=paths(ROOT).get('assets');require(music is not None,'Verified music catalog missing before player startup')
            inputs,toc=catalog_inputs(music);require(inputs==plan['music_inputs'],'Frozen music input differs')
            result['music_inputs']=inputs
            result['input_plan']=dict(record=args.plan,sha256=reader.digest(args.plan))
            require(plan['domains']and dict(software=args.software,size=list(map(int,args.size.split('x'))))in plan['domains'],'Unfrozen font domain')
            require(plan['player_binary']['sha256']==menus.sha(build/'mnm-qt-shell'),'Frozen player binary changed')
            with sqlite3.connect(ROOT/'working/font-dc-attempts.sqlite3')as db:
                db.execute('CREATE TABLE IF NOT EXISTS attempts(id INTEGER PRIMARY KEY,report TEXT UNIQUE,kind TEXT,started_ns INTEGER,plan_sha256 TEXT)')
                prior=list(db.execute('SELECT report FROM attempts'))
                require(len(prior)<plan['maximum_launch_attempts'],'Font request budget exhausted')
                previous=[reader.json(v[0])for v in prior]
                require(all(type(v.get('original_game_runs'))is int or v.get('charged_original_games')==1 for v in previous)and sum(v['original_game_runs']if type(v.get('original_game_runs'))is int else 1 for v in previous)<plan['maximum_actual_game_starts'],'Font conservative actual-start budget exhausted')
                require(not prior or all(v['success']or row[0]in plan.get('reviewed_failures',[])for row,v in zip(prior,previous)),'Stop after unreviewed failed font acquisition')
                reviewed_incomplete=plan.get('reviewed_incomplete_pilots',[])
                if any(v['success'] and row[0] not in reviewed_incomplete for row,v in zip(prior,previous)):
                    require(args.pilot_review,'Positive pilot review required before expansion')
                    review=reader.json(args.pilot_review);require(review['success']and review['font_lifetimes_verified']and review['independent_output_verified'] and review.get('native_menu_fonts',{}).get('verified'),'Pilot font/output review missing')
        binary=build/'mnm-qt-shell';record=args.report.removesuffix('.json')+'-player-binary.bin';digest=write_report(ROOT,record,binary.read_bytes())
        result['player_build']=dict(commands=build_commands,binary=dict(path='working/build/qt-shell/mnm-qt-shell',record=record,sha256=digest),compiler_sources={n:sources[n]for n in sorted(linked)})
        subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True); before = True
        menus.module('run-opengl-game.py').patch_staged_preferences(ROOT/'working/game-nocd','VIDEO',{'IsHighRes':'FALSE' if args.size=='640x480' else 'TRUE'})
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
                   MNM_SCENE_ACTORS='1', MNM_CAMPAIGN_CASTING_COMBAT='1',MNM_CAMPAIGN_INPUT_ADMISSION='1',MNM_FONT_LIFETIME='1',MNM_NATIVE_CPU_READER_JOURNAL='1',MNM_ACCEPTANCE_PILOT='1')
        require(env.get('DBUS_SESSION_BUS_ADDRESS'), 'Run font/DC controls in a dedicated dbus-run-session')
        from palette_accessibility import AccessibilityBus
        access_bus=AccessibilityBus(out,env);env=access_bus.environment;result['accessibility_bus']=access_bus.record
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
            result['player_pid']=child.pid
            os.environ.update(DISPLAY=env['DISPLAY'], XAUTHORITY=env['XAUTHORITY'])
            os.environ['AT_SPI_BUS_ADDRESS']=env['AT_SPI_BUS_ADDRESS']
            journey = FontJourney(child, out, env, map(int, args.size.split('x')))
            deadline = time.monotonic()+20
            while time.monotonic() < deadline:
                if any(v['name'] == 'Launch game' for n, v in journey.access.nodes()) and journey.input.find_window(): break
                require(child.poll() is None, 'Qt host failed to open'); time.sleep(.1)
            require(child.poll()is None and any(v['name']=='Launch game'for n,v in journey.access.nodes()),'Public launch UI did not become ready')
            journey.prepare_ui()
            budget_path=ROOT/'working/font-dc-attempts.sqlite3'
            with sqlite3.connect(budget_path)as db:
                db.execute('BEGIN IMMEDIATE')
                require(db.execute('SELECT COUNT(*) FROM attempts').fetchone()[0]<plan['maximum_launch_attempts'],'Font task request cap exhausted')
                db.execute('INSERT INTO attempts(report,kind,started_ns,plan_sha256)VALUES(?,?,?,?)',(args.report,'native-font-lifetime-'+str((args.software,args.size)),time.monotonic_ns(),result['input_plan']['sha256']))
                result['font_attempt']=db.execute('SELECT last_insert_rowid()').fetchone()[0];db.commit()
            result['player_started_host_ns']=time.monotonic_ns()
            signal.alarm(600)
            result['charged_original_games'] = 1
            result['font_lifetime_route'] = journey.run()
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
            from scene_effects import module
            module('run-rendering-copy-branches').close_after_retirement(child,journey.input);result['exit_code']=child.returncode
            require(child.returncode == 0 and result['font_lifetime_route']['success'] is True,
                    'Actual ordinary font/DC case did not complete')
        result['success'] = True
    except Exception as error: result['error'] = str(error)
    finally:
        signal.alarm(0)
        preferences.write_bytes(initial_preferences)
        if journey:
            try:result['public_launch_log']=journey.access.launch_log()
            except Exception:pass
            result.update(states=journey.states, actions=journey.actions, screenshots=journey.screenshots)
            result['font_lifetime_route'] = journey.proof
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
        if access_bus:
            access_bus.close();result['accessibility_bus']['stopped']=True
        for server in (hs, ls):
            if server and server.poll() is None: server.terminate(); server.wait(timeout=10)
        if before:
            try:
                subprocess.run([ROOT/'tools/original-manifest.sh', 'verify'], check=True)
                result['original_manifest_verified_before_after'] = True
            except Exception as error: result.update(success=False, original_verification_error=str(error))
        admissions=list((out/'owner/sessions').glob('*.json'));result['admitted_original_sessions']=len(admissions);result['original_game_runs']=len(admissions)if admissions else(None if result['charged_original_games']else 0)
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
