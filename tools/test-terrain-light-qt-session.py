#!/usr/bin/env python3
"""Bounded strict Qt/native-lighting campaign, original replay and lifecycle proof."""
import argparse
import fcntl
import hashlib
import importlib.util
import json
import mmap
import os
from pathlib import Path
import signal
import sqlite3
import struct
import subprocess
import time

from coverage_evidence import write_report, read_json
from coverage_claims import behavior_contract, required_sources, scenario_contract
import terrain_light_session as light

ROOT = Path(__file__).resolve().parents[1]
MODELS = ['terrain_lighting.cpp', 'terrain_creature_lighting.cpp',
          'terrain_static_lighting.cpp', 'terrain_lighting_cycle.cpp']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--prefix', type=Path, required=True)
    parser.add_argument('--mode', choices=['observe', 'native'], required=True)
    parser.add_argument('--journey', choices=['reload', 'stress'], required=True)
    parser.add_argument('--validated-observation', type=Path)
    parser.add_argument('--seconds', type=int, default=120)
    parser.add_argument('--worker-loss', action='store_true')
    args = parser.parse_args()
    out = args.output.resolve()
    prefix = args.prefix.resolve()
    if (out.exists() or args.report.exists() or not out.is_relative_to(ROOT / 'working')
            or not prefix.is_relative_to(ROOT / 'working') or not 10 <= args.seconds <= 120):
        parser.error('Use fresh owned output/report paths, a private prefix and10..120seconds')
    if (args.mode == 'native') != bool(args.validated_observation):
        parser.error('Native lighting requires current-source combined observation')
    out.mkdir(parents=True)
    (out / '.retain').write_text('Combined strict native Qt and lighting evidence; preserve raw results.\n')
    report = dict(schema=1, success=False, mode=args.mode, journey=args.journey,
                  worker_loss=args.worker_loss, seconds=args.seconds,
                  original_sha256=light.HASH, live_replacement=args.mode == 'native',
                  native_graphics=True, default_changed=False, results=[])
    reg = json.loads((ROOT / 'research/runtime/coverage/register.json').read_text())
    ids = ['TL.normal-session-lighting', 'TL.session-light-policy', 'NP.native-only-launch-contract']
    behaviors = [b for b in reg['behaviors'] if b['id'] in ids]
    builds = {b['id']: b for b in reg['builds']}
    scenario_id = 'TLQT26.' + ('worker-loss' if args.worker_loss else args.mode + '-' + args.journey)
    scenarios = [s for s in reg['scenarios'] if s['id'] == scenario_id]
    paths = set().union(*(required_sources(b) for b in behaviors))
    paths.add('tools/test-terrain-light-qt-session.py')
    for folder in ('apps', 'assets', 'audio', 'compat', 'game', 'reconstruction', 'renderer', 'runtime', 'protocols', 'tools'):
        for path in (ROOT / folder).rglob('*'):
            if path.is_file() and (path.suffix in ('.cpp', '.hpp', '.c', '.h', '.S', '.py', '.inc', '.sh') or path.name == 'CMakeLists.txt'):
                paths.add(path.relative_to(ROOT).as_posix())
    sources = {p: sha(ROOT / p) for p in sorted(paths)}
    report.update(sources=sources, claims=[dict(behavior=b['id'], contract_sha256=behavior_contract(b, builds),
        scenarios={s['id']: scenario_contract(s) for s in scenarios if b['id'] in s['behaviors']}) for b in behaviors])
    pending = {}
    def retain(name, data):
        path = 'research/runtime/terrain-light-qt-results-20261026/' + out.name + '-' + name
        pending[path] = data
        return dict(path=path, sha256=hashlib.sha256(data).hexdigest())
    def run(label, command, timeout=180):
        result = subprocess.run(list(map(str, command)), cwd=ROOT, env=env, capture_output=True, timeout=timeout)
        report.setdefault('commands', []).append(dict(label=label, exit_code=result.returncode,
            log=retain(label + '.log', result.stdout + result.stderr)))
        result.check_returncode()
        return result
    env = {k: v for k, v in os.environ.items() if not k.startswith(('MNM_', 'WINE'))}
    env.update(TMPDIR=str(ROOT / 'working/ramtmp'), XDG_CONFIG_HOME=str(out / 'config'),
        WINEPREFIX=str(prefix), WINEDEBUG='-all', QT_QPA_PLATFORM='xcb',
        QT_XCB_GL_INTEGRATION='xcb_glx', LIBGL_ALWAYS_SOFTWARE='1', LP_NUM_THREADS='4',
        PYTHONDONTWRITEBYTECODE='1', MNM_MENU_DIRECTPLAY_DX6='1',
        MNM_TERRAIN_LIGHTING=args.mode, MNM_TERRAIN_LIGHT_CAPTURE='reload32' if args.journey == 'reload' else 'first32',
        MNM_CAMPAIGN_SMOKE_TEST='1', MNM_CAMPAIGN_DIFFICULTY='1', MNM_CAMPAIGN_NORMAL_QUIT='1',
        MNM_CAMPAIGN_STORAGE='1' if args.journey == 'reload' else '0',
        MNM_CAMPAIGN_RECREATE_WORLD='1' if args.journey == 'reload' else '0',
        MNM_MENU_DISPLAY_OBSERVE='1', MNM_CAMPAIGN_STRESS_SECONDS=str(args.seconds if args.journey == 'stress' else 0),
        MNM_CAMPAIGN_MENU_CYCLES='1', MNM_CAMPAIGN_TIMEOUT_MS='595000',
        MNM_CAMPAIGN_STORAGE_NAME='lighting-qt-' + out.name,
        MNM_NO_ROUTE_SCREENSHOTS='1', MNM_CACHE_ADAPTERS='1',
        MNM_MENU_GAME_SOURCE=str(ROOT / 'working/game-nocd'))
    process = server = channel = lease = None
    session = None
    killed = False
    try:
        run('original-before', [ROOT / 'tools/original-manifest.sh', 'verify'])
        if args.validated_observation:
            prior = read_json(args.validated_observation.absolute(), root=ROOT)
            assert prior['success'] and prior['mode'] == 'observe' and prior['native_matches'] == 32
            assert prior['sources'] == sources, 'Combined observation sources changed'
            report['validated_observation'] = str(args.validated_observation)
        lease = (prefix.parent / 'lighting-qt.lock').open('a+b')
        fcntl.flock(lease, fcntl.LOCK_EX | fcntl.LOCK_NB)
        run('prefix-idle', ['wineserver', '-w'], timeout=5)
        from native_prefix_template import clone
        template = prefix
        prefix = out / 'wineprefix'
        report['prefix_template'] = clone(template, prefix, env)
        env['WINEPREFIX'] = str(prefix)
        launcher = load('light_qt_owner', 'tools/run-native-owned.py')
        host = out / 'host'
        host.mkdir()
        server, receipt, _ = launcher.start_display(host, allow_glx=True)
        env.update(DISPLAY=receipt['display'], XAUTHORITY=receipt['authority'])
        campaign = load('light_qt_campaign', 'tools/test-native-campaign.py')
        command = [ROOT / 'tools/run-native-owned.py', '--native-only', '--prefix', prefix,
                   '--output', out / 'owner', '--', '--skip-movies', '--no-native-music',
                   '--live-menu-test', out / 'flow.json']
        with (out / 'shell.log').open('xb') as log:
            process = subprocess.Popen(list(map(str, command)), cwd=ROOT, env=env, stdout=log,
                                       stderr=subprocess.STDOUT, start_new_session=True)
            start = time.monotonic()
            samples = []
            last_sample = -1
            while time.monotonic() - start < 650:
                journals = list((out / 'owner/sessions').glob('*.json'))
                if len(journals) > 1:
                    raise RuntimeError('Unexpected extra native ownership session')
                if journals and session is None:
                    session = Path(json.loads(journals[0].read_text())['session'])
                    assert session.is_relative_to(ROOT / 'working/experiments/menu-observer')
                    report['session'] = str(session.relative_to(ROOT))
                if session and channel is None and (session / 'terrain-light-session/channel.bin').exists():
                    with (session / 'terrain-light-session/channel.bin').open('r+b') as stream:
                        channel = mmap.mmap(stream.fileno(), 0)
                if channel is not None:
                    stats = dict(zip(light.STATS, struct.unpack_from('<17I', channel, 56)))
                    now = int(time.monotonic() - start)
                    if now != last_sample:
                        samples.append(dict(seconds=now, **stats))
                        last_sample = now
                    if args.journey == 'reload' and not report.get('reload_capture_armed'):
                        saves = list((session / 'game/Save').glob('lighting-qt-' + out.name + '*.sav'))
                        if saves and saves[0].stat().st_size:
                            struct.pack_into('<I', channel, 52, 1)
                            report['reload_capture_armed'] = dict(seconds=now, saved_sha256=sha(saves[0]))
                    if not killed and (stats['disabled'] or stats['mismatches'] or stats['error']):
                        raise RuntimeError('Lighting session refused: ' + repr(stats))
                    if args.worker_loss and not killed and stats['applied'] >= 128 and stats['records'] == 32:
                        with sqlite3.connect(session / 'terrain-light-session/outcome.sqlite3') as db:
                            pid, ticks = db.execute("SELECT pid,start_ticks FROM lifecycle WHERE phase='started'").fetchone()
                        assert int(Path('/proc', str(pid), 'stat').read_text().rsplit(') ', 1)[1].split()[19]) == ticks
                        report['worker_loss_before'] = dict(seconds=now, pid=pid, start_ticks=ticks, **stats)
                        os.kill(pid, signal.SIGKILL)
                        killed = True
                    if campaign.world_ready(session):
                        (out / 'flow.json.world-ready').touch(exist_ok=True)
                if (out / 'flow.json').exists():
                    report['flow'] = json.loads((out / 'flow.json').read_text())
                if process.poll() is not None:
                    break
                time.sleep(.05)
            else:
                raise RuntimeError('Combined Qt lighting journey timed out')
            report['shell_exit'] = process.wait(timeout=20)
            report['elapsed_seconds'] = time.monotonic() - start
            report['samples'] = samples
        assert session is not None, 'No admitted game session'
        flow = report['flow']
        if args.worker_loss:
            assert killed, 'Fault injection never reached an admitted native update'
            report['statistics'] = dict(zip(light.STATS, struct.unpack_from('<17I', channel, 56)))
            assert report['statistics']['disabled'] and report['statistics']['fallback'] >= 16
            assert report['statistics']['applied'] <= report['worker_loss_before']['applied'] + 1
        else:
            assert report['shell_exit'] == 0 and flow['success'], flow.get('error')
            if args.journey == 'reload':
                from campaign_storage import validate_storage_flow, validate_mini_inputs
                validate_storage_flow(flow)
                report['mini_inputs'] = validate_mini_inputs(out, flow)
                assert flow['reloaded_from_main'] and flow['display_rebuilds'] == 2
            else:
                campaign.validate_flow(flow, cycles=1, stress_seconds=args.seconds, min_fps=None, normal_quit=True)
            from native_only_contract import outcome
            report['graphics_outcome'] = outcome(session)
        report['original_process'] = json.loads((session / 'original-process-result.json').read_text())
        assert report['original_process']['original_exit_code'] == 0
        report['resident'] = json.loads((session / 'resident-world-report.json').read_text())
        assert not report['resident']['error'] and report['resident']['normal_quit_retired']
        with sqlite3.connect(session / 'terrain-light-session/outcome.sqlite3') as db:
            report['lighting_outcome'] = json.loads(db.execute('SELECT payload FROM results').fetchone()[0])
            pid = db.execute("SELECT pid FROM lifecycle WHERE phase='started'").fetchone()[0]
        report['worker_reaped'] = not Path('/proc', str(pid)).exists()
        assert report['worker_reaped'] and report['lighting_outcome']['success'] != args.worker_loss
        if not args.worker_loss:
            report['statistics'] = report['lighting_outcome']['statistics']
            assert report['statistics']['processed'] > 32
            assert args.mode != 'native' or report['statistics']['applied'] > 32
        models = [ROOT / 'reconstruction/rendering' / name for name in MODELS]
        original = out / 'original-light-replay'
        sanitized = out / 'sanitized-light-replay'
        run('original-build', ['g++', '-std=c++17', '-O2', '-m32', '-fno-pie', '-no-pie', '-DORIGINAL_REFERENCE', ROOT / 'tools/terrain-light-replay.cpp', *models, '-o', original])
        run('sanitized-build', ['g++', '-std=c++17', '-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie', ROOT / 'tools/terrain-light-replay.cpp', *models, '-o', sanitized])
        env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
        with (session / 'terrain-light-session/corpus.bin').open('rb') as stream:
            assert stream.read(8) == b'MNMLSC01'
            while lengths := stream.read(12):
                assert len(lengths) == 12 and len(report['results']) < 32
                ni, nr, no = struct.unpack('<3I', lengths)
                assert all(0 < n <= light.CAPACITY for n in (ni, nr, no))
                data, reply, result = stream.read(ni), stream.read(nr), stream.read(no)
                assert (len(data), len(reply), len(result)) == (ni, nr, no)
                name = 'frame-%02d' % len(report['results'])
                source, ref, san = [out / (name + suffix) for suffix in ('.in', '.original', '.sanitized')]
                source.write_bytes(data)
                run(name + '-original', [original, source, ref, ROOT / 'original/Arcane_Nocd/Chaos.exe'], timeout=20)
                run(name + '-sanitized', [sanitized, source, san], timeout=20)
                assert ref.read_bytes()[32:] == san.read_bytes()[32:] == reply[32:] == result
                report['results'].append(dict(caller=struct.unpack_from('<I', data, 108)[0],
                    clock=struct.unpack_from('<I', data, 32)[0], original_match=True, sanitized_match=True,
                    input=retain(name + '.in.bin', data), reply=retain(name + '.reply.bin', reply), result=retain(name + '.result.bin', result)))
        assert len(report['results']) == 32
        if args.journey == 'reload':
            assert report['reload_capture_armed'] and report['results'][0]['caller'] == 0x472201
        report.update(success=True, normal_exit=True, native_matches=32)
    except Exception as error:
        report['error'] = type(error).__name__ + ': ' + str(error)
    finally:
        if channel is not None:
            report['final_statistics'] = dict(zip(light.STATS, struct.unpack_from('<17I', channel, 56)))
            channel.close()
        if process is not None and process.poll() is None:
            os.killpg(process.pid, signal.SIGTERM)
            try: process.wait(timeout=15)
            except subprocess.TimeoutExpired: os.killpg(process.pid, signal.SIGKILL); process.wait(timeout=10)
        if server is not None and server.poll() is None:
            server.terminate(); server.wait(timeout=10)
        if lease: lease.close()
        if prefix.is_dir():
            for name in ('system.reg', 'user.reg', 'userdef.reg', '.mnm-directplay-dx6.json'):
                path = prefix / name
                if path.is_file(): retain('prefix-' + name + '.bin', path.read_bytes())
        if session:
            report['retained_session'] = {}
            for path in ['terrain-light-session/channel.bin', 'terrain-light-session/corpus.bin', 'terrain-light-session/outcome.sqlite3', 'terrain-light-session/worker.log', 'lighting-build/manifest.json', 'lighting-build/mnm-terrain-light-session', 'game/MnmLight.dll', 'game/Chaos.exe', 'events.bin', 'native-output-admission.json', 'native-only-session.json', 'original-process-result.json', 'resident-world-report.json', 'manifest.json', 'native-draw-retirement.bin']:
                if (session / path).is_file(): report['retained_session'][path] = retain(path.replace('/', '-') + '.bin', (session / path).read_bytes())
            for path in (session / 'game/Save').glob('lighting-qt-' + out.name + '*'):
                if path.is_file(): retain('save-' + path.name + '.bin', path.read_bytes())
        for path in out.rglob('*'):
            if path.is_file() and path.suffix in ('.json', '.log', '.png', '.conf'):
                retain('raw-' + path.relative_to(out).as_posix().replace('/', '-') + '.bin', path.read_bytes())
        try:
            run('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_manifest_verified_before_after'] = True
        except Exception as error:
            report.update(success=False, manifest_error=str(error))
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in sources.items())
        report['success'] &= report['sources_stable']
        report['driver_source'] = retain('driver-source.bin', Path(__file__).read_bytes())
        for path, data in pending.items(): write_report(ROOT, path, data)
        report['retained_artifacts'] = [dict(path=p, sha256=hashlib.sha256(data).hexdigest()) for p, data in pending.items()]
        digest = write_report(ROOT, args.report, (json.dumps(report, indent=2) + '\n').encode())
    print(json.dumps(dict(success=report['success'], error=report.get('error'), statistics=report.get('final_statistics'), sha256=digest)), flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
