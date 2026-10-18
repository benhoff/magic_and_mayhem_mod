#!/usr/bin/env python3
"""Validate an authored CPUID context, then optionally observe actual game callers.

Run under working-retention.py. Controls and game acquisitions are independent;
the CPU fixture never adds live route credit. Output paths are append-only.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

from raster_cpu_debug import ALTERNATE_ROUTES, BUILD, ROOT, build, decode, decode_rgb, sha, wine_wrapper


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=('controls', 'game'), required=True)
    parser.add_argument('--family', type=int, choices=(6, 15), default=15)
    parser.add_argument('--rgb-diagnostics', action='store_true', help='Opt-in bounded scalar inputs for RGB-add refusals; no pixel/counter writes')
    rgb_policy=parser.add_mutually_exclusive_group()
    rgb_policy.add_argument('--empty-rgb', action='store_true', help='Explicitly select the original-compared empty RGB path; default runs use native default policy')
    rgb_policy.add_argument('--disable-empty-rgb', action='store_true', help='Explicit opt-out of bounded empty RGB admission')
    parser.add_argument('--rgb-caller-trace', action='store_true', help='Read-only hardware observation of prepared RGB scalar geometry; no game-side trace I/O')
    parser.add_argument('--pixel-camera-scenarios', action='store_true', help='Use the ordinary physical camera/menu route scenarios')
    parser.add_argument('--claims', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--prefix-template', type=Path, required=True)
    parser.add_argument('--source-game', type=Path, default=ROOT / 'working/game-nocd')
    args = parser.parse_args()
    if args.report.exists():
        parser.error('Refusing evidence overwrite')
    declaration = json.loads(args.claims.read_text())
    if any(sha(ROOT / p) != h for p, h in declaration['sources'].items()):
        parser.error('Prospective source changed')
    parent = ROOT / 'working/tests/raster-cpu-game'
    parent.mkdir(parents=True, exist_ok=True)
    out = Path(tempfile.mkdtemp(prefix='run-', dir=parent))
    print(out, flush=True)
    report = dict(schema=1, success=False, mode=args.mode, family=args.family,
                  sources=declaration['sources'], claims=declaration['claims'],
                  original_sha256=BUILD, original_game_executed=False,
                  physical_alternate_cpu=False, complete_replacement=False, commands=[],
                  scope='Hardware debugger supplies only CPUID.1 EAX raw base-family bits before the unchanged original helper/initializer consumes them. Game dispatch and drawing inputs/counters untouched. Authored platform context, not physical alternate CPU or session pixel equivalence.')
    env = {k: v for k, v in os.environ.items() if not k.startswith('MNM_')}
    env.update(PYTHONDONTWRITEBYTECODE='1', WINEDEBUG='-all', WINEDLLOVERRIDES='mscoree,mshtml=;winemenubuilder.exe=d')
    verified = False

    def execute(name, command, *, environment=None, cwd=ROOT, timeout=180, check=True):
        command = [str(c) for c in command]
        result = subprocess.run(command, cwd=cwd, env=environment, capture_output=True, text=True, timeout=timeout)
        log = out / (name + '.log'); log.write_text(result.stdout + result.stderr)
        report['commands'].append(dict(command=command, cwd=str(cwd), exit_code=result.returncode,
                                       log=str(log.relative_to(ROOT)), sha256=sha(log)))
        if check:
            result.check_returncode()
        return result

    try:
        execute('original-before', [ROOT / 'tools/original-manifest.sh', 'verify']); verified = True
        report['build'] = build(out / 'build', ROOT / 'original/Arcane_Nocd/Chaos.exe')
        if args.mode == 'controls':
            from native_prefix_template import clone
            report['prefix'] = clone(args.prefix_template, out / 'wineprefix', env)
            env['WINEPREFIX'] = str(out / 'wineprefix')
            report['controls'] = []
            for name, family, negative in (('family6', 6, ''), ('family15', 15, ''),
                                            ('bad-code', 15, 'bad-code'), ('skipped-initializer', 15, 'skip')):
                directory = out / name; directory.mkdir()
                local = dict(env, MNM_RASTER_CPU_FAMILY=str(family), MNM_RASTER_CPU_NEGATIVE=negative)
                if args.rgb_caller_trace and not negative:
                    local['MNM_RASTER_RGB_TRACE'] = '1'
                result = execute(name, ['wine', out / 'build/debugger.exe', out / 'build/target.exe'],
                                 environment=local, cwd=directory, timeout=60, check=False)
                path = directory / 'raster-cpu-debug.bin'
                if negative:
                    try:
                        decode(path, family)
                    except ValueError:
                        pass
                    else:
                        raise ValueError('Negative control admitted: ' + name)
                    if not result.returncode:
                        raise ValueError('Failed target returned success: ' + name)
                    row = dict(name=name, rejected=True, exit_code=result.returncode, sha256=sha(path))
                else:
                    result.check_returncode()
                    row = dict(name=name, proof=decode(path, family))
                    if args.rgb_caller_trace:
                        row['rgb'] = decode_rgb(directory/'raster-rgb-callers.bin', row['proof'])
                        if row['rgb']['calls'] != 2 or row['rgb']['empty_calls'] != 1:
                            raise ValueError('RGB observer fixture differs from its two owned calls')
                report['controls'].append(row)
            # A previous proof must never be overwritten by another child.
            path = out / 'family15/raster-cpu-debug.bin'; before = sha(path)
            result = execute('stale-output', ['wine', out / 'build/debugger.exe', out / 'build/target.exe'],
                             environment=dict(env, MNM_RASTER_CPU_FAMILY='15'), cwd=path.parent, check=False)
            if result.returncode != 81 or sha(path) != before:
                raise ValueError('Stale output was not refused without mutation')
            report['controls'].append(dict(name='stale-output', rejected=True, original_sha256=before))
            if args.rgb_caller_trace:
                directory=out/'stale-rgb-output';directory.mkdir()
                stale=directory/'raster-rgb-callers.bin';stale.write_bytes(b'retained RGB observer fixture')
                before=sha(stale)
                result=execute('stale-rgb-output',['wine',out/'build/debugger.exe',out/'build/target.exe'],
                               environment=dict(env,MNM_RASTER_CPU_FAMILY='15',MNM_RASTER_RGB_TRACE='1'),
                               cwd=directory,check=False)
                if result.returncode != 102 or sha(stale) != before:
                    raise ValueError('Stale RGB observer output was not preserved')
                report['controls'].append(dict(name='stale-rgb-output',rejected=True,original_sha256=before))
        else:
            wrapper = wine_wrapper(out / 'bin', out / 'build/debugger.exe', args.family, args.rgb_diagnostics, args.empty_rgb, args.rgb_caller_trace, args.disable_empty_rgb)
            report['wrapper'] = dict(path=str(wrapper.relative_to(ROOT)), sha256=sha(wrapper))
            env['PATH'] = str(wrapper.parent) + os.pathsep + env['PATH']
            command = ['python3', '-B', ROOT / 'tools/test-native-campaign.py', '--claims', args.claims.resolve(),
                       '--other-pixel-producers', '--normal-quit', '--difficulty', '1', '--stress-seconds', '10',
                       '--skip-movies', '--legacy-audio', '--min-fps', '5', '--timeout', '600',
                       '--source-game', args.source_game.resolve(), '--prefix-template', args.prefix_template.resolve()]
            if args.family == 15:
                for route in ALTERNATE_ROUTES:
                    command += ['--require-pixel-route', route]
            if args.pixel_camera_scenarios:
                command += ['--pixel-camera-scenarios']
            result = execute('campaign', command, environment=env, timeout=1400, check=False)
            match = re.search(r'^Report: (.+)$', result.stdout, re.MULTILINE)
            if not match:
                raise ValueError('Campaign did not retain its result')
            path = Path(match[1]).resolve()
            if not path.is_relative_to(ROOT / 'working/tests/native-campaign'):
                raise ValueError('Unexpected campaign evidence path')
            campaign = json.loads(path.read_text())
            report['campaign'] = dict(path=str(path.relative_to(ROOT)), sha256=sha(path),
                                      success=campaign['success'], error=campaign.get('error'))
            report['original_game_executed'] = campaign.get('game_entry_verified') is True
            if not (out / 'debugger-launch.json').exists():
                raise ValueError('Game did not traverse CPU debugger: ' + str(campaign.get('error')))
            launch = json.loads((out / 'debugger-launch.json').read_text())
            game = Path(launch['game']).resolve()
            report['debugger_launch'] = launch
            proof = game.parent / 'raster-cpu-debug.bin'
            report['cpu'] = decode(proof, args.family)
            report['cpu']['path'] = str(proof.relative_to(ROOT))
            if args.family == 15 and report['cpu']['override'] == 1:
                raise ValueError('Original override prevents the required generic raster variants')
            report['original_game_executed'] = True
            report['pixel_producers'] = campaign.get('pixel_producers')
            if args.rgb_caller_trace:
                report['rgb_callers'] = decode_rgb(game.parent/'raster-rgb-callers.bin', report['cpu'])
                row = next(r for r in report['pixel_producers']['families']['pixel'] if r['route']=='rgb_add')
                if report['rgb_callers']['calls'] != row['native']+row['forwarded']:
                    raise ValueError('RGB observer does not cover the exact producer epoch/thread')
                report['rgb_callers']['native_admission_proved'] = bool(not args.disable_empty_rgb and not row['forwarded'] and report['rgb_callers']['empty_calls'])
            if result.returncode or campaign['success'] is not True:
                raise ValueError('Campaign acceptance failed: ' + str(campaign.get('error')))
            if (campaign.get('sources_unchanged') is not True or campaign.get('inputs_stable') is not True or
                    campaign.get('private_wine_stopped') is not True):
                raise ValueError('Campaign source stability/private shutdown missing')
        report['success'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        if args.mode == 'controls' and (out / 'wineprefix').exists():
            try:
                execute('wine-stop', ['wineserver', '-k'], environment=env, timeout=10, check=False)
                execute('wine-wait', ['wineserver', '-w'], environment=env, timeout=15)
                report['private_wine_stopped'] = True
            except Exception as error:
                report.update(success=False, shutdown_error=repr(error))
        try:
            execute('original-after', [ROOT / 'tools/original-manifest.sh', 'verify'])
            report['original_verified_before_after'] = verified
        except Exception as error:
            report.update(success=False, manifest_error=repr(error))
        report['sources_stable'] = all(sha(ROOT / p) == h for p, h in declaration['sources'].items())
        report['success'] &= report['sources_stable']
        (out / '.retain').write_text('Authored CPUID game-caller acquisition: ' + str(args.report) + '\n')
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as stream:
            json.dump(report, stream, indent=2); stream.write('\n')
        print(args.report, flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
