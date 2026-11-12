#!/usr/bin/env python3
"""Exercise the existing private-X11 child with disk and masked RAM authority."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import Reader, write_report
from native_output_owner import command, REQUIRED

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/runtime/native-startup-display-results-20261108/'
BID = 'HOST.private-display-authority-visibility'
SID = 'native.private-display-authority-visibility'


def module(name):
    spec = importlib.util.spec_from_file_location('startup_' + name, ROOT / 'tools' / (name + '.py'))
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--expect-hidden-authority', action='store_true')
    args = parser.parse_args()
    if not args.report.startswith(PREFIX) or (ROOT / args.report).exists():
        parser.error('Require a fresh owned report')
    out = ROOT / 'working/startup-display-controls' / Path(args.report).stem
    out.mkdir(parents=True, exist_ok=False)
    (out / '.retain').touch()
    with Reader(ROOT) as reader:
        register = reader.json('research/runtime/coverage/register.json')
        behavior = next(v for v in register['behaviors'] if v['id'] == BID)
        scenario = next(v for v in register['scenarios'] if v['id'] == SID)
        sources = {n: reader.digest(n) for n in sorted(required_sources(behavior))}
    report = dict(schema=1, kind='private_display_authority_visibility_controls', success=False,
                  sources=sources, claims=[dict(behavior=BID,
                      contract_sha256=behavior_contract(behavior, {}), scenarios={SID: scenario_contract(scenario)})],
                  original_game_runs=0, native_engine_execution=False,
                  expected_pre_fix_failure=args.expect_hidden_authority, cases=[])
    servers = []
    # These are resettable no-game Xvfb environments, not new game fixtures.
    ram = Path(tempfile.mkdtemp(prefix='mnm-startup-authority-', dir='/tmp'))
    env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE='1',
               __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
    runner, controls = module('run-native-owned'), module('test-native-output-owner')
    try:
        host_dir = out / 'host'
        host_dir.mkdir()
        server, host, _ = runner.start_display(host_dir)
        servers.append(server)
        before = controls.pixel(host)
        for name in ('disk', 'ram', 'ram-bad-cookie'):
            directory = (out if name == 'disk' else ram) / name
            directory.mkdir()
            server, legacy, cookie = runner.start_display(directory)
            servers.append(server)
            if name == 'ram-bad-cookie':
                assert cookie != bytes(16)
                wrong = directory / 'wrong-authority'
                wrong.write_bytes(runner.authority_bytes(bytes(16)))
                wrong.chmod(0o600)
                legacy = dict(legacy, authority=str(wrong),
                              authority_sha256=hashlib.sha256(wrong.read_bytes()).hexdigest())
            case = out / name
            session, prefix = case / 'session', case / 'prefix'
            session.mkdir(parents=True)
            prefix.mkdir()
            observation = session / 'probe.json'
            observation.write_text(json.dumps(dict(host_display=host['display'],
                namespaces={key: os.stat('/proc/self/ns/' + key).st_ino for key in ('user', 'pid', 'net', 'ipc')})))
            metadata = dict(origin='menu_action_bridge', **{key: True for key in REQUIRED})
            trial = dict(env, DISPLAY=host['display'], MNM_NATIVE_CPU_GUARD='3',
                         WAYLAND_DISPLAY='host-test', MNM_LEGACY_XCOOKIE='must-not-enter-child')
            argv = command(session, prefix, metadata, trial, legacy) + [
                'python3', '-B', str(ROOT / 'tests/native-output-owner-child.py'), str(observation)]
            result = subprocess.run(argv, env=trial, capture_output=True, timeout=20,
                                    stdin=subprocess.DEVNULL, close_fds=True)
            logs = []
            for suffix, raw in (('stdout', result.stdout), ('stderr', result.stderr)):
                record = args.report.removesuffix('.json') + '-' + name + '-' + suffix + '.bin'
                logs.append(dict(record=record, sha256=write_report(ROOT, record, raw), bytes=len(raw)))
            expected = name == 'disk' or name == 'ram' and not args.expect_hidden_authority
            if (result.returncode == 0) != expected:
                raise ValueError('Unexpected private authentication result: ' + name + ': ' + result.stderr.decode(errors='replace'))
            if expected:
                body = json.loads(result.stdout)
                if not body['private_x11_written'] or controls.pixel(legacy) != 0x123456:
                    raise ValueError('Private drawing did not reach its own server')
            elif b'Private authenticated display inaccessible' not in result.stderr:
                raise ValueError('Negative did not reach the authentication boundary')
            if controls.pixel(host) != before:
                raise ValueError('Private control changed host pixels')
            report['cases'].append(dict(name=name, argv=argv, exit_code=result.returncode,
                authenticated=expected, host_pixel_unchanged=True, logs=logs,
                authority_under_masked_tmp=name != 'disk'))
        report['success'] = True
    except Exception as error:
        report['error'] = str(error)
    finally:
        for server in reversed(servers):
            if server.poll() is None:
                server.terminate()
                server.wait(timeout=10)
        with Reader(ROOT) as reader:
            report['sources_stable'] = all(reader.digest(n) == h for n, h in sources.items())
        report['success'] &= report['sources_stable']
        report['scope'] = ('Existing no-game X11 child, real isolated namespaces/host socket and graphics-device exclusion. '
                           'Disk and RAM cookie inputs; wrong-cookie refusal and unchanged host pixels. No original game, native raster or physical-provider equivalence.')
        digest = write_report(ROOT, args.report, (json.dumps(report, indent=2) + '\n').encode())
    print(json.dumps(dict(success=report['success'], error=report.get('error'), sha256=digest)), flush=True)
    return int(not report['success'])


if __name__ == '__main__':
    raise SystemExit(main())
